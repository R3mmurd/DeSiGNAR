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
    RankedRBTree<int_t> tree;

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

    auto [left, right] = tree.split_pos(3);

    cout << "split_pos(3):" << endl;
    cout << "  left  (smallest 3): ";
    for (int_t v : left)
    {
        cout << v << " ";
    }
    cout << endl;
    cout << "  right (the rest):  ";
    for (int_t v : right)
    {
        cout << v << " ";
    }
    cout << endl;

    return 0;
}
