/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <treap.hpp>

using namespace Designar;

int main()
{
    Treap<int_t> treap;

    for (int_t v : {5, 3, 8, 1, 4, 7, 9})
    {
        treap.insert(v);
    }

    cout << "In-order traversal: ";

    for (int_t v : treap)
    {
        cout << v << " ";
    }

    cout << endl;

    cout << "search(4): "
         << (treap.search(4) != nullptr ? "found" : "not found") << endl;

    treap.remove(4);

    cout << "after remove(4), search(4): "
         << (treap.search(4) != nullptr ? "found" : "not found") << endl;

    return 0;
}
