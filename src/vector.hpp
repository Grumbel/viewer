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

#ifndef HEADER_VECTOR_HPP
#define HEADER_VECTOR_HPP

#include <cmath>

struct Vector
{
  float x;
  float y;
  float z;

  Vector() : x(0), y(0), z(0) {}
  
  Vector(float x_, float y_, float z_) :
    x(x_),
    y(y_),
    z(z_)
  {}

  float norm()
  {
    return std::sqrt(x*x + y*y + z*z);
  }

  void normalize()
  {
    float f = norm();
    x /= f;
    y /= f;
    z /= f;
  }
};

inline Vector operator*(float f, const Vector& a)
{
  return Vector(a.x * f,
                a.y * f,
                a.z * f);
}

inline Vector operator*(const Vector& a, float f)
{
  return Vector(a.x * f,
                a.y * f,
                a.z * f);
}

inline Vector operator-(const Vector& a, const Vector& b)
{
  return Vector(a.x - b.x,
                a.y - b.y,
                a.z - b.z);
}

inline Vector operator+(const Vector& a, const Vector& b)
{
  return Vector(a.x + b.x,
                a.y + b.y,
                a.z + b.z);
}

#endif

/* EOF */
