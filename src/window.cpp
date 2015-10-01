#include "window.hpp"

#include <utility>

Window::Window(SDL_Window* window, SDL_GLContext gl_context) :
  m_window(window),
  m_gl_context(gl_context)
{
}

Window::Window(Window&& other) :
  m_window(other.m_window),
  m_gl_context(std::move(other.m_gl_context))
{
  other.m_window = nullptr;
}

Window::~Window()
{
  if (m_window)
  {
    SDL_DestroyWindow(m_window);
  }
}

void
Window::swap()
{
  SDL_GL_SwapWindow(m_window);
}

/* EOF */
