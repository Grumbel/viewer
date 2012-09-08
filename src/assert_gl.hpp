#ifndef HEADER_ASSERT_GL_HPP
#define HEADER_ASSERT_GL_HPP

#define assert_gl(fmt) assert_gl_(__FILE__, __LINE__, fmt)

inline void assert_gl_(const char* file, int line, const char* message)
{
  GLenum error = glGetError();
  if(error != GL_NO_ERROR) 
  {
    std::ostringstream msg;
    msg << file << ':' << line << ": OpenGLError while '" << message << "': "
        << gluErrorString(error);
    throw std::runtime_error(msg.str());
  }
}

#endif

/* EOF */
