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

#include "material.hpp"

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "assert_gl.hpp"
#include "log.hpp"
#include "render_context.hpp"

Material::Material() :
  m_cast_shadow(true),
  m_program(),
  m_textures(),
  m_uniforms(std::make_shared<UniformGroup>()),
  m_capabilities(),
  m_color_mask(true, true, true, true),
  m_depth_mask(true),
  m_blend_sfactor(GL_ONE),
  m_blend_dfactor(GL_ZERO),
  m_cull_face(GL_BACK)
{
}

void
Material::color_mask(bool r, bool g, bool b, bool a)
{
  m_color_mask = glm::bvec4(r, g, b, a);
}

void
Material::depth_mask(bool flag)
{
  m_depth_mask = flag;
}

void
Material::blend_func(GLenum sfactor, GLenum dfactor)
{
  m_blend_sfactor = sfactor;
  m_blend_dfactor = dfactor;
}

void
Material::cull_face(GLenum mode)
{
  m_cull_face = mode;
}

void
Material::enable(GLenum cap)
{
  m_capabilities[cap] = true;
}

void
Material::disable(GLenum cap)
{
  m_capabilities[cap] = false;
}

void
Material::apply(RenderContext const& context)
{
  assert_gl("Material::apply:enter");

  for(auto const& cap : m_capabilities)
  {
    if (cap.second)
    {
      glEnable(cap.first);
    }
    else
    {
      glDisable(cap.first);
    }
  }
  assert_gl("caps enable");

  glColorMask(m_color_mask.r, m_color_mask.g, m_color_mask.b, m_color_mask.a);
  glDepthMask(m_depth_mask);
  glCullFace(m_cull_face);

  glBlendFunc(m_blend_sfactor, m_blend_dfactor);
  assert_gl("GL props set");

  for(auto const& it : m_textures)
  {
    auto const& texture_unit = it.first;
    auto const& texture_value = it.second;

    if (texture_value.type == TextureValue::VIDEO_TEXTURE)
    {
      TexturePtr const& texture = context.get_video_texture();
      if (texture)
      {
        glActiveTexture(GL_TEXTURE0 + texture_unit);
        glBindTexture(texture->get_target(), texture->get_id());
      }
    }
    else
    {
      switch(context.get_stereo())
      {
        case Stereo::Center:
        case Stereo::Left:
          glActiveTexture(GL_TEXTURE0 + texture_unit);
          glBindTexture(texture_value.primary->get_target(), texture_value.primary->get_id());
          break;

        case Stereo::Right:
          glActiveTexture(GL_TEXTURE0 + texture_unit);
          glBindTexture(texture_value.secondary->get_target(), texture_value.secondary->get_id());
          break;
      }
    }
  }
  assert_gl("textures bound");

  if (m_program)
  {
    glUseProgram(m_program->get_id());
    assert_gl("program bound");

    if (m_uniforms)
    {
      assert_gl("apply uniforms:enter");
      m_uniforms->apply(m_program, context);
      assert_gl("apply uniforms:exit");
    }
  }

  assert_gl("Material::apply:exit");
}

/* EOF */
