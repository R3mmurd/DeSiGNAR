/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file polygonpartition.hpp
    @brief Splitting a simple polygon (given as a counterclockwise-ordered
    `DynArray<Point2D>`, following the same convention already used by
    dynamicprogramming.hpp's optimal_polygon_triangulation()) into simpler
    pieces: monotone_partition() (the plane-sweep step behind the classic
    O(n log n) polygon triangulation algorithm), trapezoidalize() (a
    randomized-incremental trapezoidal decomposition), and
    convex_partition() (a simple greedy merge of ear-clipping triangles
    into maximal convex pieces).
    @ingroup Geometry
*/

#pragma once

#include <algorithm>
#include <utility>

#include <array.hpp>
#include <point2D.hpp>
#include <segment.hpp>
#include <random.hpp>

namespace Designar
{
    namespace detail_polygon
    {
        inline nat_t nxt(nat_t i, nat_t n)
        {
            return (i + 1) % n;
        }

        inline nat_t prv(nat_t i, nat_t n)
        {
            return (i + n - 1) % n;
        }

        /** Twice the polygon's signed area (shoelace formula) — positive
            for counterclockwise vertex order, negative for clockwise. */
        inline real_t polygon_signed_area2(const DynArray<Point2D>& poly)
        {
            real_t a = 0.0;
            nat_t n = poly.size();

            for (nat_t i = 0; i < n; ++i)
            {
                const Point2D& p = poly[i];
                const Point2D& q = poly[nxt(i, n)];
                a += p.get_x() * q.get_y() - q.get_x() * p.get_y();
            }

            return a;
        }

        inline void require_simple_ccw(const DynArray<Point2D>& poly)
        {
            if (poly.size() < 3)
            {
                throw std::domain_error(
                    "polygon must have at least 3 vertices");
            }

            if (!(polygon_signed_area2(poly) > 0.0))
            {
                throw std::domain_error(
                    "polygon must be given in counterclockwise order");
            }
        }

        /** Sweep order used by the monotone-partition algorithm below: a
            point is "higher" than another if its y-coordinate is larger,
            or, on a tie, if its x-coordinate is smaller — the standard
            tie-break (de Berg et al.) that lets the sweep treat every
            vertex as having a well-defined total order even when several
            share a y-coordinate. */
        inline bool higher(const Point2D& a, const Point2D& b)
        {
            if (a.get_y() > b.get_y())
            {
                return true;
            }

            if (b.get_y() > a.get_y())
            {
                return false;
            }

            return a.get_x() < b.get_x();
        }

    } // end namespace detail_polygon

    enum class VertexType
    {
        START,
        END,
        SPLIT,
        MERGE,
        REGULAR
    };

    /** Classifies vertex `i` of a counterclockwise simple polygon for the
        plane-sweep monotone-partition algorithm: START/SPLIT vertices are
        local maxima (both neighbors below), END/MERGE are local minima
        (both neighbors above) — convex ones are START/END, reflex ones
        are SPLIT/MERGE — and everything else is REGULAR. */
    inline VertexType classify_vertex(const DynArray<Point2D>& poly, nat_t i)
    {
        using namespace detail_polygon;

        nat_t n = poly.size();
        const Point2D& prev = poly[prv(i, n)];
        const Point2D& cur = poly[i];
        const Point2D& next = poly[nxt(i, n)];

        bool prev_below = higher(cur, prev);
        bool next_below = higher(cur, next);
        bool convex = next.is_to_left_from(prev, cur);

        if (prev_below && next_below)
        {
            return convex ? VertexType::START : VertexType::SPLIT;
        }

        if (!prev_below && !next_below)
        {
            return convex ? VertexType::END : VertexType::MERGE;
        }

        return VertexType::REGULAR;
    }

    /** Computes the set of diagonals (pairs of vertex indices into `poly`)
        that split a simple counterclockwise polygon into y-monotone
        pieces, via the classic plane-sweep algorithm: sweep top to
        bottom, track the edges currently crossing the sweep line (each
        remembering a "helper" vertex), and add a diagonal whenever a
        SPLIT or MERGE vertex is processed. The status structure is a
        plain linear scan rather than a balanced search tree — O(n^2)
        instead of the textbook O(n log n), the same "teaching-scale
        simplification" tradeoff already made for tsp_mst_approximation()
        (tsp.hpp)'s O(V^2) Prim step. */
    inline DynArray<std::pair<nat_t, nat_t>>
    monotone_partition_diagonals(const DynArray<Point2D>& poly)
    {
        using namespace detail_polygon;

        require_simple_ccw(poly);

        nat_t n = poly.size();

        struct StatusEdge
        {
            nat_t edge_id; // connects poly[edge_id] and poly[next(edge_id)]
            nat_t helper;
        };

        DynArray<StatusEdge> status;

        auto edge_upper = [&](nat_t edge_id)
        {
            nat_t a = edge_id;
            nat_t b = nxt(edge_id, n);
            return higher(poly[a], poly[b]) ? a : b;
        };

        auto edge_lower = [&](nat_t edge_id)
        {
            nat_t a = edge_id;
            nat_t b = nxt(edge_id, n);
            return higher(poly[a], poly[b]) ? b : a;
        };

        auto edge_x_at_y = [&](nat_t edge_id, real_t y)
        {
            nat_t u = edge_upper(edge_id);
            nat_t l = edge_lower(edge_id);
            const Point2D& pu = poly[u];
            const Point2D& pl = poly[l];
            real_t dy = pu.get_y() - pl.get_y();

            if (!(dy > EPSILON))
            {
                return pu.get_x();
            }

            real_t t = (pu.get_y() - y) / dy;
            return pu.get_x() + t * (pl.get_x() - pu.get_x());
        };

        auto find_left_of = [&](const Point2D& v) -> nat_t
        {
            nat_t best = status.size();
            real_t best_x = -INF;

            for (nat_t k = 0; k < status.size(); ++k)
            {
                real_t x = edge_x_at_y(status[k].edge_id, v.get_y());

                if (x < v.get_x() && x > best_x)
                {
                    best_x = x;
                    best = k;
                }
            }

            return best;
        };

        auto remove_edge = [&](nat_t edge_id)
        {
            for (nat_t k = 0; k < status.size(); ++k)
            {
                if (status[k].edge_id == edge_id)
                {
                    status.remove_pos_closing_breach(k);
                    return;
                }
            }
        };

        DynArray<nat_t> order;

        for (nat_t i = 0; i < n; ++i)
        {
            order.append(i);
        }

        std::sort(order.begin(), order.end(),
                  [&](nat_t a, nat_t b) { return higher(poly[a], poly[b]); });

        DynArray<VertexType> type(n, VertexType::REGULAR);

        for (nat_t i = 0; i < n; ++i)
        {
            type[i] = classify_vertex(poly, i);
        }

        DynArray<std::pair<nat_t, nat_t>> diagonals;

        for (nat_t idx = 0; idx < n; ++idx)
        {
            nat_t i = order[idx];
            nat_t prev_edge = prv(i, n); // edge (prev(i), i)
            nat_t next_edge = i;         // edge (i, next(i))

            switch (type[i])
            {
            case VertexType::START:
            {
                StatusEdge e{next_edge, i};
                status.append(e);
                break;
            }
            case VertexType::END:
            {
                for (nat_t k = 0; k < status.size(); ++k)
                {
                    if (status[k].edge_id == prev_edge)
                    {
                        if (type[status[k].helper] == VertexType::MERGE)
                        {
                            diagonals.append(std::make_pair(i, status[k].helper));
                        }

                        break;
                    }
                }

                remove_edge(prev_edge);
                break;
            }
            case VertexType::SPLIT:
            {
                nat_t left = find_left_of(poly[i]);

                if (left < status.size())
                {
                    diagonals.append(std::make_pair(i, status[left].helper));
                    status[left].helper = i;
                }

                StatusEdge e{next_edge, i};
                status.append(e);
                break;
            }
            case VertexType::MERGE:
            {
                for (nat_t k = 0; k < status.size(); ++k)
                {
                    if (status[k].edge_id == prev_edge)
                    {
                        if (type[status[k].helper] == VertexType::MERGE)
                        {
                            diagonals.append(std::make_pair(i, status[k].helper));
                        }

                        break;
                    }
                }

                remove_edge(prev_edge);

                nat_t left = find_left_of(poly[i]);

                if (left < status.size())
                {
                    if (type[status[left].helper] == VertexType::MERGE)
                    {
                        diagonals.append(std::make_pair(i, status[left].helper));
                    }

                    status[left].helper = i;
                }

                break;
            }
            case VertexType::REGULAR:
            {
                const Point2D& prev_p = poly[prv(i, n)];

                if (higher(prev_p, poly[i]))
                {
                    // Interior is to the right of i: prev(i)->i is
                    // ending, i->next(i) starts.
                    for (nat_t k = 0; k < status.size(); ++k)
                    {
                        if (status[k].edge_id == prev_edge)
                        {
                            if (type[status[k].helper] == VertexType::MERGE)
                            {
                                diagonals.append(
                                    std::make_pair(i, status[k].helper));
                            }

                            break;
                        }
                    }

                    remove_edge(prev_edge);

                    StatusEdge e{next_edge, i};
                    status.append(e);
                }
                else
                {
                    // Interior is to the left of i.
                    nat_t left = find_left_of(poly[i]);

                    if (left < status.size())
                    {
                        if (type[status[left].helper] == VertexType::MERGE)
                        {
                            diagonals.append(
                                std::make_pair(i, status[left].helper));
                        }

                        status[left].helper = i;
                    }
                }

                break;
            }
            }
        }

        return diagonals;
    }

    /** Splits a simple counterclockwise polygon into y-monotone pieces by
        computing monotone_partition_diagonals() and then walking the
        resulting planar subdivision (polygon edges plus diagonals) to
        extract its bounded faces: sort each vertex's incident edges by
        angle, then trace each face by always taking, at the vertex just
        reached, the next edge clockwise from the one just arrived on —
        the standard technique for reading faces back out of a planar
        straight-line graph. Each returned piece is a `DynArray<nat_t>` of
        indices into the original `poly`. */
    inline DynArray<DynArray<nat_t>>
    monotone_partition(const DynArray<Point2D>& poly)
    {
        using namespace detail_polygon;

        nat_t n = poly.size();
        DynArray<std::pair<nat_t, nat_t>> diagonals =
            monotone_partition_diagonals(poly);

        // Build, for every vertex, the list of neighbors reachable via a
        // polygon edge or a diagonal, sorted by angle around that vertex
        // (so "next clockwise from the incoming edge" is a simple index
        // step).
        DynArray<DynArray<nat_t>> neighbors(n, DynArray<nat_t>());

        for (nat_t i = 0; i < n; ++i)
        {
            neighbors[i].append(nxt(i, n));
            neighbors[nxt(i, n)].append(i);
        }

        for (const auto& d : diagonals)
        {
            neighbors[d.first].append(d.second);
            neighbors[d.second].append(d.first);
        }

        for (nat_t i = 0; i < n; ++i)
        {
            const Point2D& p = poly[i];
            std::sort(neighbors[i].begin(), neighbors[i].end(),
                      [&](nat_t a, nat_t b)
                      {
                          real_t angle_a = std::atan2(
                              poly[a].get_y() - p.get_y(),
                              poly[a].get_x() - p.get_x());
                          real_t angle_b = std::atan2(
                              poly[b].get_y() - p.get_y(),
                              poly[b].get_x() - p.get_x());
                          return angle_a < angle_b;
                      });
        }

        auto next_in_face = [&](nat_t u, nat_t v) -> nat_t
        {
            const DynArray<nat_t>& nb = neighbors[v];
            nat_t pos = 0;

            for (; pos < nb.size(); ++pos)
            {
                if (nb[pos] == u)
                {
                    break;
                }
            }

            nat_t prev_pos = (pos + nb.size() - 1) % nb.size();
            return nb[prev_pos];
        };

        // Marks directed edges (u, v) already traced into some face, flat-
        // indexed as u * n + v; a plain DynArray<bool> avoids pulling in a
        // hash table just for an n*n membership check.
        DynArray<bool> visited(n * n, false);

        DynArray<DynArray<nat_t>> faces;

        for (nat_t u = 0; u < n; ++u)
        {
            for (nat_t v : neighbors[u])
            {
                nat_t code = u * n + v;

                if (visited[code])
                {
                    continue;
                }

                DynArray<nat_t> face;
                nat_t a = u;
                nat_t b = v;

                do
                {
                    visited[a * n + b] = true;
                    face.append(a);
                    nat_t c = next_in_face(a, b);
                    a = b;
                    b = c;
                } while (a != u || b != v);

                if (polygon_signed_area2(
                        [&]
                        {
                            DynArray<Point2D> pts;

                            for (nat_t idx : face)
                            {
                                pts.append(poly[idx]);
                            }

                            return pts;
                        }()) > 0.0)
                {
                    faces.append(face);
                }
            }
        }

        return faces;
    }

    /** Whether the polygon piece described by `idx` (indices into `poly`)
        is y-monotone: walking from its topmost to its bottommost vertex
        along each of its two boundary chains, y must be non-increasing.
        Mainly useful for verifying monotone_partition()'s own output. */
    inline bool is_y_monotone(const DynArray<Point2D>& poly,
                              const DynArray<nat_t>& idx)
    {
        using namespace detail_polygon;

        nat_t n = idx.size();
        nat_t top = 0;
        nat_t bottom = 0;

        for (nat_t i = 1; i < n; ++i)
        {
            if (higher(poly[idx[i]], poly[idx[top]]))
            {
                top = i;
            }

            if (higher(poly[idx[bottom]], poly[idx[i]]))
            {
                bottom = i;
            }
        }

        auto chain_monotone = [&](nat_t from, nat_t to, int step)
        {
            nat_t i = from;

            while (i != to)
            {
                nat_t j = (i + step + n) % n;

                if (poly[idx[j]].get_y() > poly[idx[i]].get_y())
                {
                    return false;
                }

                i = j;
            }

            return true;
        };

        return chain_monotone(top, bottom, 1) && chain_monotone(top, bottom, -1);
    }

    /** A convex partition of a simple counterclockwise polygon, built by
        greedily merging adjacent triangles of an ear-clipping
        triangulation (see polygontriangulation.hpp) whenever the merge
        stays convex — the simplest correct approach (rather than an
        optimal few-pieces decomposition, which needs Steiner points or
        Hertel-Mehlhorn-style bookkeeping this teaching-scale version
        skips). `triangles` is the ear-clipping triangulation's output:
        each entry a 3-vertex-index triple (see ear_clipping_triangulation()). */
    template <class TriangleTriple>
    DynArray<DynArray<nat_t>>
    convex_partition_from_triangulation(const DynArray<Point2D>& poly,
                                        const DynArray<TriangleTriple>& triangles)
    {
        DynArray<DynArray<nat_t>> pieces;

        for (const auto& t : triangles)
        {
            DynArray<nat_t> piece;
            piece.append(t.a);
            piece.append(t.b);
            piece.append(t.c);
            pieces.append(piece);
        }

        auto is_convex_piece = [&](const DynArray<nat_t>& piece)
        {
            nat_t m = piece.size();

            for (nat_t i = 0; i < m; ++i)
            {
                const Point2D& prev = poly[piece[(i + m - 1) % m]];
                const Point2D& cur = poly[piece[i]];
                const Point2D& next = poly[piece[(i + 1) % m]];

                if (!next.is_to_left_from(prev, cur))
                {
                    return false;
                }
            }

            return true;
        };

        bool merged_any = true;

        while (merged_any)
        {
            merged_any = false;

            for (nat_t i = 0; i < pieces.size() && !merged_any; ++i)
            {
                for (nat_t j = i + 1; j < pieces.size() && !merged_any; ++j)
                {
                    // Two pieces sharing exactly one edge (two consecutive
                    // vertices, in opposite order) can potentially merge.
                    const DynArray<nat_t>& a = pieces[i];
                    const DynArray<nat_t>& b = pieces[j];

                    for (nat_t ai = 0; ai < a.size() && !merged_any; ++ai)
                    {
                        nat_t a1 = a[ai];
                        nat_t a2 = a[(ai + 1) % a.size()];

                        for (nat_t bi = 0; bi < b.size() && !merged_any; ++bi)
                        {
                            nat_t b1 = b[bi];
                            nat_t b2 = b[(bi + 1) % b.size()];

                            if (a1 != b2 || a2 != b1)
                            {
                                continue;
                            }

                            DynArray<nat_t> merged;

                            for (nat_t k = (ai + 1) % a.size(); k != ai;
                                 k = (k + 1) % a.size())
                            {
                                merged.append(a[k]);
                            }

                            merged.append(a[ai]);

                            for (nat_t k = (bi + 1) % b.size(); k != bi;
                                 k = (k + 1) % b.size())
                            {
                                if (b[k] != a1 && b[k] != a2)
                                {
                                    merged.append(b[k]);
                                }
                            }

                            if (is_convex_piece(merged))
                            {
                                pieces[i] = merged;
                                pieces.remove_pos_closing_breach(j);
                                merged_any = true;
                            }
                        }
                    }
                }
            }
        }

        return pieces;
    }

    /** A trapezoid produced by trapezoidalize(): the vertical strip
        between `left_x` and `right_x`, bounded above by `top` and below
        by `bottom`. */
    struct Trapezoid
    {
        Segment top;
        Segment bottom;
        real_t left_x;
        real_t right_x;
    };

    /** Randomized-incremental trapezoidalization of a simple
        counterclockwise polygon: for each vertex, in random order, cast
        a vertical line through it and find the nearest other polygon
        edge crossing that line above and below — the textbook expected-
        O(n log n) algorithm, minus the trapezoidal search-structure
        (query is a linear scan of every edge instead, same teaching-
        scale tradeoff as monotone_partition_diagonals()).

        @note Only produces a trapezoid for vertices whose vertical
        extension is bounded on *both* sides by some other edge; a
        vertex where one direction runs off the polygon entirely (e.g.
        any vertex on a convex polygon's upper or lower chain, or a
        reflex notch's vertex looking into the notch itself) is left
        out rather than represented as an unbounded trapezoid — doing
        that properly needs a bounding box, which this teaching-scale
        version skips. */
    inline DynArray<Trapezoid> trapezoidalize(const DynArray<Point2D>& poly,
                                              rng_t& rng)
    {
        using namespace detail_polygon;

        require_simple_ccw(poly);

        nat_t n = poly.size();
        DynArray<nat_t> vertex_order;

        for (nat_t i = 0; i < n; ++i)
        {
            vertex_order.append(i);
        }

        for (nat_t i = vertex_order.size(); i > 1; --i)
        {
            nat_t j = random_uniform(rng, i);
            std::swap(vertex_order[i - 1], vertex_order[j]);
        }

        auto edge_y_at_x = [&](nat_t edge_id, real_t x) -> real_t
        {
            const Point2D& a = poly[edge_id];
            const Point2D& b = poly[nxt(edge_id, n)];
            real_t lo_x = std::min(a.get_x(), b.get_x());
            real_t hi_x = std::max(a.get_x(), b.get_x());

            if (hi_x - lo_x < EPSILON)
            {
                return a.get_y();
            }

            real_t t = (x - a.get_x()) / (b.get_x() - a.get_x());
            return a.get_y() + t * (b.get_y() - a.get_y());
        };

        auto edge_spans_x = [&](nat_t edge_id, real_t x)
        {
            const Point2D& a = poly[edge_id];
            const Point2D& b = poly[nxt(edge_id, n)];
            real_t lo_x = std::min(a.get_x(), b.get_x());
            real_t hi_x = std::max(a.get_x(), b.get_x());
            return lo_x <= x && x <= hi_x;
        };

        DynArray<Trapezoid> result;

        // Every vertex is processed against every OTHER edge (excluding
        // the two edges incident to it) to find the nearest edge above
        // and below at that vertex's x-coordinate; `vertex_order`
        // (randomized) only determines the order results are produced
        // in, matching the incremental algorithm's spirit even though
        // the O(n) linear-scan query below (rather than a real trapezoid
        // search structure) makes the overall cost O(n^2), same as
        // monotone_partition_diagonals()'s status-structure tradeoff.
        for (nat_t vi : vertex_order)
        {
            const Point2D& v = poly[vi];
            nat_t above_edge = n; // sentinel: none found
            real_t above_y = INF;
            nat_t below_edge = n;
            real_t below_y = -INF;
            nat_t incident1 = vi;
            nat_t incident2 = prv(vi, n);

            for (nat_t e = 0; e < n; ++e)
            {
                if (e == incident1 || e == incident2)
                {
                    continue;
                }

                if (!edge_spans_x(e, v.get_x()))
                {
                    continue;
                }

                real_t y = edge_y_at_x(e, v.get_x());

                if (y > v.get_y() && y < above_y)
                {
                    above_y = y;
                    above_edge = e;
                }

                if (y < v.get_y() && y > below_y)
                {
                    below_y = y;
                    below_edge = e;
                }
            }

            if (above_edge < n && below_edge < n)
            {
                Trapezoid t{Segment(poly[above_edge], poly[nxt(above_edge, n)]),
                           Segment(poly[below_edge], poly[nxt(below_edge, n)]),
                           v.get_x(), v.get_x()};
                result.append(t);
            }
        }

        return result;
    }

} // end namespace Designar
