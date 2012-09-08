#ifndef HEADER_MODEL_HPP
#define HEADER_MODEL_HPP

#include "mesh.hpp"

class Model
{
private:
  typedef std::vector<Mesh> MeshLst;
  MeshLst meshes;

public:
  Model(const std::string& filename) :
    meshes()
  {
    std::ifstream in(filename.c_str());
    
    if(!in)
    {
      std::cout << filename << ": File not found" << std::endl;
      exit(EXIT_FAILURE);
    }

    int number_of_meshes;
    in >> number_of_meshes;
    std::cout << "number_of_meshes: " << number_of_meshes << std::endl;
    for (int i = 0; i < number_of_meshes; ++i)   
    {
      Mesh mesh(in);
      //mesh.display();
      meshes.push_back(mesh);
    }
  }

  void draw() 
  {
    for (MeshLst::iterator i = meshes.begin(); i != meshes.end(); ++i)
    {
      i->draw();
    }
  }
  
};

#endif

/* EOF */
