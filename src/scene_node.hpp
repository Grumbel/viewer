#ifndef HEADER_SCENE_NODE_HPP
#define HEADER_SCENE_NODE_HPP

#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>

#include "model.hpp"

class SceneNode
{
private:
  glm::vec3 m_position;
  glm::quat m_orientation;
  glm::vec3 m_scale;

  glm::mat4 m_global_transform;

  std::vector<SceneNode*> m_children;
  std::vector<ModelPtr> m_entities;

public:
  SceneNode() :
    m_position(0.0f, 0.0f, 0.0f),
    m_orientation(1.0f, 0.0f, 0.0f, 0.0f),
    m_scale(1.0f , 1.0f, 1.0f),
    m_global_transform(1),
    m_children(),
    m_entities()
  {}

  void set_position(const glm::vec3& p) { m_position = p; }
  glm::vec3 get_position() const { return m_position; }

  void set_orientation(const glm::quat& q) { m_orientation = q; }
  glm::quat get_orientation() const { return m_orientation; }

  void set_scale(const glm::vec3& s) { m_scale = s; }
  glm::vec3 get_scale() const { return m_scale; }

  glm::mat4 get_transform() const { return m_global_transform; }

  void update_transform(const glm::mat4& parent_transform = glm::mat4(1))
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

  void attach_entity(ModelPtr entity)
  {
    m_entities.push_back(entity);
  }

  void attach_child(SceneNode* child)
  {
    m_children.push_back(child);
  }

  SceneNode* create_child() 
  {
    SceneNode* child = new SceneNode; 
    attach_child(child);
    return child;
  }

  const std::vector<SceneNode*>& get_children() const { return m_children; }
  const std::vector<ModelPtr>&   get_entities() const { return m_entities; }
 

private:
  SceneNode(const SceneNode&);
  SceneNode& operator=(const SceneNode&);
};

#endif

/* EOF */
