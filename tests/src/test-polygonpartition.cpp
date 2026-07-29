/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>

#include <polygonpartition.hpp>
#include <polygontriangulation.hpp>

using namespace Designar;

int main()
{
    // A convex polygon needs no diagonals and is already one monotone
    // piece.
    DynArray<Point2D> square = {Point2D(0, 0), Point2D(1, 0), Point2D(1, 1),
                                Point2D(0, 1)};

    assert(monotone_partition_diagonals(square).is_empty());

    DynArray<DynArray<nat_t>> square_faces = monotone_partition(square);
    assert(square_faces.size() == 1);
    assert(is_y_monotone(square, square_faces[0]));

    // A polygon with a single reflex ("notch") vertex needs exactly one
    // split diagonal and becomes exactly two monotone pieces whose total
    // area equals the original polygon's.
    DynArray<Point2D> comb = {Point2D(0, 0), Point2D(4, 0), Point2D(4, 4),
                              Point2D(3, 4), Point2D(2, 1), Point2D(1, 4),
                              Point2D(0, 4)};

    assert(monotone_partition_diagonals(comb).size() == 1);

    DynArray<DynArray<nat_t>> comb_faces = monotone_partition(comb);
    assert(comb_faces.size() == 2);

    real_t total_area = 0.0;

    for (const DynArray<nat_t>& face : comb_faces)
    {
        assert(is_y_monotone(comb, face));

        DynArray<Point2D> pts;

        for (nat_t idx : face)
        {
            pts.append(comb[idx]);
        }

        total_area += std::abs(detail_polygon::polygon_signed_area2(pts)) / 2.0;
    }

    real_t poly_area = std::abs(detail_polygon::polygon_signed_area2(comb)) / 2.0;
    assert(std::abs(total_area - poly_area) < 1e-6);

    // Convex partition: merging ear-clipping triangles must preserve
    // total area and never produce more pieces than triangles.
    DynArray<PolygonTriangle> tris = ear_clipping_triangulation(comb);
    DynArray<DynArray<nat_t>> convex_pieces =
        convex_partition_from_triangulation(comb, tris);

    assert(convex_pieces.size() <= tris.size());

    real_t convex_total = 0.0;

    for (const DynArray<nat_t>& piece : convex_pieces)
    {
        DynArray<Point2D> pts;

        for (nat_t idx : piece)
        {
            pts.append(comb[idx]);
        }

        convex_total +=
            std::abs(detail_polygon::polygon_signed_area2(pts)) / 2.0;

        // Every piece must actually be convex.
        nat_t m = piece.size();

        for (nat_t i = 0; i < m; ++i)
        {
            const Point2D& prev = comb[piece[(i + m - 1) % m]];
            const Point2D& cur = comb[piece[i]];
            const Point2D& next = comb[piece[(i + 1) % m]];
            assert(next.is_to_left_from(prev, cur));
        }
    }

    assert(std::abs(convex_total - poly_area) < 1e-6);

    // Trapezoidalization: a "plus" shape has several reflex vertices
    // genuinely sandwiched between two other edges, so it must produce
    // at least one trapezoid, and every trapezoid's top edge must lie
    // above its bottom edge.
    DynArray<Point2D> plus = {
        Point2D(1, 0), Point2D(2, 0), Point2D(2, 1), Point2D(3, 1),
        Point2D(3, 2), Point2D(2, 2), Point2D(2, 3), Point2D(1, 3),
        Point2D(1, 2), Point2D(0, 2), Point2D(0, 1), Point2D(1, 1)};

    rng_t rng(12345);
    DynArray<Trapezoid> traps = trapezoidalize(plus, rng);
    assert(traps.size() > 0);

    for (const Trapezoid& t : traps)
    {
        real_t top_y = std::min(t.top.get_src_point().get_y(),
                                t.top.get_tgt_point().get_y());
        real_t bottom_y = std::max(t.bottom.get_src_point().get_y(),
                                   t.bottom.get_tgt_point().get_y());
        assert(top_y >= bottom_y - 1e-9);
    }

    return 0;
}
