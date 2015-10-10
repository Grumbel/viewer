#include "scene_visitor.hpp"

#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>

#include "scene_manager.hpp"
#include "scene_node.hpp"

void
SceneVisitor::visit(SceneManager* manager)
{
  SceneNode* node = manager->get_world();
  visit(node, glm::mat4(1));
}

void
SceneVisitor::visit(SceneNode* node, const glm::mat4& matrix)
{
  glm::mat4 m = matrix;
  m = glm::translate(node->get_position()) * m;
  m = glm::mat4_cast(node->get_orientation()) * m;

  for(auto& model : node->get_models())
  {
    visit(model);
  }

  for(auto const& child : node->get_children())
  {
    visit(child.get(), m);
  }
}

void
SceneVisitor::visit(ModelPtr model)
{
  // model->visit(this);
}

/* EOF */
