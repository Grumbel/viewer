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
    for (FaceLst::iterator i = faces.begin(); i != faces.end(); ++i)
    {
      draw_face_normal(*i);
    }
    glEnd();
    glEnable(GL_LIGHTING);
  }
}

void
Mesh::draw_face_normal(const Face& face)
{
  Vector normal = calc_face_normal(face);
    
  Vector pos(vertices[face.vertex1]
             + vertices[face.vertex2]
             + vertices[face.vertex3]);
  pos = pos * (1.0f/3.0f);

  glVertex3f(pos.x, pos.y, pos.z);
  pos = pos +(0.5* normal);
  glVertex3f(pos.x, pos.y, pos.z);
}

Vector
Mesh::calc_face_normal(const Face& face)
{
  Vector u(vertices[face.vertex2].x - vertices[face.vertex1].x,
           vertices[face.vertex2].y - vertices[face.vertex1].y,
           vertices[face.vertex2].z - vertices[face.vertex1].z);

  Vector v(vertices[face.vertex3].x - vertices[face.vertex2].x,
           vertices[face.vertex3].y - vertices[face.vertex2].y,
           vertices[face.vertex3].z - vertices[face.vertex2].z);

  Vector normal(u.y*v.z - u.z*v.y,
                u.z*v.x - u.x*v.z,
                u.x*v.y - u.y*v.x);

  normal.normalize();

  return normal;
}

void
Mesh::set_face_normal(const Face& face)
{
  const Vector& normal = calc_face_normal(face);
  glNormal3f(normal.x, normal.y, normal.y);
}

/* EOF */
