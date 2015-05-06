#ifndef HEADER_WIIMOTE_MANAGER_HPP
#define HEADER_WIIMOTE_MANAGER_HPP

#include <wiicpp.h>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <thread>
#include <mutex>

class WiimoteManager
{
private:
  bool m_quit;
  CWii m_cwii;
  bool m_reload_wiimotes;

  glm::vec3 m_smoothed_gravity;

  float m_pitch;
  float m_yaw;
  float m_roll;

  glm::vec3 m_accumulated;
  float m_gyro_pitch;
  float m_gyro_yaw;
  float m_gyro_roll;
  glm::quat m_gyro_orientation;

  float m_accel_pitch;
  float m_accel_roll;
  glm::quat m_accel_orientation;

  glm::quat m_orientation;

  std::thread m_thread;
  mutable std::mutex m_mutex;

public:
  WiimoteManager();
  ~WiimoteManager();

  glm::quat get_gyro_orientation() const;
  glm::quat get_accel_orientation() const;
  glm::quat get_orientation() const;
  glm::quat get_accumulated() const;

  void reset_gyro_orientation(const glm::quat& value = glm::quat());

private:
  void update(float dt);
  void handle_event(CWiimote& wm);
  void dispatch_events();
};

#endif

/* EOF */
