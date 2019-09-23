#ifndef HEADER_TEXTURE_HPP
#define HEADER_TEXTURE_HPP

#include <memory>
#include <string>
#include <boost/filesystem/path.hpp>

#include "opengl.hpp"

class Texture;

typedef std::shared_ptr<Texture> TexturePtr;

class Texture
{
private:
  GLenum m_target;
  GLuint m_id;

public:
  static TexturePtr cubemap_from_file(const boost::filesystem::path& filename);
  static TexturePtr from_file(const boost::filesystem::path& filename, bool build_mipmaps = true, bool exception_on_fail = false);
  static TexturePtr from_rgb_data(int width, int height, int pitch, void* data);
  static TexturePtr create_lightspot(int width, int height);
  static TexturePtr create_random_noise(int width, int height);
  static TexturePtr create_empty(GLenum target, GLenum format, int width, int height);
  static TexturePtr create_shadowmap(int width, int height);
  static TexturePtr create_handle(GLenum target);

public:
  Texture(GLenum target, GLuint id);
  ~Texture();

  GLuint get_id() const { return m_id; }
  GLenum get_target() const { return m_target; }

  void upload(int width, int height, int pitch, void* data);

private:
  Texture(const Texture&);
  Texture& operator=(const Texture&);
};

#endif

/* EOF */
