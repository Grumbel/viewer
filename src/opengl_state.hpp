#ifndef HEADER_OPENGL_STATE_HPP
#define HEADER_OPENGL_STATE_HPP

#include "assert_gl.hpp"

class OpenGLState
{
public:
  OpenGLState()
  {
    assert_gl("OpenGLState");
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
    assert_gl("OpenGLState-exit");
  }

  ~OpenGLState()
  {
    assert_gl("~OpenGLState");
    glPopClientAttrib();
    glPopAttrib();
    assert_gl("~OpenGLState-exit");
  }

private:
  OpenGLState(const OpenGLState&) = delete;
  OpenGLState& operator=(const OpenGLState&) = delete;
};

#endif

/* EOF */
