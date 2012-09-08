#ifndef HEADER_MESH_HPP
#define HEADER_MESH_HPP

#include <vector>

#include "opengl_state.hpp"
#include "vector.hpp"

struct Face
{
  int vertex1;
  int vertex2;
  int vertex3;
};

class Mesh
{
private:
  typedef std::vector<Vector> NormalLst;
  typedef std::vector<Vector> VertexLst;
  typedef std::vector<Face>   FaceLst;

  NormalLst normals;
  VertexLst vertices;
  FaceLst   faces;

public:
  Mesh(std::istream& in);

  void display();
  void draw();
  void draw_face_normal(const Face& face);
  Vector calc_face_normal(const Face& face);
  void set_face_normal(const Face& face);
};

#endif

/* EOF */
