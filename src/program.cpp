#include "program.hpp"

#include <vector>

ProgramPtr
Program::create(ShaderPtr shader)
{
  ProgramPtr program = std::make_shared<Program>();
  program->attach(shader);
  program->link();
  return program;
}

ProgramPtr
Program::create(ShaderPtr shader1, ShaderPtr shader2)
{
  ProgramPtr program = std::make_shared<Program>();
  program->attach(shader1);
  program->attach(shader2);
  program->link();
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

/* EOF */
