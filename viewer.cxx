#include <unistd.h>
#include <math.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

int g_screen_w = 1600;
int g_screen_h = 1000;

inline void assert_gl(const char* message)
{ // FIXME: OpenGL stuff should go into display/
  GLenum error = glGetError();
  if(error != GL_NO_ERROR) 
  {
    std::ostringstream msg;
    msg << "OpenGLError while '" << message << "': "
        << gluErrorString(error);
    throw std::runtime_error(msg.str());
  }
}

class OpenGLState
{
public:
  OpenGLState()
  {
    glPushAttrib(GL_ALL_ATTRIB_BITS);
    glPushClientAttrib(GL_CLIENT_ALL_ATTRIB_BITS);
  }

  ~OpenGLState()
  {
    glPopClientAttrib();
    glPopAttrib();
  }
};

class Framebuffer
{
public:
  Framebuffer(int width, int height) :
    m_fbo(0),
    m_color_buffer(0),
    m_depth_buffer(0)
  {
    OpenGLState state;

    // create the framebuffer
    glGenFramebuffers(1, &m_fbo);
    assert_gl("framebuffer");
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    assert_gl("framebuffer");

    // create color buffer texture
    glGenTextures(1, &m_color_buffer);
    glBindTexture(GL_TEXTURE_2D, m_color_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,  width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    assert_gl("framebuffer");

    // create depth buffer texture
    glGenTextures(1, &m_depth_buffer);
    glBindTexture(GL_TEXTURE_2D, m_depth_buffer);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,  width, height, 0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    assert_gl("framebuffer");
    
    // attach color and depth buffer
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_color_buffer, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,  GL_TEXTURE_2D, m_depth_buffer, 0);
     assert_gl("framebuffer");

    GLenum complete = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (complete != GL_FRAMEBUFFER_COMPLETE)
    {
      std::cout << "Framebuffer incomplete: " << complete << std::endl;
    }
    assert_gl("framebuffer");

    std::cout << "FBO: " << m_fbo << std::endl;
    std::cout << "Depth Buffer: " << m_depth_buffer << std::endl;
    std::cout << "Color Buffer: " << m_color_buffer << std::endl;

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }
  
  ~Framebuffer()
  {
    glDeleteFramebuffers(1, &m_fbo);
    glDeleteTextures(1, &m_depth_buffer);
    glDeleteTextures(1, &m_color_buffer);
  }

  void draw(float x, float y, float w, float h, float z)
  {
    OpenGLState state;
    
    glEnable(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, m_color_buffer);
    glBegin(GL_QUADS);
    {
      glTexCoord2f(0.0f, 0.0f);
      glVertex3f(x, y, z);

      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(x+w, y, z);

      glTexCoord2f(1.0f, 1.0f);
      glVertex3f(x+w, y+h, z); // FIXME

      glTexCoord2f(0.0f, 1.0f);
      glVertex3f(x, y+h, z);
    }
    glEnd();  
  }

  void bind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
  }

  void unbind()
  {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

private:
  GLuint m_fbo;
  GLuint m_color_buffer;
  GLuint m_depth_buffer;

private:
  Framebuffer(const Framebuffer&) = delete;
  Framebuffer& operator=(const Framebuffer&) = delete;
};

struct Face
{
  int vertex1;
  int vertex2;
  int vertex3;
};

struct Vector
{
  float x;
  float y;
  float z;

  Vector () : x(0), y(0), z(0) {}
  
  Vector (float x_, float y_, float z_) :
    x(x_),
    y(y_),
    z(z_)
  {}

  float norm ()
  {
    return sqrt(x*x + y*y + z*z);
  }

  void normalize ()
  {
    float f = norm ();
    x /= f;
    y /= f;
    z /= f;
  }
};

Vector operator*(float f, const Vector& a)
{
  return Vector (a.x * f,
                 a.y * f,
                 a.z * f);
}

Vector operator*(const Vector& a, float f)
{
  return Vector (a.x * f,
                 a.y * f,
                 a.z * f);
}

Vector operator-(const Vector& a, const Vector& b)
{
  return Vector (a.x - b.x,
                 a.y - b.y,
                 a.z - b.z);
}

Vector operator+(const Vector& a, const Vector& b)
{
  return Vector (a.x + b.x,
                 a.y + b.y,
                 a.z + b.z);
}

