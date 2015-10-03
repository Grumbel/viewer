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

#include "viewer.hpp"

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <cmath>
#include <cmath>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <vector>
#include <thread>
#include <glm/gtx/io.hpp>

#include "assert_gl.hpp"
#include "camera.hpp"
#include "compositor.hpp"
#include "log.hpp"
#include "material_factory.hpp"
#include "menu.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "opengl_state.hpp"
#include "program.hpp"
#include "render_context.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "shader.hpp"
#include "system.hpp"
#include "text_surface.hpp"

namespace {

void print_scene_graph(SceneNode* node, int depth = 0)
{
  std::cout << std::string(depth, ' ') << node
            << ": " << node->get_position()
            << " " << node->get_scale()
            << " " << node->get_orientation() << std::endl;
  for(const auto& child : node->get_children())
  {
    print_scene_graph(child.get(), depth+1);
  }
}

} // namespace

std::unique_ptr<Framebuffer> g_shadowmap;
glm::mat4 g_shadowmap_matrix;

void
Viewer::on_keyboard_event(SDL_KeyboardEvent key, int x, int y)
{
  switch (key.keysym.scancode)
  {
    case SDL_SCANCODE_TAB:
      m_show_menu = !m_show_menu;
      break;

    case SDL_SCANCODE_F3:
      m_show_calibration = !m_show_calibration;
      break;

    case SDL_SCANCODE_ESCAPE:
      exit(EXIT_SUCCESS);
      break;

    case SDL_SCANCODE_9:
      if (m_video_player)
      {
        m_video_player->seek(m_video_player->get_position() - 10 * Gst::SECOND);
      }
      break;

    case SDL_SCANCODE_0:
      if (m_video_player)
      {
        m_video_player->seek(m_video_player->get_position() + 10 * Gst::SECOND);
      }
      break;

    case SDL_SCANCODE_N:
      m_eye_distance += 0.01f;
      break;

    case SDL_SCANCODE_T:
      m_eye_distance -= 0.01f;
      break;

    case SDL_SCANCODE_SPACE:
      m_draw_look_at = !m_draw_look_at;
      break;

    case SDL_SCANCODE_C:
      m_compositor->m_ipd += 1;
      break;

    case SDL_SCANCODE_R:
      m_compositor->m_ipd -= 1;
      break;

    case SDL_SCANCODE_KP_PLUS:
      m_scale *= 1.05f;
      break;

    case SDL_SCANCODE_KP_MINUS:
      m_scale /= 1.05f;
      break;

    case SDL_SCANCODE_Z:
      {
        GLdouble clip_plane[] = { 0.0, 0.0, 1.0, 1.0 };

        clip_plane[0] = (rand() / double(RAND_MAX) - 0.5) * 2.0f;
        clip_plane[1] = (rand() / double(RAND_MAX) - 0.5) * 2.0f;
        clip_plane[2] = (rand() / double(RAND_MAX) - 0.5) * 2.0f;
        clip_plane[3] = (rand() / double(RAND_MAX) - 0.5) * 2.0f;

        glEnable(GL_CLIP_PLANE0);
        glClipPlane(GL_CLIP_PLANE0, clip_plane);
      }
      break;

    case SDL_SCANCODE_G:
      {
        GLdouble clip_plane[] = { 0.0, 1.0, 1.0, 0.0 };
        glClipPlane(GL_CLIP_PLANE0, clip_plane);
        glEnable(GL_CLIP_PLANE0);
      }
      break;

    case SDL_SCANCODE_D:
      {
#if 0
        int stereo_mode = static_cast<int>(m_stereo_mode) + 1;
        if (stereo_mode >= static_cast<int>(StereoMode::End))
        {
          m_stereo_mode = StereoMode::None;
        }
        else
        {
          m_stereo_mode = static_cast<StereoMode>(stereo_mode);
        }
#endif
      }
      break;

    case SDL_SCANCODE_KP_8: // up
      m_eye += glm::normalize(m_look_at);
      break;

    case SDL_SCANCODE_KP_2: // down
      m_eye -= glm::normalize(m_look_at);
      break;

    case SDL_SCANCODE_KP_4: // left
      {
        glm::vec3 dir = glm::normalize(m_look_at);
        dir = glm::rotate(dir, 90.0f, m_up);
        m_eye += dir;
      }
      break;

    case SDL_SCANCODE_KP_6: // right
      {
        glm::vec3 dir = glm::normalize(m_look_at);
        dir = glm::rotate(dir, 90.0f, m_up);
        m_eye -= dir;
      }
      break;

    case SDL_SCANCODE_KP_7: // kp_pos1
      m_look_at = glm::rotate(m_look_at, 5.0f, m_up);
      break;

    case SDL_SCANCODE_KP_9: // kp_raise
      m_look_at = glm::rotate(m_look_at, -5.0f, m_up);
      break;

    case SDL_SCANCODE_KP_1:
      m_eye -= glm::normalize(m_up);
      break;

    case SDL_SCANCODE_KP_3:
      m_eye += glm::normalize(m_up);
      break;

    case SDL_SCANCODE_KP_MULTIPLY:
      m_fov += glm::radians(1.0f);
      break;

    case SDL_SCANCODE_KP_DIVIDE:
      m_fov -= glm::radians(1.0f);
      break;

    case SDL_SCANCODE_F1:
      {
        // Hitchcock zoom in
        //float old_eye_z = m_eye.z;
        //g_eye.z *= 1.005f;
        //g_fov = m_fov / std::atan(1.0f / old_eye_z) * std::atan(1.0f / m_eye.z);

        float old_fov = m_fov;
        m_fov += glm::radians(1.0f);
        if (m_fov < glm::radians(160.0f))
        {
          m_eye.z = m_eye.z
            * (2.0*tan(0.5 * old_fov))
            / (2.0*tan(0.5 * m_fov));
        }
        else
        {
          m_fov = 160.0f;
        }
        log_info("fov: %5.2f %f", m_fov, m_eye.z);
        log_info("w: %f", tan(m_fov /2.0f) * m_eye.z);
      }
      break;

    case SDL_SCANCODE_F2:
      // Hitchcock zoom out
      {
        //float old_eye_z = m_eye.z;
        //g_eye.z /= 1.005f;
        //g_fov = m_fov / std::atan(1.0f / old_eye_z) * std::atan(1.0f / m_eye.z);

        float old_fov = m_fov;
        m_fov -= 1.0f;
        if (m_fov >= 7.0f)
        {
          m_eye.z = m_eye.z
            * (2.0*tan(0.5 * old_fov))
            / (2.0*tan(0.5 * m_fov));
        }
        else
        {
          m_fov = 7.0f;
        }
        log_info("fov: %5.2f %f", m_fov, m_eye.z);
        log_info("w: %f", tan(m_fov/2.0f) * m_eye.z);
      }
      break;

    case SDL_SCANCODE_F10:
      //glutReshapeWindow(1600, 1000);
      break;

    case SDL_SCANCODE_F11:
      //glutFullScreen();
      break;

    case SDL_SCANCODE_UP:
      m_convergence *= 1.1f;
      break;

    case SDL_SCANCODE_DOWN:
      m_convergence /= 1.1f;
      break;

    case SDL_SCANCODE_LEFT:
      //g_look_at.x += 1.0f;
      break;

    case SDL_SCANCODE_RIGHT:
      //g_look_at.x -= 1.0f;
      break;

    case SDL_SCANCODE_PAGEUP:
      //g_look_at.y -= 1.0f;
      break;

    case SDL_SCANCODE_PAGEDOWN:
      //g_look_at.y += 1.0f;
      break;

    default:
      log_info("unknown key: %d", static_cast<int>(key.keysym.sym));
      break;
  }
}

