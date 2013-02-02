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

#include "framebuffer.hpp"
#include "render_context.hpp"

extern glm::mat4 g_shadowmap_matrix;
extern std::unique_ptr<Framebuffer> g_shadowmap;

MaterialFactory::MaterialFactory() :
  m_materials()
{
  m_materials["phong"] = create_phong();
  m_materials["skybox"] = create_skybox();
}

MaterialPtr
MaterialFactory::create(const std::string& name)
{
  auto it = m_materials.find(name);
  if (it == m_materials.end())
  {
    throw std::runtime_error("unknown material: " + name);
  }
  else
  {
    return it->second;
  }
}

MaterialPtr
MaterialFactory::create_phong()
{
  MaterialPtr phong = std::make_shared<Material>();

  phong->enable(GL_CULL_FACE);
  phong->enable(GL_DEPTH_TEST);
  //phong->set_texture(0, Texture::from_file("data/textures/grass_01_v1.tga"));
  //phong->set_uniform("diffuse", glm::vec4(1.0f, 0.0f, 1.0f, 1.0f));
  //phong->set_uniform("diffuse_texture", 0);

  phong->set_uniform("light.diffuse",   glm::vec3(1.0f, 1.0f, 1.0f));
  phong->set_uniform("light.ambient",   glm::vec3(0.0f, 0.0f, 0.0f));
  phong->set_uniform("light.specular",  glm::vec3(0.6f, 0.6f, 0.6f));
  //phong->set_uniform("light.shininess", 3.0f);
  //phong->set_uniform("light.position",  glm::vec3(5.0f, 5.0f, 5.0f));
  phong->set_uniform("light.position", 
                              UniformCallback(
                                [](ProgramPtr prog, const std::string& name, const RenderContext& ctx) {
                                  glm::vec3 pos(ctx.get_view_matrix() * glm::vec4(50.0f, 50.0f, 50.0f, 1.0f));
                                  prog->set_uniform(name, pos);
                                }));

  phong->set_uniform("material.diffuse",   glm::vec3(0.5f, 0.5f, 0.5f));
  phong->set_uniform("material.ambient",   glm::vec3(1.0f, 1.0f, 1.0f));
  phong->set_uniform("material.specular",  glm::vec3(1.0f, 1.0f, 1.0f));
  phong->set_uniform("material.shininess", 15.0f);

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
  phong->set_uniform("LightMap", 1);
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

/* EOF */
