#include <boost/algorithm/string/predicate.hpp>
#include <boost/format.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <fstream>
#include <stdexcept>

#include "scene_node.hpp"
#include "material_factory.hpp"

#include "scene.hpp"

std::unique_ptr<SceneNode>
Scene::from_file(const std::string& filename)
{
  std::ifstream in(filename.c_str());
  if(!in)
  {
    throw std::runtime_error(format("%s: File not found", filename));
  }
  else
  {
    Scene scene;
    scene.set_directory(boost::filesystem::path(filename).parent_path());
    scene.parse_istream(in);
    return scene.get_node();
  }
}

std::unique_ptr<SceneNode>
Scene::from_istream(std::istream& in)
{
  Scene scene;
  scene.parse_istream(in);
  return scene.get_node();
}

Scene::Scene() :
  m_directory(),
  m_node(new SceneNode)
{
}

void
Scene::set_directory(const boost::filesystem::path& path)
{
  m_directory = path;
}

void
Scene::parse_istream(std::istream& in)
{
  // This is not a fully featured .obj file reader, it just takes some
  // inspiration from it:
  // http://www.martinreddy.net/gfx/3d/OBJ.spec
  std::unordered_map<std::string, SceneNode*> nodes;
  std::unordered_map<std::string, std::unique_ptr<SceneNode> > unattached_children;

  std::string name;
  std::string parent;
  std::string material = "phong";
  glm::vec3 location(0.0f, 0.0f, 0.0f);
  glm::quat rotation(1.0f, 0.0f, 0.0f, 0.0f);
  glm::vec3 scale(1.0f, 1.0f, 1.0f);
  std::vector<glm::vec3>  normal;
  std::vector<glm::vec3>  position;
  std::vector<glm::vec3>  texcoord;
  std::vector<int>        index;
  std::vector<glm::vec4>  bone_weight;
  std::vector<glm::ivec4> bone_index;
  std::vector<int>        bone_count;

  auto commit_object = [&]{
    if (!name.empty())
    {
      ModelPtr model;

      if (!position.empty())
      {
        // fill in some texcoords if there aren't enough
        if (texcoord.size() < position.size())
        {
          texcoord.resize(position.size());
          for(FaceLst::size_type i = position.size()-1; i < texcoord.size(); ++i)
          {
            texcoord[i] = glm::vec3(0.0f, 0.0f, 0.0f);
          }
        }

        {
          // create Mesh
          std::unique_ptr<Mesh> mesh(new Mesh(GL_TRIANGLES));

          mesh->attach_float_array("position", position);
          mesh->attach_float_array("texcoord", texcoord);
          mesh->attach_float_array("normal",   normal);
          mesh->attach_element_array(index);

          if (!bone_weight.empty() && !bone_index.empty())
          {
            mesh->attach_float_array("bone_weight", bone_weight);
            mesh->attach_int_array("bone_index", bone_index);
          }

          // create Model
          model = std::make_shared<Model>();
          model->add_mesh(std::move(mesh));

          if (boost::algorithm::ends_with(material, ".material"))
          {
            model->set_material(MaterialFactory::get().from_file(m_directory / boost::filesystem::path(material)));
          }
          else
          {
            model->set_material(MaterialFactory::get().create(material));
          }
        }
      }

      // create SceneNode
      {
        std::unique_ptr<SceneNode> node(new SceneNode(name));
        node->set_position(location);
        node->set_orientation(rotation);
        node->set_scale(scale);

        if (model)
        {
          node->attach_model(model);
        }

        if (nodes.find(name) != nodes.end())
        {
          throw std::runtime_error("duplicate object name: " + name);
        }

        nodes[name] = node.get();
        if (parent.empty())
        {
          m_node->attach_child(std::move(node));
        }
        else
        {
          unattached_children[parent] = std::move(node);
        }
      }

      // clear for the next mesh
      name.clear();
      parent.clear();
      normal.clear();
      texcoord.clear();
      position.clear();
      index.clear();
      location = glm::vec3(0.0f, 0.0f, 0.0f);
      rotation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
      scale = glm::vec3(1.0f, 1.0f, 1.0f);
    }
  };

  std::string line;
  int line_number = 0;
  while(std::getline(in, line))
  {
    line_number += 1;

    boost::tokenizer<boost::char_separator<char> > tokens(line, boost::char_separator<char>(" ", ""));
    auto it = tokens.begin();
    if (it != tokens.end())
    {
#define INCR_AND_CHECK {                                                \
        ++it;                                                           \
        if (it == tokens.end())                                         \
        {                                                               \
          throw std::runtime_error((boost::format("not enough tokens at line %d") % line_number).str()); \
        }                                                               \
      }

      try
      {
        if (*it == "o")
        {
          // object
          commit_object();

          INCR_AND_CHECK;
          log_debug("object: '%s'", *it);
          name = *it;
        }
        else if (*it == "g")
        {
          // group
        }
        else if (*it == "parent")
        {
          INCR_AND_CHECK;
          parent = *it;
        }
        else if (*it == "mat")
        {
          INCR_AND_CHECK;
          material = *it;
        }
        else if (*it == "loc")
        {
          INCR_AND_CHECK;
          location.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          location.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          location.z = boost::lexical_cast<float>(*it);
        }
        else if (*it == "rot")
        {
          INCR_AND_CHECK;
          rotation.w = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          rotation.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          rotation.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          rotation.z = boost::lexical_cast<float>(*it);
        }
        else if (*it == "scale")
        {
          INCR_AND_CHECK;
          scale.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          scale.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          scale.z = boost::lexical_cast<float>(*it);
        }
        else if (*it == "v")
        {
          glm::vec3 v;

          INCR_AND_CHECK;
          v.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          v.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          v.z = boost::lexical_cast<float>(*it);

          position.push_back(v);
        }
        else if (*it == "vt")
        {
          glm::vec3 vt;

          INCR_AND_CHECK;
          vt.s = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          vt.t = boost::lexical_cast<float>(*it);

          texcoord.push_back(vt);
        }
        else if (*it == "vn")
        {
          glm::vec3 vn;

          INCR_AND_CHECK;
          vn.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          vn.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          vn.z = boost::lexical_cast<float>(*it);

          normal.push_back(vn);
        }
        else if (*it == "bw")
        {
          glm::vec4 bw;

          INCR_AND_CHECK;
          bw.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          bw.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          bw.z = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          bw.w = boost::lexical_cast<float>(*it);

          bone_weight.push_back(bw);
        }
        else if (*it == "bi")
        {
          glm::ivec4 bi;

          INCR_AND_CHECK;
          bi.x = boost::lexical_cast<int>(*it);
          INCR_AND_CHECK;
          bi.y = boost::lexical_cast<int>(*it);
          INCR_AND_CHECK;
          bi.z = boost::lexical_cast<int>(*it);
          INCR_AND_CHECK;
          bi.w = boost::lexical_cast<int>(*it);

          bone_index.push_back(bi);
        }
        else if (*it == "f")
        {
          INCR_AND_CHECK;
          index.push_back(boost::lexical_cast<int>(*it));
          INCR_AND_CHECK;
          index.push_back(boost::lexical_cast<int>(*it));
          INCR_AND_CHECK;
          index.push_back(boost::lexical_cast<int>(*it));
        }
        else if ((*it)[0] == '#')
        {
          // ignore comments
        }
        else
        {
          throw std::runtime_error((boost::format("unhandled token %s") % *it).str());
        }
      }
      catch(const std::exception& err)
      {
        throw std::runtime_error((boost::format("unknown:%d: %s") % line_number % err.what()).str());
      }
    }
  }

  commit_object();

  // reconstruct parent/child relationships
  for(auto& it : unattached_children)
  {
    auto p = nodes.find(it.first);
    if (p == nodes.end())
    {
      throw std::runtime_error("parent not found: " + it.first);
    }
    else
    {
      p->second->attach_child(std::move(it.second));
    }
  }
}

std::unique_ptr<SceneNode>
Scene::get_node()
{
  return std::move(m_node);
}

/* EOF */
