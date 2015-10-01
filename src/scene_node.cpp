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

#include "scene_node.hpp"

SceneNode::SceneNode(const std::string& name) :
  m_name(name),
  m_position(0.0f, 0.0f, 0.0f),
  m_orientation(1.0f, 0.0f, 0.0f, 0.0f),
  m_scale(1.0f , 1.0f, 1.0f),
  m_global_transform(1),
  m_children(),
  m_models()
{
}

SceneNode::~SceneNode()
{
}

void
SceneNode::set_position(const glm::vec3& p)
{
  m_position = p;
}

glm::vec3
SceneNode::get_position() const
{
 return m_position;
}

void
SceneNode::set_orientation(const glm::quat& q)
{
 m_orientation = q;
}

glm::quat
SceneNode::get_orientation() const
{
 return m_orientation;
}

void
SceneNode::set_scale(const glm::vec3& s)
{
 m_scale = s;
}

glm::vec3
SceneNode::get_scale() const
{
 return m_scale;
}

glm::mat4
SceneNode::get_transform() const
{
 return m_global_transform;
}

void
SceneNode::update_transform(const glm::mat4& parent_transform)
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

void
SceneNode::attach_model(ModelPtr model)
{
  m_models.push_back(model);
}

void
SceneNode::attach_child(std::unique_ptr<SceneNode> child)
{
  m_children.push_back(std::move(child));
}

SceneNode*
SceneNode::create_child()
{
  std::unique_ptr<SceneNode> child = std::make_unique<SceneNode>();
  SceneNode* ptr = child.get();
  attach_child(std::move(child));
  return ptr;
}

/* EOF */
