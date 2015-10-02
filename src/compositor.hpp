#ifndef HEADER_COMPOSITOR_HPP
#define HEADER_COMPOSITOR_HPP

#include <memory>
#include <glm/glm.hpp>

#include "stereo.hpp"

class Framebuffer;
class Renderbuffer;
class Viewer;

enum class StereoMode { None, CrossEye, Cybermaxx, Anaglyph, Depth, End };

class Compositor
{
public:
  std::unique_ptr<Framebuffer> m_framebuffer1;
  std::unique_ptr<Framebuffer> m_framebuffer2;

  std::unique_ptr<Renderbuffer> m_renderbuffer1;
  std::unique_ptr<Renderbuffer> m_renderbuffer2;

  glm::ivec2 m_viewport_offset = {-41, 16};
  float m_barrel_power = 0.05f;
  float m_ipd = 0.0f;
  int m_screen_w = 640;
  int m_screen_h = 480;
  int m_shadowmap_resolution = 1024;

  StereoMode m_stereo_mode = StereoMode::None;
  bool m_render_shadowmap = true;

public:
  Compositor(int, int);

  void draw_scene(Viewer& viewer, Stereo stereo);
  void draw_shadowmap(Viewer& viewer);
  void render(Viewer& viewer);
  void reshape(Viewer& viewer, int w, int h);

private:
  Compositor(const Compositor&) = delete;
  Compositor& operator=(const Compositor&) = delete;
};
#endif

/* EOF */
