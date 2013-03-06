#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <unistd.h>
#include <wiicpp.h>
#include <chrono>
#include <stdexcept>

#include "wiimote_manager.hpp"

class Wiimote
{
public:
  glm::quat m_orientation;
  glm::vec3 m_accumulated;

  Wiimote() :
    m_orientation(1, 0, 0, 0),
    m_accumulated()
  {
  }

  glm::quat get_orientation() const
  {
    return m_orientation;
  }

  glm::vec3 get_accumulated() const
  {
    return m_accumulated;
  }

  void update_accel(const glm::quat& orientation)
  {
    m_orientation = glm::quat(1, 0, 0, 0);
    m_accumulated = glm::vec3(0, 0, 0);
      //glm::vec3(glm::pitch(orientation), 
      //                                glm::yaw(m_orientation),
      //                                glm::roll(orientation)));
  }

  void update_gyro(const glm::quat& rate)
  {
    // get rates in world space
    //glm::quat ws_rate = m_orientation * rate;

    m_accumulated.x += glm::pitch(rate);
    m_accumulated.y += glm::yaw(rate);
    m_accumulated.z += glm::roll(rate);

    //glm::vec3 angle_rate = glm::eulerAngles(rate);
    
    m_orientation = m_orientation * rate;
  }
};

//float pitch, roll, yaw;
glm::vec3 g_acc_orientation;

Wiimote g_wiimote;

std::chrono::high_resolution_clock::time_point g_last_time;
int g_event_count = 0;
int g_duration_count = 0;


int main(int argc, char** argv)
{
  WiimoteManager wii;
  while(true)
  {
    //    wii.update();
  }
  return 0;	
}

/* EOF */
