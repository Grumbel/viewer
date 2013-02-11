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

VideoProcessor::VideoProcessor(const std::string& filename) :
  m_mainloop(Glib::MainLoop::create()),
  m_pipeline(),
  m_playbin(),
  m_fakesink(),
  m_done(false),
  m_running(false),
  m_texture(),
  m_buffer()
{
  // Setup a second pipeline to get the actual thumbnails
  m_playbin = Gst::Parse::launch("filesrc name=mysource "
                                 "  ! decodebin2 name=src "
                                 "src. "
                                 "  ! queue "
                                 "  ! ffmpegcolorspace "
                                 "  ! videoscale "
                                 "  ! video/x-raw-rgb,depth=24,bpp=24,width=512,height=512 "
                                 "  ! fakesink name=mysink signal-handoffs=True sync=true "
                                 "src. "
                                 "  ! queue "
                                 "  ! autoaudiosink ");

  m_pipeline = Glib::RefPtr<Gst::Pipeline>::cast_dynamic(m_playbin);

  Glib::RefPtr<Gst::Element> source = m_pipeline->get_element("mysource");
  std::cout << "SOURC: " << source << std::endl;
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
      std::cout << "error: could not get format" << std::endl;
      return 0;
    }
  }
  else
  {
    std::cout << "QUERY FAILURE" << std::endl;
    return 0;
  }
}

bool
VideoProcessor::on_buffer_probe(const Glib::RefPtr<Gst::Pad>& pad, const Glib::RefPtr<Gst::MiniObject>& miniobj)
{
  // WARNING: this is called from the gstreamer thread, not the main thread

  Glib::RefPtr<Gst::Buffer> buffer = Glib::RefPtr<Gst::Buffer>::cast_dynamic(miniobj);
  m_buffer = buffer;

  // true: keep data in the pipeline, false: drop it
  return true;
}

void
VideoProcessor::on_bus_message(const Glib::RefPtr<Gst::Message>& msg)
{
  if (msg->get_message_type() & Gst::MESSAGE_ERROR)
  {
    Glib::RefPtr<Gst::MessageError> error_msg = Glib::RefPtr<Gst::MessageError>::cast_dynamic(msg);
    std::cout << "Error: "
              << msg->get_source()->get_name() << ": "
              << error_msg->parse().what() << std::endl;
    //assert(!"Failure");
    Glib::signal_idle().connect(sigc::mem_fun(this, &VideoProcessor::shutdown));
  }
  else if (msg->get_message_type() & Gst::MESSAGE_STATE_CHANGED)
  {
    Glib::RefPtr<Gst::MessageStateChanged> state_msg = Glib::RefPtr<Gst::MessageStateChanged>::cast_dynamic(msg);
    Gst::State oldstate;
    Gst::State newstate;
    Gst::State pending;
    state_msg->parse(oldstate, newstate, pending);

    std::cout << "message: " << msg->get_source()->get_name() << " " << oldstate << " " << newstate << std::endl;

    if (msg->get_source() == m_fakesink)
    {
      if (newstate == Gst::STATE_PAUSED)
      {
        std::cout << "                       --------->>>>>>> PAUSE" << std::endl;
      }

      if (newstate == Gst::STATE_PAUSED)
      {
        if (!m_running)
        {
          std::cout << "##################################### ONLY ONCE: " << " ################" << std::endl;
          //m_thumbnailer_pos = m_thumbnailer.get_thumbnail_pos(get_duration());
          //std::reverse(m_thumbnailer_pos.begin(), m_thumbnailer_pos.end());
          m_running = true;
          //seek_step();

          std::cout << "---------- send_buffer_probe()" << std::endl;
          Glib::RefPtr<Gst::Pad> pad = m_fakesink->get_static_pad("sink");
          pad->add_buffer_probe(sigc::mem_fun(this, &VideoProcessor::on_buffer_probe));
        }
      }
    }
  }
  else if (msg->get_message_type() & Gst::MESSAGE_EOS)
  {
    std::cout << "end of stream" << std::endl;
    Glib::signal_idle().connect(sigc::mem_fun(this, &VideoProcessor::shutdown));
  }
  else if (msg->get_message_type() & Gst::MESSAGE_TAG) 
  {
    std::cout << "MESSAGE_TAG" << std::endl;
  }
  else if (msg->get_message_type() & Gst::MESSAGE_ASYNC_DONE)
  {
    std::cout << "MESSAGE_ASYNC_DONE" << std::endl;
  }
  else if (msg->get_message_type() & Gst::MESSAGE_STREAM_STATUS) 
  {
    std::cout << "MESSAGE_STREAM_STATUS" << std::endl;
  }
  else if (msg->get_message_type() & Gst::MESSAGE_REQUEST_STATE) 
  {
    std::cout << "MESSAGE_REQUEST_STATE" << std::endl;
  }
  else if (msg->get_message_type() & Gst::MESSAGE_STEP_START) 
  {
    std::cout << "MESSAGE_STEP_START" << std::endl;
  }
  else if (msg->get_message_type() & Gst::MESSAGE_REQUEST_STATE)
  {
    std::cout << "MESSAGE_REQUEST_STATE" << std::endl;
  }
  else if (msg->get_message_type() & Gst::MESSAGE_QOS) 
  {
    std::cout << "MESSAGE_QOS" << std::endl;
  }
  else if (msg->get_message_type() & Gst::MESSAGE_LATENCY)
  {
    std::cout << "MESSAGE_LATENCY" << std::endl;
  }
  else if (msg->get_message_type() & Gst::MESSAGE_DURATION)
  {
    std::cout << "MESSAGE_DURATION" << std::endl;
  }
  else if (msg->get_message_type() & GST_MESSAGE_NEW_CLOCK)
  {
    std::cout << "MESSAGE_NEW_CLOCK" << std::endl;
  }
  else
  {
    std::cout << "unknown message: " << msg->get_message_type() << std::endl;
    Glib::signal_idle().connect(sigc::mem_fun(this, &VideoProcessor::shutdown));
  }
}

bool
VideoProcessor::shutdown()
{
  std::cout << "Going to shutdown!!!!!!!!!!!" << std::endl;
  m_playbin->set_state(Gst::STATE_NULL);
  m_mainloop->quit();
  return false;
}

void
VideoProcessor::update()
{
  while(m_mainloop->get_context()->iteration(false))
  {
    //std::cout << "looping" << std::endl;
  }

  if (m_buffer)
  {
    Glib::RefPtr<Gst::Caps> caps = m_buffer->get_caps();
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
      std::cout << std::this_thread::get_id() << ": on_buffer_probe: " 
                << " " << m_buffer->get_size() << " " << get_position() / Gst::SECOND
                << " " << width << "x" << height 
                << std::endl;
    }

    // std::cout << "Size: " << width << "x" << height << " pitch: " << m_buffer->get_size() / height << std::endl;
    if (!m_texture)
    {
      m_texture = Texture::from_rgb_data(width, height, m_buffer->get_size() / height, m_buffer->get_data());
    }
    else
    {
      m_texture->upload(width, height, m_buffer->get_size() / height, m_buffer->get_data());
    }
    
    if (false)
    {
      Cairo::RefPtr<Cairo::ImageSurface> img = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, width, height);

      { // blit and flip color channels
        unsigned char* op = img->get_data();
        unsigned char* ip  = m_buffer->get_data();
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

/* EOF */
