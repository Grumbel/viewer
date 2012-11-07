#ifndef HEADER_CAMERA_HPP
#define HEADER_CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera
{
private:
  enum Type { kOrtho, kPerspective }; 
  Type m_type;

  // perspective
  float m_fov;
  float m_aspect_ratio;

  // ortho
  float m_left;
  float m_right;
  float m_top;
  float m_bottom;

  // shared
  float m_znear;
  float m_zfar;

  glm::vec3 m_position;
  glm::quat m_orientation;

public:
  Camera() :
    m_type(kPerspective),
    m_fov(90.0f),
    m_aspect_ratio(1.0f),
    m_left(),
    m_right(),
    m_top(),
    m_bottom(),
    m_znear(0.1f),
    m_zfar(1000.0f),
    m_position(),
    m_orientation(1.0f, 0.0f, 0.0f, 0.0f)
  {}

  ~Camera()
  {}

  void set_position(const glm::vec3& p) { m_position = p; }
  glm::vec3 get_position() const { return m_position; }

  void set_orientation(const glm::quat& q) { m_orientation = q; }
  glm::quat get_orientation() const { return m_orientation; }

  void ortho(float left, float right, float bottom, float top, float znear, float zfar)
  {
    m_type = kOrtho;

    m_left = left;
    m_right = right;
    m_bottom = bottom;
    m_top = top;

    m_znear = znear;
    m_zfar = zfar;
  }

  void perspective(float fov, float aspect_ratio, float znear, float zfar)
  {
    m_type = kPerspective;

    m_fov = fov;
    m_aspect_ratio = aspect_ratio;

    m_znear = znear;
    m_zfar = zfar;
  }

  void look_at(const glm::vec3& eye, 
               const glm::vec3& obj, 
               const glm::vec3& up)
  {
    glm::mat4 m = glm::lookAt(eye, obj, up);
    m_orientation = glm::quat(m);
    m_position = eye;
  }

  glm::mat4 get_projection_matrix() const
  {
    if (m_type == kOrtho)
    {
      return glm::ortho(m_left, m_right, m_bottom, m_top, m_znear, m_zfar);
    }
    else
    {
      return glm::perspective(m_fov, m_aspect_ratio, m_znear, m_zfar);
    }
  }

  glm::mat4 get_view_matrix() const
  {
    return 
      glm::mat4_cast(m_orientation) * 
      glm::translate(-m_position);
  }

  glm::mat4 get_matrix() const
  {
    return get_projection_matrix() * get_view_matrix();
  }
};

#endif

/* EOF */
