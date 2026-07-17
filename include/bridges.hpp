/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file bridges.hpp
    @brief Bridges: finds every cut edge of a graph via the
    df()/low() discovery-time bookkeeping already used for cut nodes
    in graphalgorithms.hpp.
    @ingroup Graphs
*/

#pragma once

#include <graphalgorithms.hpp>

namespace Designar
{
    /** Finds every bridge (cut edge) of `g`: an edge whose removal
        disconnects the graph, exactly the edge-analogue of
        compute_cut_nodes()'s cut *nodes* in graphalgorithms.hpp, reusing
        the same df()/low() discovery-time and low-link bookkeeping via a
        DFS. A tree edge (u, v) — v being reached from u for the first
        time — is a bridge precisely when low(v) > df(u) (strictly
        greater, unlike the cut-node condition's `>=`): that means no node
        in v's subtree has a back edge reaching u or anything above it, so
        (u, v) is the *only* connection between v's subtree and the rest
        of the graph. */
    template <class GT>
    class Bridges
    {
        static void dfs(const GT& g, Node<GT>* p, Arc<GT>* parent_arc,
                        int_t& counter, SLList<Arc<GT>*>& bridges)
        {
            p->visit(GraphTag::DEPTH_FIRST);
            low<GT>(p) = df<GT>(p) = counter++;

            for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
            {
                Arc<GT>* arc = it.get_current();

                if (arc == parent_arc)
                    continue;

                Node<GT>* q = it.get_tgt_node();

                if (q->is_visited(GraphTag::DEPTH_FIRST))
                {
                    if (!arc->is_visited(GraphTag::DEPTH_FIRST))
                        low<GT>(p) = std::min(low<GT>(p), df<GT>(q));

                    continue;
                }

                if (arc->is_visited(GraphTag::DEPTH_FIRST))
                    continue;

                arc->visit(GraphTag::DEPTH_FIRST);

                dfs(g, q, arc, counter, bridges);

                low<GT>(p) = std::min(low<GT>(p), low<GT>(q));

                if (low<GT>(q) > df<GT>(p))
                    bridges.append(arc);
            }
        }

    public:
        static SLList<Arc<GT>*> find_bridges(const GT& g)
        {
            g.for_each_node(
                [](Node<GT>* p)
                {
                    p->reset_tag();
                    df<GT>(p) = 0;
                    low<GT>(p) = -1;
                });
            g.reset_arcs();

            int_t counter = 0;
            SLList<Arc<GT>*> bridges;

            g.for_each_node(
                [&](Node<GT>* p)
                {
                    if (!p->is_visited(GraphTag::DEPTH_FIRST))
                        dfs(g, p, nullptr, counter, bridges);
                });

            return bridges;
        }
    };

} // end namespace Designar
