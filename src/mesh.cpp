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
#include "log.hpp"

#include <iostream>

Mesh::Mesh(std::istream& in) :
  normals(),
  vertices(),
  faces()
{
  int num_vertices;
  in >> num_vertices;

  log_info("NumVertices: %d\n", num_vertices);

  for (int i = 0; i < num_vertices; ++i)
  {
    Vector normal;
    in >> normal.x >> normal.y >> normal.z;
    normals.push_back(normal);

    Vector vertex;
    in >> vertex.x >> vertex.y >> vertex.z;
    vertices.push_back(vertex);
  }

  int number_of_faces;
  in >> number_of_faces;
  log_info("NumFaces: %d\n", number_of_faces);
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
}

void
Mesh::display()
{
  for (FaceLst::iterator i = faces.begin(); i != faces.end(); ++i)
  {
    std::cout << "Face: " << std::endl;
    std::cout << vertices[i->vertex1].x << " "
              << vertices[i->vertex1].y << " "
              << vertices[i->vertex1].z << std::endl;
    std::cout << vertices[i->vertex2].x << " "
              << vertices[i->vertex2].y << " "
              << vertices[i->vertex2].z << std::endl;
    std::cout << vertices[i->vertex3].x << " "
              << vertices[i->vertex3].y << " "
              << vertices[i->vertex3].z << std::endl;
  }    
}

void
Mesh::draw() 
{
  for (FaceLst::iterator i = faces.begin(); i != faces.end(); ++i)
  {
    glBegin(GL_TRIANGLES);
    {
      glTexCoord2f(0.0f, 0.0f);
      glNormal3f(normals[i->vertex1].x,
                 normals[i->vertex1].y,
                 normals[i->vertex1].z);
      glVertex3f(vertices[i->vertex1].x,
                 vertices[i->vertex1].y,
                 vertices[i->vertex1].z);
        

      glTexCoord2f(1.0f, 0.0f);
      glNormal3f(normals[i->vertex2].x,
                 normals[i->vertex2].y,
                 normals[i->vertex2].z);
      glVertex3f(vertices[i->vertex2].x,
                 vertices[i->vertex2].y,
                 vertices[i->vertex2].z);

      glTexCoord2f(1.0f, 1.0f);
      glNormal3f(normals[i->vertex3].x,
                 normals[i->vertex3].y,
                 normals[i->vertex3].z);
      glVertex3f(vertices[i->vertex3].x,
                 vertices[i->vertex3].y,
                 vertices[i->vertex3].z);
    }
    glEnd();
  }

  // draw normals
  if (false)
  {
    OpenGLState state;

    glDisable(GL_LIGHTING);
    glColor3f(0, 1.0f, 1.0f);
    glBegin(GL_LINES);
    for (FaceLst::iterator face = faces.begin(); face != faces.end(); ++face)
    {
      float s = 0.1f;
      glVertex3f(vertices[face->vertex1].x, vertices[face->vertex1].y, vertices[face->vertex1].z);
      glVertex3f(vertices[face->vertex1].x + normals[face->vertex1].x * s,
                 vertices[face->vertex1].y + normals[face->vertex1].y * s,
                 vertices[face->vertex1].z + normals[face->vertex1].z * s);

      glVertex3f(vertices[face->vertex2].x, vertices[face->vertex2].y, vertices[face->vertex2].z);
      glVertex3f(vertices[face->vertex2].x + normals[face->vertex2].x * s,
                 vertices[face->vertex2].y + normals[face->vertex2].y * s,
                 vertices[face->vertex2].z + normals[face->vertex2].z * s);

      glVertex3f(vertices[face->vertex3].x, vertices[face->vertex3].y, vertices[face->vertex3].z);
      glVertex3f(vertices[face->vertex3].x + normals[face->vertex3].x * s,
                 vertices[face->vertex3].y + normals[face->vertex3].y * s,
                 vertices[face->vertex3].z + normals[face->vertex3].z * s);
    }
    glEnd();
    glEnable(GL_LIGHTING);
  }
}

/* EOF */
