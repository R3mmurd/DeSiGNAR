/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <lrucache.hpp>

using namespace Designar;
using namespace std;

int main()
{
    LRUCache<int_t, string> cache(3);
    assert(cache.is_empty());
    assert(cache.capacity() == 3);

    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");
    assert(cache.size() == 3);

    assert(*cache.get(1) == "one"); // touch 1, now order (MRU->LRU): 1,3,2

    cache.put(4, "four"); // evicts LRU = 2
    assert(cache.size() == 3);
    assert(!cache.contains(2));
    assert(cache.contains(1));
    assert(cache.contains(3));
    assert(cache.contains(4));

    cache.put(1, "ONE"); // update existing, touches it
    assert(*cache.get(1) == "ONE");

    // order now (MRU->LRU): 1,4,3
    cache.put(5, "five"); // evicts LRU = 3
    assert(!cache.contains(3));
    assert(cache.contains(1));
    assert(cache.contains(4));
    assert(cache.contains(5));

    assert(cache.get(999) == nullptr);

    LRUCache<int_t, string> cache2(std::move(cache));
    assert(cache2.contains(1));
    assert(cache2.size() == 3);

    // unbounded cache (capacity 0) never evicts
    LRUCache<int_t, int_t> unbounded(0);
    for (int_t i = 0; i < 1000; ++i)
        unbounded.put(i, i * i);
    assert(unbounded.size() == 1000);
    for (int_t i = 0; i < 1000; ++i)
        assert(*unbounded.get(i) == i * i);

    cout << "LRUCache: Everything ok!\n";
    return 0;
}
