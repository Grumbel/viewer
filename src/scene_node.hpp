//  Simple 3D Model Viewer
//  Copyright (C) 2012-2013 Ingo Ruhnke <grumbel@gmail.com>
//
//  This program is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//  GNU General Public License for more details.
//
//  You should have received a copy of the GNU General Public License
//  along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef HEADER_SCENE_NODE_HPP
#define HEADER_SCENE_NODE_HPP

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/ext.hpp>
#include <vector>

#include "model.hpp"

class SceneNode
{
private:
  std::string m_name;
  glm::vec3 m_position;
  glm::quat m_orientation;
  glm::vec3 m_scale;

  glm::mat4 m_global_transform;

  std::vector<std::unique_ptr<SceneNode> > m_children;
  std::vector<ModelPtr> m_entities;

public:
  SceneNode(const std::string& name = std::string());
  ~SceneNode(); 

  void set_position(const glm::vec3& p);
  glm::vec3 get_position() const;

  void set_orientation(const glm::quat& q);
  glm::quat get_orientation() const;

  void set_scale(const glm::vec3& s);
  glm::vec3 get_scale() const;

  glm::mat4 get_transform() const;

  void update_transform(const glm::mat4& parent_transform = glm::mat4(1));

  void attach_entity(ModelPtr entity);
  void attach_child(std::unique_ptr<SceneNode> child);
  SceneNode* create_child();

  const std::vector<std::unique_ptr<SceneNode> >& get_children() const { return m_children; }
  const std::vector<ModelPtr>&   get_entities() const { return m_entities; }

private:
  SceneNode(const SceneNode&);
  SceneNode& operator=(const SceneNode&);
};

#endif

/* EOF */
