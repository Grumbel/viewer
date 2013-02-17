//  Simple 3D Model Viewer
//  Copyright (C) 2012-2013 Ingo Ruhnke <grumbel@gmail.com>
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

#include "material_factory.hpp"

#include <stdexcept>
#include <boost/algorithm/string/predicate.hpp>

#include "framebuffer.hpp"
#include "material_parser.hpp"
#include "render_context.hpp"

extern glm::mat4 g_shadowmap_matrix;
extern std::unique_ptr<Framebuffer> g_shadowmap;

MaterialFactory::MaterialFactory() :
  m_materials()
{
  m_materials["basic_white"] = create_basic_white();
  m_materials["phong"] = create_phong(glm::vec3(0.5f, 0.5f, 0.5f),
                                      glm::vec3(1.0f, 1.0f, 1.0f),
                                      glm::vec3(1.0f, 1.0f, 1.0f),
                                      5.0f);

  m_materials["Rim"] = create_phong(glm::vec3(0.5f, 0.5f, 0.5f),
                                    glm::vec3(1.0f, 1.0f, 1.0f),
                                    glm::vec3(1.0f, 1.0f, 1.0f),
                                    1.0f);

  m_materials["Wheel"] = create_phong(glm::vec3(0.1f, 0.1f, 0.1f),
                                      glm::vec3(1.0f, 1.0f, 1.0f),
                                      glm::vec3(1.0f, 1.0f, 1.0f),
                                      8.0f);

  m_materials["Body"] = create_phong(glm::vec3(0.5f, 0.5f, 0.8f),
                                     glm::vec3(1.0f, 1.0f, 1.0f),
                                     glm::vec3(0.5f, 0.5f, 0.5f),
                                     2.5f);
                                      
  m_materials["skybox"] = create_skybox();
  m_materials["textured"] = create_textured();
  m_materials["video"] = create_video();
}

MaterialPtr
MaterialFactory::from_file(const boost::filesystem::path& filename)
{
  MaterialPtr material = MaterialParser::from_file(filename);

  material->set_uniform("ModelViewMatrix", UniformSymbol::ModelViewMatrix);
  material->set_uniform("NormalMatrix", UniformSymbol::NormalMatrix);
  material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);

  material->set_uniform("light.diffuse",   glm::vec3(1.0f, 1.0f, 1.0f));
  material->set_uniform("light.ambient",   glm::vec3(0.25f, 0.25f, 0.25f));
  material->set_uniform("light.specular",  glm::vec3(0.6f, 0.6f, 0.6f));
  material->set_uniform("light.position", 
                              UniformCallback(
                                [](ProgramPtr prog, const std::string& name, const RenderContext& ctx) {
                                  glm::vec3 pos(ctx.get_view_matrix() * glm::vec4(50.0f, 50.0f, 50.0f, 1.0f));
                                  prog->set_uniform(name, pos);
                                }));

  material->set_uniform("ShadowMapMatrix",
                              UniformCallback(
                                [](ProgramPtr prog, const std::string& name, const RenderContext& ctx) {
                                  prog->set_uniform(name, g_shadowmap_matrix * ctx.get_model_matrix());
                                }));
  material->set_texture(2, g_shadowmap->get_depth_texture());
  material->set_uniform("ShadowMap", 2);

  return material;
}

MaterialPtr
MaterialFactory::create(const std::string& name)
{
  auto it = m_materials.find(name);
  if (it == m_materials.end())
  {
    if (name == "phong")
    {
      throw std::runtime_error("unknown material: " + name);
    }
    else
    {
      return create("phong");
    }
  }
  else
  {
    return it->second;
  }
}

MaterialPtr
MaterialFactory::create_basic_white()
{
  MaterialPtr material = std::make_shared<Material>();

  material->enable(GL_CULL_FACE);
  material->enable(GL_DEPTH_TEST);

  material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);

  material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER,   "src/basic_white.vert"),
                                        Shader::from_file(GL_FRAGMENT_SHADER, "src/basic_white.frag")));
  return material;
}

