#include "uniform_group.hpp"

#include "log.hpp"
#include "render_context.hpp"

void
Uniform<UniformSymbol>::apply(ProgramPtr prog, const RenderContext& ctx)
{
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
}

void
Uniform<UniformCallback>::apply(ProgramPtr prog, const RenderContext& ctx)
{
  m_value(prog, m_name, ctx);
}

void
UniformGroup::apply(ProgramPtr prog, const RenderContext& ctx)
{
  for(auto& uniform : m_uniforms)
  {
    uniform->apply(prog, ctx);
  }
}

/* EOF */
