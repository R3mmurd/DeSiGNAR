/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file graphagent.hpp
    @brief GraphAgent: a pool of worker threads that concurrently apply
    a user-supplied operation to every node/arc of a ConcurrentGraph,
    built directly on top of this library's own ThreadPool
    (threadpool.hpp).
    @ingroup Concurrency
*/

#pragma once

#include <concurrentgraph.hpp>
#include <threadpool.hpp>

#include <future>
#include <vector>

namespace Designar
{
  /** An "agent" here is a coordinator, not a single thread: it takes a
      snapshot of a ConcurrentGraph's nodes (or arcs), then hands each
      one to a ThreadPool worker to run the caller's operation against,
      waiting for every task to finish before returning. The snapshot
      is taken with ConcurrentGraph::for_each_node()/for_each_arc()
      (so it reflects a single consistent instant of the graph, not a
      partially-updated view), but the operation itself then runs
      *without* holding the graph's lock — the whole reason to use
      this instead of ConcurrentGraph::for_each_node() directly is to
      let independent, possibly slow per-node/per-arc work (e.g.
      computing some property, or updating a separate concurrent data
      structure keyed by node) actually happen in parallel; `op` is
      responsible for its own synchronization if it touches shared
      state beyond the node/arc it was given. */
  template <class GT>
  class GraphAgent
  {
    ConcurrentGraph<GT>& graph;
    ThreadPool pool;

  public:
    /** `num_workers == 0` defers to ThreadPool's own default (hardware
        concurrency, or 1 if that cannot be determined). */
    explicit GraphAgent(ConcurrentGraph<GT>& g, nat_t num_workers = 0)
        : graph(g), pool(num_workers)
    {
      // empty
    }

    GraphAgent(const GraphAgent&) = delete;
    GraphAgent& operator=(const GraphAgent&) = delete;

    nat_t num_workers() const
    {
      return pool.size();
    }

    /** Applies `op(node)` to every node of the graph, one ThreadPool
        task per node, blocking until every one of them has run. */
    template <class Op>
    void parallel_for_each_node(Op op)
    {
      DynArray<Node<GT>*> nodes;
      graph.for_each_node([&](Node<GT>* node)
                          { nodes.append(node); });

      // std::future is move-only, so it cannot live in a DynArray (whose
      // resize() copy-assigns elements into the new storage); a
      // std::vector, which move-constructs on growth, has no such
      // restriction.
      std::vector<std::future<void>> pending;
      pending.reserve(nodes.size());

      for (nat_t i = 0; i < nodes.size(); ++i)
      {
        Node<GT>* node = nodes[i];
        pending.push_back(pool.submit([node, &op]
                                      { op(node); }));
      }

      for (std::future<void>& task : pending)
      {
        task.get();
      }
    }

    /** @see parallel_for_each_node() — same snapshot-then-dispatch
        approach, over arcs instead of nodes. */
    template <class Op>
    void parallel_for_each_arc(Op op)
    {
      DynArray<Arc<GT>*> arcs;
      graph.for_each_arc([&](Arc<GT>* arc)
                         { arcs.append(arc); });

      std::vector<std::future<void>> pending;
      pending.reserve(arcs.size());

      for (nat_t i = 0; i < arcs.size(); ++i)
      {
        Arc<GT>* arc = arcs[i];
        pending.push_back(pool.submit([arc, &op]
                                      { op(arc); }));
      }

      for (std::future<void>& task : pending)
      {
        task.get();
      }
    }
  };

} // end namespace Designar
