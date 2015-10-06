#ifndef HEADER_COMPONENT_HPP
#define HEADER_COMPONENT_HPP

class Component
{
private:
public:
  Component();
  virtual ~Component();

private:
  Component(const Component&) = delete;
  Component& operator=(const Component&) = delete;
};



#endif

/* EOF */
