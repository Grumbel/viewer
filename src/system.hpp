#ifndef HEADER_SYSTEM_HPP
#define HEADER_SYSTEM_HPP

#include <iostream>

#include "joystick.hpp"
#include "window.hpp"

class System
{
public:
  static System create();

private:
public:
  System();
  System(System&&) = default;

  Window create_gl_window(std::string const& title, int width, int height, bool fullscreen, int anti_aliasing);
  Joystick create_joystick();

private:
  System(const System&) = delete;
  System& operator=(const System&) = delete;
};

#endif

/* EOF */
