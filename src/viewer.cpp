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

#include <SDL.h>
#include <SDL_image.h>
#include <cmath>
#include <memory>
#include <unistd.h>
#include <cmath>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

#include "armature.hpp"
#include "assert_gl.hpp"
#include "camera.hpp"
#include "framebuffer.hpp"
#include "log.hpp"
#include "menu.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "opengl_state.hpp"
#include "pose.hpp"
#include "program.hpp"
#include "scene_manager.hpp"
#include "shader.hpp"
#include "text_surface.hpp"
#include "uniform_group.hpp"

void draw_models(bool shader_foo);

// global variables
namespace {

std::unique_ptr<Menu> g_menu;
std::unique_ptr<SceneManager> g_scene_manager;
std::unique_ptr<Camera> g_camera;

bool g_cross_eye = false;

float g_ipd = 0.0f;
int g_screen_w = 1280;
int g_screen_h = 800;
float g_fov = 70.0f;

float g_near_z = 0.01f;
float g_far_z  = 100000.0f;

int g_spot_halo_samples = 100;

bool g_draw_look_at = false;

float g_light_angle = 0.0f;
bool g_draw_3d = false;
bool g_headlights = false;
bool g_draw_grid = false;
bool g_draw_depth = false;
bool g_render_shadow_map = true;

int g_shadow_map_resolution = 512;

float g_shadow_map_bias = -0.015f;
float g_shadow_map_fov = 70.0f;
float g_light_diffuse = 1.0f;
float g_light_specular = 1.0f;
float g_material_shininess = 10.0f;
float g_light_up = 0.0f;

float g_spot_cutoff   = 60.0f;
float g_spot_exponent = 30.0f;

bool g_show_menu = true;

glm::vec3 g_eye(0.0f, 0.0f, 15.0f);
glm::vec3 g_look_at(0.0f, 0.0f, -100.0f);
glm::vec3 g_up(0.0f, 1.0f, 0.0f);
glm::mat4 g_shadow_map_matrix;
glm::vec4 g_grid_offset;
float g_grid_size = 2.0f;

std::string g_model_filename;
ModelPtr g_model;
std::unique_ptr<Armature> g_armature;
std::unique_ptr<Pose> g_pose;

std::unique_ptr<Framebuffer> g_framebuffer1;
std::unique_ptr<Framebuffer> g_framebuffer2;
std::unique_ptr<Framebuffer> g_shadow_map;

float g_scale = 1.0f;

enum EyeType { kLeftEye, kRightEye, kCenterEye };
float g_wiggle_offset = 0.3f;

bool g_arcball_active = false;
glm::ivec2 g_mouse;
glm::ivec2 g_last_mouse;
glm::mat4 g_object2world;
glm::mat4 g_last_object2world;
glm::mat4 g_eye_matrix;

TextSurfacePtr g_hello_world;

ProgramPtr g_program;

} // namespace

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
  g_shadow_map.reset(new Framebuffer(g_shadow_map_resolution, g_shadow_map_resolution));

  assert_gl("reshape");
}

void flip_rgb(SDL_Surface* surface)
{
  for(int y = 0; y < surface->h; ++y)
    for(int x = 0; x < surface->w; ++x)
    {
      uint8_t* p = static_cast<uint8_t*>(surface->pixels) + y * surface->pitch + x * surface->format->BytesPerPixel;
      std::swap(p[0], p[2]);
    }
}

void draw_scene(EyeType eye_type)
{
  OpenGLState state;

  glViewport(0,0, g_screen_w, g_screen_h);

  // clear the screen
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glEnable(GL_NORMALIZE);
  
  const float aspect_ratio = static_cast<GLfloat>(g_screen_w)/static_cast<GLfloat>(g_screen_h);
  g_camera->projection(g_fov, aspect_ratio, g_near_z, 100000.0f);

  glm::vec3 sideways = glm::normalize(glm::cross(g_look_at, g_up)) * g_wiggle_offset;
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

  g_camera->look_at(g_eye + sideways, g_eye + g_look_at, g_up);
  
  g_scene_manager->render(*g_camera);
}

