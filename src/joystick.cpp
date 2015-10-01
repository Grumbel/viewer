#include "joystick.hpp"

Joystick::Joystick(SDL_Joystick* joystick) :
  m_joystick(joystick)
{
}

Joystick::~Joystick()
{
  if (m_joystick)
  {
    SDL_JoystickClose(m_joystick);
  }
}

/* EOF */
