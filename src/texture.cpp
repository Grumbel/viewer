#include "texture.hpp"

#include <SDL.h>
#include <SDL_image.h>
#include <stdexcept>
#include <assert.h>

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
Texture::cube_from_file(const std::string& filename)
{
  OpenGLState state;

  SDL_Surface* up = IMG_Load((filename + "_up.tga").c_str());
  SDL_Surface* dn = IMG_Load((filename + "_dn.tga").c_str());
  SDL_Surface* ft = IMG_Load((filename + "_ft.tga").c_str());
  SDL_Surface* bk = IMG_Load((filename + "_bk.tga").c_str());
  SDL_Surface* lf = IMG_Load((filename + "_lf.tga").c_str());
  SDL_Surface* rt = IMG_Load((filename + "_rt.tga").c_str());

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
   
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGB, lf->w, lf->h, lf->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, lf->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_RGB, rt->w, rt->h, rt->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, rt->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_RGB, up->w, up->h, up->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, up->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_RGB, dn->w, dn->h, dn->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, dn->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_RGB, ft->w, ft->h, ft->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, ft->pixels);
  gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_RGB, bk->w, bk->h, bk->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, bk->pixels);

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
