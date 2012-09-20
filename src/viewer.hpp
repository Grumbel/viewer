#ifndef HEADER_VIEWER_HPP
#define HEADER_VIEWER_HPP

class Viewer
{
private:
public:
  Viewer();

  void reshape(int w, int h);
  void draw_scene();
  void display();
  void special(int key, int x, int y);
  void keyboard(unsigned char key, int x, int y);
  void init();
  void mouse(int button, int button_state, int x, int y);
  void idle_func();
  void joystick_callback(unsigned int buttonMask, int x, int y, int z);
  void mouse_motion(int x, int y);

  int main(int argc, char** argv);

private:
  Viewer(const Viewer&);
  Viewer& operator=(const Viewer&);
};

#endif

/* EOF */
