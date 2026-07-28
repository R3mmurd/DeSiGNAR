/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>

#include <polygontriangulation.hpp>

using namespace Designar;

namespace
{
    real_t triangle_area(const Point2D& a, const Point2D& b, const Point2D& c)
    {
        return std::abs((b.get_x() - a.get_x()) * (c.get_y() - a.get_y()) -
                        (c.get_x() - a.get_x()) * (b.get_y() - a.get_y())) /
               2.0;
    }

    void check_triangulation(const DynArray<Point2D>& poly,
                             const DynArray<PolygonTriangle>& tris)
    {
        nat_t n = poly.size();
        assert(tris.size() == n - 2);

        real_t total = 0.0;

        for (const PolygonTriangle& t : tris)
        {
            total += triangle_area(poly[t.a], poly[t.b], poly[t.c]);
        }

        real_t poly_area =
            std::abs(detail_polygon::polygon_signed_area2(poly)) / 2.0;
        assert(std::abs(total - poly_area) < 1e-6);
    }

} // namespace

int main()
{
    DynArray<Point2D> square = {Point2D(0, 0), Point2D(4, 0), Point2D(4, 4),
                                Point2D(0, 4)};

    check_triangulation(square, ear_clipping_triangulation(square));
    check_triangulation(square, monotone_triangulation(square));

    // A polygon with one reflex ("notch") vertex, forcing a split during
    // monotone partitioning.
    DynArray<Point2D> comb = {Point2D(0, 0), Point2D(4, 0), Point2D(4, 4),
                              Point2D(3, 4), Point2D(2, 1), Point2D(1, 4),
                              Point2D(0, 4)};

    check_triangulation(comb, ear_clipping_triangulation(comb));
    check_triangulation(comb, monotone_triangulation(comb));

    // A 10-vertex star with 5 reflex vertices.
    DynArray<Point2D> star;
    real_t outer_r = 4.0;
    real_t inner_r = 1.5;
    real_t pi = std::acos(-1.0); // M_PI isn't standard C++ (MSVC needs
                                 // _USE_MATH_DEFINES for it)

    for (int i = 0; i < 10; ++i)
    {
        real_t angle = i * pi / 5.0;
        real_t radius = (i % 2 == 0) ? outer_r : inner_r;
        star.append(
            Point2D(radius * std::cos(angle), radius * std::sin(angle)));
    }

    if (!(detail_polygon::polygon_signed_area2(star) > 0.0))
    {
        DynArray<Point2D> reversed;

        for (nat_t i = star.size(); i > 0; --i)
        {
            reversed.append(star[i - 1]);
        }

        star = reversed;
    }

    check_triangulation(star, ear_clipping_triangulation(star));
    check_triangulation(star, monotone_triangulation(star));

    // ear_clipping_triangulation must reject clockwise input.
    DynArray<Point2D> clockwise_square = {Point2D(0, 0), Point2D(0, 4),
                                          Point2D(4, 4), Point2D(4, 0)};
    bool threw = false;

    try
    {
        ear_clipping_triangulation(clockwise_square);
    }
    catch (const std::domain_error&)
    {
        threw = true;
    }

    assert(threw);

    // Art gallery guard bound: the smallest 3-coloring class must never
    // exceed floor(n / 3) + 1, and must be non-empty for any polygon with
    // at least one triangle.
    DynArray<nat_t> comb_guards = art_gallery_guards(comb);
    assert(comb_guards.size() >= 1);
    assert(comb_guards.size() <= comb.size() / 3 + 1);

    DynArray<nat_t> star_guards = art_gallery_guards(star);
    assert(star_guards.size() >= 1);
    assert(star_guards.size() <= star.size() / 3 + 1);

    // Every guard index must be distinct and in range.
    for (nat_t i = 0; i < comb_guards.size(); ++i)
    {
        for (nat_t j = i + 1; j < comb_guards.size(); ++j)
        {
            assert(comb_guards[i] != comb_guards[j]);
        }

        assert(comb_guards[i] < comb.size());
    }

    return 0;
}
