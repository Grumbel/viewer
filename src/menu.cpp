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

#include "menu.hpp"

#include "menu_item.hpp"

Menu::Menu(const TextProperties& text_prop) :
  m_text_prop(text_prop),
  m_items(),
  m_current_item(0),
  m_cursor_surface()
{
  m_cursor_surface = TextSurface::create("> ", m_text_prop);
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
    
    y += m_text_prop.get_font_size() + 2.0f;
  }
}

void
Menu::add_item(const std::string& label, float* value_ptr, float step,
               boost::optional<float> min,
               boost::optional<float> max)
{
  m_items.emplace_back(new FloatMenuItem(label, m_text_prop, value_ptr, step, min, max));
}

void
Menu::add_item(const std::string& label, int* value_ptr, int step,
               boost::optional<int> min,
               boost::optional<int> max)
{
  m_items.emplace_back(new ValueMenuItem<int>(label, m_text_prop, value_ptr, step, min, max));
}

void
Menu::add_item(const std::string& label, bool* value_ptr)
{
  m_items.emplace_back(new BoolMenuItem(label, m_text_prop, value_ptr));
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
