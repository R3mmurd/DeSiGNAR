/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <atomic>
#include <thread>
#include <vector>
#include <concurrentgraph.hpp>
#include <graphagent.hpp>

using namespace Designar;
using namespace std;

int main()
{
  using GT = Graph<int_t>;

  // Concurrent insert_node()/insert_arc(): several threads each build a
  // small star centered on their own node, all against the same shared
  // ConcurrentGraph; the final node/arc counts must reflect every
  // insertion with none lost or duplicated to a data race.
  {
    ConcurrentGraph<GT> graph;

    constexpr nat_t num_threads = 8;
    constexpr nat_t nodes_per_thread = 50;

    vector<thread> workers;

    for (nat_t t = 0; t < num_threads; ++t)
    {
      workers.emplace_back([&graph, t]
                           {
                             Node<GT>* center = graph.insert_node(int_t(t));

                             for (nat_t i = 0; i < nodes_per_thread; ++i)
                             {
                               Node<GT>* leaf = graph.insert_node(int_t(i));
                               graph.insert_arc(center, leaf);
                             }
                           });
    }

    for (thread& w : workers)
    {
      w.join();
    }

    assert(graph.num_nodes() == num_threads * (nodes_per_thread + 1));
    assert(graph.num_arcs() == num_threads * nodes_per_thread);

    cout << "ConcurrentGraph: concurrent insert Everything ok!\n";
  }

  // GraphAgent::parallel_for_each_node(): sum every node's info via
  // an atomic accumulator, cross-checked against a plain sequential sum.
  {
    ConcurrentGraph<GT> graph;

    constexpr nat_t num_nodes = 500;
    int_t expected_sum = 0;

    for (nat_t i = 0; i < num_nodes; ++i)
    {
      graph.insert_node(int_t(i));
      expected_sum += int_t(i);
    }

    GraphAgent<GT> agent(graph, 4);
    atomic<int_t> total{0};

    agent.parallel_for_each_node([&total](Node<GT>* node)
                                 { total += node->get_info(); });

    assert(total.load() == expected_sum);

    cout << "GraphAgent: parallel_for_each_node Everything ok!\n";
  }

  // GraphAgent::parallel_for_each_arc(): count arcs concurrently.
  {
    ConcurrentGraph<GT> graph;

    Node<GT>* hub = graph.insert_node(0);

    constexpr nat_t num_arcs = 200;

    for (nat_t i = 0; i < num_arcs; ++i)
    {
      Node<GT>* leaf = graph.insert_node(int_t(i));
      graph.insert_arc(hub, leaf);
    }

    GraphAgent<GT> agent(graph, 4);
    atomic<nat_t> visited{0};

    agent.parallel_for_each_arc([&visited](Arc<GT>*)
                                { ++visited; });

    assert(visited.load() == num_arcs);

    cout << "GraphAgent: parallel_for_each_arc Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
