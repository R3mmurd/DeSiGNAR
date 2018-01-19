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

# ifndef DSGPOINT_H
# define DSGPOINT_H

# include <math.H>

namespace Designar
{

  template <typename NumberType>
  class GenPoint2D
  {
    static_assert(std::is_arithmetic<NumberType>::value,
		  "Template argument must be an arithmetic type");

  protected:
    NumberType x;
    NumberType y;

  public:
    GenPoint2D()
      : x(0), y(0)
    {
      // empty
    }

    GenPoint2D(NumberType _x, NumberType _y)
      : x(_x), y(_y)
    {
      // empty
    }

    NumberType get_x() const
    {
      return x;
    }

    NumberType get_y() const
    {
      return y;
    }

    void set_x(NumberType _x)
    {
      x = _x;
    }

    void set_y(NumberType _y)
    {
      y = _y;
    }

    void nullify()
    {
      x = y = NumberType(0);
    }

    bool is_null() const
    {
      return x == 0 and y == 0;
    }

    bool is_zero() const
    {
      return is_null();
    }

    void negate()
    {
      x *= NumberType(-1);
      y *= NumberType(-1);
    }

    NumberType square_distance_with(const GenPoint2D & p) const
    {
      NumberType dx = p.x - x;
      NumberType dy = p.y - y;
      return dx * dx + dy * dy;
    }

    real_t distance_with(const GenPoint2D & p) const
    {
      return std::sqrt(square_distance_with(p));
    }

    NumberType square_distance_to_origin() const
    {
      return x * x + y * y;
    }

    NumberType distance_to_origin() const
    {
      return std::sqrt(square_distance_to_origin());
    }

    bool is_to_right_from(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) < NumberType(0);
    }

    bool is_to_right_on_from(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) <= NumberType(0);
    }

    bool is_to_left_from(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) > NumberType(0);
    }

    bool is_to_left_on_from(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) >= NumberType(0);
    }

    bool is_collinear_with(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) == NumberType(0);
    }

    bool operator ! () const
    {
      return is_null();
    }

    bool operator == (const GenPoint2D & p) const
    {
      return x == p.x and y == p.y;
    }

    bool operator != (const GenPoint2D & p) const
    {
      return not (*this == p);
    }
  };

  class PointInt2D : public GenPoint2D<lint_t>
  {
    using Base = GenPoint2D<lint_t>;
    using Base::Base;
  };

  class Point2D : public GenPoint2D<real_t>
  {
    using Base = GenPoint2D<real_t>;
    using Base::Base;
  };
  
} // end namespace Designar

# endif // DSGPOINT_H
