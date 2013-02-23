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

#include <GL/glew.h>
#include <SDL.h>
#include <SDL_image.h>
#include <cmath>
#include <cmath>
//#include <cwiid.h>
#include <fstream>
#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <unistd.h>
#include <vector>
#include <thread>

#include "armature.hpp"
#include "assert_gl.hpp"
#include "camera.hpp"
#include "framebuffer.hpp"
#include "renderbuffer.hpp"
#include "log.hpp"
#include "material_factory.hpp"
#include "menu.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "opengl_state.hpp"
#include "pose.hpp"
#include "program.hpp"
#include "render_context.hpp"
#include "scene.hpp"
#include "scene_manager.hpp"
#include "shader.hpp"
#include "text_surface.hpp"
#include "video_processor.hpp"
#include "wiimote_manager.hpp"

void draw_models(bool shader_foo);

std::string to_string(const glm::vec3& v)
{
  std::ostringstream str;
  str << "vec3(" << v.x << ", "  << v.y << ", " << v.z << ")";
  return str.str();
}

std::string to_string(const glm::vec4& v)
{
  std::ostringstream str;
  str << "vec4(" << v.x << ", "  << v.y << ", " << v.z << ", "  << v.w << ")";
  return str.str();
}

std::string to_string(const glm::quat& q)
{
  std::ostringstream str;
  str << "quat(" << q.w << ", " << q.x << ", "  << q.y << ", " << q.x << ", "  << q.z << ")";
  return str.str();
}

void print_scene_graph(SceneNode* node, int depth = 0)
{
  std::cout << std::string(depth, ' ') << node 
            << ": " << to_string(node->get_position())
            << " " << to_string(node->get_scale())
            << " " << to_string(node->get_orientation()) << std::endl;
  for(const auto& child : node->get_children())
  {
    print_scene_graph(child.get(), depth+1);
  }
}

struct Options
{
  bool wiimote = false;
  std::string video = std::string();
  bool video3d = false;
  std::string model = std::string();
};

// global variables
namespace {

Options g_opts;

int g_mouse_x = 0;
int g_mouse_y = 0;

SDL_Surface* g_screen = nullptr;

TexturePtr g_calibration_texture;
bool g_show_calibration = false;

std::unique_ptr<Menu> g_menu;
std::unique_ptr<SceneManager> g_scene_manager;
std::unique_ptr<Camera> g_camera;

MaterialPtr g_video_material;
MaterialPtr g_video_material_flip;
std::shared_ptr<VideoProcessor> g_video_player;

float g_slow_factor = 0.5f;

bool g_wiimote_camera_control = false;

glm::ivec2 g_viewport_offset(-41, 16);
float g_barrel_power = 0.05f;
float g_ipd = 0.0f;
int g_screen_w = 640;
int g_screen_h = 480;
//float g_fov = glm::radians(56.0f);
float g_fov = glm::radians(42.0f);

float g_near_z = 0.1f;
float g_far_z  = 1000.0f;

int g_spot_halo_samples = 100;

bool g_draw_look_at = false;

float g_light_angle = 0.0f;

enum class StereoMode { None, CrossEye, Cybermaxx, Anaglyph, End };
StereoMode g_stereo_mode = StereoMode::None;

bool g_headlights = false;
bool g_draw_grid = false;
bool g_draw_depth = false;
bool g_render_shadowmap = true;

int g_shadowmap_resolution = 1024;

float g_shadowmap_fov = glm::radians(25.0f);
float g_light_diffuse = 1.0f;
float g_light_specular = 1.0f;
float g_material_shininess = 10.0f;
float g_light_up = 0.0f;

float g_aspect_ratio = static_cast<GLfloat>(g_screen_w)/static_cast<GLfloat>(g_screen_h);
float g_spot_cutoff   = 60.0f;
float g_spot_exponent = 30.0f;

bool g_show_menu = true;
bool g_show_dots = true;

glm::vec3 g_eye(0.0f, 0.0f, 0.0f);
glm::vec3 g_look_at(0.0f, 0.0f, -1.0f);
glm::vec3 g_up(0.0f, 1.0f, 0.0f);
float g_pitch_offset = 0.0f;
float g_roll_offset  = 0.0f;
float g_distance_offset = 0.0f;
float g_distance_scale = 0.000f;
float g_yaw_offset   = 0.0f;
glm::vec4 g_grid_offset;
float g_grid_size = 2.0f;

std::string g_model_filename;
std::unique_ptr<Armature> g_armature;
std::unique_ptr<Pose> g_pose;

std::unique_ptr<Framebuffer> g_framebuffer1;
std::unique_ptr<Framebuffer> g_framebuffer2;

std::unique_ptr<Renderbuffer> g_renderbuffer1;
std::unique_ptr<Renderbuffer> g_renderbuffer2;

float g_scale = 1.0f;

enum EyeType { kLeftEye, kRightEye, kCenterEye };
float g_eye_distance = 0.065f;
float g_convergence = 1.0f;

bool g_arcball_active = false;
glm::ivec2 g_mouse;
glm::ivec2 g_last_mouse;
glm::mat4 g_object2world;
glm::mat4 g_last_object2world;
glm::mat4 g_eye_matrix;

TextSurfacePtr g_dot_surface;
glm::vec2 g_wiimote_dot1;
glm::vec2 g_wiimote_dot2;

//glm::vec2 g_wiimote_scale(0.84f, 0.64f);
glm::vec2 g_wiimote_scale(0.52f, 0.47f);

ProgramPtr m_composition_prog;

SceneNode* g_wiimote_accel_node = 0;
SceneNode* g_wiimote_gyro_node = 0;
SceneNode* g_wiimote_node = 0;
std::vector<SceneNode*> g_nodes;

//cwiid_wiimote_t* g_wiimote = 0;
std::shared_ptr<WiimoteManager> g_wiimote_manager;

} // namespace

std::unique_ptr<Framebuffer> g_shadowmap;
glm::mat4 g_shadowmap_matrix;

struct Stick
{
  Stick() : dir(), rot(), light_rotation(), hat() {}
  glm::vec3 dir;
  glm::vec3 rot;
  bool light_rotation;
  unsigned int hat;
};

