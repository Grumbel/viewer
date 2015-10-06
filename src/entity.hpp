#ifndef HEADER_ENTITY_HPP
#define HEADER_ENTITY_HPP

#include <vector>
#include <memory>

#include "component.hpp"

class Entity
{
private:
  std::vector<std::unique_ptr<Component> > m_components;

public:
  Entity();
  virtual ~Entity();

  void attach_component();
  void detach_component();

private:
  Entity(const Entity&) = delete;
  Entity& operator=(const Entity&) = delete;
};

#endif

/* EOF */
