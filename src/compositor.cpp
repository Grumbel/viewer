#include "compositor.hpp"

#include "camera.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "opengl_state.hpp"
#include "viewer.hpp"
#include "render_context.hpp"
#include "renderbuffer.hpp"
#include "log.hpp"

extern std::unique_ptr<Framebuffer> g_shadowmap;
extern glm::mat4 g_shadowmap_matrix;

Compositor::Compositor(int screen_w, int screen_h) :
  m_screen_w(screen_w),
  m_screen_h(screen_h)
{
  m_framebuffer1 = std::make_unique<Framebuffer>(m_screen_w, m_screen_h);
  m_framebuffer2 = std::make_unique<Framebuffer>(m_screen_w, m_screen_h);
  m_renderbuffer1 = std::make_unique<Renderbuffer>(m_screen_w, m_screen_h);
  m_renderbuffer2 = std::make_unique<Renderbuffer>(m_screen_w, m_screen_h);
  g_shadowmap = std::make_unique<Framebuffer>(m_shadowmap_resolution, m_shadowmap_resolution);

  m_cybermaxx_prog = Program::create(
    Shader::from_file(GL_VERTEX_SHADER, "src/glsl/composite.vert"),
    Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/composite.frag",
                      {"INTERLACED_COMPOSITION"}));

  m_crosseye_prog = Program::create(
    Shader::from_file(GL_VERTEX_SHADER, "src/glsl/composite.vert"),
    Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/composite.frag",
                      {"CROSSEYE_COMPOSITION"}));

  m_anaglyph_prog = Program::create(
    Shader::from_file(GL_VERTEX_SHADER, "src/glsl/composite.vert"),
    Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/composite.frag",
                      {"ANAGLYPH_COMPOSITION"}));

  m_depth_prog = Program::create(
    Shader::from_file(GL_VERTEX_SHADER, "src/glsl/composite.vert"),
    Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/composite.frag",
                      {"DEPTH_COMPOSITION"}));

  m_newsprint_prog = Program::create(
    Shader::from_file(GL_VERTEX_SHADER, "src/glsl/composite.vert"),
    Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/newsprint.frag"));

  m_mono_prog = Program::create(
    Shader::from_file(GL_VERTEX_SHADER, "src/glsl/composite.vert"),
    Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/composite.frag"));

  m_composition_prog = m_mono_prog;

  m_calibration_left_texture = Texture::from_file("data/calibration_left.png", false);
  m_calibration_right_texture = Texture::from_file("data/calibration_right.png", false);
}

void
Compositor::render(Viewer& viewer)
{
  glViewport(m_viewport_offset.x, m_viewport_offset.y, m_screen_w, m_screen_h);

  // render the world, twice if stereo is enabled
  if (true)
  {
    OpenGLState state;

    if (m_render_shadowmap)
    {
      g_shadowmap->bind();
      draw_shadowmap(viewer);
      g_shadowmap->unbind();
    }

    if (m_stereo_mode == StereoMode::None)
    {
      m_renderbuffer1->bind();
      if (viewer.m_video_material && viewer.m_opts.video3d) viewer.m_video_material->set_uniform("offset", 0.0f);
      draw_scene(viewer, Stereo::Center);
      m_renderbuffer1->unbind();

      m_renderbuffer1->blit(*m_framebuffer1);
    }
    else
    {
      m_renderbuffer1->bind();
      if (viewer.m_video_material && viewer.m_opts.video3d) viewer.m_video_material->set_uniform("offset", 0.0f);
      draw_scene(viewer, Stereo::Left);
      m_renderbuffer1->unbind();

      m_renderbuffer2->bind();
      if (viewer.m_video_material && viewer.m_opts.video3d) viewer.m_video_material->set_uniform("offset", 0.5f);
      draw_scene(viewer, Stereo::Right);
      m_renderbuffer2->unbind();

      m_renderbuffer1->blit(*m_framebuffer1);
      m_renderbuffer2->blit(*m_framebuffer2);
    }
  }

  // composit the final image
  if (true)
  {
    OpenGLState state;

    MaterialPtr material = std::make_shared<Material>();
    { // setup material
      material->set_program(m_composition_prog);

      material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);

      material->set_uniform("barrel_power", m_barrel_power);
      material->set_uniform("left_eye",  0);
      material->set_uniform("right_eye", 1);

      if (!viewer.m_show_calibration)
      {
        material->set_texture(0, m_framebuffer1->get_color_texture());
        material->set_texture(1, m_framebuffer2->get_color_texture());
      }
      else
      {
        material->set_texture(0, m_calibration_left_texture);
        material->set_texture(1, m_calibration_right_texture);
      }

      switch(m_stereo_mode)
      {
        case StereoMode::Cybermaxx:
          m_composition_prog = m_cybermaxx_prog;
          break;

        case StereoMode::CrossEye:
          m_composition_prog = m_crosseye_prog;
          break;

        case StereoMode::Anaglyph:
          m_composition_prog = m_anaglyph_prog;
          break;

        case StereoMode::Depth:
          material->set_texture(0, m_framebuffer1->get_depth_texture());
          m_composition_prog = m_depth_prog;
          break;

        case StereoMode::Newsprint:
          m_composition_prog = m_newsprint_prog;
          break;

        default:
          m_composition_prog = m_mono_prog;
          break;
      }
    } // setup material

    ModelPtr model = std::make_shared<Model>();
    model->add_mesh(Mesh::create_rect(0.0f, 0.0f, m_screen_w, m_screen_h, -20.0f));
    model->set_material(material);

    Camera camera;
    camera.ortho(0, m_screen_w, m_screen_h, 0.0f, 0.1f, 10000.0f);

    SceneManager mgr;
    mgr.get_world()->attach_model(model);

    glViewport(m_viewport_offset.x, m_viewport_offset.y, m_screen_w, m_screen_h);

    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    mgr.render(camera);

    // render menu overlay
    if (true)
    {
      glClear(GL_DEPTH_BUFFER_BIT);
      RenderContext ctx(camera, mgr.get_world());

      if (false && viewer.m_show_menu)
      {
        glDisable(GL_BLEND);
        //g_shadowmap->draw_depth(m_screen_w - 266, 10, 256, 256, -20.0f);
        g_shadowmap->draw(m_screen_w - 266 - 276, 10, 256, 256, -20.0f);
      }

      if (viewer.m_show_menu)
      {
        viewer.m_menu->draw(ctx, 120.0f, 64.0f);
      }

      if (viewer.m_show_dots)
      {
        viewer.m_dot_surface->draw(ctx, viewer.m_wiimote_dot1.x * m_screen_w, viewer.m_wiimote_dot1.y * m_screen_h);
        viewer.m_dot_surface->draw(ctx, viewer.m_wiimote_dot2.x * m_screen_w, viewer.m_wiimote_dot2.y * m_screen_h);
      }
    }
  }

  assert_gl("display:exit()");
}