void
Viewer::init()
{
  assert_gl("init()");

  m_compositor = std::make_unique<Compositor>(m_screen_w, m_screen_h);

  assert_gl("init()");

  //g_armature = Armature::from_file("/tmp/blender.bones");
  //g_pose = Pose::from_file("/tmp/blender.pose");

  //m_composition_prog = Program::create(Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/newsprint.frag"));
  m_composition_prog = Program::create(Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/composite.frag"),
                                       Shader::from_file(GL_VERTEX_SHADER, "src/glsl/composite.vert"));

  {
    m_scene_manager = std::make_unique<SceneManager>();

    {
      MaterialPtr material = std::make_unique<Material>();
      material->cull_face(GL_FRONT);
      material->enable(GL_CULL_FACE);
      material->enable(GL_DEPTH_TEST);
      material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);
      material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/glsl/shadowmap.vert"),
                                            Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/shadowmap.frag")));
      m_scene_manager->set_override_material(material);
    }

    m_camera = std::make_unique<Camera>();
    m_camera->perspective(m_fov, m_aspect_ratio, m_near_z, 100000.0f);

    if (m_video_player) // streaming video
    {
      if (!m_opts.video3d)
      {
        m_video_material = MaterialFactory::get().create("video");
        m_video_material_flip = m_video_material;
      }
      else
      {
        m_video_material = MaterialFactory::get().create("video3d");
        m_video_material_flip = MaterialFactory::get().create("video3d-flip");
      }

      if (false)
      {
        auto node = m_scene_manager->get_world()->create_child();
        ModelPtr model = std::make_shared<Model>();

        model->add_mesh(Mesh::create_plane(5.0f));
        node->set_position(glm::vec3(0.0f, 0.0f, -10.0f));
        node->set_orientation(glm::quat(glm::vec3(glm::half_pi<float>(), 0.0f, 0.0f)));
        node->set_scale(glm::vec3(4.0f, 1.0f, 2.25f));

        model->set_material(m_video_material);
        node->attach_model(model);
      }
      else
      {
        auto node = m_scene_manager->get_world()->create_child();

        int rings = 32;
        int segments = 32;

        float hfov = glm::radians(360.0f);
        float vfov = glm::radians(180.0f);

        //float hfov = glm::radians(90.0f);
        //float vfov = glm::radians(64.0f);

        //float hfov = glm::radians(125.0f);
        //float vfov = glm::radians(70.3f);

        //float hfov = glm::radians(90.0f);
        //float vfov = glm::radians(50.0f);

        ModelPtr model = std::make_shared<Model>();
        model->set_material(m_video_material);
        model->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments));
        node->attach_model(model);

        if (false)
        {
          model->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 0, 16, false, true));
          model->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 0, -16, false, true));

          ModelPtr model_flip = std::make_shared<Model>();

          model_flip->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 16, 0, true, false));
          model_flip->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, -16, 0, true, false));

          model_flip->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 16, 16, true, true));
          model_flip->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, -16, 16, true, true));

          model_flip->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 16, -16, true, true));
          model_flip->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, -16, -16, true, true));

          model_flip->set_material(m_video_material_flip);
          node->attach_model(model_flip);
        }
      }
    }

    MaterialPtr phong_material = MaterialFactory::get().create("phong");
    if (false)
    {
      // little animated planetary system thing
      if (false)
      {
        auto node = m_scene_manager->get_world()->create_child();
        node->set_position(glm::vec3(0, 0, 0));
        auto mesh = Mesh::create_plane(75.0f);
        ModelPtr model = std::make_shared<Model>();
        model->add_mesh(std::move(mesh));
        model->set_material(phong_material);
        node->attach_model(model);
      }

      auto mesh = Mesh::create_sphere(0.2, 8, 16);
      ModelPtr model = std::make_shared<Model>();
      model->add_mesh(std::move(mesh));
      model->set_material(phong_material);

      auto origin = m_scene_manager->get_world()->create_child();
      origin->set_position(glm::vec3(5, 5, 5));

      auto sun = origin->create_child();
      sun->set_scale(glm::vec3(3.0f, 3.0f, 3.0f));
      sun->attach_model(model);
      m_nodes.push_back(sun);

      auto earth = sun->create_child();
      earth->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));
      earth->set_position(glm::vec3(2.0f, 0, 0));
      earth->attach_model(model);
      m_nodes.push_back(earth);

      auto moon = earth->create_child();
      moon->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));
      moon->set_position(glm::vec3(2, 0, 0));
      moon->attach_model(model);
      //g_nodes.push_back(moon);
    }

    if (false)
    {
      // some spheres in a line
      auto root_parent = m_scene_manager->get_world()->create_child();
      auto parent = root_parent;
      parent->set_position(glm::vec3(10.0f, 0.0f, 0.0f));
      for(int i = 0; i < 5; ++i)
      {
        auto child = parent->create_child();
        child->set_position(glm::vec3(1.0f, 0.0f, -3.0f));
        ModelPtr model = std::make_shared<Model>();
        model->add_mesh(Mesh::create_sphere(0.5f, 16, 8));
        model->set_material(phong_material);
        child->attach_model(model);

        parent = child;
      }

      print_scene_graph(root_parent);
    }

    if (!m_opts.model.empty())
    { // load a mesh from file
      auto node = Scene::from_file(m_opts.model);

      std::cout << "SceneGraph:\n";
      print_scene_graph(node.get());

      m_scene_manager->get_world()->attach_child(std::move(node));
    }

    if (false)
    {
      {
        auto node = Scene::from_file("data/wiimote.mod");
        m_wiimote_gyro_node = node.get();
        m_scene_manager->get_world()->attach_child(std::move(node));
      }

      {
        auto node = Scene::from_file("data/wiimote.mod");
        m_wiimote_node = node.get();
        m_scene_manager->get_world()->attach_child(std::move(node));
      }

      {
        auto node = Scene::from_file("data/wiimote.mod");
        m_wiimote_accel_node = node.get();
        m_scene_manager->get_world()->attach_child(std::move(node));
      }
    }

    if (true)
    { // create a skybox
      auto mesh = Mesh::create_skybox(500.0f);
      ModelPtr model = std::make_shared<Model>();
      model->add_mesh(std::move(mesh));
      model->set_material(MaterialFactory::get().create("skybox"));

      auto node = m_scene_manager->get_world()->create_child();
      node->attach_model(model);
    }

    if (false)
    { // light cone
      MaterialPtr material = std::make_shared<Material>();
      material->blend_func(GL_SRC_ALPHA, GL_ONE);
      material->depth_mask(false);
      material->cast_shadow(false);
      material->enable(GL_BLEND);
      material->enable(GL_DEPTH_TEST);
      material->enable(GL_POINT_SPRITE);
      material->enable(GL_PROGRAM_POINT_SIZE);
      material->set_texture(0, Texture::create_lightspot(256, 256));
      material->set_uniform("diffuse_texture", 0);
      material->set_uniform("ModelViewMatrix", UniformSymbol::ModelViewMatrix);
      material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);
      material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/glsl/lightcone.vert"),
                                            Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/lightcone.frag")));

      auto mesh = std::make_unique<Mesh>(GL_POINTS);
      // generate light cone mesh
      {
        std::vector<glm::vec3> position;
        std::vector<float> point_size;
        std::vector<float> alpha;
        int steps = 30;
        float length = 3.0f;
        float size   = 1000.0f;
        int start = 10; // skip the first few sprites to avoid a spiky look
        for(int i = start; i < steps; ++i)
        {
          float progress = static_cast<float>(i) / static_cast<float>(steps-1);
          progress = progress * progress;

          point_size.push_back(size * progress);
          position.emplace_back(length * progress, 0.0f, 0.0f);
          alpha.push_back(0.25 * (1.0f - progress));
        }
        mesh->attach_float_array("position", position);
        mesh->attach_float_array("point_size", point_size);
        mesh->attach_float_array("alpha", alpha);
      }

      ModelPtr model = std::make_shared<Model>();
      model->add_mesh(std::move(mesh));
      model->set_material(material);

      for(int i = 0; i < 10; ++i)
      {
        auto node = m_scene_manager->get_world()->create_child();
        node->set_position(glm::vec3(1.0f, i * 5.0f, -1.0f));
        node->set_orientation(glm::quat());
        node->attach_model(model);
      }
    }
  }

  m_calibration_left_texture  = Texture::from_file("data/calibration_left.png", false);
  m_calibration_right_texture = Texture::from_file("data/calibration_right.png", false);

  m_dot_surface = TextSurface::create("+", TextProperties().set_line_width(3.0f));

  m_menu = std::make_unique<Menu>(TextProperties().set_font_size(24.0f).set_line_width(4.0f));
  //g_menu->add_item("eye.x", &g_eye.x);
  //g_menu->add_item("eye.y", &g_eye.y);
  //g_menu->add_item("eye.z", &g_eye.z);

