#ifndef HEADER_GL_CONTEXT_HPP
#define HEADER_GL_CONTEXT_HPP

#include <SDL.h>

class GLContext
{
private:
  SDL_GLContext m_context;

public:
  GLContext(SDL_GLContext context);
  GLContext(GLContext&& other);
  ~GLContext();

private:
  GLContext(const GLContext&) = delete;
  GLContext& operator=(const GLContext&) = delete;
};

#endif

/* EOF */