Stick g_stick;
Stick g_old_stick;
unsigned int g_hat_autorepeat = 0;

void reshape(int w, int h)
{
  log_info("reshape(%d, %d)", w, h);
  g_screen_w = w;
  g_screen_h = h;

  assert_gl("reshape1");

  g_framebuffer1.reset(new Framebuffer(g_screen_w, g_screen_h));
  g_framebuffer2.reset(new Framebuffer(g_screen_w, g_screen_h));

  g_renderbuffer1.reset(new Renderbuffer(g_screen_w, g_screen_h));
  g_renderbuffer2.reset(new Renderbuffer(g_screen_w, g_screen_h));

  g_screen = SDL_SetVideoMode(g_screen_w, g_screen_h, 0, SDL_OPENGL | SDL_RESIZABLE);

  g_aspect_ratio = static_cast<GLfloat>(g_screen_w)/static_cast<GLfloat>(g_screen_h);

  assert_gl("reshape");
}

void draw_scene(EyeType eye_type)
{
  OpenGLState state;

  glViewport(0, 0, g_screen_w, g_screen_h);

  // clear the screen
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  g_camera->perspective(g_fov, g_aspect_ratio, g_near_z, g_far_z);

  glm::vec3 look_at = g_look_at;
  glm::vec3 up = g_up;

  glm::vec3 sideways_ = glm::normalize(glm::cross(look_at, up));
  glm::vec3 eye = g_eye + glm::normalize(look_at) * g_distance_offset;

  if (g_wiimote_camera_control && g_wiimote_manager)
  {
    glm::quat q = glm::inverse(glm::quat(glm::mat3(glm::lookAt(glm::vec3(), look_at, up))));

    glm::quat orientation = g_wiimote_manager->get_gyro_orientation();
    look_at = (q * orientation) * glm::vec3(0,0,-1);
    up = (q * orientation) * glm::vec3(0,1,0);
  }
  else 
  {
    look_at = glm::rotate(look_at, g_yaw_offset, up);
    look_at = glm::rotate(look_at, -g_pitch_offset, sideways_);
    up = glm::rotate(up, -g_roll_offset, look_at);
  }

  glm::vec3 sideways = glm::normalize(glm::cross(look_at, up)) * g_eye_distance * 0.5f;
  switch(eye_type)
  {
    case kLeftEye:
      sideways = sideways;
      break;

    case kRightEye:
      sideways = -sideways;
      break;

    case kCenterEye:
      sideways = glm::vec3(0);
      break;
  }
  g_camera->look_at(eye + sideways, eye + look_at * g_convergence, up);
  
  g_scene_manager->render(*g_camera);
}

void draw_shadowmap()
{
  OpenGLState state;

  glViewport(0, 0, g_shadowmap->get_width(), g_shadowmap->get_height());

  glClearColor(1.0, 0.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glm::vec3 light_pos = glm::rotate(glm::vec3(10.0f, 10.0f, 10.0f), g_light_angle, glm::vec3(0.0f, 1.0f, 0.0f));
  glm::vec3 up = glm::rotate(glm::vec3(0.0f, 1.0f, 0.0f), g_light_up, glm::vec3(0.0f, 0.0f, 1.0f));
  glm::vec3 look_at(0.0f, 0.0f, 0.0f);

  Camera camera;
  camera.perspective(g_shadowmap_fov, 1.0f, g_near_z, g_far_z);
  //camera.ortho(-25.0f, 25.0f, -25.0f, 25.0f, g_near_z, g_far_z);
  
  camera.look_at(light_pos, look_at, up);

  g_shadowmap_matrix = glm::mat4(0.5, 0.0, 0.0, 0.0, 
                                  0.0, 0.5, 0.0, 0.0,
                                  0.0, 0.0, 0.5, 0.0,
                                  0.5, 0.5, 0.5, 1.0);

  g_shadowmap_matrix = g_shadowmap_matrix * camera.get_matrix();

  g_scene_manager->render(camera, true);
}

void display()
{
  glViewport(g_viewport_offset.x, g_viewport_offset.y, g_screen_w, g_screen_h);
  
  //log_info("display()\n");
  {
    OpenGLState state;

    if (g_render_shadowmap)
    {
      g_shadowmap->bind();
      draw_shadowmap();
      g_shadowmap->unbind();
    }

    if (g_stereo_mode == StereoMode::None)
    {
      g_renderbuffer1->bind();
      if (g_video_material && g_opts.video3d) g_video_material->set_uniform("offset", 0.0f);
      draw_scene(kCenterEye);
      g_renderbuffer1->unbind();

      g_renderbuffer1->blit(*g_framebuffer1);
    }
    else
    {
      g_renderbuffer1->bind();
      if (g_video_material && g_opts.video3d) g_video_material->set_uniform("offset", 0.0f);
      draw_scene(kLeftEye);
      g_renderbuffer1->unbind();

      g_renderbuffer2->bind();
      if (g_video_material && g_opts.video3d) g_video_material->set_uniform("offset", 0.5f);
      draw_scene(kRightEye);
      g_renderbuffer2->unbind();

      g_renderbuffer1->blit(*g_framebuffer1);
      g_renderbuffer2->blit(*g_framebuffer2);
    }

    // composit the final image
    if (true)
    {
      // 2d screen access
      glMatrixMode(GL_PROJECTION);
      glLoadIdentity();
      glOrtho(0, g_screen_w, 0, g_screen_h, 0.1f, 10000.0f);

      glMatrixMode(GL_MODELVIEW);
      glLoadIdentity();

      glClearColor(0.0, 0.0, 0.0, 1.0);
      glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

      switch(g_stereo_mode)
      {
        case StereoMode::Cybermaxx:
          {
            MaterialPtr material = std::make_shared<Material>();
            material->set_program(m_composition_prog);

            material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);

            material->set_uniform("barrel_power", g_barrel_power);
            material->set_uniform("offset", 0);
            material->set_uniform("left_eye",  0);
            material->set_uniform("right_eye", 1);
          
            material->set_texture(0, g_framebuffer1->get_color_texture());
            material->set_texture(1, g_framebuffer2->get_color_texture());

            ModelPtr entity = std::make_shared<Model>();
            entity->add_mesh(Mesh::create_rect(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f));
            entity->set_material(material);
            
            Camera camera;
            camera.ortho(0, g_screen_w, g_screen_h, 0.0f, 0.1f, 10000.0f);
            
            SceneManager mgr;
            mgr.get_world()->attach_entity(entity);
            mgr.render(camera);
          }
          break;

        case StereoMode::CrossEye:
          glColor3f(1.0f, 1.0f, 1.0f);
          g_framebuffer1->draw(0.0f, 0.0f, g_screen_w/2.0f, g_screen_h, -20.0f);
          g_framebuffer2->draw(g_screen_w/2.0f, 0.0f, g_screen_w/2.0f, g_screen_h, -20.0f);
          break;

        case StereoMode::Anaglyph:
          glEnable(GL_BLEND);
          glBlendFunc(GL_SRC_ALPHA, GL_ONE);

          glColor3f(0.0f, 0.7f, 0.0f);
          g_framebuffer1->draw(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f);

          glColor3f(1.0f, 0.0f, 0.0f);
          g_framebuffer2->draw(g_ipd, 0.0f, g_screen_w, g_screen_h, -20.0f);
          break;

        default:
          glEnable(GL_BLEND);
          glBlendFunc(GL_ONE, GL_ZERO);

          glColor3f(1.0f, 1.0f, 1.0f);
          if (g_draw_depth)
          {
            g_framebuffer1->draw_depth(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f);
          }
          else
          {
            g_framebuffer1->draw(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f);
          }
          break;
      }
    }

    if (g_show_calibration)
    {
      g_calibration_texture->draw(0, 0, g_screen_w, g_screen_h, -20.0f);
    }

    if (g_show_menu)
    {
      glDisable(GL_BLEND);
      //g_shadowmap->draw_depth(g_screen_w - 266, 10, 256, 256, -20.0f);
      g_shadowmap->draw(g_screen_w - 266 - 276, 10, 256, 256, -20.0f);
    }
  }

  { // 2D Rendering
    OpenGLState state;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, g_screen_w, g_screen_h, 0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    glClearColor(0.0, 0.0, 0.0, 1.0);
    //glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    
    glDisable(GL_LIGHTING);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_TEXTURE_2D);
    glColor3f(1.0f, 1.0f, 1.0f);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    if (g_show_menu)
    {
      g_menu->draw(120.0f, 64.0f);
    }

    if (g_show_dots)
    {
      g_dot_surface->draw(g_wiimote_dot1.x * g_screen_w, g_wiimote_dot1.y * g_screen_h);
      g_dot_surface->draw(g_wiimote_dot2.x * g_screen_w, g_wiimote_dot2.y * g_screen_h);
    }
  }

  //glutSwapBuffers();
  SDL_GL_SwapBuffers();
  assert_gl("display:exit()");
}

