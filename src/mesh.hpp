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
#include <memory>

#include "opengl_state.hpp"

struct Face
{
  int vertex1;
  int vertex2;
  int vertex3;
};

class Mesh
{
public:
  typedef std::vector<glm::vec3> NormalLst;
  typedef std::vector<glm::vec3> VertexLst;
  typedef std::vector<glm::vec2> TexCoordLst;
  typedef std::vector<Face>   FaceLst;

private:
  NormalLst   m_normals;
  TexCoordLst m_texcoords;
  VertexLst   m_vertices;
  FaceLst     m_faces;

  glm::vec3 m_location;
  
  GLuint m_normals_vbo;
  GLuint m_texcoords_vbo;
  GLuint m_vertices_vbo;
  GLuint m_faces_vbo;

public:
  Mesh(const NormalLst& normals,
       const TexCoordLst& texcoords,
       const VertexLst& vertices,
       const FaceLst&   faces);
  ~Mesh();

  void display();
  void draw();

  void set_location(const glm::vec3& location) {  m_location = location; }

private:
  Mesh();

  void build_vbos();

public:
  void verify() const;

private:
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
};

#endif

/* EOF */
