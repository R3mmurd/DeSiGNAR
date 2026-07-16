/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <allpairsshortestpaths.hpp>

using namespace Designar;
using namespace std;

using GT = Digraph<string, int_t>;

int main()
{
  GT g;
  auto a = g.insert_node("A");
  auto b = g.insert_node("B");
  auto c = g.insert_node("C");
  auto d = g.insert_node("D");

  g.insert_arc(a, b, 1);
  g.insert_arc(b, c, 2);
  g.insert_arc(a, c, 10);
  g.insert_arc(c, d, 1);
  g.insert_arc(b, d, 8);

  FloydWarshall<GT> fw(g);
  assert(!fw.has_negative_cycle());

  assert(fw.distance_between(a, a) == 0);
  assert(fw.distance_between(a, b) == 1);
  assert(fw.distance_between(a, c) == 3);                        // via B, not direct (10)
  assert(fw.distance_between(a, d) == 4);                        // A->B->C->D = 1+2+1=4, vs A->B->D=1+8=9
  assert(fw.distance_between(b, d) == 3);                        // B->C->D=2+1=3 vs direct 8
  assert(fw.distance_between(d, a) == DefaultDistance<GT>::MAX); // unreachable (digraph)

  auto path = fw.path_between(a, d);
  assert(path.size() == 4);
  assert(path[0] == a && path[1] == b && path[2] == c && path[3] == d);

  auto no_path = fw.path_between(d, a);
  assert(no_path.is_empty());

  cout << "FloydWarshall: Everything ok!\n";
  return 0;
}
