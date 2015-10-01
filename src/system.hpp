#ifndef HEADER_SYSTEM_HPP
#define HEADER_SYSTEM_HPP

#include <iostream>

#include "game_controller.hpp"
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
  GameController create_gamecontroller();

private:
  System(const System&) = delete;
  System& operator=(const System&) = delete;
};

#endif

/* EOF */
