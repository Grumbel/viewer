#include "wiimote_manager.hpp"

#include <chrono>
#include <stdexcept>

#include "log.hpp"

WiimoteManager::WiimoteManager() :
  m_quit(false),
  m_cwii(1),
  m_reload_wiimotes(false),
  m_smoothed_gravity(0.0f, 1.0f, 0.0f),
  m_pitch(),
  m_yaw(),
  m_roll(),
  m_accumulated(0.0f, 0.0f, 0.0f),
  m_gyro_pitch(0.0f),
  m_gyro_yaw(0.0f),
  m_gyro_roll(0.0f),
  m_gyro_orientation(1.0f, 0.0f, 0.0f, 0.0f),
  m_accel_pitch(0.0f),
  m_accel_roll(0.0f),
  m_accel_orientation(1.0f, 0.0f, 0.0f, 0.0f),
  m_orientation(),
  m_thread(),
  m_mutex()
{
  std::vector<CWiimote>& m_wiimotes = m_cwii.FindAndConnect();
  if (!m_wiimotes.size())
  {
    throw std::runtime_error("No wiimotes found.");
  }
  else
  {
    for(auto& wm : m_wiimotes)
    {
      wm.SetLEDs(CWiimote::LED_1);
      //wm.IR.SetMode(CIR::ON);
      wm.SetMotionSensingMode(CWiimote::ON);
      //wm.EnableMotionPlus(CWiimote::ON);
    }

    m_thread = std::thread([this] {
        using clock = std::chrono::high_resolution_clock;

        clock::time_point t0 = clock::now();
        clock::time_point t1 = clock::now();

        while(!m_quit)
        {
          t1 = clock::now();
          float dt = std::chrono::duration_cast<chrono::microseconds>(t1 - t0).count() / 1000000.0f;
          t0 = t1;
          update(dt);

          while(m_cwii.Poll())
          {
            dispatch_events();
          }
        }

        std::cout << ">>> Wiimote thread shutdown" << std::endl;
      });
  }
}

WiimoteManager::~WiimoteManager()
{
  m_quit = true;
  m_thread.join();
}

void
WiimoteManager::update(float dt)
{
  // dt average: 0.000574
  std::lock_guard<std::mutex> lock(m_mutex);

  //std::cout << "Time: " << dt << std::endl;
  //if (glm::abs(m_accel_pitch) < 70.0f)
  {
    glm::quat qpitch(glm::vec3(m_accel_pitch, 0.0f, 0.0f));
    glm::quat qroll(glm::vec3(0.0f, 0.0f, m_accel_roll));
    glm::quat qaccel = /*qyaw */ qpitch * qroll;

    glm::quat yaw(glm::vec3(0.0f, m_gyro_yaw, 0.0f));
    glm::quat qgyro = glm::inverse(yaw) * m_gyro_orientation;

    m_gyro_orientation = yaw * glm::normalize(glm::lerp(qgyro, qaccel, std::min(1.0f, 2.0f * dt)));

    //glm::quat yaw(glm::vec3(0.0f, m_gyro_yaw, 0.0f));
    //m_gyro_orientation = glm::inverse(yaw) * m_gyro_orientation;
    //m_gyro_orientation = glm::lerp(m_gyro_orientation, m_orientation, std::min(1.0f, 1.0f * dt));
    //m_gyro_orientation = glm::normalize(m_gyro_orientation);
    //m_gyro_orientation = yaw * m_gyro_orientation;
  }
}

void
WiimoteManager::dispatch_events()
{
  std::vector<CWiimote>& m_wiimotes = m_cwii.GetWiimotes(m_reload_wiimotes);
  m_reload_wiimotes = false;

  for(auto& wm : m_wiimotes)
  {
    // Use a reference to make working with the iterator handy.
    switch(wm.GetEvent())
    {
      case CWiimote::EVENT_EVENT:
        handle_event(wm);
        break;

      case CWiimote::EVENT_STATUS:
        //HandleStatus(wm);
        break;

      case CWiimote::EVENT_DISCONNECT:
      case CWiimote::EVENT_UNEXPECTED_DISCONNECT:
        //HandleDisconnect(wm);
        m_reload_wiimotes = true;
        break;

      case CWiimote::EVENT_READ_DATA:
        //HandleReadData(wm);
        break;

      case CWiimote::EVENT_NUNCHUK_INSERTED:
        //HandleNunchukInserted(wm);
        m_reload_wiimotes = true;
        break;

      case CWiimote::EVENT_CLASSIC_CTRL_INSERTED:
        //HandleClassicInserted(wm);
        m_reload_wiimotes = true;
        break;

      case CWiimote::EVENT_GUITAR_HERO_3_CTRL_INSERTED:
        //HandleGH3Inserted(wm);
        m_reload_wiimotes = true;
        break;

      case CWiimote::EVENT_MOTION_PLUS_INSERTED:
        //wiimote.EnableMotionPlus(CWiimote::ON);
        cout << "Motion Plus inserted." << endl;
        break;

      case CWiimote::EVENT_BALANCE_BOARD_INSERTED:
        cout << "Balance Board connected.\n"  << endl;
        break;

      case CWiimote::EVENT_BALANCE_BOARD_REMOVED:
        cout << "Balance Board disconnected.\n"  << endl;
        break;

      case CWiimote::EVENT_NUNCHUK_REMOVED:
      case CWiimote::EVENT_CLASSIC_CTRL_REMOVED:
      case CWiimote::EVENT_GUITAR_HERO_3_CTRL_REMOVED:
      case CWiimote::EVENT_MOTION_PLUS_REMOVED:
        cout << "An expansion was removed." << endl;
        //HandleStatus(wm);
        m_reload_wiimotes = true;
        break;

      default:
        break;
    }
  }
}

