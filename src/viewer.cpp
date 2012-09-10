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

// global variables
namespace {

float g_ipd = 0.0f;
int g_screen_w = 1600;
int g_screen_h = 1000;
float g_fov = 70.0f;
bool g_draw_look_at = true;
GLuint g_noise_texture = 0;
bool g_draw_3d = true;

glm::vec3 g_eye(0.0f, 0.0f, 15.0f);
glm::vec3 g_look_at(0.0f, 0.0f, -100.0f);

Model* g_model;

Framebuffer* g_framebuffer1 = 0;
Framebuffer* g_framebuffer2 = 0;

float g_scale = 1.0f;

float wiggle_int = 0;
float g_wiggle_offset = 0.3f;

bool g_arcball_active = false;
glm::ivec2 g_mouse;
glm::ivec2 g_last_mouse;
glm::mat4 g_object2world;

} // namespace

void reshape(int w, int h)
{
  log_info("reshape(%d, %d)\n", w, h);
  g_screen_w = w;
  g_screen_h = h;

  assert_gl("reshape1");
  glViewport(0,0, g_screen_w, g_screen_h);
  assert_gl("reshape2");

  delete g_framebuffer1;
  delete g_framebuffer2;

  g_framebuffer1 = new Framebuffer(g_screen_w, g_screen_h);
  g_framebuffer2 = new Framebuffer(g_screen_w, g_screen_h);

  assert_gl("reshape");
}

void draw_scene()
{
  OpenGLState state;

  // clear the screen
  glClearColor(0.0, 0.0, 0.0, 0.1);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  // setup projection
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  const float aspect_ratio = static_cast<GLfloat>(g_screen_w)/static_cast<GLfloat>(g_screen_h);
  gluPerspective(g_fov /** aspect_ratio*/, aspect_ratio, 0.1f, 150.0f);

  // setup modelview
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);

  glEnable(GL_LIGHTING);

  //GLfloat light_pos[] = {0.0f, 0.0f, 20.0f, 1.0f};
  //glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

  GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 0.0 };
  GLfloat mat_shininess[] = {  0.0 };
  GLfloat mat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };

  glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT, GL_EMISSION,  mat_specular);
  glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
  
  if (true)
  {
    glEnable(GL_LIGHT0);

    GLfloat light_pos[] = {20.0f, 10.0f, 20.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    
    GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  }

  if (true)
  {
    glEnable(GL_LIGHT1);

    GLfloat light_pos[] = {-20.0f, -10.0f, -20.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light_pos);
    
    GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);

    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  }

  {
    float wiggle_offset = wiggle_int * g_wiggle_offset;

    //glTranslatef(wiggle_offset, 0.0f, 0.0f);
    gluLookAt(
      wiggle_offset + g_eye.x, g_eye.y, g_eye.z, // eye
      /*wiggle_offset + */g_eye.x + g_look_at.x, g_eye.y + g_look_at.y, g_eye.z + g_look_at.z, // look-at
      //0.0, 0.0, -100.0, // look-at
      0.0, 1.0, 0.0   // up
      );
  }

  // sphere at 0,0,0
  glutSolidSphere(1.25, 12, 12);

  if (g_draw_look_at)
  { // draw look-at sphere
    auto target = g_eye + g_look_at;
    glColor4f(1.0f, 1.0f, 1.0f, 0.5f);
    glPushMatrix();
    {
      glTranslatef(target.x, target.y, target.z);
      glutSolidSphere(2.5, 12, 12);
    }
    glPopMatrix();
  }

  glPushMatrix();
  {
    glMultMatrixf(glm::value_ptr(g_object2world));

    // draw the model
    {
      glEnable(GL_TEXTURE_2D);
      glBindTexture(GL_TEXTURE_2D, g_noise_texture);

      glColor3f(1.0, 1.0, 1.0);

      // normalizes normals to unit length 
      glEnable(GL_NORMALIZE);

      glPushMatrix();
      glScalef(g_scale, g_scale, g_scale);
      g_model->draw();
      glPopMatrix();
    }

    if (true)
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

  assert_gl("draw_scene:exit()");
}

