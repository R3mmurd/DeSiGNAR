/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <rbtree.hpp>

using namespace Designar;

int main()
{
    RbTree<int_t> tree;

    for (int_t v : {5, 3, 8, 1, 4, 7, 9})
    {
        tree.insert(v);
    }

    cout << "In-order traversal: ";

    for (int_t v : tree)
    {
        cout << v << " ";
    }

    cout << endl;

    cout << "search(4): " << (tree.search(4) != nullptr ? "found" : "not found")
         << endl;

    tree.remove(4);

    cout << "after remove(4), search(4): "
         << (tree.search(4) != nullptr ? "found" : "not found") << endl;

    return 0;
}
