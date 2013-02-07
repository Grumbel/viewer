//  Simple 3D Model Viewer
//  Copyright (C) 013 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_RENDERBUFFER_HPP
#define HEADER_RENDERBUFFER_HPP

#include <GL/glew.h>

#include "log.hpp"

class Framebuffer;

class Renderbuffer
{
private:
  int m_width;
  int m_height;

  GLuint m_fbo;
  GLuint m_depth_buffer;
  GLuint m_color_buffer;

public:
  Renderbuffer(int width, int height);
  ~Renderbuffer();
  
  void bind();
  void unbind();

  int get_width()  const { return m_width; }
  int get_height() const { return m_height; }

  void blit(Framebuffer& target_fbo, 
            int srcX0, int srcY0, int srcX1, int srcY1,
            int dstX0, int dstY0, int dstX1, int dstY1,
            GLbitfield mask = GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT,
            GLenum filter = GL_LINEAR);
  
private:
  Renderbuffer(const Renderbuffer&);
  Renderbuffer& operator=(const Renderbuffer&);
};

#endif

/* EOF */
