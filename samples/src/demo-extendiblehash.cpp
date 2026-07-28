/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <extendiblehash.hpp>

using namespace Designar;

int main()
{
    ExtendibleHashTable<int_t> table;

    cout << "Inserting 1..64:\n";

    for (int_t i = 1; i <= 64; ++i)
    {
        table.insert(i);

        // Print whenever the directory has just doubled, to make the
        // "split -> maybe double the directory" growth pattern visible.
        static nat_t last_depth = 0;

        if (table.get_global_depth() != last_depth)
        {
            last_depth = table.get_global_depth();
            cout << "  after inserting " << i << ": global_depth = "
                 << last_depth << ", num_buckets = "
                 << table.get_num_buckets() << endl;
        }
    }

    cout << "\nFinal size: " << table.size() << endl;
    cout << "Final global_depth: " << table.get_global_depth() << endl;
    cout << "Final num_buckets: " << table.get_num_buckets() << endl;

    cout << "\nhas(42): " << (table.has(42) ? "true" : "false") << endl;
    cout << "has(1000): " << (table.has(1000) ? "true" : "false") << endl;

    table.remove(42);
    cout << "after remove(42), has(42): " << (table.has(42) ? "true" : "false")
         << endl;

    return 0;
}
