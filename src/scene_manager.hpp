#ifndef HEADER_SCENE_MANAGER_HPP
#define HEADER_SCENE_MANAGER_HPP

#include <vector>

#include "light.hpp"
#include "scene_node.hpp"
#include "opengl_state.hpp"
#include "material.hpp"

class Camera;

class SceneManager
{
private:
  std::unique_ptr<SceneNode> m_world;
  std::unique_ptr<SceneNode> m_view;
  std::vector<LightPtr> m_lights;
  MaterialPtr m_override_material;

public:
  SceneManager();
  ~SceneManager();

  SceneNode* get_world() const;
  SceneNode* get_view() const;

  LightPtr create_light();

  void render(const Camera& camera, bool geometry_pass = false);
  void render_node(const Camera& camera, SceneNode* node, bool geometry_pass);

  void set_override_material(MaterialPtr material);

private:
  SceneManager(const SceneManager&);
  SceneManager& operator=(const SceneManager&);
};

#endif

/* EOF */