#if 0
  m_menu->add_item("wiimote.camera_control", &m_wiimote_camera_control);

  m_menu->add_item("slowfactor", &m_slow_factor, 0.01f, 0.0f);

  m_menu->add_item("depth.near_z", &m_near_z, 0.01, 0.0f);
  m_menu->add_item("depth.far_z",  &m_far_z, 1.0f);

  m_menu->add_item("convergence", &m_convergence, 0.1f);

  //g_menu->add_item("viewport.offset.x",  &g_viewport_offset.x, 1);
  //g_menu->add_item("viewport.offset.y",  &g_viewport_offset.y, 1);

  m_menu->add_item("shadowmap.fov", &m_shadowmap_fov, 1.0f);

  //g_menu->add_item("spot_halo_samples",  &g_spot_halo_samples, 1, 0);

  m_menu->add_item("FOV", &m_fov);
  m_menu->add_item("Barrel Power", &m_barrel_power, 0.01f);
  //g_menu->add_item("AspectRatio", &m_aspect_ratio, 0.05f, 0.5f, 4.0f);

  //g_menu->add_item("scale", &m_scale, 0.5f, 0.0f);
  m_menu->add_item("eye.distance", &m_eye_distance, 0.1f);

  //g_menu->add_item("spot.cutoff",   &m_spot_cutoff);
  //g_menu->add_item("spot.exponent", &m_spot_exponent);

  //g_menu->add_item("light.up",  &m_light_up, 1.0f);
  //g_menu->add_item("light.angle",  &m_light_angle, 1.0f);
  //g_menu->add_item("light.diffuse",  &m_light_diffuse, 0.1f, 0.0f);
  //g_menu->add_item("light.specular", &m_light_specular, 0.1f, 0.0f);
  //g_menu->add_item("material.shininess", &m_material_shininess, 0.1f, 0.0f);

  m_menu->add_item("wiimote.distance_scale",  &m_distance_scale, 0.01f);
  m_menu->add_item("wiimote.scale_x", &m_wiimote_scale.x, 0.01f);
  m_menu->add_item("wiimote.scale_y", &m_wiimote_scale.y, 0.01f);

  //g_menu->add_item("3D", &m_draw_3d);
  //g_menu->add_item("Headlights", &m_headlights);
  //g_menu->add_item("Look At Sphere", &m_draw_look_at);
  //g_menu->add_item("draw depth", &m_draw_depth);
  //g_menu->add_item("shadow map", &m_render_shadowmap);
  //g_menu->add_item("grid.size", &m_grid_size, 0.5f);