void display()
{
  //log_info("display()\n");

  OpenGLState state;

  g_framebuffer1->bind();
  wiggle_int = 0;
  draw_scene();
  g_framebuffer1->unbind();

  g_framebuffer2->bind();
  wiggle_int = 1;
  draw_scene();
  g_framebuffer2->unbind();

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

      glColor3f(0.0f, 1.0f, 0.0f);
      g_framebuffer1->draw(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f);

      glColor3f(1.0f, 0.0f, 0.0f);
      g_framebuffer2->draw(g_ipd, 0.0f, g_screen_w, g_screen_h, -20.0f);
    }
    else
    {
      glBlendFunc(GL_ONE, GL_ZERO);

      glColor3f(1.0f, 1.0f, 1.0f);
      g_framebuffer1->draw(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f);
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
      g_look_at.z -= 1.0f;
      break;

    case GLUT_KEY_DOWN:
      g_look_at.z += 1.0f;
      break;

    case GLUT_KEY_LEFT:
      g_look_at.x += 1.0f;
      break;

    case GLUT_KEY_RIGHT:
      g_look_at.x -= 1.0f;
      break;

    case GLUT_KEY_PAGE_UP:
      g_look_at.y -= 1.0f;
      break;

    case GLUT_KEY_PAGE_DOWN:
      g_look_at.y += 1.0f;
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
      g_eye.z -= 0.1;
      break;

    case 50: // kp_down
      g_eye.z += 0.1;
      break;

    case 52: // kp_left
      g_eye.x -= 0.1;
      break;

    case 54: // kp_right
      g_eye.x += 0.1;
      break;

    case 57: // kp_raise
      g_eye.y += 0.1;
      break;

    case 51: // kp_lower
      g_eye.y -= 0.1;
      break;

    case 55: // kp_pos1
      g_fov += 1.0f;
      break;

    case 49: // kp_end
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
  // g_framebuffer1 = new Framebuffer(g_screen_w, g_screen_h);
  // g_framebuffer2 = new Framebuffer(g_screen_w, g_screen_h);
  assert_gl("init()");

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

  assert_gl("init()");
}

void mouse(int button, int button_pressed, int x, int y)
{
  log_info("mouse: %d %d - %d %d", x, y, button, button_pressed);

  if (button == 0)
  {
    if (!button_pressed)
    {
      log_info("mouse: arcball: %d %d", x, y);
      g_arcball_active = true;
      g_mouse = g_last_mouse = glm::ivec2(x, y);
    }
    else
    {
      g_arcball_active = false;
    }
  }
}

glm::vec3 get_arcball_vector(glm::ivec2 mouse)
{
  glm::vec3 P = glm::vec3(1.0 * mouse.x / std::max(g_screen_w, g_screen_w)*2 - 1.0,
                          1.0 * mouse.y / std::max(g_screen_w, g_screen_h)*2 - 1.0,
                          0);
  P.y = -P.y;
  float OP_squared = P.x * P.x + P.y * P.y;
  if (OP_squared <= 1*1)
    P.z = sqrt(1*1 - OP_squared);  // Pythagore
  else
    P = glm::normalize(P);  // nearest point
  return P;
}

void idle_func()
{
  if (g_arcball_active && g_mouse != g_last_mouse)
  {
    glm::mat4 camera_matrix;

    glGetFloatv(GL_MODELVIEW_MATRIX, glm::value_ptr(camera_matrix));

    glm::vec3 va = get_arcball_vector(g_last_mouse);
    glm::vec3 vb = get_arcball_vector(g_mouse);
    float angle = acos(std::min(1.0f, glm::dot(va, vb)));
    glm::vec3 axis_in_camera_coord = glm::cross(va, vb);
    glm::mat3 camera2object = glm::inverse(glm::mat3(camera_matrix) * glm::mat3(g_object2world));
    glm::vec3 axis_in_object_coord = camera2object * axis_in_camera_coord;
    g_object2world = glm::rotate(g_object2world, glm::degrees(angle), axis_in_object_coord);
    g_last_mouse = g_mouse;
  }

  display();
  usleep(30000);
  glutForceJoystickFunc();
}

void joystick_callback(unsigned int buttonMask, int x, int y, int z)
{
  std::cout << "BM: " << buttonMask << " " << x << " " << y << " " << z << std::endl;
}

void mouse_motion(int x, int y)
{
  if (g_arcball_active)
  {
    log_info("mouse motion: arcball: %d %d", x, y);
    g_mouse.x = x;
    g_mouse.y = y;
    auto v = get_arcball_vector(g_mouse);
    log_info("vec: %f %f", v.x, v.y);
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
    std::unique_ptr<Model> model{new Model(argv[1])};
    g_model = model.get();

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
    glutJoystickFunc(joystick_callback, 1);

    glutIdleFunc(idle_func);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(special);
    glutMainLoop();

    return 0; 
  }
}


// EOF //
