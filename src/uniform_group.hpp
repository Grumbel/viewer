#ifndef HEADER_UNIFORM_GROUP_HPP
#define HEADER_UNIFORM_GROUP_HPP

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "program.hpp"

class UniformGroup;

enum UniformType {
  UNIFORM_TYPE_1F,
  UNIFORM_TYPE_2F,
  UNIFORM_TYPE_3F,
  UNIFORM_TYPE_4F,

  UNIFORM_TYPE_1I,
  UNIFORM_TYPE_2I,
  UNIFORM_TYPE_3I,
  UNIFORM_TYPE_4I,

  UNIFORM_TYPE_MAT4,
  
  UNIFORM_TYPE_SYMBOLIC
};

enum UniformSymbol
{
  kUniformModelviewMatrix,
  kUniformProjectionMatrix
};

inline void set_uniform(GLint loc, float v) { glUniform1f(loc, v); }
inline void set_uniform(GLint loc, const glm::vec2& v) { glUniform2f(loc, v.x, v.y); }
inline void set_uniform(GLint loc, const glm::vec3& v) { glUniform3f(loc, v.x, v.y, v.z); }
inline void set_uniform(GLint loc, const glm::vec4& v) { glUniform4f(loc, v.x, v.y, v.z, v.w); }

inline void set_uniform(GLint loc, int v) { glUniform1i(loc, v); }
inline void set_uniform(GLint loc, const glm::ivec2& v) { glUniform2i(loc, v.x, v.y); }
inline void set_uniform(GLint loc, const glm::ivec3& v) { glUniform3i(loc, v.x, v.y, v.z); }
inline void set_uniform(GLint loc, const glm::ivec4& v) { glUniform4i(loc, v.x, v.y, v.z, v.w); }

inline void set_uniform(GLint loc, const glm::mat4& v) { glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(v)); }
inline void set_uniform(GLint loc, const std::function<void (GLint)>& func) { func(loc); }

class UniformBase
{
private:
  std::string m_name;

public:
  UniformBase(const std::string& name) :
    m_name(name)
  {}
  virtual ~UniformBase() {}

  std::string get_name() const { return m_name; }
  virtual void apply(int loc) = 0;
};

template<typename T>
class Uniform : public UniformBase
{
private:
  T m_value;

public:
  Uniform(const std::string& name, const T& value) :
    UniformBase(name),
    m_value(value)
  {}

  void apply(int loc)
  {
    set_uniform(loc, m_value);
  }
};

class UniformGroup
{
private:
  std::vector<std::unique_ptr<UniformBase> > m_uniforms;

public:
  UniformGroup() :
    m_uniforms()
  {}

  template<typename T>
  void set_uniform(const std::string& name, const T& value)
  {
    m_uniforms.emplace_back(new Uniform<T>(name, value));
  }

  void apply(ProgramPtr prog);

private:
  UniformGroup(const UniformGroup&);
  UniformGroup& operator=(const UniformGroup&);
};

typedef std::shared_ptr<UniformGroup> UniformGroupPtr;

#endif

/* EOF */
