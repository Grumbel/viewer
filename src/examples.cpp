#if 0
#include "examples.hpp"

void make_pose()
{
  g_armature = Armature::from_file("/tmp/blender.bones");
  g_pose = Pose::from_file("/tmp/blender.pose");
}

void make_planetary_system()
{
  // little animated planetary system thing
  if (false)
  {
    auto node = m_scene_manager->get_world()->create_child();
    node->set_position(glm::vec3(0, 0, 0));
    auto mesh = Mesh::create_plane(75.0f);
    ModelPtr model = std::make_shared<Model>();
    model->add_mesh(std::move(mesh));
    model->set_material(phong_material);
    node->attach_model(model);
  }

  auto mesh = Mesh::create_sphere(0.2, 8, 16);
  ModelPtr model = std::make_shared<Model>();
  model->add_mesh(std::move(mesh));
  model->set_material(phong_material);

  auto origin = m_scene_manager->get_world()->create_child();
  origin->set_position(glm::vec3(5, 5, 5));

  auto sun = origin->create_child();
  sun->set_scale(glm::vec3(3.0f, 3.0f, 3.0f));
  sun->attach_model(model);
  m_nodes.push_back(sun);

  auto earth = sun->create_child();
  earth->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));
  earth->set_position(glm::vec3(2.0f, 0, 0));
  earth->attach_model(model);
  m_nodes.push_back(earth);

  auto moon = earth->create_child();
  moon->set_scale(glm::vec3(0.5f, 0.5f, 0.5f));
  moon->set_position(glm::vec3(2, 0, 0));
  moon->attach_model(model);
  //g_nodes.push_back(moon);
}

void make_spheres_in_a_line()
{
  // some spheres in a line
  auto root_parent = m_scene_manager->get_world()->create_child();
  auto parent = root_parent;
  parent->set_position(glm::vec3(10.0f, 0.0f, 0.0f));
  for(int i = 0; i < 5; ++i)
  {
    auto child = parent->create_child();
    child->set_position(glm::vec3(1.0f, 0.0f, -3.0f));
    ModelPtr model = std::make_shared<Model>();
    model->add_mesh(Mesh::create_sphere(0.5f, 16, 8));
    model->set_material(phong_material);
    child->attach_model(model);

    parent = child;
  }

  print_scene_graph(root_parent);
}

void make_wiimode()
{
  {
    auto node = Scene::from_file("data/wiimote.mod");
    m_wiimote_gyro_node = node.get();
    m_scene_manager->get_world()->attach_child(std::move(node));
  }

  {
    auto node = Scene::from_file("data/wiimote.mod");
    m_wiimote_node = node.get();
    m_scene_manager->get_world()->attach_child(std::move(node));
  }

  {
    auto node = Scene::from_file("data/wiimote.mod");
    m_wiimote_accel_node = node.get();
    m_scene_manager->get_world()->attach_child(std::move(node));
  }
}

void make_lightcone()
{
  MaterialPtr material = std::make_shared<Material>();
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
  material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/glsl/lightcone.vert"),
                                        Shader::from_file(GL_FRAGMENT_SHADER, "src/glsl/lightcone.frag")));

  auto mesh = std::make_unique<Mesh>(GL_POINTS);
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

  ModelPtr model = std::make_shared<Model>();
  model->add_mesh(std::move(mesh));
  model->set_material(material);

  for(int i = 0; i < 10; ++i)
  {
    auto node = m_scene_manager->get_world()->create_child();
    node->set_position(glm::vec3(1.0f, i * 5.0f, -1.0f));
    node->set_orientation(glm::quat());
    node->attach_model(model);
  }
}

#endif

/* EOF */
