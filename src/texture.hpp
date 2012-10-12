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
  static TexturePtr cube_from_file(const std::string& filename);
  static TexturePtr from_file(const std::string& filename);

public:
  Texture(GLenum target, GLuint id);
  ~Texture();

  GLuint get_id() const { return m_id; }
  GLenum get_target() const { return m_target; }

private:
  Texture(const Texture&);
  Texture& operator=(const Texture&);
};

#endif

/* EOF */
