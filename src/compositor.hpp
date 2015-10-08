#ifndef HEADER_COMPOSITOR_HPP
#define HEADER_COMPOSITOR_HPP

#include <memory>
#include <glm/glm.hpp>

#include "program.hpp"
#include "stereo.hpp"
#include "texture.hpp"

class Camera;
class Framebuffer;
class RenderContext;
class Renderbuffer;
class Viewer;

enum class StereoMode { None, CrossEye, Cybermaxx, Anaglyph, Depth, Newsprint, End };

class Compositor
{
public:
  std::unique_ptr<Framebuffer> m_framebuffer1;
  std::unique_ptr<Framebuffer> m_framebuffer2;

  std::unique_ptr<Renderbuffer> m_renderbuffer1;
  std::unique_ptr<Renderbuffer> m_renderbuffer2;

  glm::ivec2 m_viewport_offset = { 0, 0 };
  float m_barrel_power = 0.05f;
  float m_ipd = 0.0f;
  int m_screen_w = 640;
  int m_screen_h = 480;
  int m_shadowmap_resolution = 1024;

  StereoMode m_stereo_mode = StereoMode::None;
  bool m_render_shadowmap = true;

  ProgramPtr m_composition_prog;

  ProgramPtr m_cybermaxx_prog;
  ProgramPtr m_crosseye_prog;
  ProgramPtr m_anaglyph_prog;
  ProgramPtr m_depth_prog;
  ProgramPtr m_newsprint_prog;
  ProgramPtr m_mono_prog;

  TexturePtr m_calibration_left_texture;
  TexturePtr m_calibration_right_texture;

public:
  Compositor(int, int);

  void render(Viewer& viewer);
  void reshape(Viewer& viewer, int w, int h);
  void toggle_stereo_mode();

private:
  void render_scene(Viewer& viewer, Stereo stereo);
  void render_shadowmap(Viewer& viewer);
  void render_menu(RenderContext const& ctx, Viewer const& viewer);

private:
  Compositor(const Compositor&) = delete;
  Compositor& operator=(const Compositor&) = delete;
};
#endif

/* EOF */
