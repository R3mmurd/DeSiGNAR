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

    // put()'s Node used to only accept Value by const&, forcing a copy on
    // every put() (and blocking a move-only Value entirely); the
    // update-existing-key path also copy-assigned instead of
    // move-assigning. Exercised end to end with a Value that has no
    // default constructor and is move-only: initial put() (move-
    // constructs into a new Node), an update put() on the same key (move-
    // assigns into the existing Node), and eviction.
    {
        struct NoDefaultValue
        {
            int_t payload;

            NoDefaultValue() = delete;

            explicit NoDefaultValue(int_t p) : payload(p)
            {
                // empty
            }

            NoDefaultValue(const NoDefaultValue&) = delete;
            NoDefaultValue& operator=(const NoDefaultValue&) = delete;
            NoDefaultValue(NoDefaultValue&&) = default;
            NoDefaultValue& operator=(NoDefaultValue&&) = default;
        };

        LRUCache<int_t, NoDefaultValue> mo_cache(2);
        mo_cache.put(1, NoDefaultValue(10));
        mo_cache.put(2, NoDefaultValue(20));

        assert(mo_cache.get(1)->payload == 10);
        assert(mo_cache.get(2)->payload == 20);

        mo_cache.put(1, NoDefaultValue(100)); // update path: move-assign
        assert(mo_cache.get(1)->payload == 100);

        mo_cache.put(3, NoDefaultValue(30)); // evicts LRU (key 2)
        assert(!mo_cache.contains(2));
        assert(mo_cache.get(3)->payload == 30);

        cout << "LRUCache: non-default-constructible, move-only Value "
                "Everything ok!\n";
    }

    cout << "LRUCache: Everything ok!\n";
    return 0;
}