void draw_shadowmap()
{
  OpenGLState state;

  glViewport(0, 0, g_shadow_map->get_width(), g_shadow_map->get_height());

  glClearColor(1.0, 0.0, 1.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glEnable(GL_NORMALIZE);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(g_shadow_map_fov, 1.0f, g_near_z, 1000.0f);
  // needs bias tweaking to work
  //glOrtho(-10.0f, 10.0f, -10.0f, 10.0f, g_near_z, 1000.0f);

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glm::vec3 light_pos(50.0f,50,0.0f);
  light_pos = glm::rotate(light_pos, g_light_angle, glm::vec3(0.0f, 1.0f, 0.0f));

  glm::vec3 up(0.0f, 1.0f, 0.0f);
  up = glm::rotate(up, g_light_up, glm::vec3(0.0f, 0.0f, 1.0f));
  gluLookAt(light_pos.x, light_pos.y, light_pos.z,
            0.0f, 0.0f, 0.0f /* look-at */,
            up.x, up.y, up.z /* up */);

  g_shadow_map_matrix = glm::mat4(0.5, 0.0, 0.0, 0.0, 
                                  0.0, 0.5, 0.0, 0.0,
                                  0.0, 0.0, 0.5, 0.0,
                                  0.5, 0.5, 0.5, 1.0);
  glm::mat4 tmp_matrix;
  glGetFloatv(GL_PROJECTION_MATRIX, glm::value_ptr(tmp_matrix));
  g_shadow_map_matrix *= tmp_matrix;
  glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(tmp_matrix));
  g_shadow_map_matrix *= tmp_matrix;

  draw_models(false); 
}

void draw_models(bool shader_foo)
{
  GLfloat mat_specular[] = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_ambient[]  = { 1.0, 1.0, 1.0, 1.0 };
  GLfloat mat_diffuse[]  = { 1.0, 1.0, 1.0, 1.0 };

  glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
  //glMaterialfv(GL_FRONT, GL_EMISSION,  mat_specular);
  glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);

  glMaterialf(GL_FRONT, GL_SHININESS, g_material_shininess);

  if (g_draw_look_at)
  { // draw look-at sphere
    auto target = g_eye + g_look_at;
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glPushMatrix();
    {
      glTranslatef(target.x, target.y, target.z);
      glutSolidSphere(2.5, 32, 32);
    }
    glPopMatrix();
  }

  if (g_draw_grid)
  {
    glDisable(GL_LIGHTING);
    //glEnable(GL_LIGHT0);
    //glEnable(GL_LIGHT1);
    glDisable(GL_TEXTURE_2D);

    int x_pos = g_eye.x;
    int y_pos = g_eye.y;
    int z_pos = g_eye.z;

    glColor3f(1.0f, 1.0f, 1.0f);
    int n = 1;
    int n_in = 10;
    int dist = 100;
    int step = 1;
    glPushMatrix();
    
    glBegin(GL_LINES);
    {
      for(int z = z_pos - n; z <= z_pos + n; z+=step)
      {
        for(int y = y_pos - n; y <= y_pos + n; y+=step)
        {
          glVertex3i(x_pos-n_in, y, z);
          glVertex3i(x_pos+n_in, y, z);
        }

        for(int x = x_pos - n; x <= x_pos + n; x+=step)
        {
          glVertex3i(x, y_pos-n_in, z);
          glVertex3i(x, y_pos+n_in, z);
        }
      }

      for(int y = y_pos - n_in; y <= y_pos + n_in; y+=step)
      {
        for(int x = x_pos - n; x < x_pos + n; x+=step)
        {
          glVertex3i(x, y, z_pos-dist);
          glVertex3i(x, y, z_pos+dist);
        }
      }
    }
    glEnd();
    glPopMatrix();
  }

  assert_gl("draw_scene:exit()");
}