typedef Vector Vertex;

class Mesh
{
private:
  typedef std::vector<Vertex> VertexLst;
  typedef std::vector<Face>   FaceLst;

  VertexLst vertices;
  FaceLst   faces;

public:
  Mesh (std::istream& in) :
    vertices(),
    faces()
  {
    int number_of_vertixes;
    in >> number_of_vertixes;
    //std::cout << "Number_Of_Vertixes: " << number_of_vertixes << std::endl;
    for (int i = 0; i < number_of_vertixes; ++i)
    {
      Vertex vertex;
      in >> vertex.x >> vertex.y >> vertex.z;
      vertices.push_back(vertex);
    }

    int number_of_faces;
    in >> number_of_faces;
    for (int i = 0; i < number_of_faces; ++i)
    {
      Face face;
      in >> face.vertex1 >> face.vertex2 >> face.vertex3;
      faces.push_back (face);
    }

    //std::cout << "read " << number_of_faces << " faces, " << number_of_vertixes << " vertices" << std::endl;
  }

  void display ()
  {
    for (FaceLst::iterator i = faces.begin (); i != faces.end (); ++i)
    {
      std::cout << "Face: " << std::endl;
      std::cout << vertices[i->vertex1].x << " "
                << vertices[i->vertex1].y << " "
                << vertices[i->vertex1].z << std::endl;
      std::cout << vertices[i->vertex2].x << " "
                << vertices[i->vertex2].y << " "
                << vertices[i->vertex2].z << std::endl;
      std::cout << vertices[i->vertex3].x << " "
                << vertices[i->vertex3].y << " "
                << vertices[i->vertex3].z << std::endl;
    }    
  }

  void draw () 
  {
    for (FaceLst::iterator i = faces.begin (); i != faces.end (); ++i)
    {
      glEnable(GL_COLOR_MATERIAL);
      //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT);
      //glColorMaterial(GL_FRONT_AND_BACK, GL_DIFFUSE);

      glColor3f (1.0, 1.0, 1.0);

      glBegin (GL_TRIANGLES);

      glTexCoord2f(0.0f, 0.0f);
      set_face_normal (*i);
      glVertex3f (vertices[i->vertex1].x,
                  vertices[i->vertex1].y,
                  vertices[i->vertex1].z);
        

      glTexCoord2f(1.0f, 0.0f);
      set_face_normal (*i);
      glVertex3f (vertices[i->vertex2].x,
                  vertices[i->vertex2].y,
                  vertices[i->vertex2].z);

      glTexCoord2f(1.0f, 1.0f);
      set_face_normal (*i);
      glVertex3f (vertices[i->vertex3].x,
                  vertices[i->vertex3].y,
                  vertices[i->vertex3].z);
      glEnd ();
        
      //draw_face_normal (*i);
      glDisable (GL_COLOR_MATERIAL);
    }
  }

  void draw_face_normal (const Face& face)
  {
    Vector normal = calc_face_normal (face);
    
    Vector pos(vertices[face.vertex1]
               + vertices[face.vertex2]
               + vertices[face.vertex3]);
    pos = pos * (1.0f/3.0f);

    glDisable (GL_LIGHTING);
    glColor3f (0, 1.0f, 1.0f);
    glBegin (GL_LINES);
    glVertex3f (pos.x, pos.y, pos.z);
    pos = pos + (0.5* normal);
    glVertex3f (pos.x, pos.y, pos.z);
    glEnd ();
    glEnable (GL_LIGHTING);
  }

  Vector calc_face_normal (const Face& face)
  {
    Vector u (vertices[face.vertex2].x - vertices[face.vertex1].x,
              vertices[face.vertex2].y - vertices[face.vertex1].y,
              vertices[face.vertex2].z - vertices[face.vertex1].z);

    Vector v (vertices[face.vertex3].x - vertices[face.vertex2].x,
              vertices[face.vertex3].y - vertices[face.vertex2].y,
              vertices[face.vertex3].z - vertices[face.vertex2].z);

    Vector normal (u.y*v.z - u.z*v.y,
                   u.z*v.x - u.x*v.z,
                   u.x*v.y - u.y*v.x);

    normal.normalize ();

    return normal;
  }

