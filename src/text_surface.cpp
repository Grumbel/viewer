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

#include "text_surface.hpp"

#include "assert_gl.hpp"

std::shared_ptr<TextSurface>
TextSurface::create(const std::string& text, const TextProperties& text_props)
{
  Cairo::TextExtents text_extents;
  Cairo::FontExtents font_extents;

  Cairo::RefPtr<Cairo::ImageSurface> surface = create_cairo_surface(text, text_props,
                                                                    text_extents, font_extents);
  int width  = surface->get_width();
  int height = surface->get_height();
  GLuint texture = create_opengl_texture(surface);

  return std::make_shared<TextSurface>(texture, width, height,
                                       text_extents, font_extents);
}

TextSurface::TextSurface(GLuint texture, int width, int height,
                         const Cairo::TextExtents& text_extents,
                         const Cairo::FontExtents& font_extents) :
  m_texture(texture),
  m_width(width),
  m_height(height),
  m_text_extents(text_extents),
  m_font_extents(font_extents)
{
}

TextSurface::~TextSurface()
{
  glDeleteTextures(1, &m_texture);
}

void
TextSurface::draw(float x, float y, float z)
{
  //x -= static_cast<float>(m_text_props.get_line_width())/2.0f;
  //y -= static_cast<float>(m_text_props.get_line_width())/2.0f;
  
  x += static_cast<float>(m_text_extents.x_bearing);
  y += static_cast<float>(m_text_extents.y_bearing);
  //- static_cast<float>(m_font_extents.height)
  //+ static_cast<float>(m_font_extents.descent);
  
  glBindTexture(GL_TEXTURE_2D, m_texture);
  glBegin(GL_QUADS); 
  {
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(x, y, z);

    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(x + static_cast<float>(m_width), y, z);

    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(x + static_cast<float>(m_width), 
               y + static_cast<float>(m_height),
               z);

    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(x, y + static_cast<float>(m_height), z);
  }
  glEnd();
}

GLuint
TextSurface::create_opengl_texture(Cairo::RefPtr<Cairo::ImageSurface> surface)
{
  GLuint texture = 0;

  assert(surface);

  glGenTextures(1, &texture);
  assert_gl("texture failure");
    
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->get_width());

  glBindTexture(GL_TEXTURE_2D, texture);
  assert_gl("Texture failure");

  // flip RGBA to BGRA
  for(int y = 0; y < surface->get_height(); ++y)
  {
    uint8_t* pixels = surface->get_data() + surface->get_stride() * y;

    for(int x = 0; x < surface->get_width()*4; x += 4)
    {
      uint8_t r = pixels[x+0];
      uint8_t g = pixels[x+1];
      uint8_t b = pixels[x+2];
      uint8_t a = pixels[x+3];

      // removing pre-multiplayed alpha
      if (a != 0)
      {
        pixels[x+0] = static_cast<uint8_t>(b * 255 / a);
        pixels[x+1] = static_cast<uint8_t>(g * 255 / a);
        pixels[x+2] = static_cast<uint8_t>(r * 255 / a);
      }

      // debug foobar
      //if (pixels[x+3] == 0)
      //  pixels[x+3] = 32;
    }
  }

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
               surface->get_width(), surface->get_height(), 0, 
               GL_RGBA, GL_UNSIGNED_BYTE, surface->get_data());
  assert_gl("Texture failure");

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

  return texture;
}

Cairo::RefPtr<Cairo::ImageSurface>
TextSurface::create_cairo_surface(const std::string& text, const TextProperties& text_props,
                                  Cairo::TextExtents& out_text_extents,
                                  Cairo::FontExtents& out_font_extents)
{
  { // get TextExtents and FontExtents
    Cairo::RefPtr<Cairo::ImageSurface> tmp_surface = Cairo::ImageSurface::create(Cairo::FORMAT_RGB24, 0, 0);
    Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(tmp_surface);
    cr->set_font_size(text_props.get_font_size());
    cr->select_font_face(text_props.get_font(), Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
    cr->get_text_extents(text, out_text_extents);
    cr->get_font_extents(out_font_extents);
  }

  Cairo::RefPtr<Cairo::ImageSurface>
    surface = Cairo::ImageSurface::create(Cairo::FORMAT_ARGB32, 
                                          static_cast<int>(out_text_extents.width  + text_props.get_line_width()), 
                                          static_cast<int>(out_text_extents.height + text_props.get_line_width()));

  Cairo::RefPtr<Cairo::Context> cr = Cairo::Context::create(surface);

  // set the font
  cr->set_font_size(text_props.get_font_size());
  cr->select_font_face(text_props.get_font(), Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);

  if (text_props.get_line_width() != 0)
  {
    // create path
    cr->move_to(-out_text_extents.x_bearing + text_props.get_line_width()/2.0,
                -out_text_extents.y_bearing + text_props.get_line_width()/2.0);
    cr->text_path(text);

    // paint
    cr->set_line_width(text_props.get_line_width());
    cr->set_line_join(Cairo::LINE_JOIN_ROUND);
    cr->set_source_rgb(0.0, 0.0, 0.0);
    cr->stroke();
  }

  // print text
  cr->move_to(-out_text_extents.x_bearing + text_props.get_line_width()/2.0,
              -out_text_extents.y_bearing + text_props.get_line_width()/2.0);
  cr->set_source_rgb(1.0, 1.0, 0.0);

  double y = -out_text_extents.y_bearing - out_font_extents.ascent;

  // toying around with color gradients
  if (false)
  {
    Cairo::RefPtr<Cairo::LinearGradient> gradient = Cairo::LinearGradient::create(0, y,
                                                                                  0, y + out_font_extents.ascent + out_font_extents.descent);
    gradient->add_color_stop_rgb(0.0, 1.0, 1.0, 0.0);
    gradient->add_color_stop_rgb(0.5, 1.0, 1.0, 1.0);
    gradient->add_color_stop_rgb(0.5, 0.4, 0.4, 0.2);
    gradient->add_color_stop_rgb(1.0, 1.0, 1.0, 0.0);
    cr->set_source(gradient);
  }

  cr->show_text(text);

  return surface;
}

/* EOF */