void keyboard(SDL_KeyboardEvent key, int x, int y)
{
  switch (key.keysym.sym)
  {
    case SDLK_TAB:
      g_show_menu = !g_show_menu;
      break;

    case SDLK_F3:
      g_show_calibration = !g_show_calibration;
      break;

    case SDLK_ESCAPE:
      exit(EXIT_SUCCESS);
      break;

    case SDLK_9:
      if (g_video_player)
      {
        g_video_player->seek(g_video_player->get_position() - 10 * Gst::SECOND);
      }     
      break;

    case SDLK_0:
      if (g_video_player)
      {
        g_video_player->seek(g_video_player->get_position() + 10 * Gst::SECOND);
      }
      break;

    case SDLK_n:
      g_eye_distance += 0.01f;
      break;

    case SDLK_t:
      g_eye_distance -= 0.01f;
      break;

    case SDLK_SPACE:
      g_draw_look_at = !g_draw_look_at;
      break;

    case SDLK_c:
      g_ipd += 1;
      break;

    case SDLK_r:
      g_ipd -= 1;
      break;

    case SDLK_PLUS:
      g_scale *= 1.05f;
      break;

    case SDLK_MINUS:
      g_scale /= 1.05f;
      break;

    case SDLK_z:
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

    case SDLK_g:
      {
        GLdouble clip_plane[] = { 0.0, 1.0, 1.0, 0.0 };
        glClipPlane(GL_CLIP_PLANE0, clip_plane);
        glEnable(GL_CLIP_PLANE0);
      }
      break;

    case SDLK_d:
      {
        int stereo_mode = static_cast<int>(g_stereo_mode) + 1;
        if (stereo_mode >= static_cast<int>(StereoMode::End))
        {
          g_stereo_mode = StereoMode::None;
        }
        else
        {
          g_stereo_mode = static_cast<StereoMode>(stereo_mode);
        }
      }
      break;

    case SDLK_KP8: // up
      g_eye += glm::normalize(g_look_at);
      break;

    case SDLK_KP2: // down
      g_eye -= glm::normalize(g_look_at);
      break;

    case SDLK_KP4: // left
      {
        glm::vec3 dir = glm::normalize(g_look_at);
        dir = glm::rotate(dir, 90.0f, g_up);
        g_eye += dir;
      }
      break;

    case SDLK_KP6: // right
      {
        glm::vec3 dir = glm::normalize(g_look_at);
        dir = glm::rotate(dir, 90.0f, g_up);
        g_eye -= dir;
      }
      break;

    case SDLK_KP7: // kp_pos1
      g_look_at = glm::rotate(g_look_at, 5.0f, g_up);
      break;

    case SDLK_KP9: // kp_raise
      g_look_at = glm::rotate(g_look_at, -5.0f, g_up);
      break;

    case SDLK_KP1:
      g_eye -= glm::normalize(g_up);
      break;

    case SDLK_KP3:
      g_eye += glm::normalize(g_up);
      break;

    case SDLK_KP_MULTIPLY:
      g_fov += glm::radians(1.0f);
      break;

    case SDLK_KP_DIVIDE:
      g_fov -= glm::radians(1.0f);
      break;

    case SDLK_F1:
      {
        // Hitchcock zoom in
        //float old_eye_z = g_eye.z;
        //g_eye.z *= 1.005f;
        //g_fov = g_fov / std::atan(1.0f / old_eye_z) * std::atan(1.0f / g_eye.z);

        float old_fov = g_fov;
        g_fov += glm::radians(1.0f);
        if (g_fov < glm::radians(160.0f))
        {
          g_eye.z = g_eye.z 
            * (2.0*tan(0.5 * old_fov))
            / (2.0*tan(0.5 * g_fov));
        }
        else
        {
          g_fov = 160.0f;
        }
        log_info("fov: %5.2f %f", g_fov, g_eye.z);
        log_info("w: %f", tan(g_fov /2.0f /180.0*M_PI) * g_eye.z);
      }
      break;

    case SDLK_F2:
      // Hitchcock zoom out
      {
        //float old_eye_z = g_eye.z;
        //g_eye.z /= 1.005f;
        //g_fov = g_fov / std::atan(1.0f / old_eye_z) * std::atan(1.0f / g_eye.z);
        
        float old_fov = g_fov;
        g_fov -= 1.0f;
        if (g_fov >= 7.0f)
        {
          g_eye.z = g_eye.z 
            * (2.0*tan(0.5 * old_fov / 180.0 * M_PI))
            / (2.0*tan(0.5 * g_fov / 180.0 * M_PI));
        }
        else
        {
          g_fov = 7.0f;
        }
        log_info("fov: %5.2f %f", g_fov, g_eye.z);
        log_info("w: %f", tan(g_fov/2.0f /180.0*M_PI) * g_eye.z);
      }
      break;

    case SDLK_F10:
      //glutReshapeWindow(1600, 1000);
      break;

    case SDLK_F11:
      //glutFullScreen();
      break;

    case SDLK_UP:
      g_convergence *= 1.1f;
      break;

    case SDLK_DOWN:
      g_convergence /= 1.1f;
      break;

    case SDLK_LEFT:
      //g_look_at.x += 1.0f;
      break;

    case SDLK_RIGHT:
      //g_look_at.x -= 1.0f;
      break;

    case SDLK_PAGEUP:
      //g_look_at.y -= 1.0f;
      break;

    case SDLK_PAGEDOWN:
      //g_look_at.y += 1.0f;
      break;

    default:
      std::cout << "unknown key: " << static_cast<int>(key.keysym.sym) << std::endl;
      break;
  }
}

