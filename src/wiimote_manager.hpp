#ifndef HEADER_WIIMOTE_MANAGER_HPP
#define HEADER_WIIMOTE_MANAGER_HPP

#include <wiicpp.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

class WiimoteManager
{
private:
  CWii m_cwii;
  bool m_reload_wiimotes;

  glm::quat m_orientation;
  glm::vec3 m_accumulated;

public:
  WiimoteManager();

  void update();
  void handle_event(CWiimote& wm);

  glm::quat get_orientation() const;
  glm::quat get_accumulated() const;
};

#endif

/* EOF */
