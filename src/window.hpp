#ifndef HEADER_WINDOW_HPP
#define HEADER_WINDOW_HPP

#include <SDL.h>

class Window
{
private:
  SDL_Window* m_window;
  SDL_GLContext m_gl_context;

public:
  Window(SDL_Window* window, SDL_GLContext gl_context);
  Window(Window&&) = default;
  ~Window();

  void swap();

private:
  Window(const Window&) = delete;
  Window& operator=(const Window&) = delete;
};

#endif

/* EOF */
