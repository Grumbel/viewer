#include "scene_node.hpp"

void
SceneNode::update_transform(const glm::mat4& parent_transform)
{
  m_global_transform = 
    parent_transform *
    glm::translate(m_position) *
    glm::mat4_cast(m_orientation) *
    glm::scale(m_scale);
    
  for(auto& child : m_children)
  {
    child->update_transform(m_global_transform);
  }
}

/* EOF */
