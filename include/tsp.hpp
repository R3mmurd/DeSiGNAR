/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file tsp.hpp
    @brief Two classic heuristics for the (NP-hard) traveling salesman
    problem, both over a fixed set of `n` components indexed `0..n-1`
    and a `DistanceFn: real_t operator()(nat_t i, nat_t j)` — the same
    plain-index-plus-distance-functor convention antcolony.hpp already
    uses for TSP, rather than requiring an actual complete Graph object:
    `tsp_nearest_neighbor` (always-greedy construction, no
    approximation guarantee) and `tsp_mst_approximation` (a 2-
    approximation for metric instances, via a minimum spanning tree).
    @ingroup Algorithms
*/

#pragma once

#include <limits>
#include <stdexcept>

#include <array.hpp>
#include <stack.hpp>

namespace Designar
{
    /** Builds a tour by always moving to the nearest not-yet-visited
        component — simple and fast, but with no worst-case
        approximation guarantee at all (unlike tsp_mst_approximation()
        below); included specifically for that contrast, the same
        "compare an unguaranteed greedy heuristic against a proven-
        bound one" pairing knapsack_01/greedy fractional knapsack
        (dynamicprogramming.hpp) also demonstrates. */
    template <class DistanceFn>
    DynArray<nat_t> tsp_nearest_neighbor(nat_t n, DistanceFn&& distance,
                                        nat_t start = 0)
    {
        if (n == 0)
        {
            throw std::domain_error(
                "tsp_nearest_neighbor: need at least one component");
        }

        DynArray<bool> visited(n, false);
        DynArray<nat_t> tour;
        tour.append(start);
        visited[start] = true;
        nat_t current = start;

        for (nat_t step = 1; step < n; ++step)
        {
            nat_t best = n;
            real_t best_dist = std::numeric_limits<real_t>::max();

            for (nat_t j = 0; j < n; ++j)
            {
                if (visited[j])
                {
                    continue;
                }

                real_t d = distance(current, j);

                if (d < best_dist)
                {
                    best_dist = d;
                    best = j;
                }
            }

            tour.append(best);
            visited[best] = true;
            current = best;
        }

        return tour;
    }

    /** The total cost of a closed tour (the edge from the last
        component back to the first is included) — shared by both
        heuristics here for comparing their results. */
    template <class DistanceFn>
    real_t tsp_tour_cost(const DynArray<nat_t>& tour, DistanceFn&& distance)
    {
        if (tour.size() < 2)
        {
            return real_t(0);
        }

        real_t total = real_t(0);

        for (nat_t i = 0; i + 1 < tour.size(); ++i)
        {
            total += distance(tour[i], tour[i + 1]);
        }

        total += distance(tour[tour.size() - 1], tour[0]);
        return total;
    }

    /** A 2-approximation for *metric* TSP (the distance function must
        satisfy the triangle inequality — Euclidean distance, for
        instance): build a minimum spanning tree (Prim's algorithm,
        directly over the complete graph implied by `distance`, rather
        than via this library's existing Graph-based Kruskal/Prim —
        there is no actual Graph object here to hand those classes, only
        a distance functor over plain indices), then walk it in preorder
        or (equivalently to the textbook "double every MST edge, then
        shortcut past repeated vertices" construction, without actually
        building the doubled multigraph at all) — the triangle
        inequality is exactly what guarantees a preorder walk's
        shortcuts never make the tour longer than the doubled-tree
        route would have been, which is what bounds the result at twice
        the MST's weight, and the MST's weight is itself a lower bound
        on the optimal tour's cost (removing one edge from the optimal
        tour already gives a spanning tree). */
    template <class DistanceFn>
    DynArray<nat_t> tsp_mst_approximation(nat_t n, DistanceFn&& distance)
    {
        if (n == 0)
        {
            throw std::domain_error(
                "tsp_mst_approximation: need at least one component");
        }

        if (n == 1)
        {
            return DynArray<nat_t>({nat_t(0)});
        }

        DynArray<bool> in_mst(n, false);
        DynArray<real_t> key(n, std::numeric_limits<real_t>::max());
        DynArray<nat_t> parent(n, n); // n is the "no parent" sentinel
        key[0] = real_t(0);

        for (nat_t iter = 0; iter < n; ++iter)
        {
            nat_t u = n;
            real_t best = std::numeric_limits<real_t>::max();

            for (nat_t i = 0; i < n; ++i)
            {
                if (!in_mst[i] && key[i] < best)
                {
                    best = key[i];
                    u = i;
                }
            }

            in_mst[u] = true;

            for (nat_t v = 0; v < n; ++v)
            {
                if (!in_mst[v])
                {
                    real_t d = distance(u, v);

                    if (d < key[v])
                    {
                        key[v] = d;
                        parent[v] = u;
                    }
                }
            }
        }

        DynArray<DynArray<nat_t>> children(n);

        for (nat_t i = 0; i < n; ++i)
        {
            children.append(DynArray<nat_t>());
        }

        for (nat_t v = 0; v < n; ++v)
        {
            if (parent[v] != n)
            {
                children[parent[v]].append(v);
            }
        }

        DynArray<nat_t> tour;
        DynArray<bool> visited(n, false);
        DynStack<nat_t> pending;
        pending.push(0);

        while (!pending.is_empty())
        {
            nat_t u = pending.pop();

            if (visited[u])
            {
                continue;
            }

            visited[u] = true;
            tour.append(u);

            for (nat_t i = children[u].size(); i-- > 0;)
            {
                pending.push(children[u][i]);
            }
        }

        return tour;
    }

} // end namespace Designar
