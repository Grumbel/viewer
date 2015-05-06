#ifndef HEADER_RENDER_CONTEXT_HPP
#define HEADER_RENDER_CONTEXT_HPP

#include "camera.hpp"
#include "material.hpp"
#include "scene_node.hpp"
#include "stereo.hpp"

class RenderContext
{
private:
  Camera m_camera;
  SceneNode* m_node;
  bool m_geometry_pass;
  MaterialPtr m_override_material;
  Stereo m_stero;

public:
  RenderContext(const Camera& camera,
                SceneNode* node) :
    m_camera(camera),
    m_node(node),
    m_geometry_pass(false),
    m_override_material(),
    m_stero(Stereo::Center)
  {
  }

  glm::mat4 get_view_matrix() const
  {
    return m_camera.get_view_matrix();
  }

  glm::mat4 get_model_matrix() const
  {
    return m_node->get_transform();
  }

  glm::mat4 get_projection_matrix() const
  {
    return m_camera.get_projection_matrix();
  }

  void set_geometry_pass()
  {
    m_geometry_pass = true;
  }

  bool is_geometry_pass() const
  {
    return m_geometry_pass;
  }

  Stereo get_stereo() const
  {
    return m_stero;
  }

  void set_stereo(Stereo stereo)
  {
    m_stero = stereo;
  }

  void set_override_material(MaterialPtr material)
  {
    m_override_material = material;
  }

  MaterialPtr get_override_material() const
  {
    return m_override_material;
  }

private:
  RenderContext(const RenderContext&);
  RenderContext& operator=(const RenderContext&);
};

#endif

/* EOF */