#endif
  assert_gl("init()");
}

glm::vec3
Viewer::get_arcball_vector(glm::ivec2 mouse)
{
  float radius = std::min(m_screen_w, m_screen_h) / 2.0f;
  glm::vec3 P = glm::vec3(static_cast<float>(mouse.x - m_screen_w/2) / radius,
                          static_cast<float>(mouse.y - m_screen_h/2) / radius,
                          0);

  //log_info("arcball: %f %f", P.x, P.y);
  P.y = -P.y;
  float OP_squared = P.x * P.x + P.y * P.y;
  if (glm::length(P) <= 1)
  {
    P.z = sqrt(1*1 - OP_squared);  // Pythagore
  }
  else
  {
    P = glm::normalize(P);  // nearest point
  }
  return P;
}

void
Viewer::process_events(GameController& gamecontroller)
{
  SDL_Event ev;
  while(SDL_PollEvent(&ev))
  {
    switch(ev.type)
    {
      case SDL_WINDOWEVENT:
        switch (ev.window.event)
        {
          case SDL_WINDOWEVENT_RESIZED:
            m_compositor->reshape(*this, ev.window.data1, ev.window.data2);
            break;

          default:
            break;
        }
        break;

      case SDL_QUIT:
        exit(EXIT_SUCCESS);
        break;

      case SDL_KEYUP:
        break;

      case SDL_KEYDOWN:
        on_keyboard_event(ev.key, m_mouse_x, m_mouse_y);
        break;

      case SDL_MOUSEMOTION:
        m_mouse_x = ev.motion.x;
        m_mouse_y = ev.motion.y;
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        break;

      case SDL_CONTROLLERAXISMOTION:
        //log_debug("controller axis: %d %d", static_cast<int>(ev.caxis.axis), ev.caxis.value);
        switch(ev.caxis.axis)
        {
          case SDL_CONTROLLER_AXIS_LEFTX:
            m_stick.dir.x = -ev.caxis.value / 32768.0f;
            break;

          case SDL_CONTROLLER_AXIS_LEFTY:
            m_stick.dir.z = -ev.caxis.value / 32768.0f;
            break;

          case SDL_CONTROLLER_AXIS_TRIGGERLEFT:
          case SDL_CONTROLLER_AXIS_TRIGGERRIGHT:
            m_stick.rot.z = (SDL_GameControllerGetAxis(gamecontroller.get_handle(), SDL_CONTROLLER_AXIS_TRIGGERRIGHT) -
                             SDL_GameControllerGetAxis(gamecontroller.get_handle(), SDL_CONTROLLER_AXIS_TRIGGERLEFT)) / 32768.0f;
            break;

          case SDL_CONTROLLER_AXIS_RIGHTX:
            m_stick.rot.y = -ev.caxis.value / 32768.0f;
            break;

          case SDL_CONTROLLER_AXIS_RIGHTY:
            m_stick.rot.x = -ev.caxis.value / 32768.0f;
            break;
        }
        break;

      case SDL_CONTROLLERBUTTONUP:
      case SDL_CONTROLLERBUTTONDOWN:
        //log_debug("controller button: %d %d", static_cast<int>(ev.cbutton.button), static_cast<int>(ev.cbutton.state));
        switch(ev.cbutton.button)
        {
          case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:
            m_stick.dir.y = -ev.cbutton.state;
            break;

          case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER:
            m_stick.dir.y = +ev.cbutton.state;
            break;

          case SDL_CONTROLLER_BUTTON_B:
            if (m_wiimote_manager)
            {
              m_wiimote_manager->reset_gyro_orientation();
            }
            break;

          case SDL_CONTROLLER_BUTTON_X:
            //g_light_angle += 1.0f;
            m_stick.light_rotation = ev.cbutton.state;
            break;

          case SDL_CONTROLLER_BUTTON_Y:
            m_headlights = ev.cbutton.state;
            break;

          case SDL_CONTROLLER_BUTTON_START:
            if (ev.cbutton.state)
            {
              m_show_menu = !m_show_menu;
            }
            break;

          case SDL_CONTROLLER_BUTTON_BACK:
            if (ev.cbutton.state)
            {
              m_show_dots = !m_show_dots;
            }
            break;

          case SDL_CONTROLLER_BUTTON_DPAD_UP:
            if (ev.cbutton.state)
            {
              m_stick.hat |= SDL_HAT_UP;
              m_hat_autorepeat = SDL_GetTicks() + 500;
            }
            else
            {
              m_stick.hat &= ~SDL_HAT_UP;
            }
            break;

          case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
            if (ev.cbutton.state)
            {
              m_stick.hat |= SDL_HAT_DOWN;
              m_hat_autorepeat = SDL_GetTicks() + 500;
            }
            else
            {
              m_stick.hat &= ~SDL_HAT_DOWN;
            }
            break;

          case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
            if (ev.cbutton.state)
            {
              m_stick.hat |= SDL_HAT_LEFT;
              m_hat_autorepeat = SDL_GetTicks() + 500;
            }
            else
            {
              m_stick.hat &= ~SDL_HAT_LEFT;
            }
            break;

          case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
            if (ev.cbutton.state)
            {
              m_stick.hat |= SDL_HAT_RIGHT;
              m_hat_autorepeat = SDL_GetTicks() + 500;
            }
            else
            {
              m_stick.hat &= ~SDL_HAT_RIGHT;
            }
            break;
        }
        break;

      case SDL_JOYAXISMOTION:
        //log_debug("joystick axis: %d %d", static_cast<int>(ev.jaxis.axis), ev.jaxis.value);
        if ((false))
        {
          switch(ev.jaxis.axis)
          {
            case 0: // x1
              m_stick.dir.x = -ev.jaxis.value / 32768.0f;
              break;

            case 1: // y1
              m_stick.dir.z = -ev.jaxis.value / 32768.0f;
              break;

            case 2: // z
              m_stick.rot.z = ev.jaxis.value / 32768.0f;
              break;

            case 3: // x2
              m_stick.rot.y = -ev.jaxis.value / 32768.0f;
              break;

            case 4: // y2
              m_stick.rot.x = -ev.jaxis.value / 32768.0f;
              break;
          }
        }
        break;

      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
        //log_debug("joystick button: %d %d", static_cast<int>(ev.jbutton.button), static_cast<int>(ev.jbutton.state));
        if ((false))
        {
          switch(ev.jbutton.button)
          {
            case 4:
              m_stick.dir.y = -ev.jbutton.state;
              break;

            case 5:
              m_stick.dir.y = +ev.jbutton.state;
              break;

            case 0:
              if (m_wiimote_manager)
              {
                m_wiimote_manager->reset_gyro_orientation();
              }
              break;

            case 1:
              //g_light_angle += 1.0f;
              m_stick.light_rotation = ev.jbutton.state;
              break;

            case 2:
              m_headlights = ev.jbutton.state;
              break;

            case 7:
              if (ev.jbutton.state)
              {
                m_show_menu = !m_show_menu;
              }
              break;

            case 6:
              if (ev.jbutton.state)
              {
                m_show_dots = !m_show_dots;
              }
              break;
          }
        }
        break;

      case SDL_JOYHATMOTION:
        if ((false))
        {
          m_stick.hat = ev.jhat.value;
          m_hat_autorepeat = SDL_GetTicks() + 500;
        }
        break;

      default:
        break;
    }
  }
}