void
Compositor::draw_shadowmap(Viewer& viewer)
{
  OpenGLState state;

  glViewport(0, 0, g_shadowmap->get_width(), g_shadowmap->get_height());

  glClearColor(1.0, 0.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glm::vec3 light_pos = glm::rotate(glm::vec3(10.0f, 10.0f, 10.0f), viewer.m_light_angle, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::vec3 up = glm::rotate(glm::vec3(0.0f, 1.0f, 0.0f), viewer.m_light_up, glm::vec3(0.0f, 0.0f, 1.0f));
  glm::vec3 look_at(0.0f, 0.0f, 0.0f);

  Camera camera;
  camera.perspective(viewer.m_shadowmap_fov, 1.0f, viewer.m_near_z, viewer.m_far_z);
  //camera.ortho(-25.0f, 25.0f, -25.0f, 25.0f, m_near_z, m_far_z);

  camera.look_at(light_pos, look_at, up);

  g_shadowmap_matrix = glm::mat4(0.5, 0.0, 0.0, 0.0,
                                 0.0, 0.5, 0.0, 0.0,
                                 0.0, 0.0, 0.5, 0.0,
                                 0.5, 0.5, 0.5, 1.0);

  g_shadowmap_matrix = g_shadowmap_matrix * camera.get_matrix();

  viewer.m_scene_manager->render(camera, true);
}

void
Compositor::draw_scene(Viewer& viewer, Stereo stereo)
{
  OpenGLState state;

  glViewport(0, 0, m_screen_w, m_screen_h);

  // clear the screen
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glm::vec3 look_at = viewer.m_look_at;
  glm::vec3 up = viewer.m_up;

  glm::vec3 sideways_ = glm::normalize(glm::cross(look_at, up));
  glm::vec3 eye = viewer.m_eye + glm::normalize(look_at) * viewer.m_distance_offset;

  if (viewer.m_wiimote_camera_control && viewer.m_wiimote_manager)
  {
    glm::quat q = glm::inverse(glm::quat(glm::mat3(glm::lookAt(glm::vec3(), look_at, up))));

    glm::quat orientation = viewer.m_wiimote_manager->get_gyro_orientation();
    look_at = (q * orientation) * glm::vec3(0,0,-1);
    up = (q * orientation) * glm::vec3(0,1,0);
  }
  else
  {
    look_at = glm::rotate(look_at, viewer.m_yaw_offset, up);
    look_at = glm::rotate(look_at, -viewer.m_pitch_offset, sideways_);
    up = glm::rotate(up, -viewer.m_roll_offset, look_at);
  }

  glm::vec3 sideways = glm::normalize(glm::cross(look_at, up)) * viewer.m_eye_distance * 0.5f;
  switch(stereo)
  {
    case Stereo::Left:
      sideways = sideways;
      break;

    case Stereo::Right:
      sideways = -sideways;
      break;

    case Stereo::Center:
      sideways = glm::vec3(0);
      break;
  }

  Camera camera;
  camera.perspective(viewer.m_fov, viewer.m_aspect_ratio, viewer.m_near_z, viewer.m_far_z);
  camera.look_at(eye + sideways, eye + look_at * viewer.m_convergence, up);

  viewer.m_scene_manager->render(camera, false, stereo);
}

void
Compositor::reshape(Viewer& viewer, int w, int h)
{
  log_info("reshape(%d, %d)", w, h);
  m_screen_w = w;
  m_screen_h = h;

  assert_gl("reshape1");

  m_framebuffer1 = std::make_unique<Framebuffer>(m_screen_w, m_screen_h);
  m_framebuffer2 = std::make_unique<Framebuffer>(m_screen_w, m_screen_h);

  m_renderbuffer1 = std::make_unique<Renderbuffer>(m_screen_w, m_screen_h);
  m_renderbuffer2 = std::make_unique<Renderbuffer>(m_screen_w, m_screen_h);

  viewer.m_aspect_ratio = static_cast<GLfloat>(m_screen_w)/static_cast<GLfloat>(m_screen_h);

  assert_gl("reshape");
}

void
Compositor::toggle_stereo_mode()
{
  int stereo_mode = static_cast<int>(m_stereo_mode) + 1;
  if (stereo_mode >= static_cast<int>(StereoMode::End))
  {
    m_stereo_mode = StereoMode::None;
  }
  else
  {
    m_stereo_mode = static_cast<StereoMode>(stereo_mode);
  }

  log_info("Stereo Mode changed to %d", static_cast<int>(m_stereo_mode));
}

/* EOF */