void init()
{
  assert_gl("init()");
  g_framebuffer1.reset(new Framebuffer(g_screen_w, g_screen_h));
  g_framebuffer2.reset(new Framebuffer(g_screen_w, g_screen_h));
  g_renderbuffer1.reset(new Renderbuffer(g_screen_w, g_screen_h));
  g_renderbuffer2.reset(new Renderbuffer(g_screen_w, g_screen_h));
  g_shadowmap.reset(new Framebuffer(g_shadowmap_resolution, g_shadowmap_resolution));
  assert_gl("init()");

  g_armature = Armature::from_file("/tmp/blender.bones");
  g_pose = Pose::from_file("/tmp/blender.pose");

  {
    g_scene_manager.reset(new SceneManager);

    {
      MaterialPtr material(new Material);
      material->cull_face(GL_FRONT);
      material->enable(GL_CULL_FACE);
      material->enable(GL_DEPTH_TEST);
      material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);
      material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/shadowmap.vert"),
                                            Shader::from_file(GL_FRAGMENT_SHADER, "src/shadowmap.frag")));
      g_scene_manager->set_override_material(material);
    }

    g_camera.reset(new Camera);
    g_camera->perspective(g_fov, g_aspect_ratio, g_near_z, 100000.0f);

    if (g_video_player) // streaming video
    {
      if (!g_opts.video3d)
      {
        g_video_material = MaterialFactory::get().create("video");
      }
      else
      {
        g_video_material = MaterialFactory::get().create("video3d");
        g_video_material_flip = MaterialFactory::get().create("video3d-flip");
      }

      if (false)
      {
        auto node = g_scene_manager->get_world()->create_child();
        ModelPtr entity = std::make_shared<Model>();

        entity->add_mesh(Mesh::create_plane(5.0f));
        node->set_position(glm::vec3(0.0f, 0.0f, -10.0f));
        node->set_orientation(glm::quat(glm::vec3(M_PI/2, 0.0f, 0.0f)));
        node->set_scale(glm::vec3(4.0f, 1.0f, 2.25f));

        entity->set_material(g_video_material);
        node->attach_entity(entity);
      }
      else
      {
        auto node = g_scene_manager->get_world()->create_child();
        ModelPtr entity = std::make_shared<Model>();

        int rings = 16;
        int segments = 16;
        //float hfov = glm::radians(125.0f);
        //float vfov = glm::radians(70.3f);

        float hfov = glm::radians(90.0f);
        float vfov = glm::radians(50.0f);

        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments));

        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 0, 16, false, true));
        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 0, -16, false, true));

        entity->set_material(g_video_material);
        node->attach_entity(entity);

        entity = std::make_shared<Model>();

        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 16, 0, true, false));
        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, -16, 0, true, false));

        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 16, 16, true, true));
        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, -16, 16, true, true));

        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, 16, -16, true, true));
        entity->add_mesh(Mesh::create_curved_screen(15.0f, hfov, vfov, rings, segments, -16, -16, true, true));

        entity->set_material(g_video_material_flip);
        node->attach_entity(entity);
      }
    }

    MaterialPtr phong_material = MaterialFactory::get().create("phong");
    if (false)
    {
      if (false)
      {
        auto node = g_scene_manager->get_world()->create_child();
        node->set_position(glm::vec3(0, 0, 0));
        auto mesh = Mesh::create_plane(75.0f);
        ModelPtr entity = std::make_shared<Model>();
        entity->add_mesh(std::move(mesh));
        entity->set_material(phong_material);
        node->attach_entity(entity);
      }

      auto mesh = Mesh::create_sphere(0.2, 8, 16);
      ModelPtr entity = std::make_shared<Model>();
      entity->add_mesh(std::move(mesh));
      entity->set_material(phong_material);

      auto origin = g_scene_manager->get_world()->create_child();
      origin->set_position(glm::vec3(5, 5, 5));

      auto sun = origin->create_child();
      sun->set_scale(glm::vec3(3.0f, 3.0f, 3.0f));
      sun->attach_entity(entity);
      g_nodes.push_back(sun);

      auto earth = sun->create_child();
      earth->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));
      earth->set_position(glm::vec3(2.0f, 0, 0));
      earth->attach_entity(entity);
      g_nodes.push_back(earth);

      auto moon = earth->create_child();
      moon->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));
      moon->set_position(glm::vec3(2, 0, 0));
      moon->attach_entity(entity);
      //g_nodes.push_back(moon);
    }

    if (false)
    {
      auto root_parent = g_scene_manager->get_world()->create_child();
      auto parent = root_parent;
      parent->set_position(glm::vec3(10.0f, 0.0f, 0.0f));
      for(int i = 0; i < 5; ++i)
      {
        auto child = parent->create_child();
        child->set_position(glm::vec3(1.0f, 0.0f, -3.0f));
        ModelPtr entity = std::make_shared<Model>();
        entity->add_mesh(Mesh::create_sphere(0.5f, 16, 8));
        entity->set_material(phong_material);
        child->attach_entity(entity);

        parent = child;
      }

      print_scene_graph(root_parent);
    }

    if (!g_model_filename.empty())
    { // load a mesh from file
      auto node = Scene::from_file(g_model_filename);
      print_scene_graph(node.get());
      g_scene_manager->get_world()->attach_child(std::move(node));
    }

    {
      auto node = Scene::from_file("data/wiimote.mod");
      g_wiimote_gyro_node = node.get();
      g_scene_manager->get_world()->attach_child(std::move(node));
    }

    {
      auto node = Scene::from_file("data/wiimote.mod");
      g_wiimote_node = node.get();
      g_scene_manager->get_world()->attach_child(std::move(node));
    }

    {
      auto node = Scene::from_file("data/wiimote.mod");
      g_wiimote_accel_node = node.get();
      g_scene_manager->get_world()->attach_child(std::move(node));
    }

    if (true)
    { // create a skybox
      auto mesh = Mesh::create_skybox(500.0f);
      ModelPtr entity = std::make_shared<Model>();
      entity->add_mesh(std::move(mesh));
      entity->set_material(MaterialFactory::get().create("skybox"));

      auto node = g_scene_manager->get_world()->create_child();
      node->attach_entity(entity);
    }

    if (false)
    { // light cone
      MaterialPtr material(new Material);     
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
      material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/lightcone.vert"),
                                            Shader::from_file(GL_FRAGMENT_SHADER, "src/lightcone.frag")));

      auto mesh = std::unique_ptr<Mesh>(new Mesh(GL_POINTS));
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

      ModelPtr entity = std::make_shared<Model>();
      entity->add_mesh(std::move(mesh));
      entity->set_material(material);

      for(int i = 0; i < 10; ++i)
      {
        auto node = g_scene_manager->get_world()->create_child();
        node->set_position(glm::vec3(1.0f, i * 5.0f, -1.0f));
        node->set_orientation(glm::quat());
        node->attach_entity(entity);
      }
    }
  }

  g_calibration_texture = Texture::from_file("data/calibration.png", false);

  //m_composition_prog = Program::create(Shader::from_file(GL_FRAGMENT_SHADER, "src/newsprint.frag"));
  m_composition_prog = Program::create(Shader::from_file(GL_FRAGMENT_SHADER, "src/composite.frag"));

  g_dot_surface = TextSurface::create("+", TextProperties().set_line_width(3.0f));

  g_menu.reset(new Menu(TextProperties().set_font_size(24.0f).set_line_width(4.0f)));
  //g_menu->add_item("eye.x", &g_eye.x);
  //g_menu->add_item("eye.y", &g_eye.y);
  //g_menu->add_item("eye.z", &g_eye.z);

  g_menu->add_item("wiimote.camera_control", &g_wiimote_camera_control);

  g_menu->add_item("slowfactor", &g_slow_factor, 0.01f, 0.0f);

  g_menu->add_item("depth.near_z", &g_near_z, 0.01, 0.0f);
  g_menu->add_item("depth.far_z",  &g_far_z, 1.0f);

  g_menu->add_item("convergence", &g_convergence, 0.1f);

  //g_menu->add_item("viewport.offset.x",  &g_viewport_offset.x, 1);
  //g_menu->add_item("viewport.offset.y",  &g_viewport_offset.y, 1);

  g_menu->add_item("shadowmap.fov", &g_shadowmap_fov, 1.0f);

  //g_menu->add_item("spot_halo_samples",  &g_spot_halo_samples, 1, 0);

  g_menu->add_item("FOV", &g_fov);
  g_menu->add_item("Barrel Power", &g_barrel_power, 0.01f);
  //g_menu->add_item("AspectRatio", &g_aspect_ratio, 0.05f, 0.5f, 4.0f);

  //g_menu->add_item("scale", &g_scale, 0.5f, 0.0f);
  g_menu->add_item("eye.distance", &g_eye_distance, 0.1f);

  //g_menu->add_item("spot.cutoff",   &g_spot_cutoff);
  //g_menu->add_item("spot.exponent", &g_spot_exponent);

  //g_menu->add_item("light.up",  &g_light_up, 1.0f);
  //g_menu->add_item("light.angle",  &g_light_angle, 1.0f);
  //g_menu->add_item("light.diffuse",  &g_light_diffuse, 0.1f, 0.0f);
  //g_menu->add_item("light.specular", &g_light_specular, 0.1f, 0.0f);
  //g_menu->add_item("material.shininess", &g_material_shininess, 0.1f, 0.0f);

  g_menu->add_item("wiimote.distance_scale",  &g_distance_scale, 0.01f);
  g_menu->add_item("wiimote.scale_x", &g_wiimote_scale.x, 0.01f);
  g_menu->add_item("wiimote.scale_y", &g_wiimote_scale.y, 0.01f);

  //g_menu->add_item("3D", &g_draw_3d);
  //g_menu->add_item("Grid", &g_draw_grid);
  //g_menu->add_item("Headlights", &g_headlights);
  //g_menu->add_item("Look At Sphere", &g_draw_look_at);
  //g_menu->add_item("draw depth", &g_draw_depth);
  //g_menu->add_item("shadow map", &g_render_shadowmap);
  //g_menu->add_item("grid.size", &g_grid_size, 0.5f);

  assert_gl("init()");
}