void
Viewer::process_joystick(float dt)
{
  auto current_time = SDL_GetTicks();
  if (m_stick.hat != m_old_stick.hat || (m_stick.hat && m_hat_autorepeat < current_time))
  {
    if (m_hat_autorepeat < current_time)
    {
      m_hat_autorepeat = current_time + 100 + (m_hat_autorepeat - current_time);
    }

    if (m_stick.hat & SDL_HAT_UP)
    {
      m_menu->up();
    }

    if (m_stick.hat & SDL_HAT_DOWN)
    {
      m_menu->down();
    }

    if (m_stick.hat & SDL_HAT_LEFT)
    {
      m_menu->left();
    }

    if (m_stick.hat & SDL_HAT_RIGHT)
    {
      m_menu->right();
    }
  }

  float deadzone = 0.2f;
  if (fabs(m_stick.dir.x) < deadzone) m_stick.dir.x = 0.0f;
  if (fabs(m_stick.dir.y) < deadzone) m_stick.dir.y = 0.0f;
  if (fabs(m_stick.dir.z) < deadzone) m_stick.dir.z = 0.0f;

  if (fabs(m_stick.rot.x) < deadzone) m_stick.rot.x = 0.0f;
  if (fabs(m_stick.rot.y) < deadzone) m_stick.rot.y = 0.0f;
  if (fabs(m_stick.rot.z) < deadzone) m_stick.rot.z = 0.0f;

  if (false)
    log_debug("stick: %2.2f %2.2f %2.2f  -  %2.2f %2.2f %2.2f",
              m_stick.dir.x, m_stick.dir.y, m_stick.dir.z,
              m_stick.rot.x, m_stick.rot.y, m_stick.rot.z);

  float delta = dt * 5.0f * m_slow_factor;

  if (m_stick.light_rotation)
  {
    //log_debug("light angle: %f", m_light_angle);
    m_light_angle += delta * 30.0f;
  }

  if (false)
  { // free flight mode
    {
      // forward/backward
      m_eye += glm::normalize(m_look_at) * m_stick.dir.z * delta;

      // up/down
      m_eye += glm::normalize(m_up) * m_stick.dir.y * delta;

      // left/right
      glm::vec3 dir = glm::normalize(m_look_at);
      dir = glm::rotate(dir, 90.0f, m_up);
      m_eye += glm::normalize(dir) * m_stick.dir.x * delta;
    }

    { // handle rotation
      float angle_d = 20.0f;

      // yaw
      m_look_at = glm::rotate(m_look_at, angle_d * m_stick.rot.y * delta, m_up);

      // roll
      m_up = glm::rotate(m_up, angle_d * m_stick.rot.z * delta, m_look_at);

      // pitch
      glm::vec3 cross = glm::cross(m_look_at, m_up);
      m_up = glm::rotate(m_up, angle_d * m_stick.rot.x * delta, cross);
      m_look_at = glm::rotate(m_look_at, angle_d * m_stick.rot.x * delta, cross);
    }
  }
  else
  { // fps mode
    float focus_distance = glm::length(m_look_at);
    auto tmp = m_look_at;
    auto xz_dist = glm::sqrt(tmp.x * tmp.x + tmp.z * tmp.z);
    float pitch = glm::atan(tmp.y, xz_dist);
    float yaw   = glm::atan(tmp.z, tmp.x);

    yaw   += -m_stick.rot.y * 2.0f * dt;
    pitch += m_stick.rot.x * 2.0f * dt;

    pitch = glm::clamp(pitch, -glm::half_pi<float>() + 0.001f, glm::half_pi<float>() - 0.001f);

    if (false && m_wiimote_camera_control)
    {
      pitch = 0.0f;
    }

    glm::vec3 forward(glm::cos(yaw), 0.0f, glm::sin(yaw));

    //log_debug("focus distance: %f", focus_distance);
    //log_debug("yaw: %f pitch: %f", yaw, pitch);

    // forward/backward
    m_eye += 10.0f * forward * m_stick.dir.z * dt * m_slow_factor;

    // strafe
    m_eye += 10.0f * glm::vec3(forward.z, 0.0f, -forward.x) * m_stick.dir.x * dt * m_slow_factor;

    // up/down
    m_eye.y += 10.0f * m_stick.dir.y * dt * m_slow_factor;

    m_look_at = focus_distance * glm::vec3(glm::cos(pitch) * glm::cos(yaw),
                                           glm::sin(pitch),
                                           glm::cos(pitch) * glm::sin(yaw));

    float f = sqrt(m_look_at.x * m_look_at.x + m_look_at.z * m_look_at.z);
    m_up.x = -m_look_at.x/f * m_look_at.y;
    m_up.y = f;
    m_up.z = -m_look_at.z/f * m_look_at.y;
    m_up = glm::normalize(m_up);
  }

  m_old_stick = m_stick;
}

