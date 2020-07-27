/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGPOINT_H
# define DSGPOINT_H

# include <math.H>

namespace Designar
{

  template <typename NumT>
  class GenPoint2D
  {
    static_assert(std::is_arithmetic<NumT>::value,
		  "Template argument must be an arithmetic type");

  protected:
    NumT x;
    NumT y;

  public:
    using ComponentType = NumT;
    using NumberType    = NumT;
    using ValueType     = NumT;
    
    GenPoint2D()
      : x(0), y(0)
    {
      // empty
    }

    GenPoint2D(const NumT & _x, const NumT & _y)
      : x(_x), y(_y)
    {
      // empty
    }

    GenPoint2D(const NumT & _x, NumT && _y)
      : x(_x), y(std::forward<NumT>(_y))
    {
      // empty
    }

    GenPoint2D(NumT && _x, const NumT & _y)
      : x(std::forward<NumT>(_x)), y(_y)
    {
      // empty
    }

    GenPoint2D(NumT && _x, NumT && _y)
      : x(std::forward<NumT>(_x)), y(std::forward<NumT>(_y))
    {
      // empty
    }

    GenPoint2D(const GenPoint2D & p)
      : x(p.x), y(p.y)
    {
      // empty
    }

    GenPoint2D(GenPoint2D && p)
      : GenPoint2D()
    {
      swap(p);
    }

    GenPoint2D & operator = (const GenPoint2D & p)
    {
      if (this == &p)
	return *this;

      x = p.x;
      y = p.y;

      return *this;
    }

    GenPoint2D & operator = (GenPoint2D && p)
    {
      swap(p);
      return *this;
    }

    void swap(GenPoint2D & p)
    {
      std::swap(x, p.x);
      std::swap(y, p.y);
    }

    const NumT & get_x() const
    {
      return x;
    }

    const NumT & get_y() const
    {
      return y;
    }

    void set_x(const NumT & _x)
    {
      x = _x;
    }

    void set_x(NumT && _x)
    {
      x = std::move(_x);
    }

    void set_y(const NumT & _y)
    {
      y = _y;
    }

    void set_y(NumT && _y)
    {
      y = std::move(_y);
    }

    void nullify()
    {
      x = y = NumT(0);
    }

    bool is_null() const
    {
      return num_equal(x, NumT(0)) and num_equal(y, NumT(0));
    }

    bool is_zero() const
    {
      return is_null();
    }

    NumT square_distance_with(const GenPoint2D & p) const
    {
      NumT dx = p.x - x;
      NumT dy = p.y - y;
      return dx * dx + dy * dy;
    }

    real_t distance_with(const GenPoint2D & p) const
    {
      return std::sqrt(square_distance_with(p));
    }

    NumT square_distance_to_origin() const
    {
      return x * x + y * y;
    }

    NumT distance_to_origin() const
    {
      return std::sqrt(square_distance_to_origin());
    }

    bool is_to_right_from(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) < NumT(0);
    }

    bool is_to_right_on_from(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) <= NumT(0);
    }

    bool is_to_left_from(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) > NumT(0);
    }

    bool is_to_left_on_from(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return area_of_parallelogram(p, q, *this) >= NumT(0);
    }

    bool is_collinear_with(const GenPoint2D & p, const GenPoint2D & q) const
    {
      return num_equal(area_of_parallelogram(p, q, *this), NumT(0));
    }

    bool is_between(const GenPoint2D & p, const GenPoint2D & q) const
    {
      if (not is_collinear_with(p, q))
	return false;

      if (p.get_x() != q.get_x())
	return ((p.get_x() <= this->get_x()) and (this->get_x() <= q.get_x()))
	  or ((p.get_x() >= this->get_x()) and (this->get_x() >= q.get_x()));
      else
	return ((p.get_y() <= this->get_y()) and (this->get_y() <= q.get_y()))
	  or ((p.get_y() >= this->get_y()) and (this->get_y() >= q.get_y()));
    }

    explicit operator bool() const
    {
      return not is_null();
    }

    bool operator == (const GenPoint2D & p) const
    {
      return num_equal(x, p.x) and num_equal(y, p.y);
    }

    bool operator != (const GenPoint2D & p) const
    {
      return not (*this == p);
    }

    std::string to_string() const
    {
      std::stringstream s;
      s << '(' << x << ',' << y << ')';
      return s.str();
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