void mouse(int button, int button_pressed, int x, int y)
{
  //log_info("mouse: %d %d - %d %d", x, y, button, button_pressed);

  if (button == 0)
  {
    if (!button_pressed)
    {
      //log_info("mouse: arcball: %d %d", x, y);
      g_arcball_active = true;
      g_mouse = g_last_mouse = glm::ivec2(x, y);
      g_last_object2world = g_object2world;
    }
    else
    {
      g_arcball_active = false;
    }
  }
}

glm::vec3 get_arcball_vector(glm::ivec2 mouse)
{
  float radius = std::min(g_screen_w, g_screen_h) / 2.0f;
  glm::vec3 P = glm::vec3(static_cast<float>(mouse.x - g_screen_w/2) / radius,
                          static_cast<float>(mouse.y - g_screen_h/2) / radius,
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

void process_events()
{
  SDL_Event ev;
  while(SDL_PollEvent(&ev))
  {
    switch(ev.type)
    {
      case SDL_VIDEORESIZE:
        reshape(ev.resize.w, ev.resize.h);
        break;

      case SDL_QUIT:
        exit(EXIT_SUCCESS);
        break;

      case SDL_KEYUP:
        break;

      case SDL_KEYDOWN:
        keyboard(ev.key, g_mouse_x, g_mouse_y);
        break;

      case SDL_MOUSEMOTION:
        g_mouse_x = ev.motion.x;
        g_mouse_y = ev.motion.y;
        break;

      case SDL_MOUSEBUTTONDOWN:
      case SDL_MOUSEBUTTONUP:
        break;

      case SDL_JOYAXISMOTION:
        //log_debug("joystick axis: %d %d", static_cast<int>(ev.jaxis.axis), ev.jaxis.value);
        switch(ev.jaxis.axis)
        {
          case 0: // x1
            g_stick.dir.x = -ev.jaxis.value / 32768.0f;
            break;

          case 1: // y1
            g_stick.dir.z = -ev.jaxis.value / 32768.0f;
            break;

          case 2: // z
            g_stick.rot.z = ev.jaxis.value / 32768.0f;
            break;

          case 3: // x2
            g_stick.rot.y = -ev.jaxis.value / 32768.0f;
            break;

          case 4: // y2
            g_stick.rot.x = ev.jaxis.value / 32768.0f;
            break;
        }
        break;

      case SDL_JOYBUTTONDOWN:
      case SDL_JOYBUTTONUP:
        //log_debug("joystick button: %d %d", static_cast<int>(ev.jbutton.button), static_cast<int>(ev.jbutton.state));
        switch(ev.jbutton.button)
        {
          case 4:
            g_stick.dir.y = -ev.jbutton.state;
            break;
            
          case 5:
            g_stick.dir.y = +ev.jbutton.state;
            break;

          case 0:
            g_wiimote_manager->reset_gyro_orientation();
            break;

          case 1:
            //g_light_angle += 1.0f;
            g_stick.light_rotation = ev.jbutton.state;
            break;

          case 2:
            g_headlights = ev.jbutton.state;
            break;

          case 7:
            if (ev.jbutton.state)
            {
              g_show_menu = !g_show_menu;
            }
            break;

          case 6:
            if (ev.jbutton.state)
            {
              g_show_dots = !g_show_dots;
            }
            break;
        }
        break;

      case SDL_JOYHATMOTION:
        g_stick.hat = ev.jhat.value;
        g_hat_autorepeat = SDL_GetTicks() + 500;
        break;

      default:
        break;
    }
  }
}

void process_joystick(float dt)
{
  auto current_time = SDL_GetTicks();
  if (g_stick.hat != g_old_stick.hat || (g_stick.hat && g_hat_autorepeat < current_time))
  {
    if (g_hat_autorepeat < current_time)
    {
      g_hat_autorepeat = current_time + 100 + (g_hat_autorepeat - current_time);
    }

    if (g_stick.hat & SDL_HAT_UP)
    {
      g_menu->up();
    }
        
    if (g_stick.hat & SDL_HAT_DOWN)
    {
      g_menu->down();
    }

    if (g_stick.hat & SDL_HAT_LEFT)
    {
      g_menu->left();
    }

    if (g_stick.hat & SDL_HAT_RIGHT)
    {
      g_menu->right();
    }
  }

  float deadzone = 0.2f;
  if (fabs(g_stick.dir.x) < deadzone) g_stick.dir.x = 0.0f;
  if (fabs(g_stick.dir.y) < deadzone) g_stick.dir.y = 0.0f;
  if (fabs(g_stick.dir.z) < deadzone) g_stick.dir.z = 0.0f;

  if (fabs(g_stick.rot.x) < deadzone) g_stick.rot.x = 0.0f;
  if (fabs(g_stick.rot.y) < deadzone) g_stick.rot.y = 0.0f;
  if (fabs(g_stick.rot.z) < deadzone) g_stick.rot.z = 0.0f;

  if (false)
    log_debug("stick: %2.2f %2.2f %2.2f  -  %2.2f %2.2f %2.2f", 
              g_stick.dir.x, g_stick.dir.y, g_stick.dir.z,
              g_stick.rot.x, g_stick.rot.y, g_stick.rot.z);

  float delta = dt * 5.0f * g_slow_factor;

  if (g_stick.light_rotation)
  {
    //log_debug("light angle: %f", g_light_angle);
    g_light_angle += delta * 30.0f;
  }
  
  if (false)
  { // free flight mode
    {
      // forward/backward
      g_eye += glm::normalize(g_look_at) * g_stick.dir.z * delta;

      // up/down
      g_eye += glm::normalize(g_up) * g_stick.dir.y * delta;

      // left/right
      glm::vec3 dir = glm::normalize(g_look_at);
      dir = glm::rotate(dir, 90.0f, g_up);
      g_eye += glm::normalize(dir) * g_stick.dir.x * delta;
    }

    { // handle rotation
      float angle_d = 20.0f;

      // yaw
      g_look_at = glm::rotate(g_look_at, angle_d * g_stick.rot.y * delta, g_up);

      // roll
      g_up = glm::rotate(g_up, angle_d * g_stick.rot.z * delta, g_look_at);
    
      // pitch
      glm::vec3 cross = glm::cross(g_look_at, g_up);
      g_up = glm::rotate(g_up, angle_d * g_stick.rot.x * delta, cross);
      g_look_at = glm::rotate(g_look_at, angle_d * g_stick.rot.x * delta, cross);
    }
  }
  else
  { // fps mode
    float focus_distance = glm::length(g_look_at);
    auto tmp = g_look_at;
    auto xz_dist = glm::sqrt(tmp.x * tmp.x + tmp.z * tmp.z);
    float pitch = glm::atan(tmp.y, xz_dist);
    float yaw   = glm::atan(tmp.z, tmp.x);
    
    yaw   += -g_stick.rot.y * 2.0f * dt;
    pitch += g_stick.rot.x * 2.0f * dt;

    pitch = glm::clamp(pitch, -static_cast<float>(M_PI)/2.0f + 0.001f, static_cast<float>(M_PI)/2.0f - 0.001f);

    glm::vec3 forward(glm::cos(yaw), 0.0f, glm::sin(yaw));
        
    //log_debug("focus distance: %f", focus_distance);
    //log_debug("yaw: %f pitch: %f", yaw, pitch);

    // forward/backward
    g_eye += 10.0f * forward * g_stick.dir.z * dt * g_slow_factor;

    // strafe
    g_eye += 10.0f * glm::vec3(forward.z, 0.0f, -forward.x) * g_stick.dir.x * dt * g_slow_factor;

    // up/down
    g_eye.y += 10.0f * g_stick.dir.y * dt * g_slow_factor;

    g_look_at = focus_distance * glm::vec3(glm::cos(pitch) * glm::cos(yaw), 
                                           glm::sin(pitch),
                                           glm::cos(pitch) * glm::sin(yaw));
    
    float f = sqrt(g_look_at.x*g_look_at.x + g_look_at.z*g_look_at.z);
    g_up.x = -g_look_at.x/f * g_look_at.y;
    g_up.y = f;
    g_up.z = -g_look_at.z/f * g_look_at.y;
    g_up = glm::normalize(g_up);
  }

  g_old_stick = g_stick;
}

void update_arcball()
{
  if (g_arcball_active && g_mouse != g_last_mouse)
  {
    glm::mat4 camera_matrix;// = g_object2world;

    //glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(camera_matrix));

    glm::vec3 va = get_arcball_vector(g_last_mouse);
    glm::vec3 vb = get_arcball_vector(g_mouse);
    float angle = acos(std::min(1.0f, glm::dot(va, vb)));
    glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
    glm::mat3 camera2object = glm::inverse(glm::mat3(camera_matrix) * glm::mat3(g_last_object2world));
    glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
    g_object2world = glm::rotate(g_last_object2world, angle, axis_in_object_coord);
    //g_last_mouse = g_mouse;
  }
}

void update_world(float dt)
{
  int i = 1; 
  for(auto& node : g_nodes)
  {
    float f = SDL_GetTicks()/1000.0f;
    node->set_orientation(glm::quat(glm::vec3(0.0f, f*1.3*static_cast<float>(i), 0.0f)));
    i += 3;
  } 
}

void main_loop()
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
      
    display();
    SDL_Delay(1);

    g_grid_offset += glm::vec4(0.0f, 0.0f, 0.001f, 0.0f);

    process_events();
    process_joystick(delta / 1000.0f);

    if (g_wiimote_manager)
    {
      //g_wiimote_manager->update();
      //g_wiimote_manager->get_accumulated();
      g_wiimote_gyro_node->set_orientation(g_wiimote_manager->get_gyro_orientation());
      g_wiimote_gyro_node->set_position(glm::vec3(-0.1f, 0.0f, -0.5f));

      g_wiimote_node->set_orientation(g_wiimote_manager->get_orientation());
      g_wiimote_node->set_position(glm::vec3(0.0f, 0.0f, -0.5f));

      g_wiimote_accel_node->set_orientation(g_wiimote_manager->get_accel_orientation());
      g_wiimote_accel_node->set_position(glm::vec3(0.1f, 0.0f, -0.5f));
    }

    num_frames += 1;

    if (num_frames == 1000)
    {
      int t = SDL_GetTicks() - start_ticks;
      std::cout << "frames: " << num_frames << " time: " << t 
                << " frame_delay: " << static_cast<float>(t) / static_cast<float>(num_frames)
                << " fps: " << static_cast<float>(num_frames) / static_cast<float>(t) * 1000.0f
                << std::endl;

      num_frames = 0;
      start_ticks = SDL_GetTicks();
    }

    if (g_video_player)
    {
      g_video_player->update();
      TexturePtr texture = g_video_player->get_texture();
      if (texture)
      {
        g_video_material->set_texture(0, texture);
        if (g_video_material_flip) g_video_material_flip->set_texture(0, texture);
      }
    }
  }
}

