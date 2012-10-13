#include "scene_manager.hpp"

#include "camera.hpp"

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
  render(camera, m_view.get());
}

void
SceneManager::render(const Camera& camera, SceneNode* node)
{
  OpenGLState state;

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glMultMatrixf(glm::value_ptr(camera.get_matrix()));
    
  glPushMatrix();
  {
    glMultMatrixf(glm::value_ptr(node->get_transform()));

    for(auto& entity : node->get_entities())
    {
      entity->draw();
    }
  }
  glPopMatrix();

  for(auto& child : node->get_children())
  {
    render(camera, child);
  }
}

/* EOF */
