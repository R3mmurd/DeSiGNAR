/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file delaunay.hpp
    @brief Delaunay triangulation of a point set via the Bowyer-Watson
    incremental algorithm. Dual to (but, per this file, not mechanically
    derived from) voronoi.hpp's VoronoiDiagram: the Delaunay
    triangulation's edges connect exactly the sites whose Voronoi cells
    share a boundary, but Bowyer-Watson is the more commonly implemented
    route on its own and does not need the Voronoi diagram to already
    exist.
    @ingroup Geometry
*/

#pragma once

#include <cmath>

#include <array.hpp>
#include <point2D.hpp>

namespace Designar
{
    /** One triangle of a Delaunay triangulation, as three indices into
        the original point set. */
    struct DelaunayTriangle
    {
        nat_t a;
        nat_t b;
        nat_t c;
    };

    namespace detail_delaunay
    {
        struct Circumcircle
        {
            real_t cx;
            real_t cy;
            real_t r2;
        };

        /** The circumcenter/radius^2 of the triangle (p1, p2, p3), via
            the standard perpendicular-bisector-intersection formula.
            Throws if the three points are collinear (no finite
            circumcircle exists). */
        inline Circumcircle circumcircle(const Point2D& p1, const Point2D& p2,
                                         const Point2D& p3)
        {
            real_t ax = p1.get_x();
            real_t ay = p1.get_y();
            real_t bx = p2.get_x();
            real_t by = p2.get_y();
            real_t cx = p3.get_x();
            real_t cy = p3.get_y();

            real_t d = 2.0 * (ax * (by - cy) + bx * (cy - ay) + cx * (ay - by));

            if (std::abs(d) < EPSILON)
            {
                throw std::domain_error(
                    "circumcircle: points are collinear");
            }

            real_t a2 = ax * ax + ay * ay;
            real_t b2 = bx * bx + by * by;
            real_t c2 = cx * cx + cy * cy;

            real_t ux = (a2 * (by - cy) + b2 * (cy - ay) + c2 * (ay - by)) / d;
            real_t uy = (a2 * (cx - bx) + b2 * (ax - cx) + c2 * (bx - ax)) / d;

            real_t dx = ax - ux;
            real_t dy = ay - uy;

            return Circumcircle{ux, uy, dx * dx + dy * dy};
        }

        inline bool in_circumcircle(const Circumcircle& cc, const Point2D& p)
        {
            real_t dx = p.get_x() - cc.cx;
            real_t dy = p.get_y() - cc.cy;
            return dx * dx + dy * dy < cc.r2 - EPSILON;
        }

    } // end namespace detail_delaunay

    /** Bowyer-Watson incremental Delaunay triangulation of `points`
        (`points.size() >= 3`, and not all collinear): start from one big
        "super-triangle" containing every point, insert points one at a
        time — for each, find every triangle whose circumcircle contains
        it ("bad" triangles), remove them (their union is always a star-
        shaped cavity around the new point), and re-triangulate the
        cavity by connecting the new point to every edge of its boundary
        — then discard every triangle still touching a super-triangle
        vertex at the end. Returns triangles as index triples into
        `points` (indices `points.size()`, `points.size()+1`,
        `points.size()+2` — the super-triangle's own corners — never
        appear in the result). */
    inline DynArray<DelaunayTriangle>
    delaunay_triangulation(const DynArray<Point2D>& points)
    {
        using namespace detail_delaunay;

        nat_t n = points.size();

        if (n < 3)
        {
            throw std::domain_error(
                "delaunay_triangulation: need at least 3 points");
        }

        real_t min_x = points[0].get_x();
        real_t max_x = points[0].get_x();
        real_t min_y = points[0].get_y();
        real_t max_y = points[0].get_y();

        for (const Point2D& p : points)
        {
            min_x = std::min(min_x, p.get_x());
            max_x = std::max(max_x, p.get_x());
            min_y = std::min(min_y, p.get_y());
            max_y = std::max(max_y, p.get_y());
        }

        real_t dx = max_x - min_x;
        real_t dy = max_y - min_y;
        real_t delta = std::max(dx, dy) * 20.0 + 10.0;
        real_t mid_x = (min_x + max_x) / 2.0;
        real_t mid_y = (min_y + max_y) / 2.0;

        // A working point list: the original points followed by the
        // super-triangle's 3 corners (indices n, n+1, n+2).
        DynArray<Point2D> pts = points;
        pts.append(Point2D(mid_x - 2.0 * delta, mid_y - delta));
        pts.append(Point2D(mid_x, mid_y + 2.0 * delta));
        pts.append(Point2D(mid_x + 2.0 * delta, mid_y - delta));

        DynArray<DelaunayTriangle> triangles;
        triangles.append(DelaunayTriangle{n, n + 1, n + 2});

        for (nat_t p_idx = 0; p_idx < n; ++p_idx)
        {
            const Point2D& p = pts[p_idx];

            DynArray<DelaunayTriangle> bad;
            DynArray<DelaunayTriangle> kept;

            for (const DelaunayTriangle& t : triangles)
            {
                Circumcircle cc = circumcircle(pts[t.a], pts[t.b], pts[t.c]);

                if (in_circumcircle(cc, p))
                {
                    bad.append(t);
                }
                else
                {
                    kept.append(t);
                }
            }

            // The boundary of the union of `bad` triangles: every edge
            // that belongs to exactly one bad triangle (an edge shared
            // by two bad triangles is interior to the cavity, not on its
            // boundary).
            DynArray<std::pair<nat_t, nat_t>> boundary;

            auto edge_count = [&](nat_t u, nat_t v)
            {
                nat_t count = 0;

                for (const DelaunayTriangle& t : bad)
                {
                    nat_t verts[3] = {t.a, t.b, t.c};

                    for (nat_t i = 0; i < 3; ++i)
                    {
                        nat_t x = verts[i];
                        nat_t y = verts[(i + 1) % 3];

                        if ((x == u && y == v) || (x == v && y == u))
                        {
                            ++count;
                        }
                    }
                }

                return count;
            };

            for (const DelaunayTriangle& t : bad)
            {
                nat_t verts[3] = {t.a, t.b, t.c};

                for (nat_t i = 0; i < 3; ++i)
                {
                    nat_t u = verts[i];
                    nat_t v = verts[(i + 1) % 3];

                    if (edge_count(u, v) == 1)
                    {
                        boundary.append(std::make_pair(u, v));
                    }
                }
            }

            triangles = kept;

            for (const auto& e : boundary)
            {
                triangles.append(DelaunayTriangle{e.first, e.second, p_idx});
            }
        }

        DynArray<DelaunayTriangle> result;

        for (const DelaunayTriangle& t : triangles)
        {
            if (t.a < n && t.b < n && t.c < n)
            {
                result.append(t);
            }
        }

        return result;
    }

} // end namespace Designar