void display()
{
  glViewport(0,0, g_screen_w, g_screen_h);
  
  //log_info("display()\n");
  {
    OpenGLState state;

    if (g_render_shadow_map)
    {
      g_shadow_map->bind();
      draw_shadowmap();
      g_shadow_map->unbind();
    }

    if (g_draw_3d || g_cross_eye)
    {
      g_framebuffer1->bind();
      draw_scene(kLeftEye);
      g_framebuffer1->unbind();

      g_framebuffer2->bind();
      draw_scene(kRightEye);
      g_framebuffer2->unbind();
    }
    else
    {
      g_framebuffer1->bind();
      draw_scene(kCenterEye);
      g_framebuffer1->unbind();
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

      glEnable(GL_BLEND);

      if (g_cross_eye)
      {
        glColor3f(1.0f, 1.0f, 1.0f);
        g_framebuffer1->draw(0.0f, 0.0f, g_screen_w/2.0f, g_screen_h, -20.0f);
        g_framebuffer2->draw(g_screen_w/2.0f, 0.0f, g_screen_w/2.0f, g_screen_h, -20.0f);
      }
      else if (g_draw_3d)
      {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glColor3f(0.0f, 0.7f, 0.0f);
        g_framebuffer1->draw(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f);

        glColor3f(1.0f, 0.0f, 0.0f);
        g_framebuffer2->draw(g_ipd, 0.0f, g_screen_w, g_screen_h, -20.0f);
      }
      else
      {
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
      }

      if (g_show_menu)
      {
        glDisable(GL_BLEND);
        g_shadow_map->draw_depth(g_screen_w - 266, 10, 256, 256, -20.0f);
        g_shadow_map->draw(g_screen_w - 266 - 276, 10, 256, 256, -20.0f);
      }
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
      g_menu->draw(100.0f, 100.0f);
    }
  }

  glutSwapBuffers();
  assert_gl("display:exit()");
}

void special(int key, int x, int y)
{
  switch(key)
  {
    case GLUT_KEY_F1:
      {
        // Hitchcock zoom in
        //float old_eye_z = g_eye.z;
        //g_eye.z *= 1.005f;
        //g_fov = g_fov / std::atan(1.0f / old_eye_z) * std::atan(1.0f / g_eye.z);

        float old_fov = g_fov;
        g_fov += 1.0f;
        if (g_fov < 160.0f)
        {
          g_eye.z = g_eye.z 
            * (2.0*tan(0.5 * old_fov / 180.0 * M_PI))
            / (2.0*tan(0.5 * g_fov   / 180.0 * M_PI));
        }
        else
        {
          g_fov = 160.0f;
        }
        log_info("fov: %5.2f %f", g_fov, g_eye.z);
        log_info("w: %f", tan(g_fov /2.0f /180.0*M_PI) * g_eye.z);
      }
      break;

    case GLUT_KEY_F2:
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

    case GLUT_KEY_F10:
      glutReshapeWindow(1600, 1000);
      break;

    case GLUT_KEY_F11:
      glutFullScreen();
      break;

    case GLUT_KEY_UP:
      g_look_at += glm::normalize(g_look_at);
      break;

    case GLUT_KEY_DOWN:
      g_look_at -= glm::normalize(g_look_at);
      break;

    case GLUT_KEY_LEFT:
      //g_look_at.x += 1.0f;
      break;

    case GLUT_KEY_RIGHT:
      //g_look_at.x -= 1.0f;
      break;

    case GLUT_KEY_PAGE_UP:
      //g_look_at.y -= 1.0f;
      break;

    case GLUT_KEY_PAGE_DOWN:
      //g_look_at.y += 1.0f;
      break;

    default:
      std::cout << "unknown key: " << static_cast<int>(key) << std::endl;
      break;
  };
}

