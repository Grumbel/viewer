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

#ifndef HEADER_POSE_HPP
#define HEADER_POSE_HPP

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <memory>

struct PoseBone
{
  std::string name;
  glm::mat4 matrix;
  glm::mat4 matrix_basis;

  PoseBone() :
    name(),
    matrix(),
    matrix_basis()
  {}
};

class Pose
{
private:
  std::vector<std::unique_ptr<PoseBone> > m_bones;

public:
  static std::unique_ptr<Pose> from_file(const std::string& filename);

  void bind_uniform(int loc);

public:
  Pose();

private:
  Pose(const Pose&);
  Pose& operator=(const Pose&);
};

#endif

/* EOF */
