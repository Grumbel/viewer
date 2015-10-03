#include "material_parser.hpp"

#include <GL/glew.h>
#define GL_GLEXT_PROTOTYPES 1
#include <GL/glext.h>
#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <stdexcept>
#include <fstream>
#include <iostream>

#include "tokenize.hpp"
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

//-----------------------------------------------------------------------------

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
    MaterialParser parser(filename.string());
    parser.parse(in);
    return parser.get_material();
  }
}

MaterialPtr
MaterialParser::from_stream(std::istream& in)
{
  MaterialParser parser("<unknown>");
  parser.parse(in);
  return parser.get_material();
}

//-----------------------------------------------------------------------------

MaterialParser::MaterialParser(const std::string& filename) :
  m_filename(filename),
  m_material(std::make_shared<Material>())
{
}

void
MaterialParser::parse(std::istream& in)
{
  bool default_program = true;
  bool has_diffuse_texture  = false;
  bool has_specular_texture = false;
  int current_texture_unit = 0;
  std::string program_vertex   = "src/glsl/default.vert";
  std::string program_fragment = "src/glsl/default.frag";

  m_material->enable(GL_CULL_FACE);
  m_material->enable(GL_DEPTH_TEST);

  m_material->set_uniform("material.ambient", glm::vec3(1.0f, 1.0f, 1.0f));

  int line_number = 0;
  std::string line;
  while(std::getline(in, line))
  {
    try
    {
      line_number += 1;
      std::vector<std::string> args = argument_parse(line);

      if (!args.empty())
      {
        if (args[0] == "material.diffuse")
        {
          m_material->set_uniform("material.diffuse",
                                  to_vec3(args.begin()+1, args.end(),
                                          glm::vec3(1.0f, 1.0f, 1.0f)));
        }
        else if (args[0] == "material.diffuse_texture")
        {
          has_diffuse_texture = true;
          if (args.size() == 2)
          {
            m_material->set_texture(current_texture_unit, Texture::from_file(to_string(args.begin()+1, args.end())));
          }
          else if (args.size() == 3)
          {
            m_material->set_texture(current_texture_unit,
                                    Texture::from_file(args[1]),
                                    Texture::from_file(args[2]));
          }
          else
          {
            throw std::runtime_error("broken");
          }
          m_material->set_uniform("material.diffuse_texture", current_texture_unit);
          current_texture_unit += 1;
        }
        else if (args[0] == "material.specular")
        {
          m_material->set_uniform("material.specular",
                                  to_vec3(args.begin()+1, args.end(),
                                          glm::vec3(1.0f, 1.0f, 1.0f)));
        }
        else if (args[0] == "material.specular_texture")
        {
          has_specular_texture = true;
          m_material->set_texture(current_texture_unit, Texture::from_file(to_string(args.begin()+1, args.end())));
          m_material->set_uniform("material.specular_texture", current_texture_unit);
          current_texture_unit += 1;
        }
        else if (args[0] == "material.shininess")
        {
          m_material->set_uniform("material.shininess",
                                  to_float(args.begin()+1, args.end()));
        }
        else if (args[0] == "material.ambient")
        {
          m_material->set_uniform("material.ambient",
                                  to_vec3(args.begin()+1, args.end(),
                                          glm::vec3(1.0f, 1.0f, 1.0f)));
        }
        else if (args[0] == "blend_mode")
        {
          //m_material->set_
        }
        else if (args[0] == "program.vertex")
        {
          program_vertex = args[1];
          default_program = false;
        }
        else if (args[0] == "program.fragment")
        {
          program_fragment = args[1];
          default_program = false;
        }
        else if (args[0] == "material.disable")
        {
          if (args[1] == "cull_face")
          {
            m_material->disable(GL_CULL_FACE);
          }
          else
          {
            throw std::runtime_error("unknown token: " + args[1]);
          }
        }
        else if (boost::algorithm::starts_with(args[0], "uniform."))
        {
          std::string uniform_name = args[0].substr(8);
          int count = args.end() - args.begin() - 1;
          if (count == 3)
          {
            m_material->set_uniform(uniform_name,
                                    to_vec3(args.begin()+1, args.end(),
                                            glm::vec3(1.0f, 1.0f, 1.0f)));
          }
          else if (count == 1)
          {
            m_material->set_uniform(uniform_name, to_float(args.begin()+1, args.end()));
          }
          else
          {
            throw std::runtime_error("unknown argument count: " + args[0]);
          }
        }
        else
        {
          throw std::runtime_error("unknown token: " + args[0]);
        }
      }
    }
    catch(const std::exception& err)
    {
      throw std::runtime_error(format("%s:%d: error: %s at line:\n%s", m_filename, line_number, err.what(), line));
    }
  }

  ProgramPtr program = Program::create(Shader::from_file(GL_VERTEX_SHADER,   program_vertex),
                                       Shader::from_file(GL_FRAGMENT_SHADER, program_fragment));
  m_material->set_program(program);

  if (default_program)
  {
    if (has_diffuse_texture)
    {
      m_material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "diffuse_color", "diffuse_color_from_texture");
    }
    else
    {
      m_material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "diffuse_color", "diffuse_color_from_material");
    }

    if (has_specular_texture)
    {
      m_material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "specular_color", "specular_color_from_texture");
    }
    else
    {
      m_material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "specular_color", "specular_color_from_material");
    }

    m_material->set_subroutine_uniform(GL_FRAGMENT_SHADER, "shadow_value", "shadow_value_4");
  }
}

/* EOF */
