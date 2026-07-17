/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <quadtree.hpp>
#include <random.hpp>

using namespace std;
using namespace Designar;

int main()
{
  // Basic insert + size(), and points outside the boundary are rejected.
  {
    QuadTree<Point2D> qt(QuadTree<Point2D>::Boundary(Point2D(0., 0.), 100.));

    assert(qt.is_empty());

    assert(qt.insert(Point2D(10., 10.)));
    assert(qt.insert(Point2D(-20., 30.)));
    assert(qt.insert(Point2D(50., -50.)));

    assert(qt.size() == 3);
    assert(!qt.is_empty());

    assert(!qt.insert(Point2D(200., 200.)));
    assert(qt.size() == 3);

    cout << "QuadTree: basic insert Everything ok!\n";
  }

  // Enough points to force subdivision, then verify every point that
  // was actually accepted is findable via a range query covering the
  // whole boundary.
  {
    QuadTree<Point2D> qt(QuadTree<Point2D>::Boundary(Point2D(0., 0.), 100.));

    rng_t rng(1234);
    DynArray<Point2D> inserted;

    for (int_t i = 0; i < 200; ++i)
    {
      Point2D p(real_t(random_uniform(rng, 200)) - 100.,
                real_t(random_uniform(rng, 200)) - 100.);

      if (qt.insert(p))
      {
        inserted.append(p);
      }
    }

    assert(qt.size() == inserted.size());

    auto all = qt.query_range(qt.get_boundary());
    assert(all.size() == inserted.size());

    cout << "QuadTree: subdivision + full-range query Everything ok!\n";
  }

  // Range query correctness: cross-check against a brute-force scan.
  {
    QuadTree<Point2D> qt(QuadTree<Point2D>::Boundary(Point2D(0., 0.), 100.));

    rng_t rng(5678);
    DynArray<Point2D> all_points;

    for (int_t i = 0; i < 300; ++i)
    {
      Point2D p(real_t(random_uniform(rng, 200)) - 100.,
                real_t(random_uniform(rng, 200)) - 100.);

      if (qt.insert(p))
      {
        all_points.append(p);
      }
    }

    QuadTree<Point2D>::Boundary range(Point2D(20., -10.), 15.);

    auto found = qt.query_range(range);

    DynArray<Point2D> expected;

    for (nat_t i = 0; i < all_points.size(); ++i)
    {
      if (range.contains(all_points[i]))
      {
        expected.append(all_points[i]);
      }
    }

    assert(found.size() == expected.size());

    for (nat_t i = 0; i < found.size(); ++i)
    {
      assert(range.contains(found[i]));
    }

    cout << "QuadTree: range query matches brute force Everything ok!\n";
  }

  // Many points at (nearly) the same location: MAX_DEPTH must stop the
  // subdivision from recursing forever rather than crashing/hanging.
  {
    QuadTree<Point2D> qt(QuadTree<Point2D>::Boundary(Point2D(0., 0.), 100.));

    for (int_t i = 0; i < 500; ++i)
    {
      assert(qt.insert(Point2D(1., 1.)));
    }

    assert(qt.size() == 500);

    cout << "QuadTree: degenerate (coincident points) Everything ok!\n";
  }

  // Copy and move semantics.
  {
    QuadTree<Point2D> qt(QuadTree<Point2D>::Boundary(Point2D(0., 0.), 100.));

    for (int_t i = 0; i < 50; ++i)
    {
      qt.insert(Point2D(real_t(i), real_t(i)));
    }

    QuadTree<Point2D> copy(qt);
    assert(copy.size() == qt.size());

    // The copy is independent: mutating it must not affect the original.
    copy.insert(Point2D(99., 99.));
    assert(copy.size() == qt.size() + 1);

    QuadTree<Point2D> moved(std::move(copy));
    assert(moved.size() == qt.size() + 1);

    cout << "QuadTree: copy/move Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