  void set_face_normal (const Face& face)
  {
    const Vector& normal = calc_face_normal (face);
    glNormal3f (normal.x, normal.y, normal.y);
  }
};

class Model
{
private:
  typedef std::vector<Mesh> MeshLst;
  MeshLst meshes;

public:
  Model(const std::string& filename) :
    meshes()
  {
    std::ifstream in(filename.c_str());
    
    if (!in)
    {
      std::cout << filename << ": File not found" << std::endl;
      exit(EXIT_FAILURE);
    }

    int number_of_meshes;
    in >> number_of_meshes;
    std::cout << "number_of_meshes: " << number_of_meshes << std::endl;
    for (int i = 0; i < number_of_meshes; ++i)   
    {
      Mesh mesh(in);
      //mesh.display ();
      meshes.push_back(mesh);
    }
  }

  void draw () 
  {
    for (MeshLst::iterator i = meshes.begin (); i != meshes.end (); ++i)
    {
      i->draw ();
    }
  }
  
};


Vector g_eye;

Model* the_model;

Framebuffer* g_framebuffer1 = 0;
Framebuffer* g_framebuffer2 = 0;

void reshape(int w, int h)
{
  g_screen_w = w;
  g_screen_h = h;

  glViewport (0,0, g_screen_w, g_screen_h);

  //g_framebuffer1 = new Framebuffer(g_screen_w, g_screen_h);
  //g_framebuffer2 = new Framebuffer(g_screen_w, g_screen_h);
}

float x_angle = -90.0f;
float y_angle = 0.0f;
float z_angle = 0.0f;
float zoom = -15.0;

bool wiggle = false;
float wiggle_int = 0;
float g_wiggle_offset = 0.3f;

void draw_scene();

void display()
{
  OpenGLState state;

  wiggle = true;

  g_framebuffer1->bind();
  wiggle_int = 0;
  draw_scene();
  g_framebuffer1->unbind();

  g_framebuffer2->bind();
  wiggle_int = 1;
  draw_scene();
  g_framebuffer2->unbind();

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

    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glColor3f(0.0f, 1.0f, 0.0f);
    g_framebuffer1->draw(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f);

    glColor3f(1.0f, 0.0f, 0.0f);
    g_framebuffer2->draw(0.0f, 0.0f, g_screen_w, g_screen_h, -20.0f);
  }

  glutSwapBuffers();
}

void draw_scene()
{
  OpenGLState state;

  glClearColor(0.0, 0.0, 0.0, 0.1);
  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, static_cast<GLfloat>(g_screen_w)/static_cast<GLfloat>(g_screen_h), 1.0, 150.0f);
  
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  glEnable(GL_TEXTURE_2D);

  if (wiggle)
  {
    float wiggle_offset = wiggle_int * g_wiggle_offset;

    //glTranslatef(wiggle_offset, 0.0f, 0.0f);
    gluLookAt(
      wiggle_offset + g_eye.x, g_eye.y, g_eye.z, // eye
      /*wiggle_offset + */g_eye.x, g_eye.y, g_eye.z-100.0f, // look-at
      //0.0, 0.0, -100.0, // look-at
      0.0, 1.0, 0.0   // up
      );
  }

  GLfloat light_pos[] = {0.0f, 0.0f, 20.0f, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

  glTranslatef(0.0, 0.0, zoom);

  glRotatef (x_angle, 1.0f, 0.0f, 0.0f);
  glRotatef (y_angle, 0.0f, 1.0f, 0.0f);
  glRotatef (z_angle, 0.0f, 0.0f, 1.0f);

  //glRotatef (angle, 0.0f, 0.0f, 1.0f);
  //glRotatef (angle/6, 0.0f, 1.0f, 1.0f);
  //glRotatef (angle/3, 1.0f, 0.0f, 0.0f);

  the_model->draw ();

  srand(0);
  int box = 50;
  for(int i = 0; i < 200; ++i)
  {
    glPushMatrix ();  
    glTranslatef (rand()%box - box/2, 
                  rand()%box - box/2, 
                  rand()%box - box/2);
    glutSolidSphere(0.5, 12, 12);
    glPopMatrix ();
  }

  //glutSolidSphere(5.0, 10, 10);
  glPopMatrix ();
}

