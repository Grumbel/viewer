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

#ifndef HEADER_OPENGL_STATE_HPP
#define HEADER_OPENGL_STATE_HPP

#include "assert_gl.hpp"

class OpenGLState
{
public:
  OpenGLState()
  {
    assert_gl("OpenGLState");
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    assert_gl("OpenGLState-exit");
  }

  ~OpenGLState()
  {
    assert_gl("~OpenGLState");
    glPopClientAttrib();
    glPopAttrib();
    assert_gl("~OpenGLState-exit");
  }

private:
  OpenGLState(const OpenGLState&) = delete;
  OpenGLState& operator=(const OpenGLState&) = delete;
};

#endif

/* EOF */
