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
Uniform<UniformSymbol>::apply(ProgramPtr prog, const RenderContext& ctx)
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
Uniform<UniformCallback>::apply(ProgramPtr prog, const RenderContext& ctx)
{
  m_value(prog, m_name, ctx);
}

void
UniformGroup::apply(ProgramPtr prog, const RenderContext& ctx)
{
  assert_gl("apply:enter");
  for(auto& uniform : m_uniforms)
  {
    uniform->apply(prog, ctx);
  }

#if 0
  // not supported in OpenGL3.3 Core
  apply_subroutines(prog, GL_VERTEX_SHADER, m_vertex_subroutine_uniforms);
  apply_subroutines(prog, GL_FRAGMENT_SHADER, m_fragment_subroutine_uniforms);
#endif
  assert_gl("apply:exit");
}

void
UniformGroup::apply_subroutines(ProgramPtr prog, GLenum shadertype, 
                                const std::unordered_map<std::string, std::string>& subroutines)
{
  assert_gl("apply_subroutines:enter");
  GLint num_uniform_locations;
  glGetProgramStageiv(prog->get_id(), shadertype, GL_ACTIVE_SUBROUTINE_UNIFORM_LOCATIONS, &num_uniform_locations);
  
  std::vector<GLuint> subroutine_mappings;
  for(int i = 0; i < num_uniform_locations; ++i)
  {
    char name[256];
    GLsizei length;
    glGetActiveSubroutineUniformName(prog->get_id(), shadertype, i, sizeof(name), &length, name);
    
    const auto& it = subroutines.find(name);
    if (it == subroutines.end())
    {
      log_error("unmapped subroutine: %s", name);
    }
    else
    {
      GLuint loc = glGetSubroutineIndex(prog->get_id(), shadertype, it->second.c_str());
      if (loc == GL_INVALID_INDEX)
      {
        log_error("unknown subroutine: %s", it->second);
      }
      else
      {
        subroutine_mappings.emplace_back(loc);  
      }
    }
  }

  glUniformSubroutinesuiv(shadertype, subroutine_mappings.size(), subroutine_mappings.data());

  assert_gl("apply_subroutines:exit");
}

/* EOF */
