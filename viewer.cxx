#include <math.h>
#include <GL/freeglut.h>
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <iostream>

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

  Vector () {}
  
  Vector (float x_, float y_, float z_) 
  {
    x = x_;
    y = y_;
    z = z_;
  }

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
  Mesh (std::istream& in) 
  {
    int number_of_vertixes;
    in >> number_of_vertixes;
    std::cout << "Number_Of_Vertixes: " << number_of_vertixes << std::endl;
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

    std::cout << "read " << number_of_faces << " faces, " << number_of_vertixes << " vertices" << std::endl;
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
        float normal_x;
        float normal_y;
        float normal_z;

        glEnable (GL_COLOR_MATERIAL);
        glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT);
        glColor3f (0, 0, 0);

        glColorMaterial (GL_FRONT_AND_BACK, GL_DIFFUSE);
        glColor3f (0.4, .4, 0.4);

        glBegin (GL_TRIANGLES);
        
        set_face_normal (*i);
        glVertex3f (vertices[i->vertex1].x,
                    vertices[i->vertex1].y,
                    vertices[i->vertex1].z);
        

        set_face_normal (*i);
        glVertex3f (vertices[i->vertex2].x,
                    vertices[i->vertex2].y,
                    vertices[i->vertex2].z);

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
  Model (const std::string& filename) 
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


Model* the_model;
GLuint model_lst;

void reshape(int w, int h)
{
  glViewport (0,0, w, h);
  //gluOrtho2D (-5, 5, -5, 5);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective(60.0, (GLfloat) w/(GLfloat) h, 1.0, 150.0f);
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
}

float x_angle = -90.0f;
float y_angle = 0.0f;
float z_angle = 0.0f;
float zoom = -15.0;

void display()
{
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  GLfloat light_pos[] = {0.0f, 0.0f, 20.0f, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

  glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

  glTranslatef(0.0, 0.0, zoom);


  /*glDisable (GL_CULL_FACE);
  glColor4f (0.0f, 0.0f, 0.5f, 0.5f);
  glutSolidSphere (10.0, 8, 8);
  glEnable (GL_CULL_FACE);*/


  glPushMatrix ();  
  glScalef (2, 2, 2);
  glTranslatef (-5,0,0);
  glutSolidSphere(1.0, 5, 6);
  glPopMatrix ();

  //angle += 3.0f;
  glPushMatrix ();
  //glRotatef (-90, 1.0f, 0.0f, 0.0f);

  glRotatef (x_angle, 1.0f, 0.0f, 0.0f);
  glRotatef (y_angle, 0.0f, 1.0f, 0.0f);
  glRotatef (z_angle, 0.0f, 0.0f, 1.0f);

  //glRotatef (angle, 0.0f, 0.0f, 1.0f);
  //glRotatef (angle/6, 0.0f, 1.0f, 1.0f);
  //glRotatef (angle/3, 1.0f, 0.0f, 0.0f);

  if (1)
    {
      glCallList (model_lst);
    }
  else
    {
      the_model->draw ();
    }
  //glutSolidSphere(5.0, 10, 10);
  glPopMatrix ();

  glutSwapBuffers ();
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
  default:
    break;
  }
}

void init()
{
  GLfloat mat_specular[] = { 0.0, 0.0, 0.0, 0.0 };
  GLfloat mat_shininess[] = {  0.0 };

  glClearColor (1.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_SMOOTH);

  glMaterialfv (GL_FRONT, GL_SPECULAR,  mat_specular);
  glMaterialfv (GL_FRONT, GL_SHININESS, mat_shininess);

  GLfloat light_pos[] = {40.0f, 0.0f, 30.0f, 1.0f};
  glLightfv(GL_LIGHT0, GL_POSITION, light_pos);

  glEnable (GL_CULL_FACE);

  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);

  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  model_lst = glGenLists (1);
  glNewList (model_lst, GL_COMPILE);
  the_model->draw ();
  glEndList ();
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
  display ();
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
      glutInitWindowSize(800, 600);
      glutInitWindowPosition(100, 100);
      glutCreateWindow(argv[0]);
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
