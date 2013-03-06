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

#ifndef HEADER_ASSERT_GL_HPP
#define HEADER_ASSERT_GL_HPP

#include <GL/glew.h>
#include <sstream>
#include <stdexcept>

#include "format.hpp"

#define assert_gl(...) assert_gl_1(__FILE__, __LINE__, __VA_ARGS__)

template<typename ...Arg>
inline void assert_gl_1(const char* file, int line, const std::string& fmt, Arg... arg)
{
  GLenum error = glGetError();
  if(error != GL_NO_ERROR) 
  {
    throw std::runtime_error(format("%s:%d: OpenGLError while '%s': %s", 
                                    file, line, 
                                    format(fmt, arg...), 
                                    gluErrorString(error)));
  }
}

#endif

/* EOF */
