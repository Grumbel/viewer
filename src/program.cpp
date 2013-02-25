#include "program.hpp"

#include <vector>

#include "assert_gl.hpp"
#include "log.hpp"

ProgramPtr
Program::create(ShaderPtr shader)
{
  ProgramPtr program = std::make_shared<Program>();
  program->attach(shader);
  program->link();

  program->inspect();

  return program;
}

ProgramPtr
Program::create(ShaderPtr shader1, ShaderPtr shader2)
{
  ProgramPtr program = std::make_shared<Program>();
  program->attach(shader1);
  program->attach(shader2);
  program->link();

  program->inspect();

  return program;
}

ProgramPtr
Program::create(ShaderPtr shader1, ShaderPtr shader2, ShaderPtr shader3)
{
  ProgramPtr program = std::make_shared<Program>();
  program->attach(shader1);
  program->attach(shader2);
  program->attach(shader3);
  program->link();

  program->inspect();

  return program;
}

Program::Program() :
  m_program()
{
  m_program = glCreateProgram();
}

Program::~Program()
{
  glDeleteProgram(m_program);
}

void
Program::attach(ShaderPtr shader)
{
  glAttachShader(m_program, shader->get_id());
}

void
Program::link()
{
  glLinkProgram(m_program);
}

void
Program::validate()
{
  glValidateProgram(m_program);
}

std::string
Program::get_info_log() const
{
  GLint length;
  glGetProgramiv(m_program, GL_INFO_LOG_LENGTH, &length);

  if (length == 0)
  {
    return std::string();
  }
  else
  {
    GLsizei out_length;
    std::vector<char> str(length);
    glGetProgramInfoLog(m_program, str.size(), &out_length, str.data());
    return std::string(str.begin(), str.end());
  }
}

bool
Program::get_link_status() const
{
  GLint link_status;
  glGetProgramiv(m_program, GL_LINK_STATUS, &link_status);
  return link_status == GL_TRUE;
}

bool
Program::get_validate_status() const
{
  GLint validate_status;
  glGetProgramiv(m_program, GL_VALIDATE_STATUS, &validate_status);
  return validate_status == GL_TRUE;
}

/*
	GL_BOOL,
	GL_BOOL_VEC2,
	GL_BOOL_VEC3,
	GL_BOOL_VEC4,
	GL_FLOAT,
	GL_FLOAT_MAT2,
	GL_FLOAT_MAT2,
	GL_FLOAT_MAT2x3,
	GL_FLOAT_MAT2x3,
	GL_FLOAT_MAT2x4,
	GL_FLOAT_MAT2x4,
	GL_FLOAT_MAT3,
	GL_FLOAT_MAT3,
	GL_FLOAT_MAT3x2,
	GL_FLOAT_MAT3x2,
	GL_FLOAT_MAT3x4,
	GL_FLOAT_MAT3x4,
	GL_FLOAT_MAT4,
	GL_FLOAT_MAT4,
	GL_FLOAT_MAT4x2,
	GL_FLOAT_MAT4x2,
	GL_FLOAT_MAT4x3 
	GL_FLOAT_MAT4x3,
	GL_FLOAT_VEC2,
	GL_FLOAT_VEC2,
	GL_FLOAT_VEC3,
	GL_FLOAT_VEC3,
	GL_FLOAT_VEC4,
	GL_FLOAT_VEC4,
	GL_INT,
	GL_INT_VEC2,
	GL_INT_VEC3,
	GL_INT_VEC4,
	GL_SAMPLER_1D,
	GL_SAMPLER_1D_SHADOW
	GL_SAMPLER_2D,
	GL_SAMPLER_2D_SHADOW
	GL_SAMPLER_3D,
	GL_SAMPLER_CUBE,
        GL_FLOAT,
*/

void
Program::inspect() const
{
  { // handle attributes
    GLint active_attributes;
    GLint active_attribute_max_length;

    glGetProgramiv(m_program, GL_ACTIVE_ATTRIBUTES, &active_attributes);
    glGetProgramiv(m_program, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &active_attribute_max_length);
  
    for(GLint i = 0; i < active_attributes; ++i)
    {
      GLsizei length;
      GLint size;
      GLenum type;
      std::vector<GLchar> name(active_attribute_max_length);

      glGetActiveAttrib(m_program, i, name.size(), &length, &size, &type, name.data());

      log_info("Attribute: %s type:%d size:%d", std::string(name.data(), length), type, size);
    }
  }
  
  { // handle uniforms
    GLint active_uniforms;
    GLint active_uniform_max_length;

    glGetProgramiv(m_program, GL_ACTIVE_UNIFORMS, &active_uniforms);
    glGetProgramiv(m_program, GL_ACTIVE_UNIFORM_MAX_LENGTH, &active_uniform_max_length);
  
    for(GLint i = 0; i < active_uniforms; ++i)
    {
      GLsizei length;
      GLint size;
      GLenum type;
      std::vector<GLchar> name(active_uniform_max_length);

      glGetActiveUniform(m_program, i, name.size(), &length, &size, &type, name.data());

      //log_info("Uniform: %s type:%d size:%d", std::string(name.data(), length), type, size);
    }    
  }

  assert_gl("Program::inspect");
}

/* EOF */
