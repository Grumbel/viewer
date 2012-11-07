#ifndef HEADER_SAMPLER_HPP
#define HEADER_SAMPLER_HPP

class Sampler
{
private:
  GLuint m_sampler;

public:
  Sampler()
  {
    glGenSamplers(1, &m_sampler);
  }

  ~Sampler()
  {
    glDeleteSamplers(1, &m_sampler);
  }

  void bind(GLuint unit)
  {
    glBindSampler(unit, m_sampler);
  }

  void parameter(GLenum pname, GLint param)
  {
    glSamplerParameteri(m_sampler, pname, param);
  }

  void parameter(GLenum pname, GLfloat param)
  {
    glSamplerParameterf(m_sampler, pname, param);
  }

  void parameter(GLenum pname, GLint* param)
  {
    glSamplerParameteriv(m_sampler, pname, param);
  }

  void parameter(GLenum pname, GLfloat* param)
  {
    glSamplerParameterfv(m_sampler, pname, param);
  }

  /*
    void glSamplerParameter{if}v( uint sampler, enum pname, const T *param );
    void glSamplerParameterI{i ui}v( uint sampler, enum pname, const T *params );

    void glGetSamplerParameter{if}v( uint sampler, enum pname, T *params );
    void glGetSamplerParameterI{i ui}v( uint sampler, enum pname, T *params );
  */

private:
  Sampler(const Sampler&);
  Sampler& operator=(const Sampler&);
};

#endif

/* EOF */
