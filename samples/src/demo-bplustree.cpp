/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <btree.hpp>

using namespace Designar;

int main()
{
    BPlusTree<int_t> tree;

    for (int_t v : {5, 3, 8, 1, 4, 7, 9})
    {
        tree.insert(v);
    }

    // Unlike BTree, iteration here walks a flat linked list of leaves
    // instead of recursing through the whole tree.
    cout << "In-order traversal (via the leaf chain): ";

    tree.for_each_inorder([](int_t v) { cout << v << " "; });

    cout << endl;

    cout << "search(4): " << (tree.search(4) != nullptr ? "found" : "not found")
         << endl;

    tree.remove(4);

    cout << "after remove(4), search(4): "
         << (tree.search(4) != nullptr ? "found" : "not found") << endl;

    return 0;
}