void
WiimoteManager::handle_event(CWiimote& wm)
{
  std::lock_guard<std::mutex> lock(m_mutex);

  char prefixString[64];
  sprintf(prefixString, "Controller [%i]: ", wm.GetID());

  if(wm.Buttons.isJustPressed(CButtons::BUTTON_UP))
  {
    //g_orientation = glm::vec3();
  }

  if(wm.Buttons.isJustPressed(CButtons::BUTTON_RIGHT))
  {
    log_info("activating gyro");
    wm.EnableMotionPlus(CWiimote::ON);
    wm.ExpansionDevice.MotionPlus.Gyroscope.SetGyroThreshold(1);
  }

  if(wm.Buttons.isJustPressed(CButtons::BUTTON_LEFT))
  {
    wm.EnableMotionPlus(CWiimote::OFF);
  }

  if(wm.Buttons.isJustPressed(CButtons::BUTTON_A))
  {
    m_accumulated = glm::vec3(0.0f, 0.0f, 0.0f);
    m_gyro_orientation = glm::quat();
    m_gyro_pitch = 0.0f;
    m_gyro_yaw   = 0.0f;
    m_gyro_roll  = 0.0f;
  }

  if(wm.isUsingACC())
  {
    // We are not using WiiCs orientation calculation as it doesn't
    // work properly when roll and ptich are in effect at the same
    // time
    glm::vec3 gravity;
    wm.Accelerometer.GetGravityVector(gravity.x, gravity.y, gravity.z);

    // convert WiiC gravity vector into OpenGL coordinate system
    std::swap(gravity.y, gravity.z);
    gravity.z = -gravity.z;

    float alpha = 0.05f;
    m_smoothed_gravity = alpha * gravity + (1.0f - alpha) * m_smoothed_gravity;
    gravity = m_smoothed_gravity;

    // calculate the roll
    float roll  = atan2f(gravity.y, gravity.x) - glm::half_pi<float>();
    glm::quat roll_rot = glm::quat(glm::vec3(0.0f, 0.0f, roll));

    // remove the roll from the gravity vector, so that pitch can be calculated properly
    glm::quat rot_undo = glm::normalize(glm::quat(glm::vec3(0.0f, 0.0f, -roll)));
    gravity = rot_undo * gravity;

    // calculate pitch
    float pitch = atan2f(gravity.z, gravity.y);
    glm::quat pitch_rot = glm::quat(glm::vec3(pitch, 0.0f, 0.0f));

    m_accel_orientation = pitch_rot * roll_rot;

    m_accel_roll  = roll;
    m_accel_pitch = pitch;

    m_yaw   = glm::yaw(m_gyro_orientation);
    m_pitch = glm::pitch(m_gyro_orientation);
    m_roll  = glm::roll(m_gyro_orientation);

    //m_accel_orientation = glm::quat(glm::vec3(0.0f, glm::radians(glm::yaw(m_gyro_orientation)), 0.0f)) * m_accel_orientation;

    //std::cout << m_yaw << std::endl;
    //printf("%8.2f %8.2f   %8.2f %8.2f %8.2f\n", glm::degrees(pitch), glm::degrees(roll), gravity.x, gravity.y, gravity.z);
  }

  // if the Motion Plus is turned on then print angles
  if(wm.isUsingMotionPlus())
  {
    float pitch, yaw, roll;
    wm.ExpansionDevice.MotionPlus.Gyroscope.GetRates(roll, pitch, yaw);

    // convert into radians
    roll  = glm::radians(roll  * 0.01f);
    pitch = glm::radians(pitch * -0.01f);
    yaw   = glm::radians(yaw   * 0.01f);

    //glm::vec3 rot_x = m_gyro_orientation * glm::vec3(1.0f, 0.0f, 0.0f);
    glm::vec3 rot = m_gyro_orientation * glm::vec3(0.0f, 0.0f, 1.0f);
    //glm::vec3 rot_z = m_gyro_orientation * glm::vec3(0.0f, 0.0f, 1.0f);

    m_gyro_pitch = m_accel_pitch;
    m_gyro_yaw   = atan2(rot.x, rot.z);
    m_gyro_roll  = m_accel_roll;

    //glm::quat pitch(glm::vec3(m_gyro_pitch, 0.0f, 0.0f));
    //glm::quat yaw(glm::vec3(0.0f, m_gyro_yaw, 0.0f));
    //glm::quat roll(glm::vec3(0.0f, 0.0f, m_gyro_roll));
    //return yaw * pitch * roll;

    m_gyro_orientation = m_gyro_orientation * glm::quat(glm::vec3(pitch, yaw, roll));

    //glm::quat qpitch(glm::vec3(m_accel_pitch, 0.0f, 0.0f));
    //glm::quat qyaw(glm::vec3(0.0f, m_gyro_yaw, 0.0f));
    //glm::quat qroll(glm::vec3(0.0f, 0.0f, m_accel_roll));
    //m_orientation = /*qyaw */ qpitch * qroll;

    //glm::vec3 angles = glm::eulerAngles(g_wiimote.get_orientation());
    //glm::vec3 accum = g_wiimote.get_accumulated();
    //printf("wiimote: %8.2f %8.2f %8.2f  --  ", glm::degrees(angles.x), glm::degrees(angles.y), glm::degrees(angles.z));
    //printf("%8.2f %8.2f %8.2f\n", glm::degrees(accum.x), glm::degrees(accum.y), glm::degrees(accum.z));
  }

  if(wm.isUsingIR())
  {
    std::vector<CIRDot>::iterator i;
    int x, y;
    int index;

    printf("%s Num IR Dots: %i\n", prefixString, wm.IR.GetNumDots());
    printf("%s IR State: %u\n", prefixString, wm.IR.GetState());

    std::vector<CIRDot>& dots = wm.IR.GetDots();

    for(index = 0, i = dots.begin(); i != dots.end(); ++index, ++i)
    {
      if((*i).isVisible())
      {
        (*i).GetCoordinate(x, y);
        printf("%s IR source %i: (%i, %i)\n", prefixString, index, x, y);

        wm.IR.GetCursorPosition(x, y);
        printf("%s IR cursor: (%i, %i)\n", prefixString, x, y);
        printf("%s IR z distance: %f\n", prefixString, wm.IR.GetDistance());
      }
    }
  }

  if (false)
  {
    printf("a.p: %8.2f a.r: %8.2f -- g.p: %8.2f g.y: %8.2f g.r: %8.2f\n",
           glm::degrees(m_accel_pitch), glm::degrees(m_accel_roll),
           glm::degrees(m_gyro_pitch), glm::degrees(m_gyro_yaw), glm::degrees(m_gyro_roll));
  }
}

