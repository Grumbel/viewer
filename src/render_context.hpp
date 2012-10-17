#ifndef HEADER_RENDER_CONTEXT_HPP
#define HEADER_RENDER_CONTEXT_HPP

#include "camera.hpp"
#include "scene_node.hpp"

class RenderContext
{
private:
  Camera m_camera;
  SceneNode* m_node;

public:
  RenderContext(const Camera& camera,
                SceneNode* node) :
    m_camera(camera),
    m_node(node)
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

private:
  RenderContext(const RenderContext&);
  RenderContext& operator=(const RenderContext&);
};

#endif

/* EOF */
