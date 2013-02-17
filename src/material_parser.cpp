#include "material_parser.hpp"

#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glext.h>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <fstream>
#include <iostream>

#include "assert_gl.hpp"

namespace {

template<typename T>
glm::vec3 to_vec3(T beg, T end, const glm::vec3& default_value)
{
  glm::vec3 vec = default_value;
  for(T i = beg; i != end; ++i)
  {
    vec[i - beg] = boost::lexical_cast<float>(*i);
  }
  return vec;
}

template<typename T>
float to_float(T beg, T end)
{
  if ((end - beg) != 1)
  {
    throw std::runtime_error("exactly one argument required");
  }
  else
  {
    return boost::lexical_cast<float>(*beg);
  }
}

template<typename T>
std::string to_string(T beg, T end)
{
  if ((end - beg) != 1)
  {
    throw std::runtime_error("exactly one argument required");
  }
  else
  {
    return *beg;
  }
}

} // namespace

MaterialPtr
MaterialParser::from_file(const boost::filesystem::path& filename)
{
  std::ifstream in(filename.string());
  if (!in)
  {
    throw std::runtime_error("MaterialParser: couldn't open: " + filename.string());
  }
  else
  {
    return from_stream(in);
  }
}

MaterialPtr
MaterialParser::from_stream(std::istream& in)
{
  bool has_diffuse_texture  = false;
  bool has_specular_texture = false;
  int current_texture_unit = 0;
  MaterialPtr material = std::make_shared<Material>();

  material->enable(GL_CULL_FACE);
  material->enable(GL_DEPTH_TEST);
  
  ProgramPtr program = Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/default.vert"),
                                       Shader::from_file(GL_FRAGMENT_SHADER, "src/default.frag"));
  material->set_program(program);

  std::string line;
  while(std::getline(in, line))
  {
    boost::tokenizer<boost::char_separator<char> > tokens(line, boost::char_separator<char>(" ", ""));
    std::vector<std::string> args(tokens.begin(), tokens.end());

    if (!args.empty())
    {
      if (args[0] == "material.diffuse")
      {
        material->set_uniform("material.diffuse",
                              to_vec3(args.begin()+1, args.end(), 
                                      glm::vec3(1.0f, 1.0f, 1.0f)));
      }
      else if (args[0] == "material.diffuse_texture")
      {
        has_diffuse_texture = true;
        material->set_texture(current_texture_unit, Texture::from_file(to_string(args.begin()+1, args.end())));
        material->set_uniform("material.diffuse_texture", current_texture_unit);
        current_texture_unit += 1;
      }
      else if (args[0] == "material.specular")
      {
        material->set_uniform("material.specular",
                              to_vec3(args.begin()+1, args.end(), 
                                      glm::vec3(1.0f, 1.0f, 1.0f)));
      }
      else if (args[0] == "material.specular_texture")
      {
        has_specular_texture = true;
        material->set_texture(current_texture_unit, Texture::from_file(to_string(args.begin()+1, args.end())));
        material->set_uniform("material.specular_texture", current_texture_unit);
        current_texture_unit += 1;
      }
      else if (args[0] == "material.shininess")
      {
        material->set_uniform("material.shininess",
                              to_float(args.begin()+1, args.end()));
      }
      else if (args[0] == "material.ambient")
      {
        material->set_uniform("material.ambient", 
                              to_vec3(args.begin()+1, args.end(), 
                                      glm::vec3(1.0f, 1.0f, 1.0f)));
      }
      else if (args[0] == "blend_mode")
      {
        //material->set_
      }      
      else
      {
        throw std::runtime_error("unknown token: " + args[0]);
      }
    }
  }

  if (has_diffuse_texture)
  {
    material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "diffuse_color", "diffuse_color_from_texture");
  }
  else
  {
    material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "diffuse_color", "diffuse_color_from_material");
  }

  if (has_specular_texture)
  {
    material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "specular_color", "specular_color_from_texture");
  }
  else
  {
    material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "specular_color", "specular_color_from_material");    
  }

  material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "shadow_value", "shadow_value_4");

  return material;
}

MaterialParser::MaterialParser(std::istream& in)
{
  std::string line;
  while(std::getline(in, line))
  {
    
  }
}

/* EOF */
