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

#include <stdlib.h>

#include "mesh.hpp"
#include "assert_gl.hpp"
#include "material_factory.hpp"
#include "opengl_state.hpp"

std::shared_ptr<TextSurface>
TextSurface::create(const std::string& text, const TextProperties& text_props)
{
  Cairo::TextExtents text_extents;
  Cairo::FontExtents font_extents;

  Cairo::RefPtr<Cairo::ImageSurface> surface = create_cairo_surface(text, text_props,
                                                                    text_extents, font_extents);
  int width  = surface->get_width();
  int height = surface->get_height();
  TexturePtr texture = create_opengl_texture(surface);

  MaterialPtr material = std::make_shared<Material>();

  material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER,   "src/basic_texture.vert"),
                                        Shader::from_file(GL_FRAGMENT_SHADER, "src/basic_texture.frag")));

  material->enable(GL_BLEND);
  material->blend_func(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  material->set_texture(0, texture);
  material->set_uniform("texture_diff", 0);
  material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);

  return std::make_shared<TextSurface>(material, width, height,
                                       text_extents, font_extents);
}

TextSurface::TextSurface(MaterialPtr material, int width, int height,
                         const Cairo::TextExtents& text_extents,
                         const Cairo::FontExtents& font_extents) :
  m_material(material),
  m_width(width),
  m_height(height),
  m_text_extents(text_extents),
  m_font_extents(font_extents)
{
}

TextSurface::~TextSurface()
{
}

void
TextSurface::draw(RenderContext& ctx, float x, float y, float z)
{
  OpenGLState state;

  m_material->apply(ctx);

  x += static_cast<float>(m_text_extents.x_bearing);
  y += static_cast<float>(m_text_extents.y_bearing);

  GLint program;
  glGetIntegerv(GL_CURRENT_PROGRAM, &program);

  std::vector<glm::vec2> texcoords{
    glm::vec2{ 0.0f, 1.0f },
    glm::vec2{ 1.0f, 1.0f },
    glm::vec2{ 1.0f, 0.0f },
    glm::vec2{ 0.0f, 0.0f }
  };

  std::vector<glm::vec3> positions{
    glm::vec3{ x, y + static_cast<float>(m_height), z },
    glm::vec3{ x + static_cast<float>(m_width), y + static_cast<float>(m_height), z },
    glm::vec3{ x + static_cast<float>(m_width), y, z },
    glm::vec3{ x, y, z }
  };

  GLint texcoords_loc = glGetAttribLocation(program, "texcoord");
  GLint positions_loc = glGetAttribLocation(program, "position");

  assert(texcoords_loc != -1);
  assert(positions_loc != -1);

  GLuint positions_vbo;
  GLuint texcoords_vbo;

  glGenBuffers(1, &positions_vbo);
  glGenBuffers(1, &texcoords_vbo);

  glBindBuffer(GL_ARRAY_BUFFER, positions_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(positions.front()) * positions.size(), positions.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(positions_loc, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, texcoords_vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(texcoords.front()) * texcoords.size(), texcoords.data(), GL_STATIC_DRAW);
  glVertexAttribPointer(texcoords_loc, 2, GL_FLOAT, GL_FALSE, 0, nullptr);

  glBindBuffer(GL_ARRAY_BUFFER, 0);

  glEnableVertexAttribArray(texcoords_loc);
  glEnableVertexAttribArray(positions_loc);

  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

  glDisableVertexAttribArray(texcoords_loc);
  glDisableVertexAttribArray(positions_loc);

  glDeleteBuffers(1, &texcoords_vbo);
  glDeleteBuffers(1, &positions_vbo);
}

TexturePtr
TextSurface::create_opengl_texture(Cairo::RefPtr<Cairo::ImageSurface> surface)
{
  assert(surface);

  TexturePtr texture = Texture::create_handle(GL_TEXTURE_2D);

#ifndef HAVE_OPENGLES2
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->get_width());
#endif

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture->get_id());
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
#ifndef HAVE_OPENGLES2
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

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
