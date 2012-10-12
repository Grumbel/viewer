#include "uniform_group.hpp"

#include "log.hpp"

void
UniformGroup::apply(ProgramPtr prog)
{
  for(auto& uniform : m_uniforms)
  {
    int loc = glGetUniformLocation(prog->get_id(), uniform->get_name().c_str());
    if (loc == -1)
    {
      log_warn("uniform location '%s' not found", uniform->get_name());
    }
    else
    {
      uniform->apply(loc);
    }
  }
}

/* EOF */
