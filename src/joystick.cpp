#include "joystick.hpp"

Joystick::Joystick(SDL_Joystick* joystick) :
  m_joystick(joystick)
{
}

Joystick::Joystick(Joystick&& other) :
  m_joystick(other.m_joystick)
{
  other.m_joystick = nullptr;
}

Joystick::~Joystick()
{
  if (m_joystick)
  {
    SDL_JoystickClose(m_joystick);
  }
}

/* EOF */
