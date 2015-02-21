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

#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>
#include <GL/glew.h>
#include <iostream>

#include "log.hpp"
#include "opengl_state.hpp"

namespace {

} // namespace

std::unique_ptr<Mesh> 
Mesh::create_skybox(float size)
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

  // skybox
  int faces[] = {
    0, 1, 2, 3, // top
    4, 5, 6, 7, // bottom
    3, 2, 6, 5, // front
    1, 0, 4, 7, // back
    2, 1, 7, 6, // left 
    0, 3, 5, 4  // right 
  };

  // flip front/back faces
  // std::reverse(faces, faces + sizeof(faces)/sizeof(faces[0]));

  std::unique_ptr<Mesh> mesh(new Mesh(GL_TRIANGLE_FAN));

  mesh->attach_float_array("normal", vn);
  mesh->attach_float_array("texcoord", vt);
  mesh->attach_float_array("position", vp);
  mesh->attach_element_array(FaceLst(faces, faces + sizeof(faces)/sizeof(faces[0])));

  return mesh;
}

std::unique_ptr<Mesh>
Mesh::create_plane(float size, glm::vec3 center)
{
  std::unique_ptr<Mesh> mesh(new Mesh(GL_TRIANGLE_FAN));

  NormalLst   vn;
  TexCoordLst vt;
  VertexLst   vp;

  vn.emplace_back(0.0f, 1.0f, 0.0f);
  vn.emplace_back(0.0f, 1.0f, 0.0f);
  vn.emplace_back(0.0f, 1.0f, 0.0f);
  vn.emplace_back(0.0f, 1.0f, 0.0f);

  vt.emplace_back(0.0f, 1.0f, 0.0f);
  vt.emplace_back(1.0f, 1.0f, 0.0f);
  vt.emplace_back(1.0f, 0.0f, 0.0f);
  vt.emplace_back(0.0f, 0.0f, 0.0f);

  vp.emplace_back(-size, 0, +size);
  vp.emplace_back(+size, 0, +size);
  vp.emplace_back(+size, 0, -size);
  vp.emplace_back(-size, 0, -size);

  mesh->attach_float_array("normal", vn);
  mesh->attach_float_array("texcoord", vt);
  mesh->attach_float_array("position", vp);

  return mesh;  
}

std::unique_ptr<Mesh>
Mesh::create_rect(float x1, float y1, float x2, float y2, float z)
{
  std::unique_ptr<Mesh> mesh(new Mesh(GL_TRIANGLE_FAN));

  TexCoordLst vt;
  VertexLst   vp;

  vt.emplace_back(0.0f, 0.0f, 0.0f);
  vt.emplace_back(1.0f, 0.0f, 0.0f);
  vt.emplace_back(1.0f, 1.0f, 0.0f);
  vt.emplace_back(0.0f, 1.0f, 0.0f);

  vp.emplace_back(x1, y2, z);
  vp.emplace_back(x2, y2, z);
  vp.emplace_back(x2, y1, z);
  vp.emplace_back(x1, y1, z);

  mesh->attach_float_array("texcoord", vt);
  mesh->attach_float_array("position", vp);

  return mesh;  
}

std::unique_ptr<Mesh>
Mesh::create_cube(float size)
{
  std::unique_ptr<Mesh> mesh(new Mesh(GL_TRIANGLE_FAN));
  assert(!"not implemented");
  return mesh;
}

