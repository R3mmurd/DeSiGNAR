/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <vector2D.H>

namespace Designar
{
  const Vector2D Vector2D::ZERO(0., 0.);

  bool Vector2D::is_to_right_from(const Vector2D & v) const
  {
    return Base::is_to_right_from(ZERO, v);
  }
    
  bool Vector2D::is_to_right_on_from(const Vector2D & v) const
  {
    return Base::is_to_right_on_from(ZERO, v);
  }
    
  bool Vector2D::is_to_left_from(const Vector2D & v) const
  {
    return Base::is_to_left_from(ZERO, v);
  }

  bool Vector2D::is_to_left_on_from(const Vector2D & v) const
  {
    return Base::is_to_left_on_from(ZERO, v);
  }

  bool Vector2D::is_collinear_with(const Vector2D & v) const
  {
    return Base::is_collinear_with(ZERO, v);
  }
  
  bool Vector2D::is_normalized() const
  {
    return real_equal(square_magnitude(), 1.);
  }

  bool Vector2D::is_unitarian() const
  {
    return is_normalized();
  }

  real_t Vector2D::square_magnitude() const
  {
    return square_distance_to_origin();
  }

  real_t Vector2D::magnitude() const
  {
    return distance_to_origin();
  }

  real_t Vector2D::length() const
  {
    return magnitude();
  }

  void Vector2D::negate()
  {
    x *= -1.;
    y *= -1.;
  }

  void Vector2D::scale(real_t r)
  {
    x *= r;
    y *= r;
  }

  void Vector2D::normalize()
  {
    real_t length = magnitude();

    if (real_equal(length, 0.) or real_equal(length, 1.))
      return;

    (*this) *= 1. / length;
  }

  bool Vector2D::is_opposite(const Vector2D & v) const
  {
    Vector2D a = *this;
    Vector2D b = v;

    a.normalize();
    b.normalize();

    return real_equal(a.x, -b.x) and real_equal(a.y, -b.y);
  }

  Vector2D Vector2D::get_opposite() const
  {
    return Vector2D(-x, -y);
  }

  real_t Vector2D::angle_with(const Vector2D & v) const
  {
    return atan2(cross_product(v), dot_product(v));
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
