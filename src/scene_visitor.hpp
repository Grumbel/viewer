#ifndef HEADER_SCENE_VISITOR_HPP
#define HEADER_SCENE_VISITOR_HPP

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "model.hpp"

//class Entity;
class SceneManager;
class SceneNode;

class SceneVisitor
{
private:
public:
  SceneVisitor();
  ~SceneVisitor();

  void visit(SceneManager* manager);
  void visit(SceneNode* node, const glm::mat4& matrix);
  void visit(ModelPtr);

private:
  SceneVisitor(const SceneVisitor&);
  SceneVisitor& operator=(const SceneVisitor&);
};

#endif

/* EOF */
