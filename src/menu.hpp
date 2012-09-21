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
  Menu(const TextProperties& text_prop);

  void draw(float x, float y);
  void add_item(const std::string& label, int* value_ptr, int step = 1,
                boost::optional<int> min = boost::optional<int>(),
                boost::optional<int> max = boost::optional<int>());  
  void add_item(const std::string& label, float* value_ptr, float step = 1.0f,
                boost::optional<float> min = boost::optional<float>(),
                boost::optional<float> max = boost::optional<float>());
  void add_item(const std::string& label, bool* value_ptr);

  void up();
  void down();
  void left();
  void right();

private:
  TextProperties m_text_prop;
  std::vector<std::unique_ptr<MenuItem> > m_items;
  int m_current_item;
  TextSurfacePtr m_cursor_surface;

private:
  Menu(const Menu&);
  Menu& operator=(const Menu&);
};

#endif

/* EOF */
