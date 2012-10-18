#include "texture.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <stdexcept>
#include <assert.h>
#include <math.h>
#include <vector>

#include "opengl_state.hpp"
#include "assert_gl.hpp"

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

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);

  GLfloat border_color[] = { 1.0f, 0.0f, 0.0f, 0.0f };
  glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);

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
   
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

  std::vector<uint8_t> data(width*height*3);

  for(size_t i = 0; i < sizeof(data); i+=3)
  {
    data[i+0] = data[i+1] = data[i+2] = rand() % 255;
  }

  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
  assert_gl("texture0()");
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  assert_gl("texture-0()");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  assert_gl("texture-1()");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  assert_gl("texture-2()");
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  assert_gl("texture1()");

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
  assert_gl("texture2()");

  return TexturePtr(new Texture(GL_TEXTURE_2D, texture));
}

TexturePtr
Texture::create_lightspot(int width, int height)
{
  OpenGLState state;

  GLuint texture;

  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  const int pitch = width * 3;
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

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

  gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, data.data());
  assert_gl("texture0()");
    
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);

  return TexturePtr(new Texture(GL_TEXTURE_2D, texture));
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

  glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  glPixelStorei(GL_UNPACK_ROW_LENGTH, up->pitch / up->format->BytesPerPixel);

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
  glTexParameteri(target, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_RGB, up->w, up->h, up->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, up->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_RGB, dn->w, dn->h, dn->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, dn->pixels);

  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_RGB, lf->w, lf->h, lf->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, lf->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGB, rt->w, rt->h, rt->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, rt->pixels);

  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_RGB, ft->w, ft->h, ft->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, ft->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_RGB, bk->w, bk->h, bk->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, bk->pixels);

  glTexParameteri(target, GL_TEXTURE_BASE_LEVEL, 0);
  glTexParameteri(target, GL_TEXTURE_MAX_LEVEL, 10);
  glGenerateMipmap(target);

  SDL_FreeSurface(up);
  SDL_FreeSurface(dn);
  SDL_FreeSurface(ft);
  SDL_FreeSurface(bk);
  SDL_FreeSurface(lf);
  SDL_FreeSurface(rt);

  assert_gl("cube texture");

  return TexturePtr(new Texture(target, texture));
}

TexturePtr
Texture::from_file(const std::string& filename)
{
  OpenGLState state;

  SDL_Surface* surface = IMG_Load(filename.c_str());
  if (!surface)
  {
    throw std::runtime_error("couldn't open " + filename);
  }
  else
  {
    GLenum target = GL_TEXTURE_2D;
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(target, texture);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, surface->pitch / surface->format->BytesPerPixel);  

    glTexParameteri(target, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(target, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(target, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(target, GL_TEXTURE_WRAP_T, GL_REPEAT);

    gluBuild2DMipmaps(target, GL_RGB, surface->w, surface->h, 
                      surface->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, 
                      GL_UNSIGNED_BYTE, surface->pixels);

    SDL_FreeSurface(surface);

    return TexturePtr(new Texture(target, texture));
  }
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

/* EOF */
