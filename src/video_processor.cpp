/*
**  VidThumb - Video Thumbnailer
**  Copyright (C) 2011 Ingo Ruhnke <grumbel@gmx.de>
**
**  This program is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  This program is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "video_processor.hpp"

#include <thread>
#include <algorithm>
#include <assert.h>
#include <cairomm/cairomm.h>
#include <glibmm/main.h>
#include <gstreamermm.h>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "log.hpp"

VideoProcessor::VideoProcessor(const std::string& filename) :
  m_mainloop(Glib::MainLoop::create()),
  m_pipeline(),
  m_playbin(),
  m_fakesink(),
  m_done(false),
  m_running(false),
  m_texture(),
  m_buffer_mutex(),
  m_buffer()
{
  try
  {
    // Setup a second pipeline to get the actual thumbnails
    m_playbin = Gst::Parse::launch("filesrc name=mysource "
                                   "  ! decodebin name=src "
                                   "src. "
                                   "  ! queue "
                                   "  ! videoconvert "
                                   "  ! videoscale "
                                   "  ! video/x-raw,format=RGB,width=1024,height=1024 "
                                   "  ! fakesink name=mysink signal-handoffs=True sync=true "
                                   "src. "
                                   "  ! queue "
                                   "  ! autoaudiosink ");
  }
  catch(Gst::ParseError const& err)
  {
    std::cout << "Gst::ParseError: " << err.what() << std::endl;
    throw;
  }

  m_pipeline = Glib::RefPtr<Gst::Pipeline>::cast_dynamic(m_playbin);

  Glib::RefPtr<Gst::Element> source = m_pipeline->get_element("mysource");
  log_info("SOURC: %s", source);
  source->set_property("location", filename);

  m_fakesink = m_pipeline->get_element("mysink");

  Glib::RefPtr<Gst::Bus> thumbnail_bus = m_playbin->get_bus();
  thumbnail_bus->add_signal_watch();
  thumbnail_bus->signal_message().connect(sigc::mem_fun(this, &VideoProcessor::on_bus_message));

  m_playbin->set_state(Gst::STATE_PLAYING);
}

VideoProcessor::~VideoProcessor()
{
  m_playbin->set_state(Gst::STATE_NULL);
}

gint64
VideoProcessor::get_duration()
{
  Gst::Format format = Gst::FORMAT_TIME;
  gint64 duration;
  if (m_playbin->query_duration(format, duration))
  {
    if (format == Gst::FORMAT_TIME)
    {
      return duration;
    }
    else
    {
      throw std::runtime_error("error: could not get format");
    }
  }
  else
  {
    throw std::runtime_error("error: QUERY FAILURE");
  }
}

gint64
VideoProcessor::get_position()
{
  Gst::Format format = Gst::FORMAT_TIME;
  gint64 position;
  if (m_playbin->query_position(format, position))
  {
    if (format == Gst::FORMAT_TIME)
    {
      return position;
    }
    else
    {
      log_error("could not get format");
      return 0;
    }
  }
  else
  {
    log_error("QUERY FAILURE");
    return 0;
  }
}

Gst::PadProbeReturn
VideoProcessor::on_buffer_probe(Glib::RefPtr<Gst::Pad> const& pad, Gst::PadProbeInfo const& padinfo)
{
  // WARNING: this is called from the gstreamer thread, not the main thread
  Glib::RefPtr<Gst::Caps> caps = pad->get_current_caps();
  Glib::RefPtr<Gst::Buffer> buffer = padinfo.get_buffer();

  std::lock_guard<std::mutex> lock(m_buffer_mutex);
  m_buffer = buffer;
  m_caps = caps;

  // true: keep data in the pipeline, false: drop it
  return Gst::PAD_PROBE_OK;
}

void
VideoProcessor::on_bus_message(Glib::RefPtr<Gst::Message> const& msg)
{
  switch(msg->get_message_type())
  {
    case Gst::MESSAGE_ERROR:
      {
        auto error_msg = Glib::RefPtr<Gst::MessageError>::cast_static(msg);
        log_error("Error: %s: %s", msg->get_source()->get_name(), error_msg->parse().what());
        //assert(!"Failure");
        Glib::signal_idle().connect(sigc::mem_fun(this, &VideoProcessor::shutdown));
      }
      break;

    case Gst::MESSAGE_STATE_CHANGED:
      {
        Glib::RefPtr<Gst::MessageStateChanged> state_msg = Glib::RefPtr<Gst::MessageStateChanged>::cast_static(msg);
        Gst::State oldstate;
        Gst::State newstate;
        Gst::State pending;
        state_msg->parse(oldstate, newstate, pending);

        log_info("message: %s %s %s",  msg->get_source()->get_name(), oldstate, newstate);

        if (msg->get_source() == m_fakesink)
        {
          if (newstate == Gst::STATE_PAUSED)
          {
            log_info("                       --------->>>>>>> PAUSE");
          }

          if (newstate == Gst::STATE_PAUSED)
          {
            if (!m_running)
            {
              log_info("##################################### ONLY ONCE: ################");
              //m_thumbnailer_pos = m_thumbnailer.get_thumbnail_pos(get_duration());
              //std::reverse(m_thumbnailer_pos.begin(), m_thumbnailer_pos.end());
              m_running = true;
              //seek_step();

              log_info("---------- send_buffer_probe()");
              Glib::RefPtr<Gst::Pad> pad = m_fakesink->get_static_pad("sink");
              pad->add_probe(Gst::PAD_PROBE_TYPE_BUFFER, sigc::mem_fun(this, &VideoProcessor::on_buffer_probe));
            }
          }
        }
      }
      break;

    case Gst::MESSAGE_EOS:
      log_info("end of stream");
      Glib::signal_idle().connect(sigc::mem_fun(this, &VideoProcessor::shutdown));
      break;

    case Gst::MESSAGE_TAG:
      log_info("MESSAGE_TAG");
      break;

    case Gst::MESSAGE_ASYNC_DONE:
      log_info("MESSAGE_ASYNC_DONE");
      break;

    case Gst::MESSAGE_STREAM_STATUS:
      log_info("MESSAGE_STREAM_STATUS");
      break;

    case Gst::MESSAGE_REQUEST_STATE:
      log_info("MESSAGE_REQUEST_STATE");
      break;

    case Gst::MESSAGE_STEP_START:
      log_info("MESSAGE_STEP_START");
      break;

    case Gst::MESSAGE_QOS:
      log_info("MESSAGE_QOS");
      break;

    case Gst::MESSAGE_LATENCY:
      log_info("MESSAGE_LATENCY");
      break;

    case GST_MESSAGE_NEW_CLOCK:
      log_info("MESSAGE_NEW_CLOCK");
      break;

    default:
      log_info("unknown message: %d", msg->get_message_type());
      //Glib::signal_idle().connect(sigc::mem_fun(this, &VideoProcessor::shutdown));
      break;
  }
}

bool
VideoProcessor::shutdown()
{
  log_info("Going to shutdown!!!!!!!!!!!");
  m_playbin->set_state(Gst::STATE_NULL);
  m_mainloop->quit();
  return false;
}

void
VideoProcessor::update()
{
  while(m_mainloop->get_context()->iteration(false))
  {
    //log_info("looping");
  }

  std::lock_guard<std::mutex> lock(m_buffer_mutex);
  if (m_buffer && m_caps)
  {
    Glib::RefPtr<Gst::Caps> caps = m_caps;
    const Gst::Structure structure = caps->get_structure(0);
    int width;
    int height;

    if (structure)
    {
      structure.get_field("width",  width);
      structure.get_field("height", height);
    }

    if (false)
    {
      log_info("%s: on_buffer_probe: %s %s %sx%s",
               std::this_thread::get_id(),
               m_buffer->get_size(),
               get_position() / Gst::SECOND,
               width, height);
    }

    {
      Glib::RefPtr<Gst::MapInfo> mapinfo(new Gst::MapInfo);
      m_buffer->map(mapinfo, Gst::MAP_READ);
      if (!m_texture)
      {
        m_texture = Texture::from_rgb_data(width, height, m_buffer->get_size() / height, mapinfo->get_data());
      }
      else
      {
        m_texture->upload(width, height, m_buffer->get_size() / height, mapinfo->get_data());
      }
      m_buffer->unmap(mapinfo);
    }

    if (false)
    {
      Cairo::RefPtr<Cairo::ImageSurface> img = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, width, height);

      { // blit and flip color channels
        unsigned char* op = img->get_data();
        Glib::RefPtr<Gst::MapInfo> mapinfo(new Gst::MapInfo);
        m_buffer->map(mapinfo, Gst::MAP_READ);
        unsigned char* ip  = mapinfo->get_data();
        int ostride = img->get_stride();
        int istride = m_buffer->get_size() / height;
        for(int y = 0; y < height; ++y)
        {
          for(int x = 0; x < width; ++x)
          {
            op[y*ostride + 4*x+0] = ip[y*istride + 4*x+2];
            op[y*ostride + 4*x+1] = ip[y*istride + 4*x+1];
            op[y*ostride + 4*x+2] = ip[y*istride + 4*x+0];
          }
        }
        m_buffer->unmap(mapinfo);
      }

      img->write_to_png("/tmp/out.png");
    }
  }
}

bool
VideoProcessor::is_playing() const
{
  return true;
}

void
VideoProcessor::seek(gint64 seek_pos)
{
  if (seek_pos < 0)
  {
    seek_pos = 0;
  }

  if (!m_pipeline->seek(Gst::FORMAT_TIME,
                        Gst::SEEK_FLAG_FLUSH | Gst::SEEK_FLAG_ACCURATE,
                        seek_pos))
  {
    log_info("seek failure");
  }
}

/* EOF */
