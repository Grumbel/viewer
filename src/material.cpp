#include "material.hpp"

#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "log.hpp"
#include "assert_gl.hpp"

void
Material::apply()
{
  glMaterialfv(GL_FRONT, GL_AMBIENT,  glm::value_ptr(m_ambient));
  glMaterialfv(GL_FRONT, GL_SPECULAR, glm::value_ptr(m_specular));
  glMaterialfv(GL_FRONT, GL_EMISSION, glm::value_ptr(m_emission));
  glMaterialfv(GL_FRONT, GL_DIFFUSE,  glm::value_ptr(m_diffuse));
  glMaterialf(GL_FRONT,  GL_SHININESS, m_shininess);

  for(const auto& it : m_textures)
  {
    log_debug("activate texture");
    glActiveTexture(GL_TEXTURE0 + it.first);
    glBindTexture(it.second->get_target(), it.second->get_id());
  }

  if (m_program)
  {
    log_debug("activate program %d", m_program->get_id());
    glUseProgram(m_program->get_id());
  }

  assert_gl("material");
}

/* EOF */
