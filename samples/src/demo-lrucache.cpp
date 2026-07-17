/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <lrucache.hpp>

using namespace Designar;

int main()
{
    LRUCache<int_t, string> cache(2);

    cache.put(1, "one");
    cache.put(2, "two");

    cout << "get(1): " << *cache.get(1) << endl;

    // Touching key 1 makes key 2 the least-recently-used one, so this
    // insert evicts 2, not 1.
    cache.put(3, "three");

    cout << "contains(2) after inserting a 3rd key: "
         << (cache.contains(2) ? "yes" : "no") << endl;
    cout << "contains(1) after inserting a 3rd key: "
         << (cache.contains(1) ? "yes" : "no") << endl;

    return 0;
}
