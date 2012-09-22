//  Simple 3D Model Viewer
//  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_FRAMEBUFFER_HPP
#define HEADER_FRAMEBUFFER_HPP

#include <GL/glew.h>

#include "log.hpp"

class Framebuffer
{
private:
  GLuint m_fbo;
  GLuint m_color_buffer;
  GLuint m_depth_buffer;

public:
  Framebuffer(int width, int height);  
  ~Framebuffer();

  void draw(float x, float y, float w, float h, float z);
  void draw_depth(float x, float y, float w, float h, float z);

  void bind();
  void unbind();

  GLuint get_depth_texture() const { return m_depth_buffer; }

private:
  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator=(const Framebuffer&) = delete;
};

#endif

/* EOF */
