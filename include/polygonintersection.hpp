/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file polygonintersection.hpp
    @brief Polygon-polygon intersection (both given as CCW-ordered
    `DynArray<Point2D>`): convex_polygon_intersection() (Sutherland-
    Hodgman clipping — the same technique voronoi.hpp already uses to
    clip a cell against a half-plane, generalized here to clipping
    against every edge of a second convex polygon) for the convex/convex
    case, and polygon_intersection() for the general (possibly
    non-convex) case, obtained by ear-clipping-triangulating (see
    polygontriangulation.hpp) both polygons and convex-clipping every
    pair of triangles — each triangle is trivially convex, so the union
    of those pairwise results is exactly the true intersection region,
    possibly as several disjoint convex pieces.
    @ingroup Geometry
*/

#pragma once

#include <array.hpp>
#include <point2D.hpp>
#include <polygontriangulation.hpp>

namespace Designar
{
    namespace detail_polygon
    {
        /** The intersection of infinite lines (a1, a2) and (b1, b2), via
            the standard determinant formula — unlike
            GenSegment::intersection_with() (which divides by a
            difference of slopes and produces NaN whenever either
            segment is exactly vertical, since slope() reports
            vertical as +-INF and INF - INF is NaN), this never
            special-cases on orientation and stays well-defined for any
            non-parallel pair. Throws std::domain_error if the lines are
            parallel. */
        inline Point2D line_intersection(const Point2D& a1, const Point2D& a2,
                                         const Point2D& b1, const Point2D& b2)
        {
            real_t x1 = a1.get_x();
            real_t y1 = a1.get_y();
            real_t x2 = a2.get_x();
            real_t y2 = a2.get_y();
            real_t x3 = b1.get_x();
            real_t y3 = b1.get_y();
            real_t x4 = b2.get_x();
            real_t y4 = b2.get_y();

            real_t denom = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

            if (std::abs(denom) < EPSILON)
            {
                throw std::domain_error("line_intersection: lines are parallel");
            }

            real_t cross_a = x1 * y2 - y1 * x2;
            real_t cross_b = x3 * y4 - y3 * x4;

            real_t px = (cross_a * (x3 - x4) - (x1 - x2) * cross_b) / denom;
            real_t py = (cross_a * (y3 - y4) - (y1 - y2) * cross_b) / denom;

            return Point2D(px, py);
        }

        /** Sutherland-Hodgman: clips `subject` (any simple polygon) against
            the single half-plane to the left of directed edge (a, b) —
            correct in general only when repeated over every edge of a
            *convex* clip polygon in CCW order (see
            convex_polygon_intersection() below), same restriction
            voronoi.hpp's clip_by_bisector() documents for its half-plane
            clip. */
        inline DynArray<Point2D> clip_by_halfplane(
            const DynArray<Point2D>& subject, const Point2D& a,
            const Point2D& b)
        {
            DynArray<Point2D> result;
            nat_t n = subject.size();

            if (n == 0)
            {
                return result;
            }

            auto inside = [&](const Point2D& p)
            { return p.is_to_left_on_from(a, b); };

            for (nat_t i = 0; i < n; ++i)
            {
                const Point2D& curr = subject[i];
                const Point2D& prev = subject[(i + n - 1) % n];

                bool curr_in = inside(curr);
                bool prev_in = inside(prev);

                if (curr_in)
                {
                    if (!prev_in)
                    {
                        result.append(line_intersection(a, b, prev, curr));
                    }

                    result.append(curr);
                }
                else if (prev_in)
                {
                    result.append(line_intersection(a, b, prev, curr));
                }
            }

            return result;
        }

    } // end namespace detail_polygon

    /** The intersection of two convex, counterclockwise-ordered polygons,
        via Sutherland-Hodgman: clip `subject` against every edge (as a
        half-plane) of `clip` in turn. Returns an empty `DynArray` if the
        polygons don't overlap. Only correct when `clip` is convex
        (`subject` may be any simple polygon, though in this file it is
        always another convex polygon too — see polygon_intersection()
        below for the non-convex case). */
    inline DynArray<Point2D>
    convex_polygon_intersection(const DynArray<Point2D>& subject,
                               const DynArray<Point2D>& clip)
    {
        using namespace detail_polygon;

        DynArray<Point2D> result = subject;
        nat_t m = clip.size();

        for (nat_t i = 0; i < m && !result.is_empty(); ++i)
        {
            result = clip_by_halfplane(result, clip[i], clip[(i + 1) % m]);
        }

        return result;
    }

    /** The intersection of two simple, counterclockwise-ordered polygons
        (convex or not): ear-clip-triangulate both, convex_polygon_
        intersection() every pair of triangles (each trivially convex),
        and keep the non-degenerate results. The true intersection region
        can be disconnected or have holes in general, so the result is a
        list of convex pieces rather than a single polygon. */
    inline DynArray<DynArray<Point2D>>
    polygon_intersection(const DynArray<Point2D>& poly1,
                         const DynArray<Point2D>& poly2)
    {
        DynArray<PolygonTriangle> tris1 = ear_clipping_triangulation(poly1);
        DynArray<PolygonTriangle> tris2 = ear_clipping_triangulation(poly2);

        DynArray<DynArray<Point2D>> pieces;

        for (const PolygonTriangle& t1 : tris1)
        {
            DynArray<Point2D> tri1 = {poly1[t1.a], poly1[t1.b], poly1[t1.c]};

            for (const PolygonTriangle& t2 : tris2)
            {
                DynArray<Point2D> tri2 = {poly2[t2.a], poly2[t2.b],
                                          poly2[t2.c]};

                DynArray<Point2D> piece =
                    convex_polygon_intersection(tri1, tri2);

                if (piece.size() >= 3)
                {
                    pieces.append(piece);
                }
            }
        }

        return pieces;
    }

} // end namespace Designar