glm::quat
WiimoteManager::get_gyro_orientation() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_gyro_orientation;
}

glm::quat
WiimoteManager::get_accumulated() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return glm::quat(m_accumulated);
}

glm::quat
WiimoteManager::get_accel_orientation() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  return m_accel_orientation;
}

void
WiimoteManager::reset_gyro_orientation(const glm::quat& value)
{
  std::lock_guard<std::mutex> lock(m_mutex);
  m_gyro_orientation = value;
}

glm::quat
WiimoteManager::get_orientation() const
{
  std::lock_guard<std::mutex> lock(m_mutex);
  //glm::quat pitch_q(glm::vec3(m_pitch, 0.0f, 0.0f));
  //glm::quat yaw_q(glm::vec3(0.0f, m_yaw, 0.0f)); // only gets half the yaw?!
  //glm::quat roll_q(glm::vec3(0.0f, 0.0f, m_roll));

  //glm::quat pitch(glm::vec3(m_gyro_pitch, 0.0f, 0.0f));
  //glm::quat yaw(glm::vec3(0.0f, m_gyro_yaw, 0.0f));
  //glm::quat roll(glm::vec3(0.0f, 0.0f, m_gyro_roll));
  //return yaw * pitch * roll;

  //glm::vec3 o = glm::eulerAngles(m_gyro_orientation);
  //return glm::quat(o);

  return m_orientation;
}

/* EOF */
