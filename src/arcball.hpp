#ifndef HEADER_ARCBALL_HPP
#define HEADER_ARCBALL_HPP

#if 0


glm::vec3
Viewer::get_arcball_vector(glm::ivec2 mouse)
{
  float radius = std::min(m_screen_w, m_screen_h) / 2.0f;
  glm::vec3 P = glm::vec3(static_cast<float>(mouse.x - m_screen_w/2) / radius,
                          static_cast<float>(mouse.y - m_screen_h/2) / radius,
                          0);

  //log_info("arcball: %f %f", P.x, P.y);
  P.y = -P.y;
  float OP_squared = P.x * P.x + P.y * P.y;
  if (glm::length(P) <= 1)
  {
    P.z = sqrt(1*1 - OP_squared);  // Pythagore
  }
  else
  {
    P = glm::normalize(P);  // nearest point
  }
  return P;
}


void
Viewer::update_arcball()
{
  if (m_arcball_active && m_mouse != m_last_mouse)
  {
    glm::mat4 camera_matrix;// = m_object2world;

    //glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(camera_matrix));

    glm::vec3 va = get_arcball_vector(m_last_mouse);
    glm::vec3 vb = get_arcball_vector(m_mouse);
    float angle = acos(std::min(1.0f, glm::dot(va, vb)));
    glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
    glm::mat3 camera2object = glm::inverse(glm::mat3(camera_matrix) * glm::mat3(m_last_object2world));
    glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
    m_object2world = glm::rotate(m_last_object2world, angle, axis_in_object_coord);
    //g_last_mouse = m_mouse;
  }
}

#endif

#endif

/* EOF */