void keyboard(unsigned char key, int x, int y)
{
  switch (key) {
    case 27:
    case 'q':
      exit(EXIT_SUCCESS);
      break;

    case 'n':
      g_wiggle_offset += 0.1f;
      break;

    case 't':
      g_wiggle_offset -= 0.1f;
      break;

    case ' ':
      g_draw_look_at = !g_draw_look_at;
      break;

    case 'c':
      g_ipd += 1;
      break;

    case 'r':
      g_ipd -= 1;
      break;

    case '+':
      g_scale *= 1.05f;
      break;

    case '-':
      g_scale /= 1.05f;
      break;

    case 'z':
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

    case 'g':
      {
        GLdouble clip_plane[] = { 0.0, 1.0, 1.0, 0.0 };
        glClipPlane(GL_CLIP_PLANE0, clip_plane);
        glEnable(GL_CLIP_PLANE0);
      }
      break;

    case 'd':
      g_draw_3d = !g_draw_3d;
      break;

    case 56: // kp_up
      g_eye += glm::normalize(g_look_at);
      break;

    case 50: // kp_down
      g_eye -= glm::normalize(g_look_at);
      break;

    case 52: // kp_left
      {
        glm::vec3 dir = glm::normalize(g_look_at);
        dir = glm::rotate(dir, 90.0f, g_up);
        g_eye += dir;
      }
      break;

    case 54: // kp_right
      {
        glm::vec3 dir = glm::normalize(g_look_at);
        dir = glm::rotate(dir, 90.0f, g_up);
        g_eye -= dir;
      }
      break;

    case 55: // kp_pos1
      g_look_at = glm::rotate(g_look_at, 5.0f, g_up);
      break;

    case 57: // kp_raise
      g_look_at = glm::rotate(g_look_at, -5.0f, g_up);
      break;

    case 49: // kp_end
      g_eye -= glm::normalize(g_up);
      break;

    case 51: // kp_pgdown
      g_eye += glm::normalize(g_up);
      break;

    case 42: // kp_mult
      g_fov += 1.0f;
      break;

    case 47: // kp_div
      g_fov -= 1.0f;
      break;

    default:
      std::cout << "unknown key: " << static_cast<int>(key) << std::endl;
      break;
  }
}

void init()
{
  assert_gl("init()");
  g_framebuffer1.reset(new Framebuffer(g_screen_w, g_screen_h));
  g_framebuffer2.reset(new Framebuffer(g_screen_w, g_screen_h));
  assert_gl("init()");

  g_model = Model::from_file(g_model_filename);
  g_armature = Armature::from_file("/tmp/blender.bones");
  g_pose = Pose::from_file("/tmp/blender.pose");

  {
    g_scene_manager.reset(new SceneManager);
    const float aspect_ratio = static_cast<GLfloat>(g_screen_w)/static_cast<GLfloat>(g_screen_h);
    g_camera.reset(new Camera(g_fov, aspect_ratio, g_near_z, 100000.0f));

    if (true)
    {
      auto program = Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/basic.vert"),
                                     Shader::from_file(GL_FRAGMENT_SHADER, "src/basic.frag"));

      if (true)
      {
        auto node = g_scene_manager->get_world()->create_child();
        node->set_position(glm::vec3(1.0f, -1.0f, 0.0f));

        MaterialPtr material(new Material);
        material->set_program(program);

        auto texture = Texture::from_file("data/textures/cliff_02_v2.tga");

        UniformGroupPtr ug(new UniformGroup);
        ug->set_uniform("diffuse", glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
        ug->set_uniform("diffuse_texture", 0);
        material->set_uniform(ug);
        material->set_texture(0, texture);
        material->enable(GL_DEPTH_TEST);
        material->enable(GL_CULL_FACE);

        auto entity = Model::from_file(g_model_filename);
        entity->set_material(material);

        node->attach_entity(entity);
      }

      if (true)
      {
        auto node = g_scene_manager->get_world()->create_child();
        node->set_position(glm::vec3(1.0f, -1.0f, -5.0f));

        MaterialPtr material(new Material);
        material->set_program(program);

        auto texture = Texture::from_file("data/textures/grass_01_v1.tga");

        UniformGroupPtr ug(new UniformGroup);
        ug->set_uniform("diffuse", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
        ug->set_uniform("diffuse_texture", 0);
        material->set_uniform(ug);
        material->set_texture(0, texture);
        material->enable(GL_DEPTH_TEST);
        material->enable(GL_CULL_FACE);

        auto entity = Model::from_file(g_model_filename);
        entity->set_material(material);

        node->attach_entity(entity);
      }

      {
        auto node = g_scene_manager->get_view()->create_child();
        
        auto mesh = Mesh::create_cube(1.0f);
        ModelPtr entity = std::make_shared<Model>();
        entity->add_mesh(std::move(mesh));

        MaterialPtr material(new Material);
        material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/cubemap.vert"),
                                              Shader::from_file(GL_FRAGMENT_SHADER, "src/cubemap.frag")));

        auto texture = Texture::cube_from_file("data/textures/wireframe/");

        UniformGroupPtr ug(new UniformGroup);
        ug->set_uniform("diffuse", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
        ug->set_uniform("diffuse_texture", 0);
        material->set_uniform(ug);
        material->set_texture(0, texture);
        material->enable(GL_DEPTH_TEST);
        material->enable(GL_CULL_FACE);
        material->enable(GL_BLEND);
        material->blend_func(GL_ONE, GL_ONE);

        entity->set_material(material);

        node->attach_entity(entity);
      }
    }
  }

  //g_hello_world = TextSurface::create("Hello World", TextProperties()
  //                                    .set_line_width(3.0f));

  g_menu.reset(new Menu(TextProperties().set_line_width(4.0f)));
  g_menu->add_item("eye.x", &g_eye.x);
  g_menu->add_item("eye.y", &g_eye.y);
  g_menu->add_item("eye.z", &g_eye.z);

  g_menu->add_item("depth.near_z", &g_near_z, 0.01, 0.0f);
  g_menu->add_item("depth.far_z",  &g_far_z, 1.0f);

  g_menu->add_item("shadowmap.bias", &g_shadow_map_bias, 0.005f);
  g_menu->add_item("shadowmap.fov", &g_shadow_map_fov, 1.0f);

  g_menu->add_item("spot_halo_samples",  &g_spot_halo_samples, 1, 0);

  g_menu->add_item("FOV", &g_fov);

  g_menu->add_item("scale", &g_scale, 0.5f, 0.0f);
  g_menu->add_item("eye3D.dist", &g_wiggle_offset, 0.1f);

  g_menu->add_item("spot.cutoff",   &g_spot_cutoff);
  g_menu->add_item("spot.exponent", &g_spot_exponent);

  g_menu->add_item("light.up",  &g_light_up, 1.0f);
  g_menu->add_item("light.angle",  &g_light_angle, 1.0f);
  g_menu->add_item("light.diffuse",  &g_light_diffuse, 0.1f, 0.0f);
  g_menu->add_item("light.specular", &g_light_specular, 0.1f, 0.0f);
  g_menu->add_item("material.shininess", &g_material_shininess, 0.1f, 0.0f);

  g_menu->add_item("3D", &g_draw_3d);
  g_menu->add_item("Grid", &g_draw_grid);
  g_menu->add_item("Headlights", &g_headlights);
  g_menu->add_item("Look At Sphere", &g_draw_look_at);
  g_menu->add_item("draw depth", &g_draw_depth);
  g_menu->add_item("shadow map", &g_render_shadow_map);
  g_menu->add_item("grid.size", &g_grid_size, 0.5f);

  g_program = Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/phong.vert"),
                              Shader::from_file(GL_FRAGMENT_SHADER, "src/phong.frag"));

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

