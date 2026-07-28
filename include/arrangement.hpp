/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file arrangement.hpp
    @brief Incremental construction of a line arrangement: given a set of
    lines (each clipped to a bounding box, turning the usual unbounded
    arrangement of infinite lines into a finite one an actual data
    structure can hold), incrementally insert them one at a time,
    recording every intersection vertex and splitting each line into the
    edges between consecutive vertices along it. Explicitly out of
    scope, as agreed for this geometry pass: n-dimensional arrangements
    and higher-order Voronoi diagrams — both are research-topic
    generalizations with no single standard textbook algorithm to
    implement at this scope.
    @ingroup Geometry
*/

#pragma once

#include <algorithm>

#include <array.hpp>
#include <point2D.hpp>
#include <segment.hpp>

namespace Designar
{
    /** A line given by two distinct points; only the line through them
        matters, not the two points themselves. */
    struct ArrangementLine
    {
        Point2D p1;
        Point2D p2;
    };

    /** The result of arranging a set of lines inside `[-half_size,
        half_size]^2`: every line's ordered list of vertices (its own two
        boundary-clipped endpoints plus every point where it crosses
        another line, sorted along the line) and the edges between
        consecutive such vertices. `num_faces()` applies Euler's formula
        for a connected planar arrangement of `n` lines in general
        position: `1 + n + C(n, 2)` regions (1 for the plane itself, +1
        per line, +1 per pair of lines that cross) — an upper bound in
        general (parallel lines or 3+ lines meeting at one point both
        reduce the true count), exact whenever no two of the input lines
        are parallel and no three meet at a common point. */
    class LineArrangement
    {
        real_t half_size;
        DynArray<ArrangementLine> lines;
        DynArray<DynArray<Point2D>> vertices_per_line;

        static bool nearly_parallel(const Point2D& a1, const Point2D& a2,
                                    const Point2D& b1, const Point2D& b2)
        {
            real_t dx1 = a2.get_x() - a1.get_x();
            real_t dy1 = a2.get_y() - a1.get_y();
            real_t dx2 = b2.get_x() - b1.get_x();
            real_t dy2 = b2.get_y() - b1.get_y();
            return std::abs(dx1 * dy2 - dy1 * dx2) < EPSILON;
        }

        static Point2D intersect(const Point2D& a1, const Point2D& a2,
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
            real_t cross_a = x1 * y2 - y1 * x2;
            real_t cross_b = x3 * y4 - y3 * x4;

            real_t px = (cross_a * (x3 - x4) - (x1 - x2) * cross_b) / denom;
            real_t py = (cross_a * (y3 - y4) - (y1 - y2) * cross_b) / denom;

            return Point2D(px, py);
        }

        /** Clips the infinite line through (p1, p2) to the bounding
            square, returning its two boundary points (in an arbitrary
            but consistent order later re-sorted by insert()). */
        std::pair<Point2D, Point2D> clip_to_box(const Point2D& p1,
                                                const Point2D& p2) const
        {
            DynArray<Point2D> hits;
            real_t h = half_size;

            Point2D corners[4] = {Point2D(-h, -h), Point2D(h, -h),
                                  Point2D(h, h), Point2D(-h, h)};

            for (nat_t i = 0; i < 4; ++i)
            {
                const Point2D& c1 = corners[i];
                const Point2D& c2 = corners[(i + 1) % 4];

                if (nearly_parallel(p1, p2, c1, c2))
                {
                    continue;
                }

                Point2D pt = intersect(p1, p2, c1, c2);

                if (pt.get_x() >= -h - EPSILON && pt.get_x() <= h + EPSILON &&
                    pt.get_y() >= -h - EPSILON && pt.get_y() <= h + EPSILON)
                {
                    bool duplicate = false;

                    for (const Point2D& q : hits)
                    {
                        if (q.distance_with(pt) < EPSILON)
                        {
                            duplicate = true;
                            break;
                        }
                    }

                    if (!duplicate)
                    {
                        hits.append(pt);
                    }
                }
            }

            if (hits.size() < 2)
            {
                throw std::domain_error(
                    "LineArrangement: line does not cross the bounding box "
                    "twice");
            }

            return {hits[0], hits[1]};
        }

