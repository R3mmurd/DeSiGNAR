/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file maxflow.hpp
    @brief EdmondsKarp: maximum flow via BFS-augmenting paths on an
    explicit residual graph.
    @ingroup Graphs
*/

#pragma once

#include <graphalgorithms.hpp>

namespace Designar
{
    /** Maximum flow via Edmonds-Karp: repeatedly find an augmenting path
        from `source` to `sink` in the *residual* graph using BFS
        (specifically BFS, not any path — that is what turns plain
        Ford-Fulkerson's "any augmenting path" into a strongly
        polynomial-time algorithm), push as much flow as the bottleneck
        arc on that path allows, and repeat until no augmenting path
        remains.

        The residual graph is built and maintained independently of `GT`
        (as a HashMap of HashMaps keyed by node pointers) rather than
        reusing `GT`'s own arcs, because it needs a reverse residual
        capacity for every arc — including ones that have no arc in the
        opposite direction in the original graph at all — to let flow be
        "undone" through an arc as later augmenting paths route around it. */
    template <class GT, class Capacity = DefaultDistance<GT>>
    class EdmondsKarp
    {
    public:
        using FlowType = typename Capacity::Type;

    private:
        HashMap<Node<GT>*, HashMap<Node<GT>*, FlowType>> residual;
        Capacity capacity;

        DynArray<Node<GT>*> find_augmenting_path(Node<GT>* s, Node<GT>* t)
        {
            HashMap<Node<GT>*, Node<GT>*> parent;
            HashSet<Node<GT>*> visited = {s};
            ListQueue<Node<GT>*> q;
            q.put(s);

            while (!q.is_empty())
            {
                Node<GT>* u = q.get();

                if (u == t)
                    break;

                for (auto& edge : residual.find(u))
                {
                    Node<GT>* v = edge.first;

                    if (edge.second > FlowType(0) &&
                        visited.search(v) == nullptr)
                    {
                        visited.insert(v);
                        parent.insert(v, u);
                        q.put(v);
                    }
                }
            }

            DynArray<Node<GT>*> path;

            if (visited.search(t) == nullptr)
                return path;

            for (Node<GT>* curr = t; curr != s; curr = parent.find(curr))
                path.insert(0, curr);

            path.insert(0, s);
            return path;
        }

    public:
        EdmondsKarp(const GT& g)
        {
            g.for_each_node(
                [&](Node<GT>* p)
                { residual.insert(p, HashMap<Node<GT>*, FlowType>()); });

            g.for_each_arc(
                [&](Arc<GT>* a)
                {
                    Node<GT>* u = a->get_src_node();
                    Node<GT>* v = a->get_tgt_node();
                    FlowType c = capacity(a);

                    FlowType* fwd = residual.find(u).search(v);

                    if (fwd != nullptr)
                        *fwd += c; // parallel arcs accumulate capacity
                    else
                        residual.find(u).insert(v, c);

                    if (residual.find(v).search(u) == nullptr)
                        residual.find(v).insert(u, FlowType(0));
                });
        }

        /** Computes and returns the maximum flow value from `source` to
            `sink`. Can only be called once per instance — it mutates the
            residual graph as it augments, so a second call would compute
            (nonsensically) the max additional flow on top of whatever the
            first call already routed. Construct a fresh EdmondsKarp per
            query if more than one is needed. */
        FlowType max_flow(Node<GT>* source, Node<GT>* sink)
        {
            FlowType total = FlowType(0);

            while (true)
            {
                DynArray<Node<GT>*> path = find_augmenting_path(source, sink);

                if (path.is_empty())
                    break;

                FlowType bottleneck = std::numeric_limits<FlowType>::max();

                for (nat_t i = 0; i + 1 < path.size(); ++i)
                    bottleneck =
                        std::min(bottleneck,
                                 *residual.find(path[i]).search(path[i + 1]));

                for (nat_t i = 0; i + 1 < path.size(); ++i)
                {
                    Node<GT>* u = path[i];
                    Node<GT>* v = path[i + 1];
                    *residual.find(u).search(v) -= bottleneck;
                    *residual.find(v).search(u) += bottleneck;
                }

                total += bottleneck;
            }

            return total;
        }
    };

} // end namespace Designar
