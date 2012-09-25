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

#include "log.hpp"
#include "assert_gl.hpp"
#include "mesh.hpp"
#include "model.hpp"
#include "opengl_state.hpp"
#include "framebuffer.hpp"
#include "text_surface.hpp"
#include "menu.hpp"
#include "program.hpp"
#include "shader.hpp"

void draw_models(bool shader_foo);

// global variables
namespace {

std::unique_ptr<Menu> g_menu;

float g_ipd = 0.0f;
int g_screen_w = 1280;
int g_screen_h = 800;
float g_fov = 70.0f;

float g_near_z = 1.0f;
float g_far_z  = 1000.0f;

int g_spot_halo_samples = 100;

bool g_draw_look_at = true;
GLuint g_noise_texture = 0;
GLuint g_light_texture = 0;
GLuint g_cube_texture = 0;
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

glm::vec3 g_eye(0.0f, 0.0f, 15.0f);
glm::vec3 g_look_at(0.0f, 0.0f, -100.0f);
glm::vec3 g_up(0.0f, 1.0f, 0.0f);
glm::mat4 g_shadow_map_matrix;
glm::vec4 g_grid_offset;
float g_grid_size = 2.0f;

std::string g_model_filename;
Model* g_model;

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

void draw_scene(EyeType eye_type)
{
  OpenGLState state;

  glViewport(0,0, g_screen_w, g_screen_h);

  // clear the screen
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glEnable(GL_NORMALIZE);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  
  float ambient[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
  glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);

  // must be set for correct specular reflections
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);

  if (g_headlights)
  {
    //log_debug("headlights");
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT1);

    glLightf(GL_LIGHT1, GL_CONSTANT_ATTENUATION,  0.0f);
    glLightf(GL_LIGHT1, GL_LINEAR_ATTENUATION,    0.0f);
    glLightf(GL_LIGHT1, GL_QUADRATIC_ATTENUATION, 0.25f);

    GLfloat light_pos[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light_pos);
   
    GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

    GLfloat light_diffuse[] = {2.0f, 2.0f, 2.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);

    GLfloat light_specular[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);

    glLightf( GL_LIGHT1, GL_SPOT_CUTOFF,   g_spot_cutoff );
    glLightf( GL_LIGHT1, GL_SPOT_EXPONENT, g_spot_exponent );

    GLfloat light_direction[] = { 0.0f, 0.0f, -1.0f, 1.0f };
    glLightfv(GL_LIGHT1, GL_SPOT_DIRECTION, light_direction);
  }

  // setup projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  const float aspect_ratio = static_cast<GLfloat>(g_screen_w)/static_cast<GLfloat>(g_screen_h);
  gluPerspective(g_fov /** aspect_ratio*/, aspect_ratio, g_near_z, 100000.0f);

  // setup modelview
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  {
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

    //glTranslatef(wiggle_offset, 0.0f, 0.0f);
    gluLookAt(
      g_eye.x + sideways.x, 
      g_eye.y + sideways.y, 
      g_eye.z + sideways.z, // eye
      /*wiggle_offset + */g_eye.x + g_look_at.x, g_eye.y + g_look_at.y, g_eye.z + g_look_at.z, // look-at
      //0.0, 0.0, -100.0, // look-at
      g_up.x, g_up.y, g_up.z);

    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(g_eye_matrix));
  }

  // light after gluLookAt() put it in worldspace, light before gluLookAt() puts it in eye space
  if (true)
  {
    GLfloat light_pos[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glDisable(GL_LIGHTING);

    glPushMatrix();

    glRotatef(g_light_angle, 0.0f, 1.0f, 0.0f);
    glTranslatef(10.0f,10,0.0f);

    if (true)
    {
      glEnable(GL_DEPTH_TEST);
      glDisable(GL_CULL_FACE);

      glPushMatrix();
      glTranslatef(light_pos[0], light_pos[1], light_pos[2]);
      glColor3f(1.0f, 1.0f, 1.0f);
      glutSolidSphere(1, 12, 12);
      glPopMatrix();
    }

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);

    //glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  0.0f);
    //glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    0.0f);
    //glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.05f);

    glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION,  0.0f);
    glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION,    0.2f);
    glLightf(GL_LIGHT0, GL_QUADRATIC_ATTENUATION, 0.0f);
    
    GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    GLfloat light_diffuse[] = { g_light_diffuse, g_light_diffuse, g_light_diffuse, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    GLfloat light_specular[] = { g_light_specular, g_light_specular, g_light_specular, 1.0f};
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
    
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

    glPopMatrix();
  }

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  draw_models(true);
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

  glm::vec3 light_pos(10.0f,10,0.0f);
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

