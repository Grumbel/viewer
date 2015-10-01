#include "window.hpp"

Window::Window(SDL_Window* window, SDL_GLContext gl_context) :
  m_window(window),
  m_gl_context(gl_context)
{
}

Window::~Window()
{
}

void
Window::swap()
{
  SDL_GL_SwapWindow(m_window);
}

/* EOF */
