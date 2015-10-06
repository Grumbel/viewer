#include "texture.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <assert.h>
#include <math.h>
#include <stdexcept>
#include <string.h>
#include <vector>

#include "assert_gl.hpp"
#include "opengl_state.hpp"

namespace {

void flip_rgb(SDL_Surface* surface)
{
  for(int y = 0; y < surface->h; ++y)
    for(int x = 0; x < surface->w; ++x)
    {
      uint8_t* p = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch + x * surface->format->BytesPerPixel;
      std::swap(p[0], p[2]);
    }
}

void vflip_surface(SDL_Surface* surface)
{
  size_t len = surface->w * surface->format->BytesPerPixel;
  std::vector<uint8_t> tmp_vec(len);
  for(int y = 0; y < surface->h/2; ++y)
  {
    uint8_t* src = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch;
    uint8_t* dst = static_cast<uint8_t*>(surface->pixels) + (surface->h - y - 1) * surface->pitch;
    uint8_t* tmp = tmp_vec.data();
    memcpy(tmp, src, len);
    memcpy(src, dst, len);
    memcpy(dst, tmp, len);
  }
}

} // namespace


TexturePtr
Texture::create_empty(GLenum target, GLenum format, int width, int height)
{
  OpenGLState state;

  assert_gl("framebuffer");
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(target, texture);
  glTexImage2D(target, 0, format,  width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
  glTexParameterf(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameterf(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameterf(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  assert_gl("framebuffer");

  return std::make_shared<Texture>(target, texture);
}

TexturePtr
Texture::create_shadowmap(int width, int height)
{
  OpenGLState state;

  GLuint texture;

  assert_gl("Texture::create_shadowmap: start");
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,  width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);

  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  // glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

#ifndef HAVE_OPENGLES2
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
#endif

#ifndef HAVE_OPENGLES2
  //GLfloat border_color[] = { 1.0f, 1.0f, 1.0f, 1.0f };
  GLfloat border_color[] = { 0.0f, 0.0f, 0.0f, 0.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
#endif

  assert_gl("Texture::create_shadowmap");

  return std::make_shared<Texture>(GL_TEXTURE_2D, texture);
}

TexturePtr
Texture::create_random_noise(int width, int height)
{
  OpenGLState state;

  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

#ifndef HAVE_OPENGLES2
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
#endif

  std::vector<uint8_t> data(width*height*3);

  for(size_t i = 0; i < sizeof(data); i+=3)
  {
    data[i+0] = data[i+1] = data[i+2] = rand() % 255;
  }

#ifndef HAVE_OPENGLES2
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
  assert_gl("texture0()");
#endif

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  assert_gl("texture-0()");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  assert_gl("texture-1()");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  assert_gl("texture-2()");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  assert_gl("texture1()");

#ifndef HAVE_OPENGLES2
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
  assert_gl("texture2()");
#endif

  return std::make_shared<Texture>(GL_TEXTURE_2D, texture);
}

TexturePtr
Texture::create_lightspot(int width, int height)
{
  OpenGLState state;

  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  const int pitch = width * 3;
#ifndef HAVE_OPENGLES2
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
#endif

  std::vector<uint8_t> data(width*height*3);
  for(int y = 0; y < height; ++y)
    for(int x = 0; x < width; ++x)
    {
      float xf = (static_cast<float>(x) / static_cast<float>(width)  - 0.5f) * 2.0f;
      float yf = (static_cast<float>(y) / static_cast<float>(height) - 0.5f) * 2.0f;

      float f = 1.0f - sqrtf(xf*xf + yf*yf);

      data[y * pitch + 3*x+0] = static_cast<uint8_t>(std::max(0.0f, std::min(f * 255.0f, 255.0f)));
      data[y * pitch + 3*x+1] = static_cast<uint8_t>(std::max(0.0f, std::min(f * 255.0f, 255.0f)));
      data[y * pitch + 3*x+2] = static_cast<uint8_t>(std::max(0.0f, std::min(f * 255.0f, 255.0f)));
    }

#ifndef HAVE_OPENGLES2
  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
  assert_gl("texture0()");
#endif

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

#ifndef HAVE_OPENGLES2
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
#endif

  return std::make_shared<Texture>(GL_TEXTURE_2D, texture);
}

namespace {
SDL_Surface* surface_from_file(const std::string& filename)
{
  SDL_Surface* surface = IMG_Load(filename.c_str());
  if (!surface)
  {
    throw std::runtime_error("couldn't load " + filename);
  }
  else
  {
    return surface;
  }
}
} // namespace

TexturePtr
Texture::cubemap_from_file(const std::string& filename)
{
  OpenGLState state;

  SDL_Surface* up = surface_from_file(filename + "up.png");
  SDL_Surface* dn = surface_from_file(filename + "dn.png");
  SDL_Surface* ft = surface_from_file(filename + "ft.png");
  SDL_Surface* bk = surface_from_file(filename + "bk.png");
  SDL_Surface* lf = surface_from_file(filename + "lf.png");
  SDL_Surface* rt = surface_from_file(filename + "rt.png");

  assert(up);
  assert(dn);
  assert(ft);
  assert(bk);
  assert(lf);
  assert(rt);

  flip_rgb(up);
  flip_rgb(dn);
  flip_rgb(ft);
  flip_rgb(bk);
  flip_rgb(lf);
  flip_rgb(rt);

#ifndef HAVE_OPENGLES2
  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, up->pitch / up->format->BytesPerPixel);
#endif

  //glActiveTexture(GL_TEXTURE0);
  //glEnable(GL_TEXTURE_CUBE_MAP);

  GLuint texture;
  GLenum target = GL_TEXTURE_CUBE_MAP;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_CUBE_MAP, texture);

  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
#ifndef HAVE_OPENGLES2
  glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
#endif

#ifndef HAVE_OPENGLES2
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_RGB, up->w, up->h, up->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, up->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_RGB, dn->w, dn->h, dn->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, dn->pixels);

  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_RGB, lf->w, lf->h, lf->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, lf->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGB, rt->w, rt->h, rt->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, rt->pixels);

  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_RGB, ft->w, ft->h, ft->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, ft->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_RGB, bk->w, bk->h, bk->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, bk->pixels);