void
Viewer::update_arcball()
{
  if (m_arcball_active && m_mouse != m_last_mouse)
  {
    glm::mat4 camera_matrix;// = m_object2world;

    //glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(camera_matrix));

    glm::vec3 va = get_arcball_vector(m_last_mouse);
    glm::vec3 vb = get_arcball_vector(m_mouse);
    float angle = acos(std::min(1.0f, glm::dot(va, vb)));
    glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
    glm::mat3 camera2object = glm::inverse(glm::mat3(camera_matrix) * glm::mat3(m_last_object2world));
    glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
    m_object2world = glm::rotate(m_last_object2world, angle, axis_in_object_coord);
    //g_last_mouse = m_mouse;
  }
}

void
Viewer::update_world(float dt)
{
  int i = 1;
  for(auto& node : m_nodes)
  {
    float f = SDL_GetTicks()/1000.0f;
    node->set_orientation(glm::quat(glm::vec3(0.0f, f*1.3*static_cast<float>(i), 0.0f)));
    i += 3;
  }
}

void
Viewer::main_loop(Window& window, GameController& gamecontroller)
{
  int num_frames = 0;
  unsigned int start_ticks = SDL_GetTicks();

  int ticks = SDL_GetTicks();
  while(true)
  {
    int next = SDL_GetTicks();
    int delta = next - ticks;
    ticks = next;
    update_world(delta / 1000.0f);

    m_compositor->render(*this);
    window.swap();

    SDL_Delay(1);

    m_grid_offset += glm::vec4(0.0f, 0.0f, 0.001f, 0.0f);

    process_events(gamecontroller);
    process_joystick(delta / 1000.0f);

    if (false && m_wiimote_manager)
    {
      //g_wiimote_manager->update();
      //g_wiimote_manager->get_accumulated();
      m_wiimote_gyro_node->set_orientation(m_wiimote_manager->get_gyro_orientation());
      m_wiimote_gyro_node->set_position(glm::vec3(-0.1f, 0.0f, -0.5f));

      m_wiimote_node->set_orientation(m_wiimote_manager->get_orientation());
      m_wiimote_node->set_position(glm::vec3(0.0f, 0.0f, -0.5f));

      m_wiimote_accel_node->set_orientation(m_wiimote_manager->get_accel_orientation());
      m_wiimote_accel_node->set_position(glm::vec3(0.1f, 0.0f, -0.5f));
    }

    num_frames += 1;

    if (num_frames > 100)
    {
      int t = SDL_GetTicks() - start_ticks;
      std::cout << "frames: " << num_frames << " time: " << t
                << " frame_delay: " << static_cast<float>(t) / static_cast<float>(num_frames)
                << " fps: " << static_cast<float>(num_frames) / static_cast<float>(t) * 1000.0f
                << std::endl;

      num_frames = 0;
      start_ticks = SDL_GetTicks();
    }

    if (m_video_player)
    {
      m_video_player->update();
      TexturePtr texture = m_video_player->get_texture();
      if (texture)
      {
        m_video_material->set_texture(0, texture);
        if (m_video_material_flip) m_video_material_flip->set_texture(0, texture);
      }
    }
  }
}

