#ifndef HEADER_TEXTURE_HPP
#define HEADER_TEXTURE_HPP

#include <memory>
#include <string>
#include <GL/glew.h>

class Texture;

typedef std::shared_ptr<Texture> TexturePtr;

class Texture
{
private:
  GLenum m_target;
  GLuint m_id;

public:
  static TexturePtr cubemap_from_file(const std::string& filename);
  static TexturePtr from_file(const std::string& filename, bool build_mipmaps = true);
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

  void draw(float x, float y, float w, float h, float z);

private:
  Texture(const Texture&);
  Texture& operator=(const Texture&);
};

#endif

/* EOF */
