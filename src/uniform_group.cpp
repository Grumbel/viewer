#include "uniform_group.hpp"

#include "log.hpp"
#include "render_context.hpp"

void
Uniform<UniformSymbol>::apply(ProgramPtr prog, const RenderContext& ctx)
{
  switch(m_value)
  {
    case kUniformNormalMatrix:
      prog->set_uniform(m_name, glm::mat3(ctx.get_view_matrix() * ctx.get_model_matrix()));
      break;

    case kUniformViewMatrix:
      prog->set_uniform(m_name, ctx.get_view_matrix());
      break;
      
    case kUniformModelMatrix:
      prog->set_uniform(m_name, ctx.get_model_matrix());
      break;
      
    case kUniformModelViewMatrix:
      prog->set_uniform(m_name, ctx.get_view_matrix() * ctx.get_model_matrix());
      break;

    case kUniformProjectionMatrix:
      prog->set_uniform(m_name, ctx.get_projection_matrix());
      break;

    case kUniformModelViewProjectionMatrix:
      prog->set_uniform(m_name, ctx.get_projection_matrix() * ctx.get_view_matrix() * ctx.get_model_matrix());
      break;
      
    default:
      log_error("unknown UniformSymbol %s", m_value);
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