void draw_cubemap()
{
  OpenGLState state;

  glEnable(GL_NORMALIZE);
  glDisable(GL_DEPTH_TEST);

  glDisable(GL_LIGHTING);

  glActiveTexture(GL_TEXTURE0);
  glEnable(GL_TEXTURE_CUBE_MAP);
  glBindTexture(GL_TEXTURE_CUBE_MAP, g_cube_texture);

  /*
    glTexGeni(GL_S, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glTexGeni(GL_T, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glTexGeni(GL_R, GL_TEXTURE_GEN_MODE, GL_REFLECTION_MAP);
    glEnable(GL_TEXTURE_GEN_S);
    glEnable(GL_TEXTURE_GEN_T);
    glEnable(GL_TEXTURE_GEN_R);
  */

  float d = 5.0f;
  float t = 1.0f;
  float n = 1.0f;
  
  glPushMatrix();
  glTranslatef(g_eye.x, g_eye.y, g_eye.z);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  glBegin(GL_QUADS);
  { 
    // front
    glNormal3f(-n,  n, -n); glTexCoord3f(-t,  t, -t); glVertex3f(-d,  d, -d);
    glNormal3f( n,  n, -n); glTexCoord3f( t,  t, -t); glVertex3f( d,  d, -d);
    glNormal3f( n, -n, -n); glTexCoord3f( t, -t, -t); glVertex3f( d, -d, -d);
    glNormal3f(-n, -n, -n); glTexCoord3f(-t, -t, -t); glVertex3f(-d, -d, -d);

    // back
    glNormal3f(-n, -n, n); glTexCoord3f(-t, -t, t); glVertex3f(-d, -d, d);
    glNormal3f( n, -n, n); glTexCoord3f( t, -t, t); glVertex3f( d, -d, d);
    glNormal3f( n,  n, n); glTexCoord3f( t,  t, t); glVertex3f( d,  d, d);
    glNormal3f(-n,  n, n); glTexCoord3f(-t,  t, t); glVertex3f(-d,  d, d);

    // left
    glNormal3f(-n,  n,  n); glTexCoord3f(-t,  t,  t); glVertex3f(-d,  d,  d);
    glNormal3f(-n,  n, -n); glTexCoord3f(-t,  t, -t); glVertex3f(-d,  d, -d);
    glNormal3f(-n, -n, -n); glTexCoord3f(-t, -t, -t); glVertex3f(-d, -d, -d);
    glNormal3f(-n, -n,  n); glTexCoord3f(-t, -t,  t); glVertex3f(-d, -d,  d);

    // right
    glNormal3f( n, -n,  n); glTexCoord3f( t, -t,  t); glVertex3f(d, -d,  d);
    glNormal3f( n, -n, -n); glTexCoord3f( t, -t, -t); glVertex3f(d, -d, -d);
    glNormal3f( n,  n, -n); glTexCoord3f( t,  t, -t); glVertex3f(d,  d, -d);
    glNormal3f( n,  n,  n); glTexCoord3f( t,  t,  t); glVertex3f(d,  d,  d);

    // top
    glNormal3f( n,  n, -n); glTexCoord3f( t,  t, -t); glVertex3f( d,  d, -d);
    glNormal3f(-n,  n, -n); glTexCoord3f(-t,  t, -t); glVertex3f(-d,  d, -d);
    glNormal3f(-n,  n,  n); glTexCoord3f(-t,  t,  t); glVertex3f(-d,  d,  d);
    glNormal3f( n,  n,  n); glTexCoord3f( t,  t,  t); glVertex3f( d,  d,  d);

    // bottom
    glNormal3f(-n, -n,  n); glTexCoord3f(-t, -t,  t); glVertex3f(-d, -d,  d);
    glNormal3f( n, -n,  n); glTexCoord3f( t, -t,  t); glVertex3f( d, -d,  d);
    glNormal3f( n, -n, -n); glTexCoord3f( t, -t, -t); glVertex3f( d, -d, -d);
    glNormal3f(-n, -n, -n); glTexCoord3f(-t, -t, -t); glVertex3f(-d, -d, -d);
  }
  glEnd();
  glPopMatrix();
}

