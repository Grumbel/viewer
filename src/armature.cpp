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

#include "armature.hpp"

#include <GL/glew.h>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>

#include "log.hpp"

namespace {

glm::vec3 vec3(const std::vector<std::string>& args)
{
  return glm::vec3(boost::lexical_cast<float>(args[1]),
                   boost::lexical_cast<float>(args[2]),
                   boost::lexical_cast<float>(args[3]));
}


glm::mat3 mat3(const std::vector<std::string>& args)
{
  glm::mat3 m;  
  for(int i = 0; i < 9; ++i)
  {
    glm::value_ptr(m)[i] = boost::lexical_cast<float>(args[i+1]);
  }
  return m;
}

glm::mat4 mat4(const std::vector<std::string>& args)
{
  glm::mat4 m;  
  for(int i = 0; i < 16; ++i)
  {
    glm::value_ptr(m)[i] = boost::lexical_cast<float>(args[i+1]);
  }
  return m;
}

} // namespace

std::unique_ptr<Armature>
Armature::from_file(const std::string& filename)
{
  std::ifstream in;
  in.exceptions( std::ifstream::badbit );
  in.open(filename.c_str());
    
  std::unique_ptr<Armature> armature(new Armature);
  std::unique_ptr<Bone> bone;

  std::string line;
  while(std::getline(in, line))
  {
    boost::tokenizer<boost::char_separator<char> > tokens(line, boost::char_separator<char>(" ", ""));
    std::vector<std::string> args(tokens.begin(), tokens.end());

    if (!args.empty() && args[0][0] != '#')
    {
      if (args[0] == "bone")
      {
        assert(args.size() == 2);

        if (bone)
        {
          armature->m_bones.push_back(std::move(bone));
        }

        bone.reset(new Bone);
        bone->name = args[1];
      }
      else if (args[0] == "parent")
      {
      }
      else if (args[0] == "head")
      {
        assert(args.size() == 4);
        bone->head = vec3(args);
      }
      else if (args[0] == "tail")
      {
        assert(args.size() == 4);
        bone->tail = vec3(args);
      }
      else if (args[0] == "head_local")
      {
        assert(args.size() == 4);
        bone->head_local = vec3(args);
      }
      else if (args[0] == "tail_local")
      {
        assert(args.size() == 4);
        bone->tail_local = vec3(args);
      }
      else if (args[0] == "matrix")
      {
        assert(args.size() == 10);
        bone->matrix = mat3(args);
      }
      else if (args[0] == "matrix_local")
      {
        assert(args.size() == 17);
        bone->matrix_local = glm::inverse(mat4(args));
      }
      else
      {
        log_warn("armature: unhandled tag: %s", args[0]);
      }
    }
  }

  if (bone)
  {
    armature->m_bones.push_back(std::move(bone));
  }

  return armature;
}

Armature::Armature() :
  m_bones()
{
}

void
Armature::bind_uniform(int loc)
{
  for(size_t i = 0; i < m_bones.size(); ++i)
  {
    //matrices.push_back(glm::mat4(1));
    glUniformMatrix4fv(loc + i, 1, GL_FALSE, 
                       glm::value_ptr(m_bones[i]->matrix_local));
  }
}

/* EOF */
