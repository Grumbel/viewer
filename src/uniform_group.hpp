#ifndef HEADER_UNIFORM_GROUP_HPP
#define HEADER_UNIFORM_GROUP_HPP

#include <string>
#include <vector>
#include <memory>
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <functional>

#include "program.hpp"

class RenderContext;
class UniformGroup;

enum UniformSymbol
{
  kUniformNormalMatrix,
  kUniformViewMatrix,
  kUniformModelMatrix,
  kUniformModelViewMatrix,
  kUniformProjectionMatrix,
  kUniformModelViewProjectionMatrix
};

class UniformBase
{
protected:
  std::string m_name;

public:
  UniformBase(const std::string& name) :
    m_name(name)
  {}
  virtual ~UniformBase() {}

  std::string get_name() const { return m_name; }
  virtual void apply(ProgramPtr prog, const RenderContext& ctx) = 0;
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

  void apply(ProgramPtr prog, const RenderContext& ctx)
  {
    prog->set_uniform(m_name, m_value);
  }
};

template<>
class Uniform<UniformSymbol> : public UniformBase
{
private:
  UniformSymbol m_value;

public:
  Uniform(const std::string& name, const UniformSymbol& value) :
    UniformBase(name),
    m_value(value)
  {}

  void apply(ProgramPtr prog, const RenderContext& ctx);
};

typedef std::function<void (ProgramPtr prog, const std::string& name, const RenderContext& ctx)> UniformCallback;

template<>
class Uniform<UniformCallback> : public UniformBase
{
private:
  UniformCallback m_value;

public:
  Uniform(const std::string& name, const UniformCallback& value) :
    UniformBase(name),
    m_value(value)
  {}

  void apply(ProgramPtr prog, const RenderContext& ctx);
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

  void apply(ProgramPtr prog, const RenderContext& ctx);

private:
  UniformGroup(const UniformGroup&);
  UniformGroup& operator=(const UniformGroup&);
};

typedef std::shared_ptr<UniformGroup> UniformGroupPtr;

#endif

/* EOF */
