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

#ifndef HEADER_MODEL_HPP
#define HEADER_MODEL_HPP

#include "mesh.hpp"

class Model
{
private:
  typedef std::vector<std::unique_ptr<Mesh> > MeshLst;
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

    std::unique_ptr<Mesh> mesh = Mesh::from_obj_istream(in);
    meshes.push_back(std::move(mesh));
  }

  void draw() 
  {
    for (MeshLst::iterator i = meshes.begin(); i != meshes.end(); ++i)
    {
      (*i)->draw();
    }
  }
  
};

#endif

/* EOF */
