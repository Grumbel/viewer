#ifndef HEADER_LIGHT_HPP
#define HEADER_LIGHT_HPP

#include <glm/glm.hpp>
#include <memory>

class Light;

typedef std::shared_ptr<Light> LightPtr;

class Light
{
private:
  glm::vec4 m_diffuse;
  glm::vec4 m_ambient;
  glm::vec4 m_specular;
  float m_shininess;

public:
  Light() :
    m_diffuse(1.0f, 1.0f, 1.0f, 1.0f),
    m_ambient(0.0f, 0.0f, 0.0f, 1.0f),
    m_specular(0.0f, 0.0f, 0.0f, 1.0f),
    m_shininess(0)
  {}

  void set_diffuse(const glm::vec4& diffuse) 
  {
    m_diffuse = diffuse;
  }

  void set_ambient(const glm::vec4& ambient) 
  {
    m_ambient = ambient;
  }

  void set_specular(const glm::vec4& specular) 
  {
    m_specular = specular;
  }

  glm::vec4 get_diffuse() const
  {
    return m_diffuse;
  }

  glm::vec4 get_ambient() const
  {
    return m_ambient;
  }

  glm::vec4 get_specular() const
  {
    return m_specular;
  }

private:
  Light(const Light&);
  Light& operator=(const Light&);
};

#endif

/* EOF */
