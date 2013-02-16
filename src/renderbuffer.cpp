//  Simple 3D Model Viewer
//  Copyright (C) 2013 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "renderbuffer.hpp"

#include "opengl_state.hpp"

#include "framebuffer.hpp"

Renderbuffer::Renderbuffer(int width, int height) :
  m_width(width),
  m_height(height),
  m_multisample(8),
  m_fbo(0),
  m_depth_buffer(0),
  m_color_buffer(0)
{
  log_info("Renderbuffer(%d, %d)", width, height);
  OpenGLState state;

  // create the framebuffer
  assert_gl("renderbuffer");
  glGenFramebuffers(1, &m_fbo);
  assert_gl("renderbuffer");
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  assert_gl("renderbuffer");

  glGenRenderbuffers(1, &m_color_buffer);
  glGenRenderbuffers(1, &m_depth_buffer);
  assert_gl("renderbuffer");

  glBindRenderbuffer(GL_RENDERBUFFER, m_color_buffer);
  if (!m_multisample)
  {
    glRenderbufferStorage(GL_RENDERBUFFER, GL_RGB16F, m_width, m_height);
  }
  else
  {
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_multisample, GL_RGB16F, m_width, m_height);
  }
  assert_gl("glRenderbufferStorageMultisample");

  glBindRenderbuffer(GL_RENDERBUFFER, m_depth_buffer);
  if (!m_multisample)
  {
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, m_width, m_height);
  }
  else
  {
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, m_multisample, GL_DEPTH_COMPONENT, m_width, m_height);
  }
  assert_gl("glRenderbufferStorageMultisample2");

  glBindRenderbuffer(GL_RENDERBUFFER, 0);

  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, m_color_buffer);
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_RENDERBUFFER, m_depth_buffer);

  GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  if (complete != GL_FRAMEBUFFER_COMPLETE)
  {
    std::cout << "Framebuffer incomplete: " << complete << std::endl;
  }
  assert_gl("framebuffer");

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

Renderbuffer::~Renderbuffer()
{
  glDeleteFramebuffers(1, &m_fbo);

  glDeleteRenderbuffers(1, &m_depth_buffer);
  glDeleteRenderbuffers(1, &m_color_buffer);
}

void
Renderbuffer::blit(Framebuffer& target_fbo,
                   int srcX0, int srcY0, int srcX1, int srcY1,
                   int dstX0, int dstY0, int dstX1, int dstY1,
                   GLbitfield mask, GLenum filter)
{
  // mask
  // The bitwise OR of the flags indicating which buffers are to be
  // copied. The allowed flags are GL_COLOR_BUFFER_BIT,
  // GL_DEPTH_BUFFER_BIT and GL_STENCIL_BUFFER_BIT.

  // filter
  // Specifies the interpolation to be applied if the image is
  // stretched. Must be GL_NEAREST or GL_LINEAR.

  // http://www.opengl.org/registry/specs/EXT/framebuffer_blit.txt  
  // http://www.opengl.org/wiki/GLAPI/glBlitFramebuffer

  assert_gl("enter: BlitFramebuffer");
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, target_fbo.get_id());
  assert_gl("enter: BlitFramebuffer1");
  glBindFramebuffer(GL_READ_FRAMEBUFFER, m_fbo);
  glReadBuffer(GL_COLOR_ATTACHMENT0);
  assert_gl("enter: BlitFramebuffer2");
  glBlitFramebuffer(srcX0, srcY0, srcX1, srcY1,
                    dstX0, dstY0, dstX1, dstY1,
                    mask, filter);
  assert_gl("done: BlitFramebuffer");

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
}
                                               
void
Renderbuffer::blit(Framebuffer& target_fbo)
{
  blit(target_fbo, 
       0, 0, m_width, m_height,
       0, 0, target_fbo.get_width(), target_fbo.get_height());
}

void
Renderbuffer::bind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_fbo); 
}

void
Renderbuffer::unbind()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/* EOF */
