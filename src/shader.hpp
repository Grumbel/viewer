#ifndef HEADER_SHADER_HPP
#define HEADER_SHADER_HPP

#include <memory>
#include <GL/glew.h>

class Shader;

typedef std::shared_ptr<Shader> ShaderPtr;

class Shader
{
private:
  GLuint m_shader;

public:
  static ShaderPtr from_file(GLenum type, const std::string& filename);

public:
  Shader(GLenum type);
  ~Shader();

  void source(const std::string& source);
  void compile();

  std::string get_info_log() const;
  bool get_compile_status() const;

  GLuint get_id() const { return m_shader; }

private:
  Shader(const Shader&) = delete;
  Shader& operator=(const Shader&) = delete;
};

#endif

/* EOF */
