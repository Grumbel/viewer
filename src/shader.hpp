//  Simple 3D Model Viewer
//  Copyright (C) 2012-2013 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_SHADER_HPP
#define HEADER_SHADER_HPP

#include <memory>
#include <tuple>
#include <vector>

#include "opengl.hpp"

class Shader;

typedef std::shared_ptr<Shader> ShaderPtr;

class Shader
{
private:
  GLuint m_shader;

public:
  static ShaderPtr from_file(GLenum type, std::string const& filename,
                             std::vector<std::string> const& defines = {});

public:
  Shader(GLenum type);
  ~Shader();

  void source(std::vector<std::string> const& sources);
  void source(const std::string& source);
  void compile();

  std::string get_info_log() const;
  bool get_compile_status() const;

  GLuint get_id() const { return m_shader; }

private:
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
};

#endif

/* EOF */
