/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>

#include <delaunay.hpp>
#include <random.hpp>

using namespace Designar;
using namespace Designar::detail_delaunay;

namespace
{
    bool empty_circumcircle_property(const DynArray<Point2D>& pts,
                                     const DynArray<DelaunayTriangle>& tris)
    {
        for (const DelaunayTriangle& t : tris)
        {
            Circumcircle cc = circumcircle(pts[t.a], pts[t.b], pts[t.c]);

            for (nat_t i = 0; i < pts.size(); ++i)
            {
                if (i == t.a || i == t.b || i == t.c)
                {
                    continue;
                }

                if (in_circumcircle(cc, pts[i]))
                {
                    return false;
                }
            }
        }

        return true;
    }

} // namespace

int main()
{
    // A unit square plus its center: exactly 4 triangles (fan from the
    // center to each of the square's 4 edges), all satisfying the
    // empty-circumcircle property.
    DynArray<Point2D> pts = {Point2D(0, 0), Point2D(1, 0), Point2D(1, 1),
                             Point2D(0, 1), Point2D(0.5, 0.5)};

    DynArray<DelaunayTriangle> tris = delaunay_triangulation(pts);
    assert(tris.size() == 4);
    assert(empty_circumcircle_property(pts, tris));

    // No super-triangle vertex (index >= pts.size()) should ever leak
    // into the result.
    for (const DelaunayTriangle& t : tris)
    {
        assert(t.a < pts.size());
        assert(t.b < pts.size());
        assert(t.c < pts.size());
    }

    // A larger random point cloud: the empty-circumcircle property must
    // still hold for every triangle against every other point.
    rng_t rng(7);
    DynArray<Point2D> cloud;

    for (nat_t i = 0; i < 30; ++i)
    {
        cloud.append(Point2D(random_uniform(rng, 100.0), random_uniform(rng, 100.0)));
    }

    DynArray<DelaunayTriangle> cloud_tris = delaunay_triangulation(cloud);
    assert(empty_circumcircle_property(cloud, cloud_tris));

    // Fewer than 3 points has no triangulation at all.
    DynArray<Point2D> too_few = {Point2D(0, 0), Point2D(1, 0)};
    bool threw = false;

    try
    {
        delaunay_triangulation(too_few);
    }
    catch (const std::domain_error&)
    {
        threw = true;
    }

    assert(threw);

    return 0;
}
