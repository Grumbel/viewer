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
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>

#include "log.hpp"
#include "opengl_state.hpp"

namespace {

} // namespace

std::unique_ptr<Mesh>
Mesh::from_obj_istream(std::istream& in)
{
  // This is not a fully featured .obj file reader, it just takes some
  // inspiration from it: 
  // http://www.martinreddy.net/gfx/3d/OBJ.spec

  std::unique_ptr<Mesh> mesh(new Mesh);

  std::string line;
  int line_number = 0;

  while(std::getline(in, line))
  {
    line_number += 1;

    boost::tokenizer<boost::char_separator<char> > tokens(line, boost::char_separator<char>(" ", ""));
    for(auto it = tokens.begin(); it != tokens.end(); ++it)
    {
#define INCR_AND_CHECK {                                                \
        ++it;                                                           \
        if (it == tokens.end())                                         \
        {                                                               \
          throw std::runtime_error((boost::format("not enough tokens at line %d") % line_number).str()); \
        }                                                               \
      }

      try
      {
        if (*it == "g")
        {
          // group
          break;
        }
        else if (*it == "v")
        {
          glm::vec3 v;

          INCR_AND_CHECK;
          v.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          v.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          v.z = boost::lexical_cast<float>(*it);

          mesh->m_vertices.push_back(v);
        }
        else if (*it == "vt")
        {
          glm::vec2 vt;

          INCR_AND_CHECK;
          vt.s = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          vt.t = boost::lexical_cast<float>(*it);

          mesh->m_texcoords.push_back(vt);
        }
        else if (*it == "vn")
        {
          glm::vec3 vn;

          INCR_AND_CHECK;
          vn.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          vn.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          vn.z = boost::lexical_cast<float>(*it);

          mesh->m_normals.push_back(vn);
        }
        else if (*it == "f")
        {
          Face face;

          INCR_AND_CHECK;
          face.vertex1 = boost::lexical_cast<int>(*it);
          INCR_AND_CHECK;
          face.vertex2 = boost::lexical_cast<int>(*it);
          INCR_AND_CHECK;
          face.vertex3 = boost::lexical_cast<int>(*it);

          mesh->m_faces.push_back(face);
        }
        else if ((*it)[0] == '#')
        {
          // ignore comments
          break;
        }
        else
        {
          throw std::runtime_error((boost::format("unhandled token %s") % *it).str());
        }
      }
      catch(const std::exception& err)
      {
        throw std::runtime_error((boost::format("unknown:%d: %s") % line_number % err.what()).str());
      }
    }
  }

  // fill in some texcoords if there aren't enough
  if (mesh->m_texcoords.size() < mesh->m_vertices.size())
  {
    auto& texcoords = mesh->m_texcoords;

    texcoords.resize(mesh->m_vertices.size());
    for(FaceLst::size_type i = mesh->m_vertices.size()-1; i < texcoords.size(); ++i)
    {
      texcoords[i] = glm::vec2(0.0f, 0.0f);
    }
  }

  mesh->build_vbos();
  mesh->verify();

  return mesh;
}

std::unique_ptr<Mesh>
Mesh::from_istream(std::istream& in)
{
  NormalLst normals;
  VertexLst vertices;
  FaceLst   faces;
  TexCoordLst texcoords;

  int num_vertices;
  in >> num_vertices;

  log_info("NumVertices: %d", num_vertices);

  for (int i = 0; i < num_vertices; ++i)
  {
    glm::vec3 normal;
    in >> normal.x >> normal.y >> normal.z;
    normals.push_back(normal);

    glm::vec3 vertex;
    in >> vertex.x >> vertex.y >> vertex.z;
    vertices.push_back(vertex);
  }

  int number_of_faces;
  in >> number_of_faces;
  log_info("NumFaces: %d", number_of_faces);
  for (int i = 0; i < number_of_faces; ++i)
  {
    Face face;
    in >> face.vertex1 >> face.vertex2 >> face.vertex3;
    
    if (0 <= face.vertex1 && face.vertex1 < num_vertices &&
        0 <= face.vertex2 && face.vertex2 < num_vertices &&
        0 <= face.vertex3 && face.vertex3 < num_vertices)
    {
      faces.push_back(face);
    }
    else
    {
      log_info("invalid face: %d %d %d", face.vertex1, face.vertex2, face.vertex3);
    }
  }

  // generate some texcoords
  texcoords.resize(faces.size() * 3);
  for(FaceLst::size_type i = 0; i < faces.size(); ++i)
  {
    if (false)
    {
      texcoords[faces[i].vertex1] = glm::vec2(0.0f, 0.0f);
      texcoords[faces[i].vertex2] = glm::vec2(1.0f, 0.0f);
      texcoords[faces[i].vertex3] = glm::vec2(0.0f, 1.0f);
    }
    else
    {
      texcoords[3*i+0] = glm::vec2(0.0f, 0.0f);
      texcoords[3*i+1] = glm::vec2(1.0f, 0.0f);
      texcoords[3*i+2] = glm::vec2(0.0f, 1.0f);
    }
  }

  return std::unique_ptr<Mesh>(new Mesh(normals, texcoords, vertices, faces));
}

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
    if (face.vertex1 < 0 ||
                       face.vertex2 < 0 ||
                                      face.vertex3 < 0 ||
                                                     face.vertex1 >= static_cast<int>(m_vertices.size()) ||
                                      face.vertex2 >= static_cast<int>(m_vertices.size()) ||
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
