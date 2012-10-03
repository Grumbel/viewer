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

#include "model.hpp"

#include <fstream>
#include <iostream>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>

#include "log.hpp"

std::unique_ptr<Model>
Model::from_file(const std::string& filename)
{
  std::ifstream in(filename.c_str());
    
  if(!in)
  {
    std::cout << filename << ": File not found" << std::endl;
    exit(EXIT_FAILURE);
  }
  else
  {
    return Model::from_istream(in);
  }
}

std::unique_ptr<Model>
Model::from_istream(std::istream& in)
{
  // This is not a fully featured .obj file reader, it just takes some
  // inspiration from it: 
  // http://www.martinreddy.net/gfx/3d/OBJ.spec

  Mesh::NormalLst   normals;
  Mesh::TexCoordLst texcoords;
  Mesh::VertexLst   vertices;
  Mesh::FaceLst     faces;
  Mesh::BoneWeights bone_weights;
  Mesh::BoneIndices bone_indices;
  glm::vec3         location;

  std::unique_ptr<Model> model(new Model);

  auto commit_object = [&]{
    if (!vertices.empty())
    {
      // fill in some texcoords if there aren't enough
      if (texcoords.size() < vertices.size())
      {
        texcoords.resize(vertices.size());
        for(Mesh::FaceLst::size_type i = vertices.size()-1; i < texcoords.size(); ++i)
        {
          texcoords[i] = glm::vec2(0.0f, 0.0f);
        }
      }

      std::unique_ptr<Mesh> mesh(new Mesh(normals, texcoords, vertices, faces, bone_weights, bone_indices));

      mesh->verify();

      mesh->set_location(location);

      model->m_meshes.push_back(std::move(mesh));

      // clear for the next
      location = glm::vec3();
      normals.clear();
      texcoords.clear();
      vertices.clear();
      faces.clear();
    }
  };

  std::string line;
  int line_number = 0;
  while(std::getline(in, line))
  {
    line_number += 1;

    boost::tokenizer<boost::char_separator<char> > tokens(line, boost::char_separator<char>(" ", ""));
    auto it = tokens.begin();
    if (it != tokens.end())
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
        }
        else if (*it == "o")
        {
          INCR_AND_CHECK;
          log_debug("object: %s", *it);

          // object
          commit_object();
        }
        else if (*it == "loc")
        {
          glm::vec3 v;

          INCR_AND_CHECK;
          v.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          v.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          v.z = boost::lexical_cast<float>(*it);
        
          location = v;
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
          
          vertices.push_back(v);
        }
        else if (*it == "vt")
        {
          glm::vec2 vt;

          INCR_AND_CHECK;
          vt.s = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          vt.t = boost::lexical_cast<float>(*it);

          texcoords.push_back(vt);
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

          normals.push_back(vn);
        }
        else if (*it == "bw")
        {
          glm::vec4 bw;

          INCR_AND_CHECK;
          bw.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          bw.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          bw.z = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          bw.w = boost::lexical_cast<float>(*it);
          
          bone_weights.push_back(bw);
        }
        else if (*it == "bi")
        {
          glm::ivec4 bi;

          INCR_AND_CHECK;
          bi.x = boost::lexical_cast<int>(*it);
          INCR_AND_CHECK;
          bi.y = boost::lexical_cast<int>(*it);
          INCR_AND_CHECK;
          bi.z = boost::lexical_cast<int>(*it);
          INCR_AND_CHECK;
          bi.w = boost::lexical_cast<int>(*it);
          
          bone_indices.push_back(bi);
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

          faces.push_back(face);
        }
        else if ((*it)[0] == '#')
        {
          // ignore comments
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

  commit_object();

  return model;
}

/* EOF */