#endif

#ifndef HAVE_OPENGLES2
  glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 10);
  glGenerateMipmap(target);
#endif

  SDL_FreeSurface(up);
  SDL_FreeSurface(dn);
  SDL_FreeSurface(ft);
  SDL_FreeSurface(bk);
  SDL_FreeSurface(lf);
  SDL_FreeSurface(rt);

  assert_gl("cube texture");

  return std::make_shared<Texture>(target, texture);
}

TexturePtr
Texture::from_file(const std::string& filename, bool build_mipmaps)
{
  OpenGLState state;

  SDL_Surface* surface = IMG_Load(filename.c_str());
  if (!surface)
  {
    throw std::runtime_error("Texture: couldn't open " + filename);
  }
  else
  {
    vflip_surface(surface);

    GLenum target = GL_TEXTURE_2D;
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(target, texture);

#ifndef HAVE_OPENGLES2
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / surface->format->BytesPerPixel);
#endif

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, build_mipmaps ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);

    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

#ifndef HAVE_OPENGLES2
    float max_anisotrophy = 0.0f;
    glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_anisotrophy);
    glTexParameteri(target, GL_TEXTURE_MAX_ANISOTROPY_EXT, max_anisotrophy);

    if (build_mipmaps)
    {
      gluBuild2DMipmaps(target, GL_RGB, surface->w, surface->h,
                        surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB,
                        GL_UNSIGNED_BYTE, surface->pixels);
    }
    else
#endif
    {
      glTexImage2D(target, 0, GL_RGB, surface->w, surface->h, 0,
                   surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB,
                   GL_UNSIGNED_BYTE, surface->pixels);
    }

    SDL_FreeSurface(surface);

    return std::make_shared<Texture>(target, texture);
  }
}


TexturePtr
Texture::from_rgb_data(int width, int height, int pitch, void* data)
{
  OpenGLState state;

  GLenum target = GL_TEXTURE_2D;
  GLuint texture;
  glGenTextures(1, &texture);
  glBindTexture(target, texture);

#ifndef HAVE_OPENGLES2
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
#endif

  glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

  glTexImage2D(target, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

  return std::make_shared<Texture>(target, texture);
}

TexturePtr
Texture::create_handle(GLenum target)
{
  GLuint texture;
  glGenTextures(1, &texture);
  return std::make_shared<Texture>(target, texture);
}

Texture::Texture(GLenum target, GLuint id) :
  m_target(target),
  m_id(id)
{
}

Texture::~Texture()
{
  glDeleteTextures(1, &m_id);
}

void
Texture::upload(int width, int height, int pitch, void* data)
{
  OpenGLState state;

#ifndef HAVE_OPENGLES2
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
#endif

  glBindTexture(m_target, m_id);
  glTexSubImage2D(m_target, 0, 0, 0, width, height, GL_RGB, GL_UNSIGNED_BYTE, data);
  assert_gl("Texture::upload");
}

/* EOF */
