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
  int m_element_array_size;

public:
  /** Create a cube with cubemap texture coordinates */
  static std::unique_ptr<Mesh> create_skybox(float size);

public:
  Mesh(GLenum primitive_type);
  ~Mesh();

  void draw();

  void attach_float_array(const std::string& name, const std::vector<float>& vec)
  {
    assert(m_attribute_arrays.find(name) == m_attribute_arrays.end());

    GLuint vbo = build_vbo(GL_ARRAY_BUFFER, vec);
    m_attribute_arrays[name] = Array(Array::Float, 1, vbo);
  }

  template<typename T>  
  void attach_float_array(const std::string& name, const std::vector<T>& vec)
  {
    assert(m_attribute_arrays.find(name) == m_attribute_arrays.end());

    GLuint vbo = build_vbo(GL_ARRAY_BUFFER, vec);
    m_attribute_arrays[name] = Array(Array::Float, T::value_size(), vbo);
  }

  template<typename T>  
  void attach_int_array(const std::string& name, const std::vector<T>& vec)
  {
    assert(m_attribute_arrays.find(name) == m_attribute_arrays.end());

    GLuint vbo = build_vbo(GL_ARRAY_BUFFER, vec);
    m_attribute_arrays[name] = Array(Array::Integer, T::value_size(), vbo);
  } 

  template<typename T>  
  void attach_element_array(const std::vector<T>& vec)
  {
    assert(m_element_array_vbo == 0);
    m_element_array_vbo = build_vbo(GL_ELEMENT_ARRAY_BUFFER, vec);
    m_element_array_size = vec.size();
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
