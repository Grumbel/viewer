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

#include <iostream>

#include "log.hpp"
#include "opengl_state.hpp"

namespace {

} // namespace

Mesh::Mesh() :
  m_normals(),
  m_texcoords(),
  m_vertices(),
  m_faces(),
  m_normals_vbo(0),
  m_texcoords_vbo(0),
  m_vertices_vbo(0),
  m_faces_vbo(0)
{
}

Mesh::Mesh(const NormalLst& normals,
           const TexCoordLst& texcoords,
           const VertexLst& vertices,
           const FaceLst&   faces) :
  m_normals(normals),
  m_texcoords(texcoords),
  m_vertices(vertices),
  m_faces(faces),
  m_normals_vbo(0),
  m_texcoords_vbo(0),
  m_vertices_vbo(0),
  m_faces_vbo(0)
{
  build_vbos();
}

Mesh::~Mesh()
{
  glDeleteBuffers(1, &m_normals_vbo);
  glDeleteBuffers(1, &m_vertices_vbo);
  glDeleteBuffers(1, &m_faces_vbo);
}

void
Mesh::build_vbos()
{
  OpenGLState state;

  log_debug("Normals: %d %d", sizeof(m_normals[0]), m_normals.size());
  glGenBuffers(1, &m_normals_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_normals_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_normals[0]) * m_normals.size(), m_normals.data(), GL_STATIC_DRAW);

  log_debug("Texcoords");
  glGenBuffers(1, &m_texcoords_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_texcoords_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_texcoords[0]) * m_texcoords.size(), m_texcoords.data(), GL_STATIC_DRAW);

  log_debug("Vertices");
  glGenBuffers(1, &m_vertices_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertices_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(m_vertices[0]) * m_vertices.size(), m_vertices.data(), GL_STATIC_DRAW);

  log_debug("Indices");
  glGenBuffers(1, &m_faces_vbo);
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_faces_vbo);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(m_faces[0]) * m_faces.size(), m_faces.data(), GL_STATIC_DRAW);

  assert_gl("VBO upload");
}

void
Mesh::verify() const
{
  std::cout << "Mesh::verify:\n" 
            << "  texcoords: " << m_texcoords.size() << '\n'
            << "  normals:   " << m_normals.size() << '\n'
            << "  vertices:  " << m_vertices.size() << '\n'
            << "  faces:     " << m_faces.size() << '\n';

  for (const auto& face : m_faces)
  {
    if ((face.vertex1<0) ||
         (face.vertex2<0) ||
          (face.vertex3<0) ||
           (face.vertex1 >= static_cast<int>(m_vertices.size())) ||
          (face.vertex2 >= static_cast<int>(m_vertices.size())) ||
         face.vertex3 >= static_cast<int>(m_vertices.size()))
    {
      throw std::runtime_error("face tries to access non existing vertex");
    }
  }

  if (m_vertices.size() != m_texcoords.size())
  {
    throw std::runtime_error("texcoord count doesn't match vertex count");
  }
  
  if (m_vertices.size() != m_normals.size())
  {
    throw std::runtime_error("normal count doesn't match vertex count");
  }
}

void
Mesh::display()
{
  for (FaceLst::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
  {
    std::cout << "Face: " << std::endl;
    std::cout << m_vertices[i->vertex1].x << " "
              << m_vertices[i->vertex1].y << " "
              << m_vertices[i->vertex1].z << std::endl;
    std::cout << m_vertices[i->vertex2].x << " "
              << m_vertices[i->vertex2].y << " "
              << m_vertices[i->vertex2].z << std::endl;
    std::cout << m_vertices[i->vertex3].x << " "
              << m_vertices[i->vertex3].y << " "
              << m_vertices[i->vertex3].z << std::endl;
  }
}

void
Mesh::draw() 
{
  OpenGLState state;

  if (true)
  {
    glBindBuffer(GL_ARRAY_BUFFER, m_normals_vbo);
    glNormalPointer(GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_texcoords_vbo);
    glTexCoordPointer(2, GL_FLOAT, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, m_vertices_vbo);
    glVertexPointer(3, GL_FLOAT, 0, 0);
   
    glEnableClientState(GL_NORMAL_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_VERTEX_ARRAY);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_faces_vbo);
    glDrawElements(GL_TRIANGLES, 3*m_faces.size(), GL_UNSIGNED_INT, 0);
  }
  else
  {
    for (FaceLst::iterator i = m_faces.begin(); i != m_faces.end(); ++i)
    {
      glBegin(GL_TRIANGLES);
      {
        glTexCoord2f(0.0f, 0.0f);
        glNormal3f(m_normals[i->vertex1].x,
                   m_normals[i->vertex1].y,
                   m_normals[i->vertex1].z);
        glVertex3f(m_vertices[i->vertex1].x,
                   m_vertices[i->vertex1].y,
                   m_vertices[i->vertex1].z);
        

        glTexCoord2f(1.0f, 0.0f);
        glNormal3f(m_normals[i->vertex2].x,
                   m_normals[i->vertex2].y,
                   m_normals[i->vertex2].z);
        glVertex3f(m_vertices[i->vertex2].x,
                   m_vertices[i->vertex2].y,
                   m_vertices[i->vertex2].z);

        glTexCoord2f(1.0f, 1.0f);
        glNormal3f(m_normals[i->vertex3].x,
                   m_normals[i->vertex3].y,
                   m_normals[i->vertex3].z);
        glVertex3f(m_vertices[i->vertex3].x,
                   m_vertices[i->vertex3].y,
                   m_vertices[i->vertex3].z);
      }
      glEnd();
    }
  }

  // draw m_normals
  if (false)
  {
    glDisable(GL_LIGHTING);
    glColor3f(0, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (FaceLst::iterator face = m_faces.begin(); face != m_faces.end(); ++face)
    {
      float s = 0.1f;
      glVertex3f(m_vertices[face->vertex1].x, m_vertices[face->vertex1].y, m_vertices[face->vertex1].z);
      glVertex3f(m_vertices[face->vertex1].x + m_normals[face->vertex1].x * s,
                 m_vertices[face->vertex1].y + m_normals[face->vertex1].y * s,
                 m_vertices[face->vertex1].z + m_normals[face->vertex1].z * s);

      glVertex3f(m_vertices[face->vertex2].x, m_vertices[face->vertex2].y, m_vertices[face->vertex2].z);
      glVertex3f(m_vertices[face->vertex2].x + m_normals[face->vertex2].x * s,
                 m_vertices[face->vertex2].y + m_normals[face->vertex2].y * s,
                 m_vertices[face->vertex2].z + m_normals[face->vertex2].z * s);

      glVertex3f(m_vertices[face->vertex3].x, m_vertices[face->vertex3].y, m_vertices[face->vertex3].z);
      glVertex3f(m_vertices[face->vertex3].x + m_normals[face->vertex3].x * s,
                 m_vertices[face->vertex3].y + m_normals[face->vertex3].y * s,
                 m_vertices[face->vertex3].z + m_normals[face->vertex3].z * s);
    }
    glEnd();
    glEnable(GL_LIGHTING);
  }
}

/* EOF */
