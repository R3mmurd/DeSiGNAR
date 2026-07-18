/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file vector2D.hpp
    @brief Vector2D: a free 2D vector built on top of Point2D's (x,y)
    storage, plus magnitude, orientation and arithmetic operations.
    @ingroup Geometry
*/

#pragma once

#include <point2D.hpp>

namespace Designar
{
    /** A 2D vector: reuses Point2D's (x,y) storage and its arithmetic base,
        but is interpreted as a direction/displacement from the origin
        rather than a location, which is why every orientation predicate
        here (is_to_right_from, is_to_left_from, is_collinear_with, ...)
        compares `this` against ZERO and the given vector instead of taking
        two reference points the way the corresponding GenPoint2D predicates
        do. ZERO is the origin vector, i.e. the additive identity. */
    class Vector2D : public Point2D
    {
        using Base = Point2D;
        using Base::Base;

    public:
        /// The origin vector (0, 0), the additive identity.
        static const Vector2D ZERO;

        /// Whether `this` is clockwise from `v` (i.e. to the right of the
        /// ZERO->v direction); equivalent to a negative cross product.
        bool is_to_right_from(const Vector2D&) const;

        /// Whether `this` is clockwise from or collinear with `v`.
        bool is_to_right_on_from(const Vector2D&) const;

        /// Whether `this` is counterclockwise from `v` (i.e. to the left of
        /// the ZERO->v direction); equivalent to a positive cross product.
        bool is_to_left_from(const Vector2D&) const;

        /// Whether `this` is counterclockwise from or collinear with `v`.
        bool is_to_left_on_from(const Vector2D&) const;

        /// Whether `this` and `v` point along the same line through the
        /// origin (their cross product is zero).
        bool is_collinear_with(const Vector2D&) const;

        /// Whether this vector has unit magnitude (magnitude equal to 1).
        bool is_normalized() const;

        /// Performs is_normalized.
        bool is_unitarian() const;

        real_t square_magnitude() const;

        real_t magnitude() const;

        real_t length() const;

        /// Scales this vector to unit magnitude in place; a null or already
        /// unit vector is left unchanged.
        void normalize();

        void negate();

        void scale(double);

        /// Whether `v`, once both vectors are normalized, points in exactly
        /// the opposite direction of this one.
        bool is_opposite(const Vector2D&) const;

        Vector2D get_opposite() const;

        /// Signed angle from this vector to `v`, computed from the cross
        /// and dot products via atan2 so the result is in (-pi, pi].
        real_t angle_with(const Vector2D&) const;

        void add_scaled_vector(const Vector2D&, real_t);

        real_t dot_product(const Vector2D&) const;

        /// Performs dot_product.
        real_t scalar_product(const Vector2D&) const;

        /// The z-component of the 3D cross product of this vector and `v`
        /// treated as lying in the z=0 plane; its sign is what the
        /// is_to_*_from orientation predicates test.
        real_t cross_product(const Vector2D&) const;

        /// Performs cross_product.
        real_t vector_product(const Vector2D&) const;

        /// Multiplies this vector and `v` component-wise (x*x, y*y), not to
        /// be confused with dot_product or cross_product.
        Vector2D component_product(const Vector2D&) const;

        /// Performs get_opossite.
        Vector2D operator-() const;

        /// Multiplies this by a scalar value
        Vector2D operator*(real_t) const;

        /// Accumulates the product of this by a scalar value.
        void operator*=(real_t);

        /// Multiplies a scalar value by a vector.
        friend Vector2D operator*(real_t, const Vector2D&);

        /// Performs dot_product.
        real_t operator*(const Vector2D&) const;

        /// Performs vector addition.
        Vector2D operator+(const Vector2D&) const;

        /// Accumulative vector addition.
        void operator+=(const Vector2D&);

        /// Performs vector substraction.
        Vector2D operator-(const Vector2D&) const;

        /// Accumulative vector substraction.
        void operator-=(const Vector2D&);

        static std::tuple<Vector2D, Vector2D>
        make_orthonormal_basis(const Vector2D&);
    };

} // end namespace Designar
