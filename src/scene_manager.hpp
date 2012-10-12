#ifndef HEADER_SCENE_MANAGER_HPP
#define HEADER_SCENE_MANAGER_HPP

#include <vector>

#include "light.hpp"
#include "scene_node.hpp"
#include "opengl_state.hpp"

class SceneManager
{
private:
  SceneNode* m_root;
  std::vector<Light*> m_lights;

public:
  SceneManager() :
    m_root(new SceneNode),
    m_lights()
  {}

  SceneNode* get_root() const { return m_root; }

  Light* create_light() 
  {
    Light* light = new Light;
    
    m_lights.push_back(light);

    return light;
  }

  void render()
  {
    m_root->update_transform();
    render(m_root);
  }

  void render(SceneNode* node)
  {
    OpenGLState state;

    glPushMatrix();
    {
      glMultMatrixf(glm::value_ptr(node->get_transform()));
      for(auto& entity : node->get_entities())
      {
        std::cout << "Render entity: " << entity << std::endl;
        entity->draw();
      }
    }
    glPopMatrix();

    for(auto& child : node->get_children())
    {
      render(child);
    }   
  }

private:
  SceneManager(const SceneManager&);
  SceneManager& operator=(const SceneManager&);
};

#endif

/* EOF */
