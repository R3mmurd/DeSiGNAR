/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <openhash.hpp>

using namespace Designar;

int main()
{
    // OpenAddressingHashTable is templated on the probing strategy;
    // LinearHashTable/QuadraticHashTable/DoubleHashingTable are the three
    // ready-made flavors (linear, quadratic, and double-hashing probing)
    // — same interface either way, so this demo (using LinearHashTable)
    // applies identically to the other two.
    LinearHashTable<int_t> table;

    for (int_t v : {5, 3, 8, 1, 4, 7, 9})
    {
        table.insert(v);
    }

    cout << "search(4): "
         << (table.search(4) != nullptr ? "found" : "not found") << endl;

    table.remove(4);

    cout << "after remove(4), search(4): "
         << (table.search(4) != nullptr ? "found" : "not found") << endl;

    cout << "size: " << table.size() << endl;

    return 0;
}
