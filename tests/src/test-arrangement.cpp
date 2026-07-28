/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>

#include <arrangement.hpp>

using namespace Designar;

int main()
{
    // Two lines in general position (one crossing point).
    LineArrangement arr(10.0);
    arr.insert(Point2D(-5, 1), Point2D(5, 1));  // y = 1
    arr.insert(Point2D(1, -5), Point2D(1, 5));  // x = 1, crosses at (1, 1)

    assert(arr.num_lines() == 2);
    // 4 box-clipped endpoints + 1 crossing point.
    assert(arr.vertices().size() == 5);
    // Each line split into 2 edges by the single crossing.
    assert(arr.edges().size() == 4);
    assert(arr.num_faces_upper_bound() == 1 + 2 + 1); // 1 + n + C(n,2)

    // Three concurrent lines (all through the origin) is a degenerate
    // (non-general-position) case: Euler's formula's C(n,2) term still
    // counts every pair as a crossing even though all three meet at one
    // shared point, so this is only an upper bound here, not exact —
    // just check it doesn't underestimate the true count.
    LineArrangement arr2(10.0);
    arr2.insert(Point2D(-5, 0), Point2D(5, 0));   // y = 0
    arr2.insert(Point2D(0, -5), Point2D(0, 5));   // x = 0
    arr2.insert(Point2D(-5, -5), Point2D(5, 5));  // y = x

    assert(arr2.num_lines() == 3);
    assert(arr2.num_faces_upper_bound() == 1 + 3 + 3);

    // All three lines pass through the origin, so every pairwise
    // crossing coincides there: 3 lines * 2 box-clipped endpoints each
    // (6) plus exactly 1 shared crossing vertex, not 3 distinct ones.
    assert(arr2.vertices().size() == 7);

    return 0;
}
