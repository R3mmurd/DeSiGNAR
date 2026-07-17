/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <bridges.hpp>

using namespace Designar;

using GT = Graph<int_t, int_t>;

int main()
{
  // A-B-C, with a B-D-E-B triangle hanging off B, and F hanging off C.
  // Bridges (removing them disconnects the graph): A-B, B-C, C-F.
  // Not bridges: B-D, D-E, E-B (each is part of the D-E-B cycle, so
  // removing just one of them still leaves a path around the other two).
  GT g;
  Node<GT>* a = g.insert_node(1);
  Node<GT>* b = g.insert_node(2);
  Node<GT>* c = g.insert_node(3);
  Node<GT>* d = g.insert_node(4);
  Node<GT>* e = g.insert_node(5);
  Node<GT>* f = g.insert_node(6);

  g.insert_arc(a, b, 0);
  g.insert_arc(b, c, 0);
  g.insert_arc(b, d, 0);
  g.insert_arc(d, e, 0);
  g.insert_arc(e, b, 0);
  g.insert_arc(c, f, 0);

  auto bridges = Bridges<GT>::find_bridges(g);

  cout << "number of bridges: " << bridges.size() << endl;

  for (Arc<GT>* arc : bridges)
  {
    cout << arc->get_src_node()->get_info() << " -- "
         << arc->get_tgt_node()->get_info() << endl;
  }

  return 0;
}
