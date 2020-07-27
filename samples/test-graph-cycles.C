/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <graphalgorithms.H>

using namespace Designar;

using GT = Graph<char>;

int main()
{
  GT g1;

  GT::Node * g1a = g1.insert_node('a');
  GT::Node * g1b = g1.insert_node('b');
  GT::Node * g1c = g1.insert_node('c');
  GT::Node * g1d = g1.insert_node('d');

  g1.insert_arc(g1a, g1b);
  g1.insert_arc(g1b, g1c);
  g1.insert_arc(g1c, g1d);
  g1.insert_arc(g1d, g1a);
  g1.insert_arc(g1d, g1b);

  assert(has_cycle(g1));
  assert(not is_acyclique(g1));

  GT g2;

  GT::Node * g2a = g2.insert_node('a');
  GT::Node * g2b = g2.insert_node('b');
  GT::Node * g2c = g2.insert_node('c');
  GT::Node * g2d = g2.insert_node('d');

  g2.insert_arc(g2a, g2b);
  g2.insert_arc(g2b, g2c);
  g2.insert_arc(g2c, g2d);
  g2.insert_arc(g2d, g2a);

  assert(has_cycle(g2));
  assert(not is_acyclique(g2));

  GT g3;

  GT::Node * g3a = g3.insert_node('a');
  GT::Node * g3b = g3.insert_node('b');
  GT::Node * g3c = g3.insert_node('c');
  g3.insert_node('d');

  g3.insert_arc(g3a, g3b);
  g3.insert_arc(g3b, g3c);
  g3.insert_arc(g3c, g3a);

  assert(has_cycle(g3));
  assert(not is_acyclique(g3));

  GT g4;

  GT::Node * g4a = g4.insert_node('a');
  GT::Node * g4b = g4.insert_node('b');
  GT::Node * g4c = g4.insert_node('c');
  GT::Node * g4d = g4.insert_node('d');

  g4.insert_arc(g4a, g4b);
  g4.insert_arc(g4b, g4c);
  g4.insert_arc(g4c, g4d);

  assert(not has_cycle(g4));
  assert(is_acyclique(g4));

  cout << "Everything ok!\n";
  
  return 0;
}
