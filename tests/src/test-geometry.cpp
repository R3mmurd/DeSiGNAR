/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>

using namespace std;

#include <segment.hpp>
#include <polygon.hpp>

using namespace Designar;

int main()
{
  // Regression: GenPoint2D::is_between() used a raw `!=` on floating
  // point x-coordinates (`p.get_x() != q.get_x()`) to decide which axis
  // to compare along, instead of the epsilon-tolerant num_equal() used
  // everywhere else in the class. Two points whose x-coordinates are
  // mathematically equal but differ by a rounding ulp would pick the
  // wrong axis and could report containment incorrectly. This
  // near-vertical segment's endpoints differ in x only by less than
  // math.hpp's EPSILON.
  {
    Point2D p(1.0, 0.0);
    Point2D q(1.0 + 1e-13, 10.0);
    Point2D mid(1.0, 5.0);

    assert(mid.is_between(p, q));
  }

  // Regression: Segment::slope() compared x-coordinates with `==`
  // instead of num_equal() to detect a vertical segment; is_parallel_with
  // compared the resulting slopes with `==` instead of real_equal(), so
  // two segments that are parallel but whose slopes differ by a rounding
  // ulp were reported as non-parallel -- and intersection_with() would
  // then divide by an (almost) zero denominator instead of throwing.
  // Vertical segments (slope() == +-INF) are a special case handled
  // separately, since INF - INF is NaN and would otherwise never compare
  // as real_equal to itself.
  {
    Segment s1(Point2D(0., 0.), Point2D(4., 4.));
    Segment s2(Point2D(0., 1.), Point2D(4., 5.));

    assert(s1.is_parallel_with(s2));

    Segment vertical1(Point2D(2., 0.), Point2D(2., 5.));
    Segment vertical2(Point2D(7., -3.), Point2D(7., 3.));

    assert(vertical1.is_parallel_with(vertical2));
    assert(!vertical1.is_parallel_with(s1));

    bool threw = false;
    try
    {
      s1.intersection_with(s2);
    }
    catch (const domain_error&)
    {
      threw = true;
    }
    assert(threw);
  }

  // Regression: Segment::get_perpendicular() computed `-1. / slope()`
  // unconditionally; for a horizontal segment (slope 0) that divides by
  // zero, and for a vertical segment (slope +-INF, itself already the
  // result of a division by zero in slope()) it mixes zeros and
  // infinities, producing a NaN/garbage point instead of the actual
  // (trivial) perpendicular line.
  {
    Segment horizontal(Point2D(0., 3.), Point2D(5., 3.));
    Point2D through(2., 2.);
    auto perp_h = horizontal.get_perpendicular(through);

    // Perpendicular to a horizontal line is vertical: same x throughout.
    assert(num_equal(perp_h.get_src_point().get_x(),
                     perp_h.get_tgt_point().get_x()));

    Segment vertical(Point2D(4., 0.), Point2D(4., 5.));
    auto perp_v = vertical.get_perpendicular(through);

    // Perpendicular to a vertical line is horizontal: same y throughout.
    assert(num_equal(perp_v.get_src_point().get_y(),
                     perp_v.get_tgt_point().get_y()));

    // General (non-degenerate) case still works and is genuinely
    // perpendicular (slopes multiply to -1).
    Segment diagonal(Point2D(0., 0.), Point2D(4., 4.));
    auto perp_d = diagonal.get_perpendicular(Point2D(1., 3.));
    assert(diagonal.is_perpendicular_with(perp_d));
  }

  // point_in_polygon: ray-casting (even-odd rule).
  {
    // unit square [0,0]-[4,0]-[4,4]-[0,4]
    Polygon poly;
    poly.add_vertex(Point2D(0., 0.));
    poly.add_vertex(Point2D(4., 0.));
    poly.add_vertex(Point2D(4., 4.));
    poly.add_vertex(Point2D(0., 4.));

    assert(point_in_polygon(poly, Point2D(2., 2.)));   // center: inside
    assert(!point_in_polygon(poly, Point2D(5., 5.)));  // outside
    assert(!point_in_polygon(poly, Point2D(-1., 2.))); // outside, left
    assert(point_in_polygon(poly, Point2D(0.5, 0.5))); // inside, near corner

    // concave polygon (an "L" shape): (0,0)-(4,0)-(4,2)-(2,2)-(2,4)-(0,4)
    Polygon lshape;
    lshape.add_vertex(Point2D(0., 0.));
    lshape.add_vertex(Point2D(4., 0.));
    lshape.add_vertex(Point2D(4., 2.));
    lshape.add_vertex(Point2D(2., 2.));
    lshape.add_vertex(Point2D(2., 4.));
    lshape.add_vertex(Point2D(0., 4.));

    assert(point_in_polygon(lshape, Point2D(1., 1.)));  // inside the L
    assert(point_in_polygon(lshape, Point2D(3., 1.)));  // inside the L
    assert(!point_in_polygon(lshape, Point2D(3., 3.))); // in the "notch", outside
  }

  cout << "Everything ok!\n";

  return 0;
}
