#ifndef HEADER_PROGRAM_HPP
#define HEADER_PROGRAM_HPP

#include <memory>
#include <string>
#include <GL/glew.h>

#include "shader.hpp"

class Program;

typedef std::shared_ptr<Program> ProgramPtr;

class Program
{
private:
  GLuint m_program;

public:
  static ProgramPtr create(ShaderPtr shader);
  static ProgramPtr create(ShaderPtr shader1, ShaderPtr shader2);
  static ProgramPtr create(ShaderPtr shader1, ShaderPtr shader2, ShaderPtr shader3);

public:
  Program();
  ~Program();

  void attach(ShaderPtr shader);
  void link();
  void validate();

  std::string get_info_log() const;
  bool get_link_status() const;
  bool get_validate_status() const;
  
  GLuint get_id() const { return m_program; }

  void inspect() const;

private:
  Program(const Program&);
  Program& operator=(const Program&);
};

#endif

/* EOF */
