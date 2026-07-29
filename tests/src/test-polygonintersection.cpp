/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>

#include <polygonintersection.hpp>

using namespace Designar;

namespace
{
    real_t poly_area(const DynArray<Point2D>& p)
    {
        return std::abs(detail_polygon::polygon_signed_area2(p)) / 2.0;
    }

} // namespace

int main()
{
    // Two overlapping convex squares: [0,2]x[0,2] and [1,3]x[1,3], whose
    // overlap is exactly [1,2]x[1,2] (area 1).
    DynArray<Point2D> s1 = {Point2D(0, 0), Point2D(2, 0), Point2D(2, 2),
                            Point2D(0, 2)};
    DynArray<Point2D> s2 = {Point2D(1, 1), Point2D(3, 1), Point2D(3, 3),
                            Point2D(1, 3)};

    DynArray<Point2D> inter = convex_polygon_intersection(s1, s2);
    assert(std::abs(poly_area(inter) - 1.0) < 1e-6);

    // Disjoint squares intersect to nothing.
    DynArray<Point2D> s3 = {Point2D(10, 10), Point2D(12, 10), Point2D(12, 12),
                            Point2D(10, 12)};
    assert(convex_polygon_intersection(s1, s3).is_empty());

    // One polygon fully containing the other: the intersection is the
    // smaller one, unchanged in area.
    DynArray<Point2D> big = {Point2D(-5, -5), Point2D(5, -5), Point2D(5, 5),
                             Point2D(-5, 5)};
    DynArray<Point2D> small = {Point2D(-1, -1), Point2D(1, -1), Point2D(1, 1),
                               Point2D(-1, 1)};
    assert(std::abs(poly_area(convex_polygon_intersection(big, small)) -
                    poly_area(small)) < 1e-6);

    // Non-convex case: an L-shaped polygon intersected with a square.
    // L = bottom strip [0,3]x[0,1] plus left strip [0,1]x[1,3].
    DynArray<Point2D> l_shape = {Point2D(0, 0), Point2D(3, 0), Point2D(3, 1),
                                Point2D(1, 1), Point2D(1, 3), Point2D(0, 3)};
    DynArray<Point2D> sq = {Point2D(0, 0), Point2D(2, 0), Point2D(2, 2),
                            Point2D(0, 2)};

    DynArray<DynArray<Point2D>> pieces = polygon_intersection(l_shape, sq);
    real_t total = 0.0;

    for (const DynArray<Point2D>& piece : pieces)
    {
        total += poly_area(piece);
    }

    // Bottom-strip part inside [0,2]x[0,2]: area 2. Left-strip part
    // inside the same square, above the bottom strip: area 1. Total: 3.
    assert(std::abs(total - 3.0) < 1e-6);

    return 0;
}