void mouse_motion(int x, int y)
{
  if (g_arcball_active)
  {
    //log_info("mouse motion: arcball: %d %d", x, y);
    g_mouse.x = x;
    g_mouse.y = y;
  }
}

void update_offsets(glm::vec2 p1, glm::vec2 p2)
{
  if (p1.x > p2.x)
  {
    std::swap(p1, p2);
  }

  glm::vec2 r = p2 - p1;
  float angle = glm::atan(-r.y, r.x);
  g_roll_offset = angle;
  g_distance_offset = g_distance_scale * glm::length(r) / 2.0f * glm::tan(glm::radians(g_fov));
  glm::vec2 c = (p1+p2)/2.0f;

  c -= glm::vec2(512, 384);
  c = glm::rotate(c, g_roll_offset);
  c += glm::vec2(512, 384);

  g_yaw_offset   = ((c.x / 1024.0f) - 0.5f) * M_PI/2.0f * g_wiimote_scale.x;
  g_pitch_offset = ((c.y /  768.0f) - 0.5f) * (M_PI/2.0f) * g_wiimote_scale.y;

  g_wiimote_dot1.x = p1.x / 1024.0f;
  g_wiimote_dot1.y = 1.0f - (p1.y /  768.0f);
  g_wiimote_dot2.x = p2.x / 1024.0f;
  g_wiimote_dot2.y = 1.0f - (p2.y /  768.0f);

  //std::cout << "offset: " << boost::format("%4.2f %4.2f %4.2f") % g_roll_offset % g_yaw_offset % g_pitch_offset << std::endl;
}

