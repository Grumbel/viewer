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

#include "pose.hpp"

#include <GL/glew.h>
#include <fstream>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>

#include "log.hpp"

namespace {

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

std::unique_ptr<Pose>
Pose::from_file(const std::string& filename)
{
  std::ifstream in(filename);

  std::unique_ptr<Pose> pose(new Pose);
  std::unique_ptr<PoseBone> bone;

  std::string line;
  while(std::getline(in, line))
  {
    log_debug("pose: tokenizer: %s", line);
    boost::tokenizer<boost::char_separator<char> > tokens(line, boost::char_separator<char>(" ", ""));
    std::vector<std::string> args(tokens.begin(), tokens.end());
    log_debug("pose: tokenizer: done");
    if (!args.empty() && args[0][0] != '#')
    {
      if (args[0] == "bone")
      {
        assert(args.size() == 2);

        if (bone)
        {
          pose->m_bones.push_back(std::move(bone));
        }

        bone.reset(new PoseBone);
        bone->name = args[1];
      }
      else if (args[0] == "matrix")
      {
        assert(args.size() == 17);
        assert(bone);
        bone->matrix = mat4(args);
      }
      else if (args[0] == "matrix_basis")
      {
        assert(args.size() == 17);
        assert(bone);
        bone->matrix_basis = mat4(args);
      }
      else
      {
        log_warn("pose: unhandled tag: %s", args[0]);
      }
    }
  }

  if (bone)
  {
    pose->m_bones.push_back(std::move(bone));
  }

  return pose;
}

Pose::Pose() :
  m_bones()
{
}

void
Pose::bind_uniform(int loc)
{
  for(size_t i = 0; i < m_bones.size(); ++i)
  {
    //matrices.push_back(glm::mat4(1));
    glUniformMatrix4fv(loc + i, 1, GL_FALSE,
                       glm::value_ptr(m_bones[i]->matrix));
  }
}

/* EOF */
