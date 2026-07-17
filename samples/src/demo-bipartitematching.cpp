/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <bipartitematching.hpp>

using namespace Designar;

using GT = Graph<int_t, int_t>;

int main()
{
  // Bipartite graph: L1-R1, L1-R2, L2-R1, L3-R2, L3-R3 -> a perfect
  // matching on the left side exists (size 3).
  GT g;
  Node<GT>* l1 = g.insert_node(1);
  Node<GT>* l2 = g.insert_node(2);
  Node<GT>* l3 = g.insert_node(3);
  Node<GT>* r1 = g.insert_node(11);
  Node<GT>* r2 = g.insert_node(12);
  Node<GT>* r3 = g.insert_node(13);

  g.insert_arc(l1, r1, 0);
  g.insert_arc(l1, r2, 0);
  g.insert_arc(l2, r1, 0);
  g.insert_arc(l3, r2, 0);
  g.insert_arc(l3, r3, 0);

  HashSet<Node<GT>*> left = {l1, l2, l3};

  BipartiteMatching<GT> bm(g, left);
  auto matching = bm.compute();

  for (Node<GT>* l : left)
  {
    Node<GT>** r = matching.search(l);

    if (r != nullptr)
    {
      cout << l->get_info() << " -- " << (*r)->get_info() << endl;
    }
  }

  return 0;
}