void idle_func()
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
    g_object2world = glm::rotate(g_last_object2world, glm::degrees(angle), axis_in_object_coord);
    //g_last_mouse = g_mouse;
  }

  display();
  usleep(10000);

  g_grid_offset += glm::vec4(0.0f, 0.0f, 0.001f, 0.0f);

  SDL_Event ev;
  while(SDL_PollEvent(&ev))
  {
    switch(ev.type)
    {
      case SDL_QUIT:
        exit(EXIT_SUCCESS);
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

  float delta = 0.1f;

  if (g_stick.light_rotation)
  {
    //log_debug("light angle: %f", g_light_angle);
    g_light_angle += delta * 30.0f;
  }

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

  g_old_stick = g_stick;
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

int main(int argc, char** argv)
{
  if (argc != 2)
  {
    puts("Usage: viewer [MODELFILE]");
    return EXIT_FAILURE;
  }
  else
  {
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

    SDL_Joystick* joystick = nullptr;
    log_info("SDL_NumJoysticks: %d", SDL_NumJoysticks());
    if (SDL_NumJoysticks() > 0)
    {
      joystick = SDLCALL SDL_JoystickOpen(0);
    }

    g_model_filename = argv[1];

    log_info("glutInit()");
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(g_screen_w, g_screen_h);
    //glutInitWindowPosition(100, 100);
    log_info("glutCreateWindow()");
    glutCreateWindow(argv[0]);
    log_info("glewInit()");
    glewInit();
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc(mouse_motion);      

    glutIdleFunc(idle_func);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMainLoop();

    if (joystick)
    {
      SDL_JoystickClose(joystick);
    }

    return 0; 
  }
}


// EOF //
