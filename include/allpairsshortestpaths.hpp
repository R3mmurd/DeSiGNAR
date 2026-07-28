/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file allpairsshortestpaths.hpp
    @brief FloydWarshall and Johnson: two all-pairs shortest-path
    algorithms, complementing the single-source algorithms
    (Dijkstra/Astar/BellmanFord) in graphalgorithms.hpp. Both work with
    negative arc weights and detect negative cycles; FloydWarshall's
    simpler O(V^3) triple loop tends to win on dense graphs, while
    Johnson's Bellman-Ford-reweight-then-per-vertex-Dijkstra approach
    (O(VE + V^2 lg V) with a binary-heap Dijkstra) tends to win on
    sparse ones — the same "compare techniques on the same problem"
    pairing this library already uses elsewhere (SLR/LALR/LR(1),
    Kosaraju/Tarjan).
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

    /** All-pairs shortest paths via reweighting: first, Bellman-Ford
        computes a potential `h(v)` for every vertex (simulated without
        actually adding a virtual source node — initializing every
        vertex's Bellman-Ford distance to `Distance::ZERO` instead of
        `Distance::MAX` has the same effect as a zero-weight edge from
        an implicit source to every vertex, without needing to build or
        tear down that extra node), detecting a negative cycle along the
        way exactly like BellmanFord (graphalgorithms.hpp) does. Every
        arc is then reweighted as `w(u, v) + h(u) - h(v)`, which is
        always non-negative when there is no negative cycle — the
        textbook fact that makes running a plain (Dijkstra-style, no-
        negative-weights) shortest-path search from every vertex on the
        reweighted graph valid, after which each result is converted
        back via `d(u, v) = d'(u, v) - h(u) + h(v)`. Deliberately not
        built by templating over the existing single-source `Dijkstra`
        class (graphalgorithms.hpp): that class operates on the actual
        graph's arcs, and reweighting only makes sense as a temporary,
        entirely separate view over the same edge set — an O(V^2)
        selection-based Dijkstra written directly against a small
        adjacency list here is simpler and avoids needing to mutate (or
        build a temporary copy of) the real graph's arc weights. */
    template <class GT, class Distance = DefaultDistance<GT>,
              class Plus = std::plus<typename Distance::Type>,
              class Cmp = std::less<typename Distance::Type>>
    class Johnson
    {
    public:
        using DistanceType = typename Distance::Type;

    private:
        struct Edge
        {
            nat_t u;
            nat_t v;
            DistanceType w;
        };

        DynArray<Node<GT>*> node_of;
        MultiDimArray<DistanceType, 2> dist;
        MultiDimArray<int_t, 2> next;
        bool has_negative_cycle_flag;

        Distance distance;
        Plus plus;
        Cmp cmp;

        /** Bellman-Ford over every edge, `rounds` times, relaxing `h`
            in place; returns whether any edge was still relaxable on
            the final (n-th) pass — a negative cycle exists exactly
            when it is. */
        bool relax_potentials(const DynArray<Edge>& edges,
                              DynArray<DistanceType>& h, nat_t rounds)
        {
            for (nat_t iter = 0; iter < rounds; ++iter)
            {
                bool changed = false;

                for (const Edge& e : edges)
                {
                    DistanceType relaxed = plus(h[e.u], e.w);

                    if (cmp(relaxed, h[e.v]))
                    {
                        h[e.v] = relaxed;
                        changed = true;
                    }
                }

                if (!changed)
                {
                    return false;
                }
            }

            return true;
        }

    public:
        /** Runs Johnson's algorithm on `g`; throws if `g` is empty or
            not a digraph. Detects (but does not attempt to route
            around) negative cycles — see has_negative_cycle(); when one
            exists, distance_between()/path_between() are not
            meaningful. */
        Johnson(const GT& g)
            : node_of(g.get_num_nodes()),
              dist(g.get_num_nodes(), g.get_num_nodes()),
              next(g.get_num_nodes(), g.get_num_nodes()),
              has_negative_cycle_flag(false)
        {
            if (!g.is_digraph())
            {
                throw std::domain_error("Johnson: argument must be a "
                                        "directed graph");
            }

            nat_t n = g.get_num_nodes();

            if (n == 0)
            {
                throw std::domain_error("Johnson: graph has no nodes");
            }

            nat_t idx = 0;

            g.for_each_node(
                [&](Node<GT>* p)
                {
                    p->cookie_as_int() = int_t(idx);
                    node_of.append(p);
                    ++idx;
                });

            DynArray<Edge> edges;

            g.for_each_arc(
                [&](Arc<GT>* a)
                {
                    nat_t u = nat_t(a->get_src_node()->cookie_as_int());
                    nat_t v = nat_t(a->get_tgt_node()->cookie_as_int());
                    edges.append(Edge{u, v, distance(a)});
                });

            DynArray<DistanceType> h(n, Distance::ZERO);

            // n - 1 relaxation rounds are enough for any shortest path
            // (no cycle) to have settled; one extra round that still
            // finds a relaxable edge proves a negative cycle exists.
            if (n >= 2)
            {
                relax_potentials(edges, h, n - 1);
            }

            has_negative_cycle_flag = relax_potentials(edges, h, 1);

            if (has_negative_cycle_flag)
            {
                return;
            }

            DynArray<DynArray<std::pair<nat_t, DistanceType>>> adj(n);

            for (nat_t i = 0; i < n; ++i)
            {
                adj.append(DynArray<std::pair<nat_t, DistanceType>>());
            }

            for (const Edge& e : edges)
            {
                DistanceType reweighted = e.w + h[e.u] - h[e.v];
                adj[e.u].append(std::make_pair(e.v, reweighted));
            }

            for (nat_t src = 0; src < n; ++src)
            {
                DynArray<DistanceType> d(n, Distance::MAX);
                DynArray<int_t> nxt(n, int_t(-1));
                DynArray<bool> visited(n, false);
                d[src] = Distance::ZERO;

                for (nat_t iter = 0; iter < n; ++iter)
                {
                    nat_t u = n;
                    DistanceType best = Distance::MAX;
                    bool found = false;

                    for (nat_t i = 0; i < n; ++i)
                    {
                        if (!visited[i] && d[i] != Distance::MAX &&
                            (!found || cmp(d[i], best)))
                        {
                            best = d[i];
                            u = i;
                            found = true;
                        }
                    }

                    if (!found)
                    {
                        break;
                    }

                    visited[u] = true;

                    for (const auto& edge : adj[u])
                    {
                        nat_t v = edge.first;
                        DistanceType relaxed = plus(d[u], edge.second);

                        if (cmp(relaxed, d[v]))
                        {
                            d[v] = relaxed;
                            nxt[v] = (u == src) ? int_t(v) : nxt[u];
                        }
                    }
                }

                for (nat_t v = 0; v < n; ++v)
                {
                    dist(src, v) =
                        (d[v] == Distance::MAX) ? Distance::MAX
                                                : (d[v] - h[src] + h[v]);
                    next(src, v) = nxt[v];
                }
            }
        }

        bool has_negative_cycle() const
        {
            return has_negative_cycle_flag;
        }

        nat_t index_of(Node<GT>* p) const
        {
            return nat_t(p->cookie_as_int());
        }

        const DistanceType& distance_between(Node<GT>* src,
                                             Node<GT>* tgt) const
        {
            return dist(index_of(src), index_of(tgt));
        }

        /** Reconstructs the shortest path from `src` to `tgt` as a
            sequence of nodes (empty if none exists). */
        DynArray<Node<GT>*> path_between(Node<GT>* src, Node<GT>* tgt) const
        {
            DynArray<Node<GT>*> path;

            nat_t i = index_of(src);
            nat_t j = index_of(tgt);

            if (next(i, j) == -1 && i != j)
            {
                return path;
            }

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