#if 0
void
wiimote_mesg_callback(cwiid_wiimote_t*, int mesg_count, union cwiid_mesg msg[], timespec*)
{
  // WARNING: the mesg_callback() comes from another thread, so
  // syncronize it with SDL_PushEvent() or only do things that are thread-safe
  for (int i=0; i < mesg_count; ++i)
  {
    switch (msg[i].type) 
    {
      case CWIID_MESG_STATUS:
        std::cout << "wiimote mesg status" << std::endl;
        break;

      case CWIID_MESG_BTN:
        //g_wiimote->on_button(msg[i].btn_mesg);
        break;

      case CWIID_MESG_ACC:
        //s_wiimote->on_acc(msg[i].acc_mesg);
        break;

      case CWIID_MESG_IR:
        {
          cwiid_ir_mesg& ir = msg[i].ir_mesg;
          //std::cout << "wiimote mesg ir: ";
          glm::vec2 p[2];
          int j = 0;
          for(; j < 2 /*CWIID_IR_SRC_COUNT*/; ++j)
          {
            if (ir.src[j].valid)
            {
              if (false)
              {
                std::cout << j << "[ "
                          << ir.src[j].pos[0] << "," << ir.src[j].pos[1] << " - " 
                          << static_cast<int>(ir.src[j].size)
                          << "] ";
              }
              p[j].x = ir.src[j].pos[0];
              p[j].y = ir.src[j].pos[1];
            }
          }

          if (j == 2)
          {
            update_offsets(p[0], p[1]);
          }
          //std::cout << std::endl;
        }
        break;
        
      case CWIID_MESG_ERROR:
        std::cout << "wiimote mesg error" << std::endl;
        break;

      default:
        log_error("unknown report");
        break;
    }
  }
}
#endif

