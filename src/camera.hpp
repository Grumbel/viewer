#ifndef HEADER_CAMERA_HPP
#define HEADER_CAMERA_HPP

#include <glm/glm.hpp>
#include <glm/ext.hpp>

class Camera
{
private:
  float m_fov;
  float m_aspect_ratio;
  float m_znear;
  float m_zfar;

  glm::vec3 m_position;
  glm::quat m_orientation;

public:
  Camera(float fov, float aspect_ratio, float znear, float zfar) :
    m_fov(fov),
    m_aspect_ratio(aspect_ratio),
    m_znear(znear),
    m_zfar(zfar),
    m_position(),
    m_orientation(1.0f, 0.0f, 0.0f, 0.0f)
  {}

  ~Camera()
  {}

  void set_position(const glm::vec3& p) { m_position = p; }
  glm::vec3 get_position() const { return m_position; }

  void set_orientation(const glm::quat& q) { m_orientation = q; }
  glm::quat get_orientation() const { return m_orientation; }

  void projection(float fov, float aspect_ratio, float znear, float zfar)
  {
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
    return glm::perspective(m_fov, m_aspect_ratio, m_znear, m_zfar);
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

private:
  Camera(const Camera&);
  Camera& operator=(const Camera&);
};

#endif

/* EOF */
