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

class GameController;
class Compositor;

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
public:
  Options m_opts;

  TexturePtr m_calibration_left_texture;
  TexturePtr m_calibration_right_texture;
  bool m_show_calibration = false;

  std::unique_ptr<Menu> m_menu;
  std::unique_ptr<SceneManager> m_scene_manager;
  std::unique_ptr<Camera> m_camera;

  MaterialPtr m_video_material;
  MaterialPtr m_video_material_flip;
  std::shared_ptr<VideoProcessor> m_video_player;

  float m_slow_factor = 0.5f;

  bool m_wiimote_camera_control = false;

  int m_screen_w = 640;
  int m_screen_h = 480;
  //float m_fov = glm::radians(56.0f);
  float m_fov = glm::radians(42.0f);

  float m_near_z = 0.1f;
  float m_far_z  = 1000.0f;

  int m_spot_halo_samples = 100;

  bool m_draw_look_at = false;

  float m_light_angle = 0.0f;

  bool m_headlights = false;

  float m_shadowmap_fov = glm::radians(25.0f);
  float m_light_diffuse = 1.0f;
  float m_light_specular = 1.0f;
  float m_material_shininess = 10.0f;
  float m_light_up = 0.0f;

  float m_aspect_ratio = static_cast<GLfloat>(m_screen_w)/static_cast<GLfloat>(m_screen_h);
  float m_spot_cutoff   = 60.0f;
  float m_spot_exponent = 30.0f;

  bool m_show_menu = true;
  bool m_show_dots = true;

  glm::vec3 m_eye = {0.0f, 0.0f, 0.0f};
  glm::vec3 m_look_at = {0.0f, 0.0f, -1.0f};
  glm::vec3 m_up = {0.0f, 1.0f, 0.0f};
  float m_pitch_offset = 0.0f;
  float m_roll_offset  = 0.0f;
  float m_distance_offset = 0.0f;
  float m_distance_scale = 0.000f;
  float m_yaw_offset   = 0.0f;
  glm::vec4 m_grid_offset;
  float m_grid_size = 2.0f;

  std::unique_ptr<Armature> m_armature;
  std::unique_ptr<Pose> m_pose;

  float m_scale = 1.0f;

  float m_eye_distance = 0.065f;
  float m_convergence = 1.0f;

  bool m_arcball_active = false;
  glm::ivec2 m_mouse;
  glm::ivec2 m_last_mouse;
  glm::mat4 m_object2world;
  glm::mat4 m_last_object2world;
  glm::mat4 m_eye_matrix;

  TextSurfacePtr m_dot_surface;
  glm::vec2 m_wiimote_dot1;
  glm::vec2 m_wiimote_dot2;

  //glm::vec2 m_wiimote_scale(0.84f, 0.64f);
  glm::vec2 m_wiimote_scale = {0.52f, 0.47f};

  ProgramPtr m_composition_prog;

  SceneNode* m_wiimote_accel_node = 0;
  SceneNode* m_wiimote_gyro_node = 0;
  SceneNode* m_wiimote_node = 0;
  std::vector<SceneNode*> m_nodes;

  std::shared_ptr<WiimoteManager> m_wiimote_manager;

  Stick m_stick;
  Stick m_old_stick;
  unsigned int m_hat_autorepeat = 0;

  std::unique_ptr<Compositor> m_compositor;

private:
  void on_keyboard_event(SDL_KeyboardEvent key);
  void process_events(GameController& gamecontroller);
  void process_joystick(float dt);
  glm::vec3 get_arcball_vector(glm::ivec2 mouse);
  void update_arcball();

  void init();

  void update_world(float dt);
  void update_offsets(glm::vec2 p1, glm::vec2 p2);

  void main_loop(Window& window, GameController& gamecontroller);
  void parse_args(int argc, char** argv, Options& opts);

public:
  Viewer() {}

  int main(int argc, char** argv);

private:
  Viewer(const Viewer&);
  Viewer& operator=(const Viewer&);
};

#endif

/* EOF */
