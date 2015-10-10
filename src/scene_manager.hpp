#ifndef HEADER_SCENE_MANAGER_HPP
#define HEADER_SCENE_MANAGER_HPP

#include <vector>

#include "light.hpp"
#include "scene_node.hpp"
#include "opengl_state.hpp"
#include "material.hpp"
#include "stereo.hpp"

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

  void render(Camera const& camera, bool geometry_pass = false, Stereo stereo = Stereo::Center);
  void render_node(Camera const& camera, SceneNode* node, bool geometry_pass, Stereo stereo);

  void set_override_material(MaterialPtr material);

private:
  SceneManager(const SceneManager&);
  SceneManager& operator=(const SceneManager&);
};

#endif

/* EOF */
