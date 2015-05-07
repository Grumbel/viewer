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
#include <iostream>
#include <stdexcept>
#include <vector>

#include "log.hpp"

VideoProcessor::VideoProcessor(const std::string& filename) :
  m_mainloop(g_main_loop_new(nullptr, false)),
  m_pipeline(),
  m_playbin(),
  m_fakesink(),
  m_done(false),
  m_running(false),
  m_texture(),
  m_buffer_mutex(),
  m_buffer()
{
  // Setup a second pipeline to get the actual thumbnails
  GError* error = nullptr;
  m_playbin = gst_parse_launch("filesrc name=mysource "
                               "  ! decodebin2 name=src "
                               "src. "
                               "  ! queue "
                               "  ! ffmpegcolorspace "
                               "  ! videoscale "
                               "  ! video/x-raw-rgb,depth=24,bpp=24,width=1024,height=1024 "
                               "  ! fakesink name=mysink signal-handoffs=True sync=true "
                               "src. "
                               "  ! queue "
                               "  ! autoaudiosink ",
                               &error);
  if (error)
  {
    std::runtime_error exception(error->message);
    g_error_free(error);
    throw exception;
  }

  m_pipeline = GST_PIPELINE(m_playbin);

  GstElement* source = gst_bin_get_by_name(GST_BIN(m_pipeline), "mysource");
  log_info("SOURC: %s", source);
  g_object_set(source, "location", filename.c_str(), nullptr);
  g_object_unref(source);

  m_fakesink = gst_bin_get_by_name(GST_BIN(m_pipeline), "mysink");

  GstBus* thumbnail_bus = gst_pipeline_get_bus(GST_PIPELINE(m_playbin));
  //TODO: thumbnail_bus->add_signal_watch();
  //TODO: thumbnail_bus->signal_message().connect(sigc::mem_fun(this, &VideoProcessor::on_bus_message));

  gst_element_set_state(GST_ELEMENT(m_playbin), GST_STATE_PLAYING);
}

VideoProcessor::~VideoProcessor()
{
  gst_element_set_state(GST_ELEMENT(m_playbin), GST_STATE_NULL);
}

gint64
VideoProcessor::get_duration()
{
  GstQuery* query = gst_query_new_duration(GST_FORMAT_TIME);
  gboolean ret = gst_element_query(GST_ELEMENT(m_pipeline), query);
  if (ret)
  {
    GstFormat format;
    gint64 duration;
    gst_query_parse_duration(query, &format, &duration);

    if (format == GST_FORMAT_TIME)
    {
      gst_query_unref(query);
      return duration;
    }
    else
    {
      gst_query_unref(query);
      throw std::runtime_error("error: could not get format");
    }
  }
  else
  {
    gst_query_unref(query);
    throw std::runtime_error("error: QUERY FAILURE");
  }
}

gint64
VideoProcessor::get_position()
{
  GstQuery* query = gst_query_new_position(GST_FORMAT_TIME);
  gboolean ret = gst_element_query(GST_ELEMENT(m_pipeline), query);
  if (ret)
  {
    GstFormat format;
    gint64 position;
    gst_query_parse_position(query, &format, &position);

    if (format == GST_FORMAT_TIME)
    {
      gst_query_unref(query);
      return position;
    }
    else
    {
      gst_query_unref(query);
      throw std::runtime_error("error: could not get format");
    }
  }
  else
  {
    gst_query_unref(query);
    throw std::runtime_error("error: QUERY FAILURE");
  }
}

bool
VideoProcessor::on_buffer_probe(GstPad* pad, GstMiniObject* miniobj)
{
  // WARNING: this is called from the gstreamer thread, not the main thread

  GstBuffer* buffer = GST_BUFFER(miniobj);

  std::lock_guard<std::mutex> lock(m_buffer_mutex);
  m_buffer = buffer;

  // true: keep data in the pipeline, false: drop it
  return true;
}

