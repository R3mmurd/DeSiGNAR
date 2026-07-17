/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <skiplist.hpp>

using namespace Designar;

int main()
{
  SkipList<int_t> list;

  for (int_t v : {5, 3, 8, 1, 4, 7, 9})
  {
    list.insert(v);
  }

  cout << "search(4): " << (list.search(4) != nullptr ? "found" : "not found") << endl;

  list.remove(4);

  cout << "after remove(4), search(4): "
       << (list.search(4) != nullptr ? "found" : "not found") << endl;

  return 0;
}
