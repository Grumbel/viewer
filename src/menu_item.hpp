#ifndef HEADER_MENU_ITEM_HPP
#define HEADER_MENU_ITEM_HPP

#include <string>
#include "text_surface.hpp"

class MenuItem
{
public:
  MenuItem(const std::string& label) :
    m_label(label),
    m_label_surface()
  {
    m_label_surface = TextSurface::create(m_label, TextProperties());
  }

  virtual ~MenuItem()
  {}

  virtual void draw(float x, float y)
  {
    m_label_surface->draw(x, y);
  }

  virtual void left()   = 0;
  virtual void right() = 0;

private:
  std::string m_label;
  TextSurfacePtr m_label_surface;

private:
  MenuItem(const MenuItem&) = delete;
  MenuItem& operator=(const MenuItem&) = delete;
};

class BoolMenuItem : public MenuItem
{
public:
  BoolMenuItem(const std::string& label, bool* value_ptr) : 
    MenuItem(label),
    m_value_ptr(value_ptr),
    m_true_surface(),
    m_false_surface()
  {
    m_true_surface  = TextSurface::create("[X]", TextProperties());
    m_false_surface = TextSurface::create("[ ]", TextProperties());
  }

  void left()   
  {
    *m_value_ptr = !*m_value_ptr;
  }

  void right() 
  { 
    left();
  }

  void draw(float x, float y)
  {
    MenuItem::draw(x, y);

    auto& surface = (*m_value_ptr) ? m_true_surface : m_false_surface;
    
    surface->draw(x+300.0f - surface->get_width(), y);
  }

private:
  bool* m_value_ptr;

  TextSurfacePtr m_true_surface;
  TextSurfacePtr m_false_surface;

private:
  BoolMenuItem(const BoolMenuItem&) = delete;
  BoolMenuItem& operator=(const BoolMenuItem&) = delete; 
};

class FloatMenuItem : public MenuItem
{
public:
  FloatMenuItem(const std::string& label, float* value_ptr) : 
    MenuItem(label),
    m_value_ptr(value_ptr),
    m_step(1.0f),
    m_min(),
    m_max(),
    m_value_surface(),
    m_old_value()
  {
    check_range();
  }

  void left()   
  {
    *m_value_ptr -= m_step; 
    check_range();
  }

  void right() 
  { 
    *m_value_ptr += m_step; 
    check_range();
  }

  void check_range()
  {
    if (m_min)
    {
      *m_value_ptr = std::max(*m_min, *m_value_ptr);
    }

    if (m_max)
    {
      *m_value_ptr = std::min(*m_max, *m_value_ptr);
    }
  }

  void draw(float x, float y)
  {
    MenuItem::draw(x, y);

    update_surface();

    m_value_surface->draw(x+300.0f - m_value_surface->get_width(), y);
  }

  void update_surface()
  {
    if (m_old_value != *m_value_ptr || !m_value_surface)
    {
      m_value_surface = TextSurface::create(std::to_string(*m_value_ptr), TextProperties());
      m_old_value = *m_value_ptr;
    }
  }

private:
  float* m_value_ptr;
  float m_step;
  boost::optional<float> m_min;
  boost::optional<float> m_max;

  TextSurfacePtr m_value_surface;
  float m_old_value;

private:
  FloatMenuItem(const FloatMenuItem&) = delete;
  FloatMenuItem& operator=(const FloatMenuItem&) = delete;
};

#endif

/* EOF */