    public:
        explicit LineArrangement(real_t _half_size) : half_size(_half_size)
        {
            if (!(half_size > 0.0))
            {
                throw std::domain_error(
                    "LineArrangement: half_size must be positive");
            }
        }

        nat_t num_lines() const
        {
            return lines.size();
        }

        /** Inserts a new line (through `p1`, `p2`), clips it to the
            bounding box, and recomputes every already-inserted line's
            vertex list along with the new line's own — the "incremental"
            part of the construction: each insertion only needs to
            intersect the new line against every existing one (O(n) new
            intersection tests), not recompute the whole arrangement from
            scratch. */
        void insert(const Point2D& p1, const Point2D& p2)
        {
            auto clipped = clip_to_box(p1, p2);
            lines.append(ArrangementLine{clipped.first, clipped.second});

            DynArray<Point2D> new_line_vertices;
            new_line_vertices.append(clipped.first);
            new_line_vertices.append(clipped.second);

            nat_t new_idx = lines.size() - 1;

            for (nat_t i = 0; i + 1 < lines.size(); ++i)
            {
                const ArrangementLine& other = lines[i];

                if (nearly_parallel(clipped.first, clipped.second, other.p1,
                                    other.p2))
                {
                    continue;
                }

                Point2D pt = intersect(clipped.first, clipped.second,
                                       other.p1, other.p2);

                if (pt.get_x() < -half_size - EPSILON ||
                    pt.get_x() > half_size + EPSILON ||
                    pt.get_y() < -half_size - EPSILON ||
                    pt.get_y() > half_size + EPSILON)
                {
                    continue; // crosses outside the bounding box
                }

                new_line_vertices.append(pt);
                vertices_per_line[i].append(pt);
            }

            vertices_per_line.append(new_line_vertices);

            // Keep every line's vertex list sorted along the line so
            // consecutive entries are the endpoints of one edge.
            for (nat_t i = 0; i <= new_idx; ++i)
            {
                const ArrangementLine& l = lines[i];
                std::sort(vertices_per_line[i].begin(),
                         vertices_per_line[i].end(),
                         [&](const Point2D& a, const Point2D& b)
                         {
                             return a.square_distance_with(l.p1) <
                                    b.square_distance_with(l.p1);
                         });
            }
        }

        /** Every edge (as a segment between two consecutive vertices) of
            every inserted line. */
        DynArray<Segment> edges() const
        {
            DynArray<Segment> result;

            for (const DynArray<Point2D>& verts : vertices_per_line)
            {
                for (nat_t i = 0; i + 1 < verts.size(); ++i)
                {
                    result.append(Segment(verts[i], verts[i + 1]));
                }
            }

            return result;
        }

        /** Every distinct vertex (line endpoint or crossing point) in the
            arrangement. */
        DynArray<Point2D> vertices() const
        {
            DynArray<Point2D> result;

            for (const DynArray<Point2D>& verts : vertices_per_line)
            {
                for (const Point2D& p : verts)
                {
                    bool duplicate = false;

                    for (const Point2D& q : result)
                    {
                        if (q.distance_with(p) < EPSILON)
                        {
                            duplicate = true;
                            break;
                        }
                    }

                    if (!duplicate)
                    {
                        result.append(p);
                    }
                }
            }

            return result;
        }

        /** Euler's-formula upper bound on the number of regions the
            arrangement's lines cut the bounding box into (see the class
            comment for when this is exact rather than just an upper
            bound). */
        nat_t num_faces_upper_bound() const
        {
            nat_t n = lines.size();
            return 1 + n + (n * (n > 0 ? n - 1 : 0)) / 2;
        }
    };

} // end namespace Designar
