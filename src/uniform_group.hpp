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
  virtual void apply(ProgramPtr prog, RenderContext const& ctx) = 0;
};

template<typename T>
class Uniform : public UniformBase
{
private:
  T m_value;

public:
  Uniform(const std::string& name, T const& value) :
    UniformBase(name),
    m_value(value)
  {}

  void apply(ProgramPtr prog, RenderContext const& ctx)
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
  Uniform(const std::string& name, UniformSymbol const& value) :
    UniformBase(name),
    m_value(value)
  {}

  void apply(ProgramPtr prog, RenderContext const& ctx);
};

typedef std::function<void (ProgramPtr prog, const std::string& name, RenderContext const& ctx)> UniformCallback;

template<>
class Uniform<UniformCallback> : public UniformBase
{
private:
  UniformCallback m_value;

public:
  Uniform(const std::string& name, UniformCallback const& value) :
    UniformBase(name),
    m_value(value)
  {}

  void apply(ProgramPtr prog, RenderContext const& ctx);
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
  void set_uniform(const std::string& name, T const& value)
  {
    m_uniforms.emplace_back(std::make_unique<Uniform<T> >(name, value));
  }

  void apply(ProgramPtr prog, RenderContext const& ctx);

private:
  UniformGroup(const UniformGroup&);
  UniformGroup& operator=(const UniformGroup&);
};

typedef std::shared_ptr<UniformGroup> UniformGroupPtr;

#endif

/* EOF */
