#ifndef HEADER_MESH_HPP
#define HEADER_MESH_HPP

#include "opengl_state.hpp"
#include "vector.hpp"

struct Face
{
  int vertex1;
  int vertex2;
  int vertex3;
};

typedef Vector Vertex;

class Mesh
{
private:
  typedef std::vector<Vertex> VertexLst;
  typedef std::vector<Face>   FaceLst;

  VertexLst vertices;
  FaceLst   faces;

public:
  Mesh(std::istream& in) :
    vertices(),
    faces()
  {
    int number_of_vertixes;
    in >> number_of_vertixes;
    //std::cout << "Number_Of_Vertixes: " << number_of_vertixes << std::endl;
    for (int i = 0; i < number_of_vertixes; ++i)
    {
      Vertex vertex;
      in >> vertex.x >> vertex.y >> vertex.z;
      vertices.push_back(vertex);
    }

    int number_of_faces;
    in >> number_of_faces;
    for (int i = 0; i < number_of_faces; ++i)
    {
      Face face;
      in >> face.vertex1 >> face.vertex2 >> face.vertex3;
      faces.push_back(face);
    }

    //std::cout << "read " << number_of_faces << " faces, " << number_of_vertixes << " vertices" << std::endl;
  }

  void display()
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

  void draw() 
  {
    for (FaceLst::iterator i = faces.begin(); i != faces.end(); ++i)
    {
      glBegin(GL_TRIANGLES);
      {
        glTexCoord2f(0.0f, 0.0f);
        set_face_normal(*i);
        glVertex3f(vertices[i->vertex1].x,
                   vertices[i->vertex1].y,
                   vertices[i->vertex1].z);
        

        glTexCoord2f(1.0f, 0.0f);
        set_face_normal(*i);
        glVertex3f(vertices[i->vertex2].x,
                   vertices[i->vertex2].y,
                   vertices[i->vertex2].z);

        glTexCoord2f(1.0f, 1.0f);
        set_face_normal(*i);
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

  void draw_face_normal(const Face& face)
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

  Vector calc_face_normal(const Face& face)
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

  void set_face_normal(const Face& face)
  {
    const Vector& normal = calc_face_normal(face);
    glNormal3f(normal.x, normal.y, normal.y);
  }
};

#endif

/* EOF */
