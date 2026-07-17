/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file allpairsshortestpaths.hpp
    @brief FloydWarshall: all-pairs shortest paths, complementing the
    single-source algorithms (Dijkstra/Astar/BellmanFord) in
    graphalgorithms.hpp.
    @ingroup Graphs
*/

#pragma once

#include <graphalgorithms.hpp>

namespace Designar
{
    /** Computes shortest-path distances between *every* pair of nodes at
        once — unlike Dijkstra/Bellman-Ford/A* in graphalgorithms.hpp,
        which each find paths from a single source. Works with negative
        arc weights (unlike Dijkstra), and its O(V^3) triple loop is
        simpler and often faster in practice than running Bellman-Ford
        from every node individually when V is not too large.

        `next(i, j)` records, for the shortest path from node i to node j,
        the index of the first node to move to from i — the standard way
        to reconstruct an actual path from an all-pairs distance matrix
        without storing the paths themselves (which would cost O(V^3)
        space instead of O(V^2)). */
    template <class GT, class Distance = DefaultDistance<GT>,
              class Plus = std::plus<typename Distance::Type>,
              class Cmp = std::less<typename Distance::Type>>
    class FloydWarshall
    {
    public:
        using DistanceType = typename Distance::Type;

    private:
        DynArray<Node<GT>*> node_of;
        MultiDimArray<DistanceType, 2> dist;
        MultiDimArray<int_t, 2> next;
        bool has_negative_cycle_flag;

        Distance distance;
        Plus plus;
        Cmp cmp;

    public:
        /** Runs Floyd-Warshall on `g`; throws if `g` is empty. Detects (but
            does not attempt to route around) negative cycles — see
            has_negative_cycle(). */
        FloydWarshall(const GT& g)
            : node_of(g.get_num_nodes()),
              dist(g.get_num_nodes(), g.get_num_nodes()),
              next(g.get_num_nodes(), g.get_num_nodes()),
              has_negative_cycle_flag(false)
        {
            nat_t n = g.get_num_nodes();

            if (n == 0)
                throw std::domain_error("FloydWarshall: graph has no nodes");

            nat_t idx = 0;
            g.for_each_node(
                [&](Node<GT>* p)
                {
                    p->cookie_as_int() = int_t(idx);
                    node_of.append(p);
                    ++idx;
                });

            for (nat_t i = 0; i < n; ++i)
                for (nat_t j = 0; j < n; ++j)
                {
                    dist(i, j) = i == j ? Distance::ZERO : Distance::MAX;
                    next(i, j) = -1;
                }

            g.for_each_arc(
                [&](Arc<GT>* a)
                {
                    nat_t i = nat_t(a->get_src_node()->cookie_as_int());
                    nat_t j = nat_t(a->get_tgt_node()->cookie_as_int());
                    DistanceType w = distance(a);

                    if (cmp(w, dist(i, j)))
                    {
                        dist(i, j) = w;
                        next(i, j) = int_t(j);
                    }

                    if (!g.is_digraph() && cmp(w, dist(j, i)))
                    {
                        dist(j, i) = w;
                        next(j, i) = int_t(i);
                    }
                });

            for (nat_t k = 0; k < n; ++k)
                for (nat_t i = 0; i < n; ++i)
                {
                    if (dist(i, k) == Distance::MAX)
                        continue;

                    for (nat_t j = 0; j < n; ++j)
                    {
                        if (dist(k, j) == Distance::MAX)
                            continue;

                        DistanceType via = plus(dist(i, k), dist(k, j));

                        if (cmp(via, dist(i, j)))
                        {
                            dist(i, j) = via;
                            next(i, j) = next(i, k);
                        }
                    }
                }

            for (nat_t i = 0; i < n; ++i)
                if (cmp(dist(i, i), Distance::ZERO))
                    has_negative_cycle_flag = true;
        }

        bool has_negative_cycle() const
        {
            return has_negative_cycle_flag;
        }

        nat_t index_of(Node<GT>* p) const
        {
            return nat_t(p->cookie_as_int());
        }

        const DistanceType& distance_between(Node<GT>* src, Node<GT>* tgt) const
        {
            return dist(index_of(src), index_of(tgt));
        }

        /** Reconstructs the shortest path from `src` to `tgt` as a sequence
            of nodes (empty if none exists). */
        DynArray<Node<GT>*> path_between(Node<GT>* src, Node<GT>* tgt) const
        {
            DynArray<Node<GT>*> path;

            nat_t i = index_of(src);
            nat_t j = index_of(tgt);

            if (next(i, j) == -1 && i != j)
                return path;

            path.append(src);

            while (i != j)
            {
                i = nat_t(next(i, j));
                path.append(node_of[i]);
            }

            return path;
        }
    };

} // end namespace Designar