void draw_models(bool shader_foo)
{
  draw_cubemap();

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

  glPushMatrix();
  {
    glMultMatrixf(glm::value_ptr(g_object2world));

    // draw the model
    {
      OpenGLState state2;

      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, g_noise_texture);
      glColor3f(1.0, 1.0, 1.0);
      
      if (shader_foo)
      {
        g_program->validate();
        if (!g_program->get_validate_status())
        {
          log_debug("validation failure: %s", g_program->get_info_log());
        }

        glActiveTexture(GL_TEXTURE1);
        //glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	
        glBindTexture(GL_TEXTURE_2D, g_shadow_map->get_depth_texture());
        
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
	//glTexParameteri(GL_TEXTURE_2D, GL_DEPTH_TEXTURE_MODE, GL_INTENSITY); 
        
        glActiveTexture(GL_TEXTURE2);
        glEnable(GL_TEXTURE_CUBE_MAP);
        glBindTexture(GL_TEXTURE_CUBE_MAP, g_cube_texture);

        assert_gl("use program4");
        glUseProgram(g_program->get_id());
        assert_gl("use program5");
        assert_gl("use program2");
        glUniform1f(glGetUniformLocation(g_program->get_id(), "shadowmap_bias"), g_shadow_map_bias);
        glUniform1i(glGetUniformLocation(g_program->get_id(), "tex"), 0);
        glUniform1i(glGetUniformLocation(g_program->get_id(), "ShadowMap"), 1);
        glUniform1i(glGetUniformLocation(g_program->get_id(), "cubemap"), 2);

        glUniform4fv(glGetUniformLocation(g_program->get_id(), "world_eye_pos"), 4, glm::value_ptr(g_eye));
        
        glUniform4fv(glGetUniformLocation(g_program->get_id(), "grid_offset"), 4, glm::value_ptr(g_grid_offset));
        glUniform1f(glGetUniformLocation(g_program->get_id(), "grid_size"), g_grid_size);

        assert_gl("use program3");

        glUniform1f(glGetUniformLocation(g_program->get_id(), "xPixelOffset"), 1.0f/static_cast<float>(g_shadow_map->get_width()));
        glUniform1f(glGetUniformLocation(g_program->get_id(), "yPixelOffset"), 1.0f/static_cast<float>(g_shadow_map->get_height()));

        glUniformMatrix4fv(glGetUniformLocation(g_program->get_id(), "ShadowMapMatrix"),
                           1, GL_FALSE,
                           glm::value_ptr(g_shadow_map_matrix));

        glUniformMatrix4fv(glGetUniformLocation(g_program->get_id(), "eye_matrix"), 1, GL_FALSE, glm::value_ptr(glm::inverse(g_eye_matrix)));
      }

      int dim = 0;
      for(int y = -dim; y <= dim; ++y)
        for(int x = -dim; x <= dim; ++x)
        {
          glPushMatrix();
          glScalef(g_scale, g_scale, g_scale);
          glTranslatef(20*x, 0, 20*y);
          g_model->draw();
          if (true)
          {
            float plane_size = 50.0f;
            float plane_y = 0.0f;
            glBegin(GL_QUADS);
            {
              glNormal3f(0.0f, 1.0f, 0.0f);
              glTexCoord2f(0.0f, 0.0f);
              glVertex3f(-plane_size, plane_y, -plane_size);

              glNormal3f(0.0f, 1.0f, 0.0f);
              glTexCoord2f(1.0f, 0.0f);
              glVertex3f(-plane_size, plane_y,  plane_size);

              glNormal3f(0.0f, 1.0f, 0.0f);
              glTexCoord2f(1.0f, 1.0f);
              glVertex3f( plane_size, plane_y,  plane_size);

              glNormal3f(0.0f, 1.0f, 0.0f);
              glTexCoord2f(0.0f, 1.0f);
              glVertex3f( plane_size, plane_y, -plane_size);
            }
            glEnd();
          }
          glPopMatrix();
        }

      if (shader_foo)
      {
        glUseProgram(0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, 0);
      }
    }

    if (false)
    { // draw starfield
      srand(0);
      int box = 50;
      for(int i = 0; i < 200; ++i)
      {
        glPushMatrix();  
        glTranslatef(rand()%box - box/2, 
                     rand()%box - box/2, 
                     rand()%box - box/2);
        glutSolidSphere(0.5, 12, 12);
        glPopMatrix();
      }
    }
  }
  glPopMatrix();

  {
    glPushMatrix();

    //GLfloat fSizes[2];
    //glGetFloatv(GL_ALIASED_POINT_SIZE_RANGE, fSizes);
    //log_debug("point size: %f %f", fSizes[0], fSizes[1]);
    // 1.000000 8192.000000

    OpenGLState save_state;
    glDisable(GL_LIGHTING);
    //glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glEnable(GL_POINT_SPRITE);
 
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, g_light_texture);


    glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

    glTranslatef(0.0f, 0.0f, 1.0f);

    float quadratic[] =  { 0.0f, 0.0f, 1.0f };
    glPointParameterfv( GL_POINT_DISTANCE_ATTENUATION, quadratic );

    glPointParameterf(GL_POINT_SIZE_MIN, 16.0f);
    glPointParameterf(GL_POINT_SIZE_MAX, 1000.0f);
    
    glPointSize(200.0f);

    //glPointParameterfARB( GL_POINT_FADE_THRESHOLD_SIZE, 200.0f );

    glColor4f(1.0, 1.0, 1.0, 0.1f);
    glBegin(GL_POINTS);
    for(int i = 0; i < 60; ++i)
    {
      glVertex3f(i/60.0f,  1.0f, 0.0f);
    }
    glEnd();
    
    glEnable(GL_LIGHTING);

    glPopMatrix();
  }

  {
    OpenGLState gl_state;
    glDisable(GL_LIGHTING);

    glPushMatrix();
    {
      glm::mat4 mat;
      glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(mat));

      //glm::mat3 rot = glm::mat3(mat);
      //mat = mat * 
      
      glTranslatef(0.0f, 0.0f, -3.0f);
      
      glDisable(GL_TEXTURE_2D);

      glColor3f(1.0f, 1.0f, 1.0f);
      glPushMatrix();
      glutSolidSphere(0.25, 16, 16);
      glPopMatrix();

      if (true)
      {
        glDepthMask(GL_FALSE);
        glEnable(GL_TEXTURE_2D);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        if (false)
        {
          // billboard
          glMultMatrixf(glm::value_ptr(glm::mat4(glm::transpose(glm::mat3(mat)))));

          glBindTexture(GL_TEXTURE_2D, g_light_texture);

          glBegin(GL_QUADS);
          {
            glTexCoord2f(0.0f, 0.0f);
            glVertex3f(-1.0f, -1.0f, 0.0f);

            glTexCoord2f(1.0f, 0.0f);
            glVertex3f(1.0f, -1.0f, 0.0f);

            glTexCoord2f(1.0f, 1.0f);
            glVertex3f(1.0f, 1.0f, 0.0f);

            glTexCoord2f(0.0f, 1.0f);
            glVertex3f(-1.0f, 1.0f, 0.0f);
          }
          glEnd();
        }
        glDepthMask(GL_TRUE);
      }
    }
    glPopMatrix();
  }

  if (true)
  {
    glm::mat4 mat;

    glDisable(GL_LIGHTING);
    glEnable(GL_TEXTURE_2D);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    
    float g_spot_halo_length = 5.0f;
    int n = g_spot_halo_samples;
    for(int i = 0; i < n; ++i)
    {
      float p = static_cast<float>(i) / static_cast<float>(n-1);

      glPushMatrix();
      {
        glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(mat));

        glScalef(0.1f + 1.0f * p * p,
                 0.1f + 1.0f * p * p,
                 0.1f + 1.0f * p * p);
        glTranslatef(0.0f, 0.0f, g_spot_halo_length * static_cast<float>(i) / static_cast<float>(n));

        // billboard
        glMultMatrixf(glm::value_ptr(glm::mat4(glm::transpose(glm::mat3(mat)))));

        glBindTexture(GL_TEXTURE_2D, g_light_texture);

        glColor4f(1.0f, 1.0f, 1.0f, 0.25f * (1.0f-p) * (100.0f / g_spot_halo_samples));
        glBegin(GL_QUADS);
        {
          glTexCoord2f(0.0f, 0.0f);
          glVertex3f(-1.0f, -1.0f, 0.0f);

          glTexCoord2f(1.0f, 0.0f);
          glVertex3f(1.0f, -1.0f, 0.0f);

          glTexCoord2f(1.0f, 1.0f);
          glVertex3f(1.0f, 1.0f, 0.0f);

          glTexCoord2f(0.0f, 1.0f);
          glVertex3f(-1.0f, 1.0f, 0.0f);
        }
        glEnd();
      }
      glPopMatrix();
    }
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

    if (g_draw_3d)
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

      if (g_draw_3d)
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

      if (g_render_shadow_map)
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

    g_menu->draw(100.0f, 100.0f);
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

  g_model = new Model(g_model_filename);

  { // upload noise texture
    glGenTextures(1, &g_noise_texture);
    glBindTexture(GL_TEXTURE_2D, g_noise_texture);
   
    const int width = 64;
    const int height = 64;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

    unsigned char data[width*height*3];

    for(size_t i = 0; i < sizeof(data); i+=3)
    {
      data[i+0] = data[i+1] = data[i+2] = rand() % 255;
    }

    gluBuild2DMipmaps
      (GL_TEXTURE_2D, GL_RGB,
       width, height,
       GL_RGB, GL_UNSIGNED_BYTE, data);
    assert_gl("texture0()");
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    assert_gl("texture-0()");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    assert_gl("texture-1()");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    assert_gl("texture-2()");
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    assert_gl("texture1()");

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
    assert_gl("texture2()");
  }

  {
    glGenTextures(1, &g_light_texture);
    glBindTexture(GL_TEXTURE_2D, g_light_texture);

    const int width = 64;
    const int height = 64;
    const int pitch = width * 3;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

    unsigned char data[width*height*3];
    for(int y = 0; y < height; ++y)
      for(int x = 0; x < width; ++x)
      {
        float xf = (static_cast<float>(x) / static_cast<float>(width)  - 0.5f) * 2.0f;
        float yf = (static_cast<float>(y) / static_cast<float>(height) - 0.5f) * 2.0f;
        
        float f = 1.0f - sqrtf(xf*xf + yf*yf);

        data[y * pitch + 3*x+0] = static_cast<uint8_t>(std::max(0.0f, std::min(f * 255.0f, 255.0f)));
        data[y * pitch + 3*x+1] = static_cast<uint8_t>(std::max(0.0f, std::min(f * 255.0f, 255.0f)));
        data[y * pitch + 3*x+2] = static_cast<uint8_t>(std::max(0.0f, std::min(f * 255.0f, 255.0f)));
      }

    gluBuild2DMipmaps
      (GL_TEXTURE_2D, GL_RGB,
       width, height,
       GL_RGB, GL_UNSIGNED_BYTE, data);
    assert_gl("texture0()");
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
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

  g_program = Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/shadowmap.vert"),
                              Shader::from_file(GL_FRAGMENT_SHADER, "src/shadowmap.frag"));

  {
    OpenGLState cube_state;

    SDL_Surface* up = IMG_Load("data/textures/miramar_up.tga");
    SDL_Surface* dn = IMG_Load("data/textures/miramar_dn.tga");
    SDL_Surface* ft = IMG_Load("data/textures/miramar_ft.tga");
    SDL_Surface* bk = IMG_Load("data/textures/miramar_bk.tga");
    SDL_Surface* lf = IMG_Load("data/textures/miramar_lf.tga");
    SDL_Surface* rt = IMG_Load("data/textures/miramar_rt.tga");

    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, up->pitch / up->format->BytesPerPixel);

    glActiveTexture(GL_TEXTURE0);
    glEnable(GL_TEXTURE_CUBE_MAP);
    glGenTextures(1, &g_cube_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, g_cube_texture);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
   
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_X, GL_RGB, lf->w, lf->h, lf->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, lf->pixels);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, GL_RGB, rt->w, rt->h, rt->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, rt->pixels);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, GL_RGB, up->w, up->h, up->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, up->pixels);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, GL_RGB, dn->w, dn->h, dn->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, dn->pixels);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, GL_RGB, ft->w, ft->h, ft->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, ft->pixels);
    gluBuild2DMipmaps(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, GL_RGB, bk->w, bk->h, bk->format->BytesPerPixel == 4 ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, bk->pixels);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_BASE_LEVEL, 0);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAX_LEVEL, 10);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    SDL_FreeSurface(up);
    SDL_FreeSurface(dn);
    SDL_FreeSurface(ft);
    SDL_FreeSurface(bk);
    SDL_FreeSurface(lf);
    SDL_FreeSurface(rt);

    assert_gl("cube texture");
  }

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

    log_info("glutInit()\n");
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(g_screen_w, g_screen_h);
    //glutInitWindowPosition(100, 100);
    log_info("glutCreateWindow()\n");
    glutCreateWindow(argv[0]);
    log_info("glewInit\n");
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
