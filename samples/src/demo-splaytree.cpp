/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <splaytree.hpp>

using namespace Designar;

int main()
{
    SplayTree<int_t> tree;

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

    // Splaying moves the most recently searched key to the root, so a
    // repeated search for the same key is fast — searching here doesn't
    // change the logical contents, only the tree's internal shape.
    cout << "search(4): " << (tree.search(4) != nullptr ? "found" : "not found")
         << endl;

    tree.remove(4);

    cout << "after remove(4), search(4): "
         << (tree.search(4) != nullptr ? "found" : "not found") << endl;

    return 0;
}
