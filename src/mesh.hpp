//  Simple 3D Model Viewer
//  Copyright (C) 2012 Ingo Ruhnke <grumbel@gmail.com>
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

#ifndef HEADER_MESH_HPP
#define HEADER_MESH_HPP

#include <vector>
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <memory>
#include <unordered_map>

#include "opengl_state.hpp"

typedef std::vector<glm::vec3>  NormalLst;
typedef std::vector<glm::vec3>  VertexLst;
typedef std::vector<glm::vec3>  TexCoordLst;
typedef std::vector<int>        FaceLst;
typedef std::vector<glm::vec4>  BoneWeights;
typedef std::vector<glm::ivec4> BoneIndices;
typedef std::vector<int>        BoneCounts;

template<typename C> 
inline size_t glm_vec_length() 
{
  assert(!"glm_vec_length not implemented");
  return 0;
}

template<> inline size_t glm_vec_length<glm::vec2>() { return 2; }
template<> inline size_t glm_vec_length<glm::vec3>() { return 3; }
template<> inline size_t glm_vec_length<glm::vec4>() { return 4; }

class Mesh
{
private:
  struct Array
  {
    enum Type { Integer, Float } type;
    int size;
    GLuint vbo;

    Array() : type(), size(), vbo()
    {}

    Array(Type type_, int size_, GLuint vbo_) :
      type(type_),
      size(size_),
      vbo(vbo_)
    {}
  };

private:
  GLenum m_primitive_type;
  std::unordered_map<std::string, Array> m_attribute_arrays;
  GLuint m_element_array_vbo;
  int m_element_count;
  
public:
  /** Create a cube with cubemap texture coordinates */
  static std::unique_ptr<Mesh> create_skybox(float size);
  static std::unique_ptr<Mesh> create_plane(float size, glm::vec3 center = glm::vec3(0.0f, 0.0f, 0.0f));
  static std::unique_ptr<Mesh> create_rect(float x1, float y1, float x2, float y2, float z);
  static std::unique_ptr<Mesh> create_cube(float size);
  static std::unique_ptr<Mesh> create_sphere(float size, int rings = 16, int segments = 32);
  static std::unique_ptr<Mesh> create_curved_screen(float size, float hfov, float vfov, int rings = 16, int segments = 32, 
                                                    int offset_x = 0, int offset_y = 0,
                                                    bool flip_uv_x = false, bool flip_uv_y = false);

public:
  Mesh(GLenum primitive_type);
  ~Mesh();

  void draw();

  void attach_array(const std::string& name, const Array& array, int element_count)
  {
    if (m_attribute_arrays.find(name) != m_attribute_arrays.end())
    {
      throw std::runtime_error("array '" + name + "' already present");
    }
    else if (m_element_count != -1 && m_element_count != element_count)
    {
      throw std::runtime_error("element count does not match");
    }
    else
    {
      m_element_count = element_count;
      m_attribute_arrays[name] = array;
    }
  }

  void attach_float_array(const std::string& name, const std::vector<float>& vec)
  {
    GLuint vbo = build_vbo(GL_ARRAY_BUFFER, vec);
    attach_array(name, Array(Array::Float, 1, vbo), vec.size());
  }

  template<typename T>  
  void attach_float_array(const std::string& name, const std::vector<T>& vec)
  {
    GLuint vbo = build_vbo(GL_ARRAY_BUFFER, vec);
    attach_array(name, Array(Array::Float, glm_vec_length<T>(), vbo), vec.size());
  }

  void attach_int_array(const std::string& name, const std::vector<int>& vec)
  {
    GLuint vbo = build_vbo(GL_ARRAY_BUFFER, vec);
    attach_array(name, Array(Array::Integer, 1, vbo), vec.size());
  }

  template<typename T>  
  void attach_int_array(const std::string& name, const std::vector<T>& vec)
  {
    GLuint vbo = build_vbo(GL_ARRAY_BUFFER, vec);
    attach_array(name, Array(Array::Integer, glm_vec_length<T>(), vbo), vec.size());
  } 

  void attach_element_array(const std::vector<int>& vec)
  {
    if (m_element_array_vbo != 0)
    {
      throw std::runtime_error("element array already present");
    }
    else
    {
      m_element_array_vbo = build_vbo(GL_ELEMENT_ARRAY_BUFFER, vec);
      m_element_count = vec.size();
    }
  }

private:
  template<typename T>
  GLuint build_vbo(GLenum target, const std::vector<T>& vec)
  {
    GLuint vbo;
    glGenBuffers(1, &vbo);
    glBindBuffer(target, vbo);
    glBufferData(target, sizeof(T) * vec.size(), vec.data(), GL_STATIC_DRAW);
    return vbo;
  }

private:
  Mesh(const Mesh&) = delete;
  Mesh& operator=(const Mesh&) = delete;
};

#endif

/* EOF */