std::unique_ptr<Mesh>
Mesh::create_curved_screen(float size, float hfov, float vfov, int rings, int segments, int offset_x, int offset_y, bool flip_uv_x, bool flip_uv_y)
{
  std::unique_ptr<Mesh> mesh(new Mesh(GL_TRIANGLE_FAN));

  NormalLst   vn;
  TexCoordLst vt;
  VertexLst   vp;

  // hfov = 2.0f * glm::pi<float>()
  // vfov = glm::pi<float>()

  // FIXME: should use indexed array instead to save some vertices
  auto add_point = [&](int ring, int seg) {
    float r = static_cast<float>(ring + offset_y) / rings;
    float s = static_cast<float>(seg + offset_x)  / segments;

    float f = sinf((r-0.5f) * vfov + glm::half_pi<float>());
    glm::vec3 p(cosf((s-0.5f) * hfov - glm::half_pi<float>()) * f, 
                cosf((r-0.5f) * vfov + glm::half_pi<float>()),
                sinf((s-0.5f) * hfov - glm::half_pi<float>()) * f);

    vn.push_back(-p);
    vt.emplace_back(!flip_uv_x ? s : 1.0f - s, !flip_uv_y ? r : 1.0f - r, 0.0f);
    vp.push_back(p * size);
  };

  for(int ring = 0; ring < rings; ++ring)
  {
    for(int seg = 0; seg < segments; ++seg)
    {
      add_point(ring+1, seg  );
      add_point(ring+1, seg+1);
      add_point(ring,   seg+1);
      add_point(ring,   seg  );
    }
  }

  mesh->attach_float_array("normal", vn);
  mesh->attach_float_array("texcoord", vt);
  mesh->attach_float_array("position", vp);

  return mesh;  
}

std::unique_ptr<Mesh>
Mesh::create_sphere(float size, int rings, int segments)
{
  std::unique_ptr<Mesh> mesh(new Mesh(GL_TRIANGLE_FAN));

  NormalLst   vn;
  TexCoordLst vt;
  VertexLst   vp;

  // FIXME: should use indexed array instead to save some vertices
  auto add_point = [&](int ring, int seg) {
    float r = static_cast<float>(ring) / rings;
    float s = static_cast<float>(seg)  / segments;

    float f = sinf(r * glm::pi<float>());
    glm::vec3 p(cosf(s * 2.0f * glm::pi<float>()) * f, 
                cosf(r * glm::pi<float>()), 
                sinf(s * 2.0f * glm::pi<float>()) * f);

    vn.push_back(p);
    vt.emplace_back(r, s, 0.0f);
    vp.push_back(p * size);
  };

  for(int ring = 0; ring < rings; ++ring)
  {
    for(int seg = 0; seg < segments; ++seg)
    {
      add_point(ring,   seg  );
      add_point(ring,   seg+1);
      add_point(ring+1, seg+1);
      add_point(ring+1, seg  );
    }
  }

  mesh->attach_float_array("normal", vn);
  mesh->attach_float_array("texcoord", vt);
  mesh->attach_float_array("position", vp);

  return mesh;  
}

Mesh::Mesh(GLenum primitive_type) :
  m_primitive_type(primitive_type),
  m_attribute_arrays(),
  m_element_array_vbo(0),
  m_element_count(-1)
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

  assert_gl("Mesh::draw1");
  //log_debug("Mesh::draw: %d", program);

  // activate attribute arrays
  for(const auto& array : m_attribute_arrays)
  {
    int loc = glGetAttribLocation(program, array.first.c_str());
    if (loc == -1)
    {
      //log_error("%s: attribute not found", array.first);
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

      glBindBuffer(GL_ARRAY_BUFFER, 0);

      glEnableVertexAttribArray(loc);
    }
  }
  assert_gl("Mesh::draw2");

  // activate element array and draw the mesh
  if (m_element_array_vbo)
  {
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_element_array_vbo);
    assert_gl("Mesh::draw: glBindBuffer");
    glDrawElements(m_primitive_type, m_element_count, GL_UNSIGNED_INT, 0);
    assert_gl("Mesh::draw: glDrawElements");
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  }
  else
  {
    glDrawArrays(m_primitive_type, 0, m_element_count);
    assert_gl("Mesh::draw: glDrawArrays");
  }

  // FIXME: missing glDisableVertexAttribArray()
}

/* EOF */
