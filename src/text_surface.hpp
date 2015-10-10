//  Simple 3D Model Viewer
//  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_TEXT_SURFACE_HPP
#define HEADER_TEXT_SURFACE_HPP

#include <iostream>
#include <assert.h>
#include <stdexcept>
#include <sstream>
#include <iostream>
#include <memory>
#include <SDL.h>
#include <cairommconfig.h>
#include <cairomm/context.h>
#include <cairomm/surface.h>

#include "opengl.hpp"
#include "texture.hpp"
#include "material.hpp"

class RenderContext;
class TextSurface;

typedef std::shared_ptr<TextSurface> TextSurfacePtr;

class TextProperties
{
public:
  TextProperties() :
    m_font("Inconsolata"),
    m_font_size(24),
    m_line_width(0)
  {}

  std::string get_font() const { return m_font; }
  int get_font_size() const { return m_font_size; }
  int get_line_width() const { return m_line_width; }

  TextProperties& set_font(const std::string& font) { m_font = font; return *this; }
  TextProperties& set_font_size(int font_size) { m_font_size = font_size; return *this; }
  TextProperties& set_line_width(int line_width) { m_line_width = line_width; return *this; }

private:
  std::string m_font;
  int m_font_size;
  int m_line_width;
};

class TextSurface
{
public:
  static std::shared_ptr<TextSurface> create(const std::string& text, TextProperties const& text_props);

  TextSurface(MaterialPtr material, int width, int height,
              const Cairo::TextExtents& text_extents,
              const Cairo::FontExtents& font_extents);
  ~TextSurface();

  void draw(RenderContext const& ctx, float x, float y, float z = -1.0f);

  int get_width() const
  {
    return static_cast<int>(m_text_extents.x_advance);
  }

  int get_height() const
  {
    return static_cast<int>(m_font_extents.height);
  }

private:
  static Cairo::RefPtr<Cairo::ImageSurface> create_cairo_surface(const std::string& text, TextProperties const& text_props,
                                                                 Cairo::TextExtents& out_text_extents,
                                                                 Cairo::FontExtents& out_font_extents);
  static TexturePtr create_opengl_texture(Cairo::RefPtr<Cairo::ImageSurface> surface);

private:
  MaterialPtr m_material;
  int m_width;
  int m_height;

  Cairo::TextExtents m_text_extents;
  Cairo::FontExtents m_font_extents;

public:
  TextSurface(const TextSurface&) = delete;
  TextSurface& operator=(const TextSurface&) = delete;
};

#endif

/* EOF */