void
VideoProcessor::on_bus_message(GstMessage* msg)
{
  if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_ERROR)
  {
    GError* gerror = nullptr;
    gchar* debug = nullptr;
    gst_message_parse_error(msg, &gerror, &debug);

    log_info("Error: %s: %s", GST_MESSAGE_SRC_NAME(msg), gerror->message);

    queue_shutdown();

    g_error_free(gerror);
    g_free(debug);
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_STATE_CHANGED)
  {
    GstState oldstate;
    GstState newstate;
    GstState pending;
    gst_message_parse_state_changed(msg, &oldstate, &newstate, &pending);

    //TODO: log_info("message: %s %s %s",  msg->get_source()->get_name(), oldstate, newstate);

    if (GST_ELEMENT(GST_MESSAGE_SRC(msg)) == m_fakesink)
    {
      if (newstate == GST_STATE_PAUSED)
      {
        log_info("                       --------->>>>>>> PAUSE");
      }

      if (newstate == GST_STATE_PAUSED)
      {
        if (!m_running)
        {
          log_info("##################################### ONLY ONCE: ################");
          //m_thumbnailer_pos = m_thumbnailer.get_thumbnail_pos(get_duration());
          //std::reverse(m_thumbnailer_pos.begin(), m_thumbnailer_pos.end());
          m_running = true;
          //seek_step();

          log_info("---------- send_buffer_probe()");
          //TODO: GstPad* pad = m_fakesink->get_static_pad("sink");
          //TODO: pad->add_buffer_probe(sigc::mem_fun(this, &VideoProcessor::on_buffer_probe));
        }
      }
    }
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_EOS)
  {
    log_info("end of stream");
    queue_shutdown();
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_TAG)
  {
    log_info("MESSAGE_TAG");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_ASYNC_DONE)
  {
    log_info("MESSAGE_ASYNC_DONE");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_STREAM_STATUS)
  {
    log_info("MESSAGE_STREAM_STATUS");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_REQUEST_STATE)
  {
    log_info("MESSAGE_REQUEST_STATE");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_STEP_START)
  {
    log_info("MESSAGE_STEP_START");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_REQUEST_STATE)
  {
    log_info("MESSAGE_REQUEST_STATE");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_QOS)
  {
    log_info("MESSAGE_QOS");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_LATENCY)
  {
    log_info("MESSAGE_LATENCY");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_DURATION)
  {
    log_info("MESSAGE_DURATION");
  }
  else if (GST_MESSAGE_TYPE(msg) & GST_MESSAGE_NEW_CLOCK)
  {
    log_info("MESSAGE_NEW_CLOCK");
  }
  else
  {
    log_info("unknown message: %d", GST_MESSAGE_TYPE(msg));
    queue_shutdown();
  }
}

void
VideoProcessor::queue_shutdown()
{
  auto callback = [](gpointer user_data) -> gboolean
    {
      static_cast<VideoProcessor*>(user_data)->shutdown();
      return false;
    };
  g_idle_add(callback, this);
}

bool
VideoProcessor::shutdown()
{
  log_info("Going to shutdown!!!!!!!!!!!");
  gst_element_set_state(GST_ELEMENT(m_playbin), GST_STATE_NULL);
  g_main_loop_quit(m_mainloop);
  return false;
}

void
VideoProcessor::update()
{
  while(g_main_context_iteration(g_main_loop_get_context(m_mainloop), false))
  {
    //log_info("looping");
  }

  std::lock_guard<std::mutex> lock(m_buffer_mutex);
  if (m_buffer)
  {
    /* TODO:
    GstCaps* caps = gst_buffer_get_caps(m_buffer);
    GstStructure* structure = gst_caps_get_structure(caps, 0);
    int width;
    int height;

    if (structure)
    {
      gst_structure_get_int(structure, "width",  &width);
      gst_structure_get_int(structure, "height", &height);
    }

    if (false)
    {
      log_info("%s: on_buffer_probe: %s %s %sx%s",
               std::this_thread::get_id(),
               gst_buffer_get_size(m_buffer),
               get_position() / GST_SECOND,
               width, height);
    }
    */

    if (!m_texture)
    {
      //TODO: m_texture = Texture::from_rgb_data(width, height, gst_buffer_get_size(m_buffer) / height, m_buffer->get_data());
    }
    else
    {
      //TODO: m_texture->upload(width, height, gst_buffer_get_size(m_buffer) / height, m_buffer->get_data());
    }

    if (false)
    {
      /* TODO:
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
      */
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

  if (!gst_element_seek_simple(GST_ELEMENT(m_pipeline),
                               GST_FORMAT_TIME,
                               GstSeekFlags(GST_SEEK_FLAG_FLUSH | GST_SEEK_FLAG_ACCURATE),
                               seek_pos))
  {
    log_info("seek failure");
  }
}

/* EOF */