void
Viewer::update_offsets(glm::vec2 p1, glm::vec2 p2)
{
  if (p1.x > p2.x)
  {
    std::swap(p1, p2);
  }

  glm::vec2 r = p2 - p1;
  float angle = glm::atan(-r.y, r.x);
  m_roll_offset = angle;
  m_distance_offset = m_distance_scale * glm::length(r) / 2.0f * glm::tan(glm::radians(m_fov));
  glm::vec2 c = (p1+p2)/2.0f;

  c -= glm::vec2(512, 384);
  c = glm::rotate(c, m_roll_offset);
  c += glm::vec2(512, 384);

  m_yaw_offset   = ((c.x / 1024.0f) - 0.5f) * glm::half_pi<float>() * m_wiimote_scale.x;
  m_pitch_offset = ((c.y /  768.0f) - 0.5f) * glm::half_pi<float>() * m_wiimote_scale.y;

  m_wiimote_dot1.x = p1.x / 1024.0f;
  m_wiimote_dot1.y = 1.0f - (p1.y /  768.0f);
  m_wiimote_dot2.x = p2.x / 1024.0f;
  m_wiimote_dot2.y = 1.0f - (p2.y /  768.0f);

  //std::cout << "offset: " << boost::format("%4.2f %4.2f %4.2f") % m_roll_offset % m_yaw_offset % m_pitch_offset << std::endl;
}

