/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <avltree.hpp>

using namespace Designar;

int main()
{
    RankedAVLTree<int_t> tree;

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

    // The whole point of a ranked tree: order-statistic queries in
    // O(log n), on top of everything a plain search tree already does.
    cout << "select(0) (smallest): " << tree.select(0) << endl;
    cout << "select(3) (4th smallest): " << tree.select(3) << endl;
    cout << "position(8): " << tree.position(8) << endl;

    return 0;
}