void init_display(const std::string& title, bool fullscreen, int anti_aliasing)
{
  //SDL_GL_SetAttribute(SDL_GL_SWAP_CONTROL, 1); // vsync
  SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1); 

  SDL_GL_SetAttribute(SDL_GL_RED_SIZE,     8);
  SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE,   8);
  SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE,    8);
  SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);

  SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);

  if (anti_aliasing)
  {
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLEBUFFERS, 1 ); // boolean value, either it's enabled or not
    SDL_GL_SetAttribute( SDL_GL_MULTISAMPLESAMPLES, anti_aliasing ); // 0, 2, or 4 for number of samples
  }
  
  SDL_WM_SetCaption(title.c_str(), title.c_str());
  //SDL_WM_SetIcon(IMG_Load(Pathname("icon.png").get_sys_path().c_str()), NULL);

  g_screen = SDL_SetVideoMode(g_screen_w, g_screen_h,
                              0, SDL_OPENGL | SDL_RESIZABLE | (fullscreen ? SDL_FULLSCREEN : 0));
}

int main(int argc, char** argv)
{
  Options& opts = g_opts;

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
      else
      {
        throw std::runtime_error("unknown option: " + std::string(argv[i]));
      }
    }
    else
    {
      opts.model = argv[i];
      g_model_filename = opts.model;
    }
  }

  if (SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0)
  {
    std::ostringstream msg;
    msg << "Couldn't initialize SDL: " << SDL_GetError();
    throw std::runtime_error(msg.str());
  }
  else
  {
    atexit(SDL_Quit);
  }

  init_display("OpenGL Viewer", false, 0);

  SDL_Joystick* joystick = nullptr;
  log_info("SDL_NumJoysticks: %d", SDL_NumJoysticks());
  if (SDL_NumJoysticks() > 0)
  {
    joystick = SDLCALL SDL_JoystickOpen(0);
  }

  glewInit();

#if 0
  if (opts.wiimote)
  {
    std::cout  << "Put Wiimote in discoverable mode now (press 1+2)..." << std::endl;

    bdaddr_t addr_any = {{0, 0, 0, 0, 0, 0}};
    g_wiimote = cwiid_open_timeout(&addr_any, CWIID_FLAG_MESG_IFC, -1);

    if (g_wiimote)
    {
      std::cout << "Wiimote connected: " << g_wiimote << std::endl;
      if (cwiid_set_mesg_callback(g_wiimote, &wiimote_mesg_callback)) 
      {
        std::cerr << "Unable to set message callback" << std::endl;
      }
      
      if (cwiid_command(g_wiimote, CWIID_CMD_RPT_MODE, 
                        CWIID_RPT_STATUS  |
                        //CWIID_RPT_ACC   |
                        CWIID_RPT_IR      |
                        CWIID_RPT_BTN))
      {
        std::cerr << "Wiimote: Error setting report mode" << std::endl;
      }
    }
  }
#endif 
  if (opts.wiimote)
  {
    g_wiimote_manager = std::make_shared<WiimoteManager>();
  }
  
  if (!opts.video.empty())
  {
    Gst::init(argc, argv);
    std::cout << "Playing video: " << opts.video << std::endl;
    g_video_player = std::make_shared<VideoProcessor>(opts.video);
  }

  init();

  std::cout << "main: " << std::this_thread::get_id() << std::endl;

  main_loop();

  if (joystick)
  {
    SDL_JoystickClose(joystick);
  }

  return 0; 
}


// EOF //