void
Viewer::parse_args(int argc, char** argv, Options& opts)
{
  for(int i = 1; i < argc; ++i)
  {
    if (argv[i][0] == '-')
    {
      if (strcmp("--wiimote", argv[i]) == 0)
      {
        opts.wiimote = true;
      }
      else if (strcmp("--video", argv[i]) == 0)
      {
        opts.video = argv[i+1];
        ++i;
      }
      else if (strcmp("--video3d", argv[i]) == 0)
      {
        opts.video3d = true;
        opts.video = argv[i+1];
        ++i;
      }
      else if (strcmp("--help", argv[i]) == 0 ||
               strcmp("-h", argv[i]) == 0)
      {
        std::cout << "Usage: " << argv[0] << " [OPTIONS]\n"
                  << "\n"
                  << "Options:\n"
                  << "  --wiimote       Enable Wiimote support\n"
                  << "  --video FILE    Play video\n"
                  << "  --video3d FILE  Play 3D video\n";
        exit(0);
      }
      else
      {
        throw std::runtime_error("unknown option: " + std::string(argv[i]));
      }
    }
    else
    {
      opts.model = argv[i];
    }
  }
}

int
Viewer::main(int argc, char** argv)
{
  parse_args(argc, argv, m_opts);

  System system = System::create();
  Window window = system.create_gl_window("OpenGL Viewer", m_screen_w, m_screen_h, false, 0);
  //Joystick joystick = system.create_joystick();
  GameController gamecontroller = system.create_gamecontroller();

  // glew throws 'invalid enum' error in OpenGL3.3Core, thus we eat up the error code
  glewExperimental = true;
  glewInit();
  glGetError();

  // In OpenGL3.3Core VAO are mandatory, this hack might work
  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  if (m_opts.wiimote)
  {
    m_wiimote_manager = std::make_shared<WiimoteManager>();
  }

  if (!m_opts.video.empty())
  {
    Gst::init(argc, argv);
    std::cout << "Playing video: " << m_opts.video << std::endl;
    m_video_player = std::make_shared<VideoProcessor>(m_opts.video);
  }

  init();

  std::cout << "main: " << std::this_thread::get_id() << std::endl;

  main_loop(window, gamecontroller);

  return 0;
}

int
main(int argc, char** argv)
{
  Viewer viewer;
  return viewer.main(argc, argv);
}

// EOF //
