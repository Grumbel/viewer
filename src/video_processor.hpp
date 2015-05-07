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

#ifndef HEADER_VIDEO_PROCESSOR_HPP
#define HEADER_VIDEO_PROCESSOR_HPP

#include <assert.h>
#include <algorithm>
#include <vector>
#include <iostream>
#include <stdexcept>
#include <glib.h>
#include <gst/gst.h>
#include <mutex>

#include "texture.hpp"

class VideoProcessor
{
private:
  GMainLoop* m_mainloop;

  GstPipeline* m_pipeline;
  GstElement* m_playbin;
  GstElement* m_fakesink;

  bool m_done;
  bool m_running;

  TexturePtr m_texture;
  std::mutex m_buffer_mutex;
  GstBuffer* m_buffer;

public:
  VideoProcessor(const std::string& filename);
  ~VideoProcessor();

  gint64 get_duration();
  gint64 get_position();

  void queue_shutdown();
  bool shutdown();

  void update();
  bool is_playing() const;
  TexturePtr get_texture() const { return m_texture; }
  void seek(gint64 seek_pos);

private:
  bool on_buffer_probe(GstPad* pad, GstMiniObject* miniobj);
  void on_bus_message(GstMessage* msg);
};

#endif

/* EOF */
