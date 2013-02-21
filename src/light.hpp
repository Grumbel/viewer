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

#ifndef HEADER_LIGHT_HPP
#define HEADER_LIGHT_HPP

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>

class Light;

typedef std::shared_ptr<Light> LightPtr;

class Light
{
private:
  glm::vec4 m_diffuse;
  glm::vec4 m_ambient;
  glm::vec4 m_specular;
  float m_shininess;

public:
  Light() :
    m_diffuse(1.0f, 1.0f, 1.0f, 1.0f),
    m_ambient(0.0f, 0.0f, 0.0f, 1.0f),
    m_specular(0.0f, 0.0f, 0.0f, 1.0f),
    m_shininess(0)
  {}

  void set_diffuse(const glm::vec4& diffuse) 
  {
    m_diffuse = diffuse;
  }

  void set_ambient(const glm::vec4& ambient) 
  {
    m_ambient = ambient;
  }

  void set_specular(const glm::vec4& specular) 
  {
    m_specular = specular;
  }

  glm::vec4 get_diffuse() const
  {
    return m_diffuse;
  }

  glm::vec4 get_ambient() const
  {
    return m_ambient;
  }

  glm::vec4 get_specular() const
  {
    return m_specular;
  }

private:
  Light(const Light&);
  Light& operator=(const Light&);
};

#endif

/* EOF */
