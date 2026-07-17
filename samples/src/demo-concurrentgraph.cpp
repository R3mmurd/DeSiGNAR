/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <atomic>

using namespace std;

#include <concurrentgraph.hpp>
#include <graphagent.hpp>

using namespace Designar;

using GT = Graph<int_t>;

int main()
{
  ConcurrentGraph<GT> graph;

  Node<GT>* hub = graph.insert_node(0);

  for (int_t i = 1; i <= 100; ++i)
  {
    Node<GT>* leaf = graph.insert_node(i);
    graph.insert_arc(hub, leaf);
  }

  cout << "nodes: " << graph.num_nodes() << ", arcs: " << graph.num_arcs() << endl;

  // GraphAgent: apply an operation to every node in parallel, across a
  // pool of worker threads.
  GraphAgent<GT> agent(graph, 4);
  atomic<int_t> total{0};

  agent.parallel_for_each_node([&total](Node<GT>* node)
                               { total += node->get_info(); });

  cout << "sum of every node's info, computed in parallel: " << total.load() << endl;

  return 0;
}
