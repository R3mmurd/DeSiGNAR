/*
  This file is part of Designar Library.
  Copyright (C) 2017 by Alejandro J. Mujica

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Any user request of this software, write to 

  Alejandro Mujica

  aledrums@gmail.com
*/

# include <vector2D.H>

namespace Designar
{
  const Vector2D Vector2D::ZERO;
  
  Vector2D::Vector2D()
    : x(0.), y(0.)
  {
    // Empty
  }

  Vector2D::Vector2D(real_t _x, real_t _y)
    : x(_x), y(_y)
  {
    // Empty
  }

  real_t Vector2D::get_x() const
  {
    return x;
  }

  real_t Vector2D::get_y() const
  {
    return y;
  }

  void Vector2D::set_x(real_t _x)
  {
    x = _x;
  }

  void Vector2D::set_y(real_t _y)
  {
    y = _y;
  }

  bool Vector2D::is_null() const
  {
    return *this == ZERO;
  }

  bool Vector2D::is_zero() const
  {
    return is_null();
  }

  bool Vector2D::is_normalized() const
  {
    return real_equal(square_magnitude(), 1.);
  }

  bool Vector2D::is_unitarian() const
  {
    return is_normalized();
  }

  void Vector2D::nullify()
  {
    x = y = 0.;
  }

  real_t Vector2D::square_magnitude() const
  {
    return x * x + y * y;
  }

  real_t Vector2D::magnitude() const
  {
    return std::sqrt(square_magnitude());
  }

  real_t Vector2D::length() const
  {
    return magnitude();
  }

  void Vector2D::normalize()
  {
    real_t length = magnitude();

    if (real_equal(length, 0.))
      return;

    (*this) *= 1. / length;
  }

  bool Vector2D::is_opposite(const Vector2D & v) const
  {
    Vector2D a = *this;
    Vector2D b = v;

    a.normalize();
    b.normalize();

    return a.x == -b.x and a.y == -b.y;
  }

  Vector2D Vector2D::get_opposite() const
  {
    return Vector2D(-x, -y);
  }

  void Vector2D::negate()
  {
    x *= -1;
    y *= -1;
  }

  real_t Vector2D::angle_with(const Vector2D & v) const
  {
    return atan2(cross_product(v), dot_product(v));
  }

  bool Vector2D::is_to_right_from(const Vector2D & v) const
  {
    return area_of_parallelogram(ZERO, v, *this) < 0.;
  }
  
  bool Vector2D::is_to_left_from(const Vector2D & v) const
  {
    return area_of_parallelogram(ZERO, v, *this) > 0.;
  }

  void Vector2D::add_scaled_vector(const Vector2D & v, real_t scale)
  {
    x += v.x * scale;
    y += v.y * scale;
  }

  Vector2D Vector2D::component_product(const Vector2D & v) const
  {
    Vector2D ret_val;

    ret_val.x = x * v.x;
    ret_val.y = y * v.y;

    return ret_val;
  }

  real_t Vector2D::dot_product(const Vector2D & v) const
  {
    return x * v.x + y * v.y;
  }

  real_t Vector2D::scalar_product(const Vector2D & v) const
  {
    return dot_product(v);
  }

  real_t Vector2D::cross_product(const Vector2D & v) const
  {
    return x * v.y - y * v.x;
  }

  real_t Vector2D::vector_product(const Vector2D & v) const
  {
    return cross_product(v);
  }

  bool Vector2D::operator ! () const
  {
    return is_null();
  }

  Vector2D Vector2D::operator - () const
  {
    return get_opposite();
  }

  Vector2D Vector2D::operator * (real_t scalar) const
  {
    Vector2D ret_val;

    ret_val.x = x * scalar;
    ret_val.y = y * scalar;

    return ret_val;
  }

  Vector2D operator * (real_t scalar, const Vector2D & v)
  {
    Vector2D ret_val;

    ret_val.x = v.x * scalar;
    ret_val.y = v.y * scalar;

    return ret_val;
  }

  real_t Vector2D::operator * (const Vector2D & v) const
  {
    return dot_product(v);
  }

  void Vector2D::operator *= (real_t scalar)
  {
    x *= scalar;
    y *= scalar;
  }

  Vector2D Vector2D::operator + (const Vector2D & v) const
  {
    Vector2D ret_val;

    ret_val.x = x + v.x;
    ret_val.y = y + v.y;

    return ret_val;
  }

  void Vector2D::operator += (const Vector2D & v)
  {
    x += v.x;
    y += v.y;
  }

  Vector2D Vector2D::operator - (const Vector2D & v) const
  {
    Vector2D ret_val;

    ret_val.x = x - v.x;
    ret_val.y = y - v.y;

    return ret_val;
  }

  void Vector2D::operator -= (const Vector2D & v)
  {
    x -= v.x;
    y -= v.y;
  }

  bool Vector2D::operator == (const Vector2D & v) const
  {
    return real_equal(x, v.x) and real_equal(y, v.y);
  }

  bool Vector2D::operator != (const Vector2D & v) const
  {
    return not(*this == v);
  }

  std::string Vector2D::to_string() const
  {
    std::stringstream sstr;
    sstr << "(" << x << "," << y << ")";
    return sstr.str();
  }

  std::tuple<Vector2D, Vector2D>
  Vector2D::make_orthonormal_basis(const Vector2D & v)
  {
    Vector2D a = v;
    Vector2D b;
    a.normalize();
    b.set_y(-1. * a.get_x());
    b.set_x(a.get_y());
    return std::make_tuple(a, b);
  }
} // end namespace Designar
