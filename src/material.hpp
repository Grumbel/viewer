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

#ifndef HEADER_MATERIAL_HPP
#define HEADER_MATERIAL_HPP

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>
#include <tuple>
#include <unordered_map>

#include "program.hpp"
#include "texture.hpp"
#include "uniform_group.hpp"

class RenderContext;

class TextureValue
{
public:
  enum { REGULAR_TEXTURE, VIDEO_TEXTURE } type;
  TexturePtr primary;
  TexturePtr secondary; // used for right eye in stereo
};

class Material
{
private:
  bool m_cast_shadow;

  ProgramPtr m_program;
  std::unordered_map<int, TextureValue> m_textures;
  UniformGroupPtr m_uniforms;

  std::unordered_map<GLenum, bool> m_capabilities;

  glm::bvec4 m_color_mask;
  bool m_depth_mask;

  GLenum m_blend_sfactor;
  GLenum m_blend_dfactor;

  GLenum m_cull_face;

public:
  Material();

  void cast_shadow(bool v) { m_cast_shadow = v; }
  bool cast_shadow() const { return m_cast_shadow; }

  void set_program(ProgramPtr program) { m_program = program; }
  void set_texture(int unit, TexturePtr texture) { m_textures[unit] = {TextureValue::REGULAR_TEXTURE, texture, texture}; }
  void set_texture(int unit, TexturePtr left, TexturePtr right) { m_textures[unit] = {TextureValue::REGULAR_TEXTURE, left, right}; }
  void set_video_texture(int unit) { m_textures[unit] = {TextureValue::VIDEO_TEXTURE, {}, {}}; }

  void color_mask(bool r, bool g, bool b, bool a);
  void depth_mask(bool flag);
  void blend_func(GLenum sfactor, GLenum dfactor);
  void cull_face(GLenum mode);

  void enable(GLenum cap);
  void disable(GLenum cap);

  template<typename T>
  void set_uniform(const std::string& name, T const& value)
  {
    m_uniforms->set_uniform(name, value);
  }

  void apply(RenderContext const& context);

private:
  Material(const Material&);
  Material& operator=(const Material&);
};

typedef std::shared_ptr<Material> MaterialPtr;

#endif

/* EOF */
