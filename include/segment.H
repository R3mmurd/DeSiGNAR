/*
  This file is part of Designar.
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

# ifndef DSGSEGMENT_H
# define DSGSEGMENT_H

# include <point2D.H>

namespace Designar
{

  template <class PointT>
  class GenSegment
  {
    PointT src_point;
    PointT tgt_point;

  public:
    using PointType = PointT;
    
    GenSegment() = default;

    GenSegment(const PointT & sp, const PointT & tp)
      : src_point(sp), tgt_point(tp)
    {
      // empty
    }

    GenSegment(const PointT & sp, PointT && tp)
      : src_point(sp), tgt_point(std::forward<PointT>(tp))
    {
      // empty
    }

    GenSegment(PointT && sp, const PointT & tp)
      : src_point(std::forward<PointT>(sp)), tgt_point(tp)
    {
      // empty
    }

    GenSegment(PointT && sp, PointT && tp)
      : src_point(std::forward<PointT>(sp)), tgt_point(std::forward<PointT>(tp))
    {
      // empty
    }

    GenSegment(const GenSegment & s)
      : src_point(s.src_point), tgt_point(s.tgt_point)
    {
      // empty
    }

    GenSegment(GenSegment && s)
      : GenSegment()
    {
      swap(s);
    }

    GenSegment & operator = (const GenSegment & s)
    {
      if (this == &s)
	return *this;

      src_point = s.src_point;
      tgt_point = s.tgt_point;
      
      return *this;
    }

    GenSegment & operator = (GenSegment && s)
    {
      swap(s);
      return *this;
    }
    
    void swap(GenSegment & s)
    {
      std::swap(src_point, s.src_point);
      std::swap(tgt_point, s.tgt_point);
    }

    const PointT & get_src_point() const
    {
      return src_point;
    }

    const PointT & get_tgt_point() const
    {
      return tgt_point;
    }

    void set_src_point(const PointT & sp)
    {
      src_point = sp;
    }

    void set_src_point(PointT && sp)
    {
      src_point = std::move(sp);
    }

    void set_tgt_point(const PointT & tp)
    {
      tgt_point = tp;
    }

    void set_tgt_point(PointT && tp)
    {
      tgt_point = std::move(tp);
    }

    real_t length() const
    {
      return src_point.distance_with(tgt_point);
    }

    bool is_to_left_from(const PointT & p) const
    {
      return p.is_to_right_from(src_point, tgt_point);
    }

    bool is_to_left_on_from(const PointT & p) const
    {
      return p.is_to_right_on_from(src_point, tgt_point);
    }

    bool is_to_right_from(const PointT & p) const
    {
      return p.is_to_left_from(src_point, tgt_point);
    }

    bool is_to_right_on_from(const PointT & p) const
    {
      return p.is_to_left_on_from(src_point, tgt_point);
    }

    bool is_collinear_with(const PointT & p) const
    {
      return p.is_collinear_with(src_point, tgt_point);
    }

    bool contains_to(const PointT & p) const
    {
      return p.is_between(src_point, tgt_point);
    }

    bool contains_to(const GenSegment & s) const
    {
      return contains_to(s.src_point) and contains_to(s.tgt_point);
    }

    bool intersects_properly_with(const GenSegment & s) const
    {
      if (s.is_collinear_with(src_point) or s.is_collinear_with(tgt_point) or
	  is_collinear_with(s.src_point) or is_collinear_with(s.tgt_point))
	return false;

      return (s.is_to_right_from(src_point) xor
	      s.is_to_right_from(tgt_point)) and
	(is_to_right_from(src_point) xor
	 is_to_right_from(tgt_point));      
    }

    bool intersects_with(const GenSegment & s) const
    {
      if (intersects_properly_with(s))
	return true;
      
      return contains_to(s.src_point) or contains_to(s.tgt_point) or
	s.contains_to(src_point) or s.contains_to(tgt_point);
    }

    real_t slope() const
    {
      if (tgt_point.get_x() == src_point.get_x())
	{
	  if  (src_point.get_y() < tgt_point.get_y())
	    return INF;
	  else
	    return -INF;
	}
      
      return real_t(tgt_point.get_y() - src_point.get_y()) /
	real_t(tgt_point.get_x() - src_point.get_x());
    }

    bool is_parallel_with(const GenSegment & s) const
    {
      return slope() == s.slope();
    }

    GenSegment get_opposite() const
    {
      return GenSegment(tgt_point, src_point);
    }

    PointT intersection_with(const GenSegment & s) const
    {
      if (is_parallel_with(s))
	throw std::domain_error("Segments are parallels");
      
      const typename PointT::NumberType & x1 = src_point.get_x();
      const typename PointT::NumberType & y1 = src_point.get_y();
      
      const typename PointT::NumberType & x2 = s.src_point.get_x();
      const typename PointT::NumberType & y2 = s.src_point.get_y();
      
      const real_t & m1 = slope();
      const real_t & m2 = s.slope();
      
      const typename PointT::NumberType x = 
	real_t(y2 - y1 + m1 * x1 - m2 * x2) / real_t(m1 - m2);
      
      const typename PointT::Number_Type y = m1 * (x - x1) + y1;
      
      return PointT(x, y);
    }

    bool is_null() const
    {
      return src_point == tgt_point;
    }

    bool operator ! () const
    {
      return is_null();
    }

    bool operator == (const GenSegment & s) const
    {
      return src_point == s.src_point and tgt_point == s.tgt_point;
    }

    bool operator != (const GenSegment & s) const
    {
      return not (*this == s);
    }
  };

  class SegmentInt : public GenSegment<PointInt2D>
  {
    using Base = GenSegment<PointInt2D>;
    using Base::Base;
  };

  class Segment : public GenSegment<Point2D>
  {
    using Base = GenSegment<Point2D>;
    using Base::Base;
  };
  
} // end namespace Designar

# endif // DSGSEGMENT_H
