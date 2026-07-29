/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file polygontriangulation.hpp
    @brief Two distinct techniques for triangulating a simple
    counterclockwise polygon (`DynArray<Point2D>`, same convention as
    dynamicprogramming.hpp's optimal_polygon_triangulation()) kept side
    by side for comparison, the same "compare techniques" pattern as
    SLR/LALR/LR(1) or Kosaraju/Tarjan: ear_clipping_triangulation() (the
    simple O(n^2) textbook algorithm) and monotone_triangulation() (the
    O(n log n) route: split into y-monotone pieces via
    polygonpartition.hpp, then triangulate each piece in linear time).
    Also art_gallery_guards(): the classic art-gallery-theorem guard
    placement, via 3-coloring the triangulation's vertices and picking
    the smallest color class.
    @ingroup Geometry
*/

#pragma once

#include <algorithm>

#include <array.hpp>
#include <point2D.hpp>
#include <polygonpartition.hpp>

namespace Designar
{
    /** One triangle of a triangulation, as three indices into the
        original polygon's vertex array. */
    struct PolygonTriangle
    {
        nat_t a;
        nat_t b;
        nat_t c;
    };

    namespace detail_polygon
    {
        inline bool point_in_triangle_strict(const Point2D& p,
                                             const Point2D& a,
                                             const Point2D& b,
                                             const Point2D& c)
        {
            // Strictly inside (not merely on an edge): p must be
            // strictly to the left of each directed edge a->b->c->a
            // (the triangle is assumed counterclockwise).
            return p.is_to_left_from(a, b) && p.is_to_left_from(b, c) &&
                   p.is_to_left_from(c, a);
        }

    } // end namespace detail_polygon

    /** Triangulates a simple counterclockwise polygon by repeatedly
        clipping off "ears" (a vertex whose two neighbors' triangle
        contains no other polygon vertex and is itself convex) — the
        classic O(n^2) algorithm: each of the up to n iterations scans
        the remaining vertices twice, once to find a convex vertex, once
        to check no other vertex has slipped inside its ear triangle. */
    inline DynArray<PolygonTriangle>
    ear_clipping_triangulation(const DynArray<Point2D>& poly)
    {
        using namespace detail_polygon;

        require_simple_ccw(poly);

        nat_t n = poly.size();
        DynArray<nat_t> remaining;

        for (nat_t i = 0; i < n; ++i)
        {
            remaining.append(i);
        }

        DynArray<PolygonTriangle> triangles;

        while (remaining.size() > 3)
        {
            nat_t m = remaining.size();
            bool clipped = false;

            for (nat_t i = 0; i < m && !clipped; ++i)
            {
                nat_t ip = (i + m - 1) % m;
                nat_t in = (i + 1) % m;

                nat_t a = remaining[ip];
                nat_t b = remaining[i];
                nat_t c = remaining[in];

                const Point2D& pa = poly[a];
                const Point2D& pb = poly[b];
                const Point2D& pc = poly[c];

                if (!pc.is_to_left_from(pa, pb))
                {
                    continue; // reflex vertex, not an ear
                }

                bool any_inside = false;

                for (nat_t k = 0; k < m && !any_inside; ++k)
                {
                    if (k == ip || k == i || k == in)
                    {
                        continue;
                    }

                    if (point_in_triangle_strict(poly[remaining[k]], pa, pb,
                                                 pc))
                    {
                        any_inside = true;
                    }
                }

                if (any_inside)
                {
                    continue;
                }

                triangles.append(PolygonTriangle{a, b, c});
                remaining.remove_pos_closing_breach(i);
                clipped = true;
            }

            if (!clipped)
            {
                throw std::logic_error(
                    "ear_clipping_triangulation: no ear found; the input "
                    "polygon is likely not simple");
            }
        }

        triangles.append(
            PolygonTriangle{remaining[0], remaining[1], remaining[2]});

        return triangles;
    }

    /** Triangulates a single y-monotone polygon piece (as produced by
        polygonpartition.hpp's monotone_partition()) in linear time via
        the standard stack-based sweep: walk vertices top to bottom,
        merging the two boundary chains, connecting the current vertex
        to every stacked vertex that forms a valid (interior) diagonal,
        then pushing back whichever remain. */
    inline DynArray<PolygonTriangle>
    triangulate_monotone_piece(const DynArray<Point2D>& poly,
                              const DynArray<nat_t>& piece)
    {
        using namespace detail_polygon;

        nat_t m = piece.size();

        if (m < 3)
        {
            return DynArray<PolygonTriangle>();
        }

        DynArray<nat_t> order;

        for (nat_t i = 0; i < m; ++i)
        {
            order.append(i);
        }

        std::sort(order.begin(), order.end(),
                  [&](nat_t a, nat_t b)
                  { return higher(poly[piece[a]], poly[piece[b]]); });

        nat_t top = order[0];
        nat_t bottom = order[m - 1];

        // side[i] is true if piece[i] is on the chain that runs from top
        // to bottom in increasing index order (the "left" chain when
        // walking the polygon counterclockwise from its topmost vertex).
        DynArray<bool> on_left_chain(m, false);
        nat_t i = top;

        while (i != bottom)
        {
            on_left_chain[i] = true;
            i = (i + 1) % m;
        }

        DynArray<PolygonTriangle> triangles;
        DynArray<nat_t> stack;
        stack.append(order[0]);
        stack.append(order[1]);

        for (nat_t idx = 2; idx < m; ++idx)
        {
            nat_t v = order[idx];
            nat_t stack_top = stack[stack.size() - 1];

            if (on_left_chain[v] != on_left_chain[stack_top])
            {
                // v is on the opposite chain from the whole stack: every
                // consecutive pair on the stack forms a valid diagonal
                // with v.
                for (nat_t k = 0; k + 1 < stack.size(); ++k)
                {
                    nat_t p1 = stack[k];
                    nat_t p2 = stack[k + 1];

                    if (on_left_chain[v])
                    {
                        triangles.append(
                            PolygonTriangle{piece[p2], piece[p1], piece[v]});
                    }
                    else
                    {
                        triangles.append(
                            PolygonTriangle{piece[p1], piece[p2], piece[v]});
                    }
                }

                nat_t last = stack[stack.size() - 1];
                stack.clear();
                stack.append(last);
                stack.append(v);
            }
            else
            {
                nat_t last_popped = stack[stack.size() - 1];
                stack.remove_pos_closing_breach(stack.size() - 1);

                while (stack.size() > 0)
                {
                    nat_t candidate = stack[stack.size() - 1];
                    const Point2D& pv = poly[piece[v]];
                    const Point2D& pc = poly[piece[candidate]];
                    const Point2D& pl = poly[piece[last_popped]];

                    bool diagonal_inside =
                        on_left_chain[v] ? pv.is_to_left_from(pc, pl)
                                        : pv.is_to_right_from(pc, pl);

                    if (!diagonal_inside)
                    {
                        break;
                    }

                    if (on_left_chain[v])
                    {
                        triangles.append(PolygonTriangle{
                            piece[candidate], piece[last_popped], piece[v]});
                    }
                    else
                    {
                        triangles.append(PolygonTriangle{
                            piece[last_popped], piece[candidate], piece[v]});
                    }

                    last_popped = candidate;
                    stack.remove_pos_closing_breach(stack.size() - 1);
                }

                stack.append(last_popped);
                stack.append(v);
            }
        }

        return triangles;
    }

    /** Triangulates a simple counterclockwise polygon via the O(n log n)
        route: split into y-monotone pieces (polygonpartition.hpp's
        monotone_partition(), itself O(n^2) here for the same status-
        structure reason explained there) and triangulate each piece in
        linear time. */
    inline DynArray<PolygonTriangle>
    monotone_triangulation(const DynArray<Point2D>& poly)
    {
        DynArray<PolygonTriangle> triangles;

        for (const DynArray<nat_t>& piece : monotone_partition(poly))
        {
            for (const PolygonTriangle& t : triangulate_monotone_piece(poly, piece))
            {
                triangles.append(t);
            }
        }

        return triangles;
    }

    /** The art-gallery theorem's classic guard-placement bound: 3-color
        the triangulation's vertices (every triangle, having one vertex
        of each color, guarantees this is always possible for a
        triangulated simple polygon) and place a guard at every vertex of
        whichever color class is smallest — since the three classes
        partition all n vertices, the smallest has at most floor(n/3)
        vertices, guards there see every triangle (and hence the whole
        polygon). Returns the chosen guards as indices into `poly`. */
    inline DynArray<nat_t> art_gallery_guards(const DynArray<Point2D>& poly)
    {
        nat_t n = poly.size();
        DynArray<PolygonTriangle> triangles = ear_clipping_triangulation(poly);
        nat_t t_count = triangles.size();

        DynArray<int_t> color(n, -1);

        if (t_count == 0)
        {
            DynArray<nat_t> guards;

            if (n > 0)
            {
                guards.append(0);
            }

            return guards;
        }

        // The dual graph of a triangulated simple polygon (one node per
        // triangle, an edge between two triangles that share a diagonal
        // or polygon edge) is always a tree, so a BFS from any triangle,
        // coloring each newly-reached triangle's one new vertex with
        // whichever of the 3 colors its shared edge's two (already
        // colored) vertices don't use, produces a valid 3-coloring in
        // one pass — unlike propagating in ear-removal order (which
        // isn't a traversal of that tree and can reach a triangle with
        // 0 or 1 colored vertices instead of the 2 this relies on).
        auto shared_edge = [](const PolygonTriangle& t1,
                              const PolygonTriangle& t2, nat_t& u, nat_t& v)
        {
            nat_t v1[3] = {t1.a, t1.b, t1.c};
            nat_t v2[3] = {t2.a, t2.b, t2.c};
            nat_t found[2];
            nat_t count = 0;

            for (nat_t x : v1)
            {
                for (nat_t y : v2)
                {
                    if (x == y && count < 2)
                    {
                        found[count] = x;
                        ++count;
                    }
                }
            }

            if (count == 2)
            {
                u = found[0];
                v = found[1];
                return true;
            }

            return false;
        };

        DynArray<bool> tri_done(t_count, false);
        DynArray<nat_t> queue;

        color[triangles[0].a] = 0;
        color[triangles[0].b] = 1;
        color[triangles[0].c] = 2;
        tri_done[0] = true;
        queue.append(0);

        for (nat_t qi = 0; qi < queue.size(); ++qi)
        {
            nat_t cur = queue[qi];

            for (nat_t j = 0; j < t_count; ++j)
            {
                if (tri_done[j])
                {
                    continue;
                }

                nat_t su = 0;
                nat_t sv = 0;

                if (!shared_edge(triangles[cur], triangles[j], su, sv))
                {
                    continue;
                }

                nat_t verts[3] = {triangles[j].a, triangles[j].b,
                                  triangles[j].c};
                nat_t third = su;

                for (nat_t v : verts)
                {
                    if (v != su && v != sv)
                    {
                        third = v;
                    }
                }

                bool used[3] = {false, false, false};
                used[color[su]] = true;
                used[color[sv]] = true;

                for (int_t c = 0; c < 3; ++c)
                {
                    if (!used[c])
                    {
                        color[third] = c;
                        break;
                    }
                }

                tri_done[j] = true;
                queue.append(j);
            }
        }

        nat_t class_size[3] = {0, 0, 0};

        for (nat_t i = 0; i < n; ++i)
        {
            ++class_size[color[i]];
        }

        nat_t smallest = 0;

        for (nat_t c = 1; c < 3; ++c)
        {
            if (class_size[c] < class_size[smallest])
            {
                smallest = c;
            }
        }

        DynArray<nat_t> guards;

        for (nat_t i = 0; i < n; ++i)
        {
            if (color[i] == static_cast<int_t>(smallest))
            {
                guards.append(i);
            }
        }

        return guards;
    }

} // end namespace Designar
