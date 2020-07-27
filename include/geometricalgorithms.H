/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGGEOMETRICALGORITHMS_H
# define DSGGEOMETRICALGORITHMS_H

# include <polygon.H>
# include <list.H>
# include <set.H>
# include <sort.H>

namespace Designar
{

  // Convex hull algorithms

  /** Builds a convex hull from a points set by brute force.

      @author Alejandro J. Mujica
      @ingroup Geometry
  */
  template <class PolygonType>
  class BruteForceConvexHull
  {
    using PointType =   typename PolygonType::PointType;
    using SegmentType = typename PolygonType::SegmentType;
  
    struct CmpSegment
    {
      bool cmp_point(const PointType & p1, const PointType & p2) const
      {
	if (p1.get_x() < p2.get_x())
	  return true;
      
	return not (p2.get_x() < p1.get_x()) and p1.get_y() < p2.get_y();
      }
    
      bool operator () (const SegmentType & s1, const SegmentType & s2)
      {
	if (cmp_point(s1.get_src_point(), s2.get_src_point()))
	  return true;
      
	return not (cmp_point(s2.get_src_point(), s1.get_src_point())) and
	  cmp_point(s1.get_tgt_point(), s2.get_tgt_point());
      }
    };

    using SegmentTypeSet = TreeSet<SegmentType, CmpSegment>;

    using PointTypeIt = typename SLList<PointType>::Iterator;

    bool are_all_points_on_left(const SLList<PointType> & l,
				const SegmentType & s)
    {
      return l.all([&s] (const PointType & p)
		   {
		     return
		       p.is_to_left_on_from(s.get_src_point(), s.get_tgt_point());
		   });
    }
  
    SegmentTypeSet extreme_edges(const SLList<PointType> & point_set)
    {
      SegmentTypeSet ret;
  
      for (PointTypeIt i(point_set); i.has_curr(); i.next())
	{
	  const PointType & p_i = i.get_curr();
	
	  for (PointTypeIt j(point_set); j.has_curr(); j.next())
	    {
	      const PointType & p_j = j.get_curr();
	    
	      if (p_i == p_j)
		continue;
	    
	      SegmentType s(p_i, p_j);
	    
	      if (are_all_points_on_left(point_set, s))
		ret.insert(s);
	    }	  
	}
    
      return ret;
    }
  
  public:
    PolygonType operator () (const SLList<PointType> & point_set)
    {
      PolygonType ret;

      SegmentTypeSet extremes = extreme_edges(point_set);

      SegmentType first_segment = extremes.remove_pos(0);
      ret.add_vertex(first_segment.get_src_point());
      ret.add_vertex(first_segment.get_tgt_point());

      while (true)
	{
	  const PointType & last_vertex = ret.get_last_vertex();
	
	  SegmentType * ptr =
	    extremes.search_ptr([&last_vertex](const SegmentType & s) {
		return s.get_src_point() == last_vertex;
	      });
	
	  assert(ptr != nullptr);
	
	  if (ptr->get_tgt_point() == ret.get_first_vertex())
	    break;
	
	  ret.add_vertex(ptr->get_tgt_point());
	
	  extremes.remove(*ptr);
	}

      return ret;
    }
  };

  /** Builds a convex hull from a points set by quick hull algorithm.

      @author Alejandro J. Mujica
      @ingroup Geometry
  */
  template <class PolygonType>
  class QuickHull
  {
    using PointType =   typename PolygonType::PointType;
    using SegmentType = typename PolygonType::SegmentType;
    using NumType     = typename PointType::NumberType;
    using PointTypeIt = typename SLList<PointType>::Iterator;
    
    PointType get_fartest_point(const SLList<PointType> & point_set,
				const SegmentType & s)
    {
      NumType max_distance = 0;
      PointType ret;
    
      for (PointTypeIt it(point_set); it.has_curr(); it.next())
	{
	  const PointType & p = it.get_curr();

	  SegmentType s1 = s.get_perpendicular(p);

	  NumType sz = s1.length();

	  if (sz > max_distance)
	    {
	      ret = p;
	      max_distance = sz;
	    }
	}

      return ret;
    }

    std::pair<SLList<PointType>, SLList<PointType>>
    get_right_points(SLList<PointType> & point_set,
		     const PointType & a, const PointType & b,
		     const PointType & c)
    {
      std::pair<SLList<PointType>, SLList<PointType>> ret;
      
      while (not point_set.is_empty())
	{
	  PointType p = point_set.remove_first();
	  
	  if (p != a and p != c and p.is_to_right_from(a, c))
	    {
	      ret.first.append(p);
	      continue;
	    }

	  if (p != c and p != b and p.is_to_right_from(c, b))
	    ret.second.append(p);
	}

      return ret;
    }
  
    SLList<PointType> quick_hull(SLList<PointType> & point_set,
				 const PointType & a,
				 const PointType & b)
    {
      if (point_set.is_empty())
	return SLList<PointType>();
    
      PointType c = get_fartest_point(point_set, SegmentType(a, b));

      auto r = get_right_points(point_set, a, b, c);

      SLList<PointType> ret = quick_hull(r.first, a, c);
      SLList<PointType> tmp = quick_hull(r.second, c, b);
      ret.append(c);
      ret.concat(tmp);

      return ret;
    }
  
    std::pair<PointType, PointType>
    search_extremes(const SLList<PointType> & point_set)
    {
      PointTypeIt it(point_set);
      PointType leftmost = it.get_curr();
      PointType rightmost = it.get_curr();
      it.next();

      for ( ; it.has_curr(); it.next())
	{
	  const PointType & p = it.get_curr();

	  if (p.get_x() < leftmost.get_x())
	    leftmost = p;

	  if (p.get_x() > rightmost.get_x())
	    rightmost = p;
	}

      return std::make_pair(leftmost, rightmost);
    }

    std::pair<SLList<PointType>, SLList<PointType>>
    partition(SLList<PointType> & point_set, const PointType & a,
	      const PointType & b)
    {
      std::pair<SLList<PointType>, SLList<PointType>> ret;
    
      for (PointTypeIt it(point_set); it.has_curr(); it.next())
	{
	  const PointType & p = it.get_curr();

	  if (p.is_to_right_from(a, b))
	    ret.first.append(p);
	  else
	    ret.second.append(p);
	}

      return ret;
    }
  
  public:
    PolygonType operator () (SLList<PointType> & point_set)
    {
      PolygonType ret;

      auto e = search_extremes(point_set);
      auto p = partition(point_set, e.first, e.second);

      SLList<PointType> s1 = quick_hull(p.first, e.first, e.second);
      SLList<PointType> s2 = quick_hull(p.second, e.second, e.first);

      SLList<PointType> convex_set;
      convex_set.append(e.first);
      convex_set.concat(s1);
      convex_set.append(e.second);
      convex_set.concat(s2);

      for (PointTypeIt it(convex_set); it.has_curr(); it.next())
	ret.add_vertex(it.get_curr());
    
      return ret;
    }
  };
  
} // end namespace Designar

# endif // DSGGEOMETRICALGORITHMS_H
