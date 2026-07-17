/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file concurrentgraph.hpp
    @brief ConcurrentGraph: a mutex-guarded facade around any of this
    library's graph types, following the same design ConcurrentMap/
    ConcurrentSet (concurrentmap.hpp) already established.
    @ingroup Concurrency
*/

#pragma once

#include <mutex>

#include <graph.hpp>

namespace Designar
{
    /** Wraps a `GT` (any graph type shaped like Graph<NodeInfo, ArcInfo,
        GraphInfo> from graph.hpp) behind a single mutex, so that
        insert_node()/insert_arc()/remove_node()/remove_arc() and the
        for_each_node()/for_each_arc() traversals can be called from
        multiple threads without racing on the underlying node/arc lists.

        Unlike ConcurrentMap/ConcurrentSet, whose operations return
        *values* to sidestep holding a reference past the lock's lifetime,
        a graph's whole reason for existing is to hand out Node* / Arc*
        pointers into its own internal structure, and there is no useful
        "value" to copy instead (a Node is identity, not data). This
        class does not try to hide that: a Node* / Arc* returned by
        insert_node()/insert_arc()/search_node() remains valid only until
        some other thread removes that same node/arc — exactly as when
        using `GT` directly from a single thread, except now "some other
        thread" is a real possibility the caller must account for (e.g.
        by only ever removing a node/arc from within a for_each_node()/
        for_each_arc() callback of this same ConcurrentGraph, which holds
        the lock for the callback's whole duration, or by layering the
        caller's own higher-level synchronization on top). What this
        class does guarantee is that the graph's own bookkeeping (node
        count, adjacency lists, arc lists) never gets corrupted by two
        threads mutating it at once. */
    template <class GT>
    class ConcurrentGraph
    {
        mutable std::mutex mtx;
        GT graph;

    public:
        using NodeInfoType = typename GT::NodeInfoType;
        using ArcInfoType = typename GT::ArcInfoType;

        ConcurrentGraph() = default;

        ConcurrentGraph(const ConcurrentGraph&) = delete;
        ConcurrentGraph& operator=(const ConcurrentGraph&) = delete;

        Node<GT>* insert_node()
        {
            std::lock_guard<std::mutex> lck(mtx);
            return graph.insert_node();
        }

        Node<GT>* insert_node(const NodeInfoType& info)
        {
            std::lock_guard<std::mutex> lck(mtx);
            return graph.insert_node(info);
        }

        Arc<GT>* insert_arc(Node<GT>* src, Node<GT>* tgt)
        {
            std::lock_guard<std::mutex> lck(mtx);
            return graph.insert_arc(src, tgt);
        }

        Arc<GT>* insert_arc(Node<GT>* src, Node<GT>* tgt,
                            const ArcInfoType& info)
        {
            std::lock_guard<std::mutex> lck(mtx);
            return graph.insert_arc(src, tgt, info);
        }

        void remove_node(Node<GT>* node)
        {
            std::lock_guard<std::mutex> lck(mtx);
            graph.remove_node(node);
        }

        void remove_arc(Arc<GT>* arc)
        {
            std::lock_guard<std::mutex> lck(mtx);
            graph.remove_arc(arc);
        }

        nat_t num_nodes() const
        {
            std::lock_guard<std::mutex> lck(mtx);
            return graph.get_num_nodes();
        }

        nat_t num_arcs() const
        {
            std::lock_guard<std::mutex> lck(mtx);
            return graph.get_num_arcs();
        }

        bool is_digraph() const
        {
            std::lock_guard<std::mutex> lck(mtx);
            return graph.is_digraph();
        }

        /** Runs `op(node)` for every node currently in the graph, holding
            the lock for the whole traversal — the same re-entrancy caveat
            as ConcurrentMap::with_lock() applies: `op` must not call back
            into this ConcurrentGraph (directly or transitively), or it
            will deadlock against `std::mutex` not being recursive. This is
            also the safe place to call remove_node()/remove_arc() on the
            *underlying* `GT` directly (via with_lock()) if a caller needs
            to mutate while traversing. */
        template <class Op>
        void for_each_node(Op&& op) const
        {
            std::lock_guard<std::mutex> lck(mtx);
            graph.for_each_node(std::forward<Op>(op));
        }

        /** @see for_each_node() — same locking and re-entrancy caveat. */
        template <class Op>
        void for_each_arc(Op&& op) const
        {
            std::lock_guard<std::mutex> lck(mtx);
            graph.for_each_arc(std::forward<Op>(op));
        }

        /** Runs `fn(underlying_graph)` while holding the lock — the escape
            hatch for any operation not exposed above (e.g. search_node(),
            or a batch of inserts that must appear atomic to other
            threads). `fn` must not itself try to lock this same
            ConcurrentGraph. */
        template <class Fn>
        decltype(auto) with_lock(Fn&& fn)
        {
            std::lock_guard<std::mutex> lck(mtx);
            return fn(graph);
        }
    };

} // end namespace Designar
