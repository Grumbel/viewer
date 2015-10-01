#include "gl_context.hpp"

GLContext::GLContext(SDL_GLContext context) :
  m_context(context)
{
}

GLContext::GLContext(GLContext&& other) :
  m_context(other.m_context)
{
  other.m_context = 0;
}

GLContext::~GLContext()
{
  SDL_GL_DeleteContext(m_context);
}

/* EOF */
