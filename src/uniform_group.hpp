//  Simple 3D Model Viewer
//  Copyright (C) 2012-2013 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_UNIFORM_GROUP_HPP
#define HEADER_UNIFORM_GROUP_HPP

#include <string>
#include <vector>
#include <memory>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <functional>
#include <unordered_map>
#include <tuple>

#include "program.hpp"

class RenderContext;
class UniformGroup;

enum class UniformSymbol {
  NormalMatrix,
    ViewMatrix,
    ModelMatrix,
    ModelViewMatrix,
    ProjectionMatrix,
    ModelViewProjectionMatrix
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
#ifndef HAVE_OPENGLES2
  std::unordered_map<std::string, std::string> m_vertex_subroutine_uniforms;
  std::unordered_map<std::string, std::string> m_fragment_subroutine_uniforms;
#endif

public:
  UniformGroup()
    : m_uniforms()
#ifndef HAVE_OPENGLES2
    , m_vertex_subroutine_uniforms()
    , m_fragment_subroutine_uniforms()
#endif
  {}

  template<typename T>
  void set_uniform(const std::string& name, const T& value)
  {
    m_uniforms.emplace_back(new Uniform<T>(name, value));
  }

  void set_subroutine_uniform(GLenum shadertype,
                              const std::string& name,
                              const std::string& subroutine)
  {
#ifndef HAVE_OPENGLES2
    if (shadertype == GL_FRAGMENT_SHADER)
    {
      m_fragment_subroutine_uniforms[name] = subroutine;
    }
    else if (shadertype == GL_VERTEX_SHADER)
    {
      m_vertex_subroutine_uniforms[name] = subroutine;
    }
    else
    {
      assert(!"not implemented");
    }
#endif
  }

  void apply(ProgramPtr prog, const RenderContext& ctx);
  void apply_subroutines(ProgramPtr prog, GLenum shadertype, const std::unordered_map<std::string, std::string>& subroutines);

private:
  UniformGroup(const UniformGroup&);
  UniformGroup& operator=(const UniformGroup&);
};

typedef std::shared_ptr<UniformGroup> UniformGroupPtr;

#endif

/* EOF */
