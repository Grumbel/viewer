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

#define assert_gl(fmt) assert_gl_1(__FILE__, __LINE__, fmt)

inline void assert_gl_1(const char* file, int line, const char* message)
{
  GLenum error = glGetError();
  if(error != GL_NO_ERROR) 
  {
    std::ostringstream msg;
    msg << file << ':' << line << ": OpenGLError while '" << message << "': "
        << gluErrorString(error);
    throw std::runtime_error(msg.str());
  }
}

#endif

/* EOF */
