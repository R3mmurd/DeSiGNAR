/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGTRIANGLE_H
# define DSGTRIANGLE_H

# include <segment.H>

namespace Designar
{

  template <class PointT>
  class GenTriangle
  {
    PointT p1;
    PointT p2;
    PointT p3;

  public:
    using PointType   = PointT;
    using SegmentType = GenSegment<PointT>;

    GenTriangle(const PointT & _p1, const PointT & _p2, const PointT & _p3)
      : p1(_p1), p2(_p2), p3(_p3)
    {
      if (p1.is_collinear_with(p2, p3))
	throw std::logic_error("Points are collinear");
    }

    GenTriangle(const GenTriangle & t)
      : p1(t.p1), p2(t.p2), p3(t.p3)
    {
      // Empty
    }

    GenTriangle(GenTriangle && t)
    {
      swap(t);
    }

    GenTriangle & operator = (const GenTriangle & t)
    {
      if (this == &t)
	return *this;

      p1 = t.p1;
      p2 = t.p2;
      p3 = t.p3;
      
      return *this;
    }

    GenTriangle & operator = (GenTriangle && t)
    {
      swap(t);
      return *this;
    }

    void swap(GenTriangle & t)
    {
      std::swap(p1, t.p1);
      std::swap(p2, t.p2);
      std::swap(p3, t.p3);
    }

    const PointT & get_p1() const
    {
      return p1;
    }

    const PointT & get_p2() const
    {
      return p2;
    }

    const PointT & get_p3() const
    {
      return p3;
    }

    void set_p1(const PointT & _p1)
    {
      if (_p1.is_collinear_with(p2, p3))
	throw std::domain_error("The new point is collinear with the others");

      p1 = _p1;
    }

    void set_p1(PointT && _p1)
    {
      if (_p1.is_collinear_with(p2, p3))
	throw std::domain_error("The new point is collinear with the others");

      p1 = std::move(_p1);
    }

    void set_p2(const PointT & _p2)
    {
      if (p1.is_collinear_with(_p2, p3))
	throw std::domain_error("The new point is collinear with the others");

      p2 = _p2;
    }

    void set_p2(PointT && _p2)
    {
      if (p1.is_collinear_with(_p2, p3))
	throw std::domain_error("The new point is collinear with the others");

      p2 = std::move(_p2);
    }

    void set_p3(const PointT & _p3)
    {
      if (p1.is_collinear_with(p2, _p3))
	throw std::domain_error("The new point is collinear with the others");

      p3 = _p3;
    }

    void set_p3(PointT && _p3)
    {
      if (p1.is_collinear_with(p2, _p3))
	throw std::domain_error("The new point is collinear with the others");

      p3 = std::move(_p3);
    }

    real_t area() const
    {
      return Designar::abs(area_of_parallelogram(p1, p2, p3)) / 2.;
    }

    bool is_clockwise() const
    {
      return p3.is_to_right_from(p1, p2);
    }

    bool is_counterclockwise() const
    {
      return p3.is_to_left_from(p1, p2);
    }

    bool contains_to(const PointT & p) const
    {
      if (p.is_between(p1, p2) or p.is_between(p2, p3) or p.is_between(p3, p1))
	return true;

      bool test = p.is_to_left_from(p1, p2);

      if (p.is_to_left_from(p2, p3) != test)
	return false;

      if (p.is_to_left_from(p3, p1) != test)
	return false;

      return true;
    }

    bool contains_to(const SegmentType & s) const
    {
      return contains_to(s.get_src_point()) and contains_to(s.get_tgt_point());
    }

    bool intersects_properly_with(const SegmentType & s) const
    {
      return s.intersects_properly_with(SegmentType(p1, p2)) or
	s.intersects_properly_with(SegmentType(p2, p3)) or
	s.intersects_properly_with(SegmentType(p3, p1));
    }

    bool intersects_with(const SegmentType & s) const
    {
      return s.intersects_with(SegmentType(p1, p2)) or
	s.intersects_with(SegmentType(p2, p3)) or
	s.intersects_with(SegmentType(p3, p1));
    }

    SegmentType intersection_with(const SegmentType & s)
    {
      PointT p[2];

      int i = 0;

      try
	{
	  p[i] = s.intersection_with(SegmentType(p1, p2));

	  if (p[i].is_between(p1, p2))
	    ++i;
	}
      catch(const std::domain_error &)
	{
	  // Nothing to do
	}

      try
	{
	  p[i] = s.intersection_with(SegmentType(p2, p3));

	  if (p[i].is_between(p2, p3))
	    ++i;
	}
      catch(const std::domain_error &)
	{
	  // Nothing to do
	}

      if (i == 2)
	return SegmentType (p[0], p[1]);

      try
	{
	  p[i] = s.intersection_with(SegmentType(p3, p1));

	  if (p[i].is_between(p3, p1))
	    ++i;
	}
      catch(const std::domain_error &)
	{
	  // Nothing to do
	}

      if (i == 0)
	throw std::domain_error("Triangle does not intersects with segment");

      return (i == 2) ? SegmentType(p[0], p[1]) : SegmentType(p[0], p[0]);
    }

    bool operator == (const GenTriangle & t) const
    {
      return p1 == t.p1 and p2 == t.p2 and p3 == t.p3;
    }

    bool operator != (const GenTriangle & t) const
    {
      return not (*this == t);
    }
  };

  class TriangleInt : public GenTriangle<PointInt2D>
  {
    using Base = GenTriangle<PointInt2D>;
    using Base::Base;
  };

  class Triangle : public GenTriangle<Point2D>
  {
    using Base = GenTriangle<Point2D>;
    using Base::Base;
  };
  
} // end namespace Designar

# endif // DSGTRIANGLE_H
