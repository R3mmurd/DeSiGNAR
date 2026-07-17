/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <allpairsshortestpaths.hpp>

using namespace Designar;

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

  cout << "distance A->D: " << fw.distance_between(a, d) << endl;

  cout << "path A->D: ";

  for (auto* node : fw.path_between(a, d))
  {
    cout << node->get_info() << " ";
  }

  cout << endl;

  return 0;
}
