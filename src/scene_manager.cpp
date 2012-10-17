#include "scene_manager.hpp"

#include "camera.hpp"
#include "render_context.hpp"

SceneManager::SceneManager() :
  m_world(new SceneNode),
  m_view(new SceneNode),
  m_lights()
{}

SceneManager::~SceneManager()
{
}

SceneNode*
SceneManager::get_world() const
{
  return m_world.get();
}

SceneNode* 
SceneManager::get_view() const
{
  return m_view.get();
}

LightPtr
SceneManager::create_light() 
{
  LightPtr light = std::make_shared<Light>();
  m_lights.push_back(light);
  return light;
}

void
SceneManager::render(const Camera& camera)
{
  m_world->update_transform();
  m_view->update_transform();

  render(camera, m_world.get());

  Camera id = camera;
  id.set_position(glm::vec3(0.0f, 0.0f, 0.0f));
  render(id, m_view.get());
}

void
SceneManager::render(const Camera& camera, SceneNode* node)
{
  OpenGLState state;

  RenderContext context(camera, node);
  for(auto& entity : node->get_entities())
  {
    entity->draw(context);
  }

  for(auto& child : node->get_children())
  {
    render(camera, child);
  }
}

/* EOF */
