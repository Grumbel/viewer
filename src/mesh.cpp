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

#include "mesh.hpp"

#include <GL/glew.h>
#include <iostream>

#include "log.hpp"
#include "opengl_state.hpp"

namespace {

} // namespace

std::unique_ptr<Mesh> 
Mesh::create_cube(float size)
{
  NormalLst   vn;
  TexCoordLst vt;
  VertexLst   vp;
  
  float d = size;
  float t = 1.0f;
  float n = 1.0f;

  // top
  vn.emplace_back(-n,  n, -n); 
  vt.emplace_back(-t,  t, -t); 
  vp.emplace_back(-d,  d, -d);

  vn.emplace_back( n,  n, -n); 
  vt.emplace_back( t,  t, -t); 
  vp.emplace_back( d,  d, -d);

  vn.emplace_back( n,  n,  n); 
  vt.emplace_back( t,  t,  t);
  vp.emplace_back( d,  d,  d);

  vn.emplace_back(-n,  n,  n); 
  vt.emplace_back(-t,  t,  t); 
  vp.emplace_back(-d,  d,  d);

  // bottom
  vn.emplace_back(-n, -n, -n);
  vt.emplace_back(-t, -t, -t);
  vp.emplace_back(-d, -d, -d);

  vn.emplace_back(-n, -n,  n);
  vt.emplace_back(-t, -t,  t);
  vp.emplace_back(-d, -d,  d);

  vn.emplace_back( n, -n,  n);
  vt.emplace_back( t, -t,  t);
  vp.emplace_back( d, -d,  d);

  vn.emplace_back( n, -n, -n);
  vt.emplace_back( t, -t, -t);
  vp.emplace_back( d, -d, -d);

  int faces[] = {
    0, 1, 2, 3, // top
    4, 5, 6, 7, // bottom
    3, 2, 6, 5, // front
    1, 0, 4, 7, // back
    2, 1, 7, 6, // left 
    0, 3, 5, 4  // right 
  };

  std::unique_ptr<Mesh> mesh(new Mesh(GL_QUADS));

  mesh->attach_float_array("normal",   vn);
  mesh->attach_float_array("texcoord", vt);
  mesh->attach_float_array("position",   vp);
  mesh->attach_element_array(FaceLst(faces, faces + sizeof(faces)/sizeof(faces[0])));

   return mesh;
}

Mesh::Mesh(GLenum primitive_type) :
  m_primitive_type(primitive_type),
  m_attribute_arrays(),
  m_element_array_vbo(0),
  m_element_array_size(0)
{
}

Mesh::~Mesh()
{
  for(const auto& array : m_attribute_arrays)
  {
    glDeleteBuffers(1, &array.second.vbo);
  }
  glDeleteBuffers(1, &m_element_array_vbo);
}

void
Mesh::draw() 
{
  OpenGLState state;

  GLint program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);

  // activate attribute arrays
  for(const auto& array : m_attribute_arrays)
  {
    int loc = glGetAttribLocation(program, array.first.c_str());
    if (loc == -1)
    {
      log_error("%s: attribute not found", array.first);
    }
    else
    {
      glBindBuffer(GL_ARRAY_BUFFER, array.second.vbo);

      if (array.second.type == Array::Integer)
      {
        glVertexAttribIPointer(loc, array.second.size, GL_INT, 0, nullptr);
      }
      else // if (array.second.type == Array::Float)
      {
        glVertexAttribPointer(loc, array.second.size, GL_FLOAT, GL_FALSE, 0, nullptr);
      }

      glEnableVertexAttribArray(loc);
    }
  }

  // activate element array and draw the mesh
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_array_vbo);
  glDrawElements(m_primitive_type, m_element_array_size, GL_UNSIGNED_INT, 0);
}

/* EOF */
