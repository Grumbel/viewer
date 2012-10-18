#ifndef HEADER_PROGRAM_HPP
#define HEADER_PROGRAM_HPP

#include <memory>
#include <string>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "log.hpp"
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

  template<typename T>
  void set_uniform(const std::string& name, const T& v)
  {
    int loc = glGetUniformLocation(m_program, name.c_str());
    if (loc == -1)
    {
      log_warn("uniform location '%s' not found, ignoring", name);
    }
    else
    {
      set_uniform(loc, v);
    }
  }

 void set_uniform(GLint loc, float v) { glUniform1f(loc, v); }
 void set_uniform(GLint loc, const glm::vec2& v) { glUniform2f(loc, v.x, v.y); }
 void set_uniform(GLint loc, const glm::vec3& v) { glUniform3f(loc, v.x, v.y, v.z); }
 void set_uniform(GLint loc, const glm::vec4& v) { glUniform4f(loc, v.x, v.y, v.z, v.w); }

 void set_uniform(GLint loc, int v) { glUniform1i(loc, v); }
 void set_uniform(GLint loc, unsigned int v) { glUniform1i(loc, v); }
 void set_uniform(GLint loc, const glm::ivec2& v) { glUniform2i(loc, v.x, v.y); }
 void set_uniform(GLint loc, const glm::ivec3& v) { glUniform3i(loc, v.x, v.y, v.z); }
 void set_uniform(GLint loc, const glm::ivec4& v) { glUniform4i(loc, v.x, v.y, v.z, v.w); }

 void set_uniform(GLint loc, const glm::mat3& v) { glUniformMatrix3fv(loc, 1, GL_FALSE, glm::value_ptr(v)); }
 void set_uniform(GLint loc, const glm::mat4& v) { glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v)); }

private:
  Program(const Program&);
  Program& operator=(const Program&);
};

#endif

/* EOF */
