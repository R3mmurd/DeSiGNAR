/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGVECTOR2D_H
# define DSGVECTOR2D_H

# include <point2D.H>

namespace Designar
{
  class Vector2D : public Point2D
  {
    using Base = Point2D;
    using Base::Base;

  public:
    static const Vector2D ZERO;

    bool is_to_right_from(const Vector2D &) const;

    bool is_to_right_on_from(const Vector2D &) const;

    bool is_to_left_from(const Vector2D &) const;

    bool is_to_left_on_from(const Vector2D &) const;

    bool is_collinear_with(const Vector2D &) const;
    
    bool is_normalized() const;

    bool is_unitarian() const;

    real_t square_magnitude() const;

    real_t magnitude() const;

    real_t length() const;

    void normalize();

    void negate();

    void scale(double);

    bool is_opposite(const Vector2D &) const;

    Vector2D get_opposite() const;

    real_t angle_with(const Vector2D &) const;

    void add_scaled_vector(const Vector2D &, real_t);

    real_t dot_product(const Vector2D &) const;

    real_t scalar_product(const Vector2D &) const;

    real_t cross_product(const Vector2D &) const;

    real_t vector_product(const Vector2D &) const;

    Vector2D component_product(const Vector2D &) const;

     /// Performs get_opossite.
    Vector2D operator - () const;

    /// Multiplies this by a scalar value
    Vector2D operator * (real_t) const;

    /// Accumulates the product of this by a scalar value.
    void operator *= (real_t);

    /// Multiplies a scalar value by a vector.
    friend Vector2D operator * (real_t, const Vector2D &);

    /// Performs dot_product.
    real_t operator * (const Vector2D &) const;

    /// Performs vector addition.
    Vector2D operator + (const Vector2D &) const;

    /// Accumulative vector addition.
    void operator += (const Vector2D &);

    /// Performs vector substraction.
    Vector2D operator - (const Vector2D &) const;

    /// Accumulative vector substraction.
    void operator -= (const Vector2D &);

    static std::tuple<Vector2D, Vector2D>
    make_orthonormal_basis(const Vector2D &);
  };
  
} // end namespace Designar

# endif // DSGVECTOR_2D_H

