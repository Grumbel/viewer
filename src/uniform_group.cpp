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

#include "uniform_group.hpp"

#include "log.hpp"
#include "render_context.hpp"

void
Uniform<UniformSymbol>::apply(ProgramPtr prog, RenderContext const& ctx)
{
  assert_gl("Uniform<UniformSymbol>::apply:enter");
  switch(m_value)
  {
    case UniformSymbol::NormalMatrix:
      prog->set_uniform(m_name, glm::mat3(ctx.get_view_matrix() * ctx.get_model_matrix()));
      break;

    case UniformSymbol::ViewMatrix:
      prog->set_uniform(m_name, ctx.get_view_matrix());
      break;

    case UniformSymbol::ModelMatrix:
      prog->set_uniform(m_name, ctx.get_model_matrix());
      break;

    case UniformSymbol::ModelViewMatrix:
      prog->set_uniform(m_name, ctx.get_view_matrix() * ctx.get_model_matrix());
      break;

    case UniformSymbol::ProjectionMatrix:
      prog->set_uniform(m_name, ctx.get_projection_matrix());
      break;

    case UniformSymbol::ModelViewProjectionMatrix:
      prog->set_uniform(m_name, ctx.get_projection_matrix() * ctx.get_view_matrix() * ctx.get_model_matrix());
      break;

    default:
      log_error("unknown UniformSymbol %d", static_cast<int>(m_value));
      break;
  }
  assert_gl("Uniform<UniformSymbol>::apply:exit");
}

void
Uniform<UniformCallback>::apply(ProgramPtr prog, RenderContext const& ctx)
{
  m_value(prog, m_name, ctx);
}

void
UniformGroup::apply(ProgramPtr prog, RenderContext const& ctx)
{
  assert_gl("apply:enter");
  for(auto& uniform : m_uniforms)
  {
    uniform->apply(prog, ctx);
  }
  assert_gl("apply:exit");
}

/* EOF */
