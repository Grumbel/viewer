#ifndef HEADER_MATERIAL_HPP
#define HEADER_MATERIAL_HPP

#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

#include "program.hpp"
#include "texture.hpp"
#include "uniform_group.hpp"

class Material
{
private:
  glm::vec4 m_diffuse;
  glm::vec4 m_ambient;
  glm::vec4 m_specular;
  glm::vec4 m_emission;
  float m_shininess;

  ProgramPtr m_program;
  std::unordered_map<int, TexturePtr> m_textures;
  UniformGroupPtr m_uniforms;

  std::unordered_map<GLenum, bool> m_capabilities;

  bool m_depth_mask;

  GLenum m_blend_sfactor;
  GLenum m_blend_dfactor;

public:
  Material();

  glm::vec4 get_diffuse() const { return m_diffuse; }
  glm::vec4 get_ambient() const { return m_ambient; }
  glm::vec4 get_specular() const { return m_specular; }
  glm::vec4 get_emission() const { return m_emission; }
  float get_shininess() const { return m_shininess; }

  void set_diffuse(const glm::vec4& c)  { m_diffuse = c; }
  void set_ambient(const glm::vec4& c) { m_ambient = c; }
  void set_specular(const glm::vec4& c) { m_specular = c; }
  void set_emission(const glm::vec4& c) { m_emission = c; }
  void set_shininess(float s) { m_shininess = s; }

  void set_program(ProgramPtr program) { m_program = program; }
  void set_texture(int unit, TexturePtr texture) { m_textures[unit] = texture; }

  void depth_mask(bool flag);
  void blend_func(GLenum sfactor, GLenum dfactor);

  void enable(GLenum cap);
  void disable(GLenum cap);

  template<typename T>
  void set_uniform(const std::string& name, const T& value)
  {
    m_uniforms->set_uniform(name, value);
  }
  
  void apply();

private:
  Material(const Material&);
  Material& operator=(const Material&);
};

typedef std::shared_ptr<Material> MaterialPtr;

#endif

/* EOF */
