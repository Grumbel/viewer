#include "opengl_state.hpp"

OpenGLState::OpenGLState()
{
  assert_gl("OpenGLState");
  glPushAttrib(GL_ALL_ATTRIB_BITS);
  glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  assert_gl("OpenGLState-exit");
}
  
OpenGLState::~OpenGLState()
{
  assert_gl("~OpenGLState");
  glPopClientAttrib();
  glPopAttrib();
  assert_gl("~OpenGLState-exit");
}

/* EOF */
