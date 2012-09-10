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

#ifndef HEADER_MESH_HPP
#define HEADER_MESH_HPP

#include <vector>
#include <glm/glm.hpp>

#include "opengl_state.hpp"

struct Face
{
  int vertex1;
  int vertex2;
  int vertex3;
};

class Mesh
{
private:
  typedef std::vector<glm::vec3> NormalLst;
  typedef std::vector<glm::vec3> VertexLst;
  typedef std::vector<Face>   FaceLst;

  NormalLst normals;
  VertexLst vertices;
  FaceLst   faces;

public:
  Mesh(std::istream& in);

  void display();
  void draw();
  void draw_face_normal(const Face& face);
  glm::vec3 calc_face_normal(const Face& face);
  void set_face_normal(const Face& face);
};

#endif

/* EOF */
