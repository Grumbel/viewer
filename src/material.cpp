#include "material.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "log.hpp"
#include "assert_gl.hpp"

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
Material::apply()
{
  glMaterialfv(GL_FRONT, GL_AMBIENT,  glm::value_ptr(m_ambient));
  glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(m_specular));
  glMaterialfv(GL_FRONT, GL_EMISSION, glm::value_ptr(m_emission));
  glMaterialfv(GL_FRONT, GL_DIFFUSE,  glm::value_ptr(m_diffuse));
  glMaterialf(GL_FRONT,  GL_SHININESS, m_shininess);

  for(const auto& cap : m_capabilities)
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

  for(const auto& it : m_textures)
  {
    glActiveTexture(GL_TEXTURE0 + it.first);
    glBindTexture(it.second->get_target(), it.second->get_id());
  }

  if (m_program)
  {
    glUseProgram(m_program->get_id());

    if (m_uniforms)
    {
      m_uniforms->apply(m_program);
    }
  }

  assert_gl("material");
}

/* EOF */
