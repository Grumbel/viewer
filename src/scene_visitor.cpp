#include "scene_visitor.hpp"

#include <glm/ext.hpp>

#include "scene_manager.hpp"
#include "scene_node.hpp"

void
SceneVisitor::visit(SceneManager* manager)
{
  SceneNode* node = manager->get_root();
  visit(node, glm::mat4(1));
}

void
SceneVisitor::visit(SceneNode* node, const glm::mat4& matrix)
{
  glm::mat4 m = matrix;
  m = glm::translate(node->get_position()) * m;
  m = glm::mat4_cast(node->get_orientation()) * m;

  for(auto& entity : node->get_entities())
  {
    visit(entity);
  }

  for(auto& child : node->get_children())
  {
    visit(child, m);
  }
}

void
SceneVisitor::visit(ModelPtr entity)
{
  // entity->visit(this);
}

/* EOF */
