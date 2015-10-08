#ifndef HEADER_VIEWER_HPP
#define HEADER_VIEWER_HPP

#include <string>
#include <memory>

#define GLM_FORCE_RADIANS
#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include "entity.hpp"
#include "framebuffer.hpp"
#include "material.hpp"
#include "menu.hpp"
#include "scene_manager.hpp"
#include "texture.hpp"
#include "video_processor.hpp"
#include "wiimote_manager.hpp"
#include "window.hpp"

class GameController;
class Compositor;

struct Options
{
  bool wiimote = false;
  std::string video = {};
  bool video3d = false;
  std::vector<std::string> models = {};
};

struct Stick
{
  Stick() : dir(), rot(), light_rotation(), hat() {}
  glm::vec3 dir;
  glm::vec3 rot;
  bool light_rotation;
  unsigned int hat;
};

class Configuration
{
public:
  bool m_show_calibration = false;
  float m_slow_factor = 0.5f;
  bool m_wiimote_camera_control = false;
  //float m_fov = glm::radians(56.0f);
  float m_fov = glm::radians(42.0f);

  float m_near_z = 0.1f;
  float m_far_z  = 1000.0f;

  float m_shadowmap_fov = glm::radians(25.0f);

  float m_light_angle = 0.0f;
  float m_light_up = 0.0f;

  float m_aspect_ratio = static_cast<GLfloat>(640)/static_cast<GLfloat>(480);

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

  float m_eye_distance = 0.065f;
  float m_convergence = 1.0f;
};

class Viewer
{
public:
  Configuration m_cfg;

  // resources
  MaterialPtr m_video_material;
  MaterialPtr m_video_material_flip;

  TextSurfacePtr m_dot_surface;

  // systems
  std::unique_ptr<Menu> m_menu;
  std::unique_ptr<SceneManager> m_scene_manager;
  std::unique_ptr<VideoProcessor> m_video_player;
  std::unique_ptr<WiimoteManager> m_wiimote_manager;

  int m_screen_w = 640;
  int m_screen_h = 480;

  glm::vec2 m_wiimote_dot1;
  glm::vec2 m_wiimote_dot2;

  //glm::vec2 m_wiimote_scale(0.84f, 0.64f);
  glm::vec2 m_wiimote_scale = {0.52f, 0.47f};

  SceneNode* m_wiimote_accel_node = 0;
  SceneNode* m_wiimote_gyro_node = 0;
  SceneNode* m_wiimote_node = 0;

  Stick m_stick;
  Stick m_old_stick;
  unsigned int m_hat_autorepeat = 0;

  std::unique_ptr<Compositor> m_compositor;
  std::vector<std::unique_ptr<Entity> > m_entities;

private:
  void on_keyboard_event(SDL_KeyboardEvent key);
  void process_events(GameController& gamecontroller);
  void process_joystick(float dt);
  void update_menu();
  void update_freeflight_mode(float dt);
  void update_fps_mode(float dt);

  void init_scene(std::vector<std::string> const& model_filenames);
  void init_menu();
  void init_video_player(bool video3d);

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
