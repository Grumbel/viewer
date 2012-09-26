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
  MeshLst m_meshes;

public:
  static std::unique_ptr<Model> from_istream(std::istream& in);
  static std::unique_ptr<Model> from_file(const std::string& filename);
  
public:
  Model() :
    m_meshes()
  {}

  void draw() 
  {
    for (MeshLst::iterator i = m_meshes.begin(); i != m_meshes.end(); ++i)
    {
      (*i)->draw();
    }
  }
  
};

#endif

/* EOF */
