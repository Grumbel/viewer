#ifndef HEADER_WINDOW_HPP
#define HEADER_WINDOW_HPP

#include <SDL.h>

#include "gl_context.hpp"

class Window
{
private:
  SDL_Window* m_window;
  GLContext m_gl_context;

public:
  Window(SDL_Window* window, SDL_GLContext gl_context);
  Window(Window&& other);
  ~Window();

  void swap();

private:
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
};

#endif

/* EOF */