MaterialPtr
MaterialFactory::create_phong(const glm::vec3& diffuse, 
                              const glm::vec3& ambient, 
                              const glm::vec3& specular,
                              float shininess) 
{
  MaterialPtr phong = std::make_shared<Material>();

  phong->enable(GL_CULL_FACE);
  phong->enable(GL_DEPTH_TEST);
  //phong->set_texture(0, Texture::from_file("data/textures/grass_01_v1.tga"));
  //phong->set_uniform("diffuse", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
  //phong->set_uniform("diffuse_texture", 0);

  phong->set_uniform("light.diffuse",   glm::vec3(1.0f, 1.0f, 1.0f));
  phong->set_uniform("light.ambient",   glm::vec3(0.0f, 0.0f, 0.0f));
  phong->set_uniform("light.specular",  glm::vec3(1.0f, 1.0f, 1.0f));
  //phong->set_uniform("light.shininess", 3.0f);
  //phong->set_uniform("light.position",  glm::vec3(5.0f, 5.0f, 5.0f));
  phong->set_uniform("light.position", 
                              UniformCallback(
                                [](ProgramPtr prog, const std::string& name, const RenderContext& ctx) {
                                  glm::vec3 pos(ctx.get_view_matrix() * glm::vec4(50.0f, 50.0f, 50.0f, 1.0f));
                                  prog->set_uniform(name, pos);
                                }));

  phong->set_uniform("material.diffuse",   diffuse);
  phong->set_uniform("material.ambient",   ambient);
  phong->set_uniform("material.specular",  specular);
  phong->set_uniform("material.shininess", shininess);

  phong->set_uniform("ModelViewMatrix", UniformSymbol::ModelViewMatrix);
  phong->set_uniform("NormalMatrix", UniformSymbol::NormalMatrix);
  phong->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);

  phong->set_uniform("ShadowMapMatrix",
                              UniformCallback(
                                [](ProgramPtr prog, const std::string& name, const RenderContext& ctx) {
                                  prog->set_uniform(name, g_shadowmap_matrix * ctx.get_model_matrix());
                                }));
  phong->set_texture(0, g_shadowmap->get_depth_texture());
  phong->set_uniform("ShadowMap", 0);
  phong->set_texture(1, Texture::cubemap_from_file("data/textures/miramar/"));
  //phong->set_uniform("LightMap", 1);
  phong->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/phong.vert"),
                                              Shader::from_file(GL_FRAGMENT_SHADER, "src/phong.frag")));
  return phong;
}

MaterialPtr
MaterialFactory::create_skybox()
{
  MaterialPtr material = std::make_shared<Material>();

  material->blend_func(GL_ONE, GL_ONE);
  material->enable(GL_BLEND);
  material->enable(GL_CULL_FACE);
  material->enable(GL_DEPTH_TEST);
  material->set_texture(0, Texture::cubemap_from_file("data/textures/miramar/"));
  material->set_uniform("diffuse", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));
  material->set_uniform("diffuse_texture", 0);
  material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);
  material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/cubemap.vert"),
                                        Shader::from_file(GL_FRAGMENT_SHADER, "src/cubemap.frag")));

  return material;
}

MaterialPtr
MaterialFactory::create_textured()
{
  MaterialPtr material = std::make_shared<Material>();

  material->enable(GL_CULL_FACE);
  material->enable(GL_DEPTH_TEST);

  material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/textured.vert"),
                                        Shader::from_file(GL_FRAGMENT_SHADER, "src/textured.frag")));

  material->set_texture(0, Texture::from_file("data/textures/uvtest.png"));
  material->set_texture(1, Texture::from_file("data/textures/uvtest.png"));
  material->set_uniform("texture_diff", 0);
  material->set_uniform("texture_spec", 1);

  material->set_uniform("ModelViewMatrix", UniformSymbol::ModelViewMatrix);
  material->set_uniform("NormalMatrix", UniformSymbol::NormalMatrix);
  material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);

  material->set_uniform("light.diffuse",   glm::vec3(1.0f, 1.0f, 1.0f));
  material->set_uniform("light.ambient",   glm::vec3(0.25f, 0.25f, 0.25f));
  material->set_uniform("light.specular",  glm::vec3(0.6f, 0.6f, 0.6f));
  //material->set_uniform("light.shininess", 3.0f);
  //material->set_uniform("light.position",  glm::vec3(5.0f, 5.0f, 5.0f));
  material->set_uniform("light.position", 
                              UniformCallback(
                                [](ProgramPtr prog, const std::string& name, const RenderContext& ctx) {
                                  glm::vec3 pos(ctx.get_view_matrix() * glm::vec4(50.0f, 50.0f, 50.0f, 1.0f));
                                  prog->set_uniform(name, pos);
                                }));

  material->set_uniform("material.ambient",   glm::vec3(1.0f, 1.0f, 1.0f));
  material->set_uniform("material.shininess", 64.0f);

  material->set_uniform("ShadowMapMatrix",
                              UniformCallback(
                                [](ProgramPtr prog, const std::string& name, const RenderContext& ctx) {
                                  prog->set_uniform(name, g_shadowmap_matrix * ctx.get_model_matrix());
                                }));
  material->set_texture(2, g_shadowmap->get_depth_texture());
  material->set_uniform("ShadowMap", 2);

  return material;
}

MaterialPtr
MaterialFactory::create_video()
{
  MaterialPtr material = std::make_shared<Material>();

  material->enable(GL_CULL_FACE);
  material->enable(GL_DEPTH_TEST);

  material->set_program(Program::create(Shader::from_file(GL_VERTEX_SHADER, "src/video.vert"),
                                        Shader::from_file(GL_FRAGMENT_SHADER, "src/video3d.frag")));

  material->set_texture(0, Texture::from_file("data/textures/uvtest.png"));
  material->set_uniform("texture_diff", 0);
  material->set_uniform("offset", 0.0f);

  material->set_uniform("MVP", UniformSymbol::ModelViewProjectionMatrix);

  return material;
}

/* EOF */
