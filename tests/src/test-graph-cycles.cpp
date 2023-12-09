/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <graphalgorithms.hpp>

using namespace Designar;

using UGT = Graph<char>;
using DGT = Digraph<char>;

template <class GT>
void test()
{
  GT g1;

  Node<GT> *g1a = g1.insert_node('a');
  Node<GT> *g1b = g1.insert_node('b');
  Node<GT> *g1c = g1.insert_node('c');
  Node<GT> *g1d = g1.insert_node('d');

  g1.insert_arc(g1a, g1b);
  g1.insert_arc(g1b, g1c);
  g1.insert_arc(g1c, g1d);
  g1.insert_arc(g1d, g1a);
  g1.insert_arc(g1d, g1b);

  assert(has_cycle(g1));
  assert(!is_acyclique(g1));

  GT g2;

  Node<GT> *g2a = g2.insert_node('a');
  Node<GT> *g2b = g2.insert_node('b');
  Node<GT> *g2c = g2.insert_node('c');
  Node<GT> *g2d = g2.insert_node('d');

  g2.insert_arc(g2a, g2b);
  g2.insert_arc(g2b, g2c);
  g2.insert_arc(g2c, g2d);
  g2.insert_arc(g2d, g2a);

  assert(has_cycle(g2));
  assert(!is_acyclique(g2));

  GT g3;

  Node<GT> *g3a = g3.insert_node('a');
  Node<GT> *g3b = g3.insert_node('b');
  Node<GT> *g3c = g3.insert_node('c');
  g3.insert_node('d');

  g3.insert_arc(g3a, g3b);
  g3.insert_arc(g3b, g3c);
  g3.insert_arc(g3c, g3a);

  assert(has_cycle(g3));
  assert(!is_acyclique(g3));

  GT g4;

  Node<GT> *g4a = g4.insert_node('a');
  Node<GT> *g4b = g4.insert_node('b');
  Node<GT> *g4c = g4.insert_node('c');
  Node<GT> *g4d = g4.insert_node('d');

  g4.insert_arc(g4a, g4b);
  g4.insert_arc(g4b, g4c);
  g4.insert_arc(g4c, g4d);

  assert(!has_cycle(g4));
  assert(is_acyclique(g4));

  GT g5;

  Node<GT> *g5a = g5.insert_node('a');
  Node<GT> *g5b = g5.insert_node('b');
  Node<GT> *g5c = g5.insert_node('c');
  Node<GT> *g5d = g5.insert_node('d');

  g5.insert_arc(g5a, g5b);
  g5.insert_arc(g5a, g5c);
  g5.insert_arc(g5b, g5d);
  g5.insert_arc(g5c, g5d);

  assert(g5.is_digraph() xor has_cycle(g5));
  assert(!g5.is_digraph() xor is_acyclique(g5));

  GT g6;

  Node<GT> *g6a = g6.insert_node('a');
  Node<GT> *g6b = g6.insert_node('b');
  Node<GT> *g6c = g6.insert_node('c');
  Node<GT> *g6d = g6.insert_node('d');
  Node<GT> *g6e = g6.insert_node('e');

  g6.insert_arc(g6a, g6e);
  g6.insert_arc(g6a, g6b);
  g6.insert_arc(g6a, g6c);
  g6.insert_arc(g6e, g6b);
  g6.insert_arc(g6e, g6c);
  g6.insert_arc(g6c, g6d);
  g6.insert_arc(g6b, g6d);

  assert(g6.is_digraph() xor has_cycle(g6));
  assert(!g6.is_digraph() xor is_acyclique(g6));

  GT g7;

  Node<GT> *g7a = g7.insert_node('a');
  Node<GT> *g7b = g7.insert_node('b');
  Node<GT> *g7c = g7.insert_node('c');
  Node<GT> *g7d = g7.insert_node('d');
  Node<GT> *g7e = g7.insert_node('e');

  g7.insert_arc(g7e, g7a);
  g7.insert_arc(g7a, g7b);
  g7.insert_arc(g7a, g7c);
  g7.insert_arc(g7c, g7d);
  g7.insert_arc(g7b, g7d);
  g7.insert_arc(g7d, g7e);
  g7.insert_arc(g7e, g7c);

  assert(has_cycle(g7));
  assert(!is_acyclique(g7));
}

int main()
{
  cout << "Testing for undirected graph\n";

  test<UGT>();

  cout << "Testing for directed graph\n";

  test<DGT>();

  cout << "Everything ok!\n";

  return 0;
}
