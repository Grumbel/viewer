#ifndef HEADER_MENU_HPP
#define HEADER_MENU_HPP

#include <vector>
#include <boost/optional.hpp>
#include <memory>

#include "menu_item.hpp"

class Menu
{
private:
public:
  Menu();

  void draw(float x, float y);
  void add_item(const std::string& label, float* value_ptr);
  void add_item(const std::string& label, bool* value_ptr);

  void up();
  void down();
  void left();
  void right();

private:
  std::vector<std::unique_ptr<MenuItem> > m_items;
  int m_current_item;
  TextSurfacePtr m_cursor_surface;

private:
  Menu(const Menu&);
  Menu& operator=(const Menu&);
};

#endif

/* EOF */