void keyboard (unsigned char key, int x, int y)
{
  puts ("keyboard");

  switch (key) {
    case 27:
    case 'q':
      exit(EXIT_SUCCESS);
      break;
    case 'd':
      //angle += 3.0f;
      display();
      break;
    case 'a':
      z_angle += 5.0f;
      break;
    case 'o':
      z_angle -= 5.0f;
      break;

    case 'n':
      g_wiggle_offset += 0.1f;
      break;

    case 't':
      g_wiggle_offset -= 0.1f;
      break;

    case 'c':
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

    default:
      std::cout << "unknown key: " << static_cast<int>(key) << std::endl;
      break;
  }
}

void init()
{
  g_framebuffer1 = new Framebuffer(g_screen_w, g_screen_h);
  g_framebuffer2 = new Framebuffer(g_screen_w, g_screen_h);

  glShadeModel(GL_SMOOTH);

  GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 0.0 };
  GLfloat mat_shininess[] = {  0.0 };
  GLfloat mat_ambient[] = { 0.0, 0.0, 0.0, 1.0 };
  GLfloat mat_diffuse[] = { 1.0, 1.0, 1.0, 1.0 };

  glMaterialfv(GL_FRONT, GL_AMBIENT,   mat_ambient);
  glMaterialfv(GL_FRONT, GL_SPECULAR,  mat_specular);
  glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);
  glMaterialfv(GL_FRONT, GL_EMISSION,  mat_specular);
  glMaterialfv(GL_FRONT, GL_DIFFUSE,   mat_diffuse);
  
  {
    GLfloat light_pos[] = {20.0f, 0.0f, 30.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_POSITION, light_pos);
    
    GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_AMBIENT, light_ambient);

    GLfloat light_diffuse[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light_diffuse);

    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT0, GL_SPECULAR, light_specular);
  }

  {
    GLfloat light_pos[] = {-20.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_POSITION, light_pos);
   
    GLfloat light_ambient[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_AMBIENT, light_ambient);

    GLfloat light_diffuse[] = {0.0f, 0.0f, 0.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_DIFFUSE, light_diffuse);

    GLfloat light_specular[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glLightfv(GL_LIGHT1, GL_SPECULAR, light_specular);
  }

  { // upload noise texture
    GLuint texture;
    glGenTextures(1, &texture);
    
    const int width = 32;
    const int height = 32;
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);

    glBindTexture(GL_TEXTURE_2D, texture);

    unsigned char data[width*height*3];

    for(size_t i = 0; i < sizeof(data); i+=3)
    {
      data[i+0] = data[i+1] = data[i+2] = rand() % 255;
    }

    gluBuild2DMipmaps
      (GL_TEXTURE_2D, GL_RGB,
       width, height,
       GL_RGB, GL_UNSIGNED_BYTE, data);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, 8.0f);
  }
}

void mouse (int button, int button_state, int x, int y)
{
  std::cout << x << " " << y << " " 
            << button_state << " " << button << std::endl;
  
  x_angle = x - 400;
  y_angle = y - 300;
  //display ();
  
  if (button == 3 && button_state)
  {
    zoom -= 1.0f;
  }
  else if (button == 4 && button_state)
  {
    zoom += 1.0f;
  }
}

void idle_func ()
{
  display();
  usleep(30000);
  glutForceJoystickFunc ();
}

void joystick_callback (unsigned int buttonMask, int x, int y, int z)
{
  std::cout << "BM: " << buttonMask << " " << x << " " << y << " " << z << std::endl;
}

void mouse_motion (int x, int y)
{
  //std::cout << "Motion: " << x << " " << y << std::endl;
  x_angle = x - 400;
  y_angle = y - 300;
  //display ();
}

int main (int argc, char** argv)
{
  if (argc != 2)
  {
    puts ("Usage: viewer [MODELFILE]");
    return EXIT_FAILURE;
  }
  else
  {
    Model* model = new Model(argv[1]);
    the_model = model;
      
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(g_screen_w, g_screen_h);
    glutInitWindowPosition(100, 100);
    glutCreateWindow(argv[0]);
    glewInit();
    init();
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutMouseFunc(mouse);
    glutMotionFunc (mouse_motion);      
    glutJoystickFunc (joystick_callback, 1);

    glutIdleFunc (idle_func);
    glutKeyboardFunc(keyboard);
    glutMainLoop();

    delete model;

    return 0; 
  }
}


// EOF //
