#ifndef HEADER_JOYSTICK_HPP
#define HEADER_JOYSTICK_HPP

#include <SDL.h>

class Joystick
{
private:
  SDL_Joystick* m_joystick;

public:
  Joystick(SDL_Joystick* joystick = nullptr);
  Joystick(Joystick&& other);
  ~Joystick();

  explicit operator bool() const { return m_joystick != nullptr; }

private:
  Joystick(const Joystick&) = delete;
  Joystick& operator=(const Joystick&) = delete;
};

#endif

/* EOF */
