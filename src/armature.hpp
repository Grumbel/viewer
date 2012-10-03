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

#ifndef HEADER_ARMATURE_HPP
#define HEADER_ARMATURE_HPP

#include <string>
#include <vector>
#include <glm/glm.hpp>
#include <memory>

struct Bone
{
  std::string name;
  glm::vec3 head;
  glm::vec3 tail;
  glm::vec3 head_local;
  glm::vec3 tail_local;
  glm::mat3 matrix;
  glm::mat4 matrix_local;

  Bone() :
    name(),
    head(),
    tail(),
    head_local(),
    tail_local(),
    matrix(),
    matrix_local()
  {} 
};

class Armature
{
private:
  std::vector<std::unique_ptr<Bone> > m_bones;

public:
  static std::unique_ptr<Armature> from_file(const std::string& filename);

public:
  Armature();
  
  void bind_uniform(int loc);

private:
  Armature(const Armature&);
  Armature& operator=(const Armature&);
};

#endif

/* EOF */
