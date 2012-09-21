#include "menu.hpp"

#include "menu_item.hpp"

Menu::Menu() :
  m_items(),
  m_current_item(0),
  m_cursor_surface()
{
  m_cursor_surface = TextSurface::create("> ", TextProperties());
}

void
Menu::draw(float x, float y)
{
  for(auto it = m_items.begin(); it != m_items.end(); ++it)
  {
    if (it - m_items.begin() == m_current_item)
    {
      m_cursor_surface->draw(x - m_cursor_surface->get_width(), y);
    }

    (*it)->draw(x, y);
    
    y += 24.0f;
  }
}

void
Menu::add_item(const std::string& label, float* value_ptr)
{
  m_items.emplace_back(new FloatMenuItem(label, value_ptr));
}

void
Menu::add_item(const std::string& label, bool* value_ptr)
{
  m_items.emplace_back(new BoolMenuItem(label, value_ptr));
}

void
Menu::up()
{
  m_current_item -= 1;
  if (m_current_item < 0)
  {
    m_current_item = m_items.size() - 1;
  }
}

void
Menu::down()
{
  m_current_item += 1;
  if (m_current_item >= static_cast<int>(m_items.size()))
  {
    m_current_item = 0;
  }
}

void
Menu::left()
{
  m_items[m_current_item]->left();
}

void
Menu::right()
{
  m_items[m_current_item]->right();
}

/* EOF */
