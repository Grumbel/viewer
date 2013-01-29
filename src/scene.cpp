#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/format.hpp>
#include <fstream>

#include "scene_node.hpp"

#include "scene.hpp"

SceneNode*
Scene::from_file(MaterialPtr material, const std::string& filename)
{
  std::ifstream in(filename.c_str());
  if(!in)
  {
    std::cout << filename << ": File not found" << std::endl;
    exit(EXIT_FAILURE);
  }
  else
  {
    return Scene::from_istream(material, in);
  }
}

SceneNode*
Scene::from_istream(MaterialPtr material, std::istream& in)
{
  // This is not a fully featured .obj file reader, it just takes some
  // inspiration from it: 
  // http://www.martinreddy.net/gfx/3d/OBJ.spec
  std::unique_ptr<SceneNode> root(new SceneNode);

  glm::vec3 location(0.0f, 0.0f, 0.0f);
  std::vector<glm::vec3>  normal;
  std::vector<glm::vec3>  position;
  std::vector<glm::vec3>  texcoord;
  std::vector<int>        index;
  std::vector<glm::vec4>  bone_weight;
  std::vector<glm::ivec4> bone_index;
  std::vector<int>        bone_count;

  auto commit_object = [&]{
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
      ModelPtr model = std::make_shared<Model>();
      model->add_mesh(std::move(mesh));
      model->set_material(material);

      // create SceneNode
      SceneNode* node = root->create_child();
      node->set_position(location);
      node->attach_entity(model);

      // clear for the next mesh
      normal.clear();
      texcoord.clear();
      position.clear();
      index.clear();
      location = glm::vec3(0.0f, 0.0f, 0.0f);
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
        if (*it == "loc")
        {
          INCR_AND_CHECK;
          location.x = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          location.y = boost::lexical_cast<float>(*it);
          INCR_AND_CHECK;
          location.z = boost::lexical_cast<float>(*it);
        }
        else if (*it == "g")
        {
          // group
        }
        else if (*it == "o")
        {
          INCR_AND_CHECK;
          log_debug("object: %s", *it);

          // object
          commit_object();
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

  return root.release();
}

/* EOF */
