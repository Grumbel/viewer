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
