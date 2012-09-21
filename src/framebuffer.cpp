#include "framebuffer.hpp"

#include "opengl_state.hpp"

Framebuffer::Framebuffer(int width, int height) :
  m_fbo(0),
  m_color_buffer(0),
  m_depth_buffer(0)
{
  log_info("Framebuffer(%d, %d)", width, height);
  OpenGLState state;

  // create the framebuffer
  assert_gl("framebuffer");
  glGenFramebuffers(1, &m_fbo);
  assert_gl("framebuffer");
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  assert_gl("framebuffer");

  // create color buffer texture
  glGenTextures(1, &m_color_buffer);
  glBindTexture(GL_TEXTURE_2D, m_color_buffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F,  width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  assert_gl("framebuffer");

  // create depth buffer texture
  glGenTextures(1, &m_depth_buffer);
  glBindTexture(GL_TEXTURE_2D, m_depth_buffer);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,  width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  assert_gl("framebuffer");
    
  // attach color and depth buffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_buffer, 0);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, m_depth_buffer, 0);
  assert_gl("framebuffer");

  GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (complete != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "Framebuffer incomplete: " << complete << std::endl;
  }
  assert_gl("framebuffer");

  std::cout << "FBO: " << m_fbo << std::endl;
  std::cout << "Depth Buffer: " << m_depth_buffer << std::endl;
  std::cout << "Color Buffer: " << m_color_buffer << std::endl;

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
  
Framebuffer::~Framebuffer()
{
  assert_gl("~Framebuffer-enter()");
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  log_info("~Framebuffer()");
  glDeleteFramebuffers(1, &m_fbo);
  glDeleteTextures(1, &m_depth_buffer);
  glDeleteTextures(1, &m_color_buffer);
  assert_gl("~Framebuffer()");
}

void
Framebuffer::draw(float x, float y, float w, float h, float z)
{
  OpenGLState state;
    
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, m_color_buffer);
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x, y, z);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x+w, y, z);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x+w, y+h, z); // FIXME

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, y+h, z);
  }
  glEnd();  
}

void
Framebuffer::draw_depth(float x, float y, float w, float h, float z)
{
  OpenGLState state;
    
  glEnable(GL_TEXTURE_2D);

  glBindTexture(GL_TEXTURE_2D, m_depth_buffer);
  glBegin(GL_QUADS);
  {
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x, y, z);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x+w, y, z);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x+w, y+h, z); // FIXME

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, y+h, z);
  }
  glEnd();  
}

void
Framebuffer::bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
}

void 
Framebuffer::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* EOF */
