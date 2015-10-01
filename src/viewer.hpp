#ifndef HEADER_VIEWER_HPP
#define HEADER_VIEWER_HPP

#include <string>
#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "armature.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "menu.hpp"
#include "pose.hpp"
#include "scene_manager.hpp"
#include "texture.hpp"
#include "video_processor.hpp"
#include "renderbuffer.hpp"
#include "wiimote_manager.hpp"
#include "window.hpp"

struct Options
{
  bool wiimote = false;
  std::string video = std::string();
  bool video3d = false;
  std::string model = std::string();
};

struct Stick
{
  Stick() : dir(), rot(), light_rotation(), hat() {}
  glm::vec3 dir;
  glm::vec3 rot;
  bool light_rotation;
  unsigned int hat;
};

class Viewer
{
private:
  Options g_opts;

  int g_mouse_x = 0;
  int g_mouse_y = 0;

  TexturePtr g_calibration_left_texture;
  TexturePtr g_calibration_right_texture;
  bool g_show_calibration = false;

  std::unique_ptr<Menu> g_menu;
  std::unique_ptr<SceneManager> g_scene_manager;
  std::unique_ptr<Camera> g_camera;

  MaterialPtr g_video_material;
  MaterialPtr g_video_material_flip;
  std::shared_ptr<VideoProcessor> g_video_player;

  float g_slow_factor = 0.5f;

  bool g_wiimote_camera_control = false;

  glm::ivec2 g_viewport_offset = {-41, 16};
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

  enum class StereoMode { None, CrossEye, Cybermaxx, Anaglyph, Depth, End };
  StereoMode g_stereo_mode = StereoMode::None;

  bool g_headlights = false;
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

  glm::vec3 g_eye = {0.0f, 0.0f, 0.0f};
  glm::vec3 g_look_at = {0.0f, 0.0f, -1.0f};
  glm::vec3 g_up = {0.0f, 1.0f, 0.0f};
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
  glm::vec2 g_wiimote_scale = {0.52f, 0.47f};

  ProgramPtr m_composition_prog;

  SceneNode* g_wiimote_accel_node = 0;
  SceneNode* g_wiimote_gyro_node = 0;
  SceneNode* g_wiimote_node = 0;
  std::vector<SceneNode*> g_nodes;

  std::shared_ptr<WiimoteManager> g_wiimote_manager;

  Stick g_stick;
  Stick g_old_stick;
  unsigned int g_hat_autorepeat = 0;

public:
  Viewer() {}

  void draw_scene(Stereo stereo);
  void draw_shadowmap();
  void display();
  void keyboard(SDL_KeyboardEvent key, int x, int y);
  void reshape(int w, int h);
  void init();
  void mouse(int button, int button_state, int x, int y);
  glm::vec3 get_arcball_vector(glm::ivec2 mouse);
  void process_events();
  void process_joystick(float dt);
  void update_arcball();
  void update_world(float dt);
  void main_loop(Window& window);
  void mouse_motion(int x, int y);
  void update_offsets(glm::vec2 p1, glm::vec2 p2);
  void parse_args(int argc, char** argv, Options& opts);
  int main(int argc, char** argv);

private:
  Viewer(const Viewer&);
  Viewer& operator=(const Viewer&);
};

#endif

/* EOF */
