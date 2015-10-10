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

#ifndef HEADER_MATERIAL_FACTORY_HPP
#define HEADER_MATERIAL_FACTORY_HPP

#include <boost/filesystem/path.hpp>
#include <string>
#include <unordered_map>

#include "material.hpp"

class MaterialFactory
{
public:
  static MaterialFactory& get()
  {
    static MaterialFactory instance;
    return instance;
  }

private:
  std::unordered_map<std::string, MaterialPtr> m_materials;

public:
  MaterialFactory();

  MaterialPtr from_file(const boost::filesystem::path& name);
  MaterialPtr create(const std::string& name);

private:
  static MaterialPtr create_phong(const glm::vec3& diffuse,
                                  const glm::vec3& ambient,
                                  const glm::vec3& specular,
                                  float shininess);
  static MaterialPtr create_skybox();
  static MaterialPtr create_basic_white();
  static MaterialPtr create_textured();

private:
  MaterialFactory(const MaterialFactory&);
  MaterialFactory& operator=(const MaterialFactory&);
};

#endif

/* EOF */
