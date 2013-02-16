#include "material_parser.hpp"

#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <stdexcept>
#include <fstream>
#include <iostream>

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
MaterialParser::from_file(const std::string& filename)
{
  std::ifstream in(filename);
  if (!in)
  {
    throw std::runtime_error("couldn't open: " + filename);
  }
  else
  {
    return from_stream(in);
  }
}

MaterialPtr
MaterialParser::from_stream(std::istream& in)
{
  int current_texture_unit = 0;
  MaterialPtr material = std::make_shared<Material>();

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
