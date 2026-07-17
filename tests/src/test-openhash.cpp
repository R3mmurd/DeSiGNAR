/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <set.hpp>
#include <random.hpp>

using namespace Designar;
using namespace std;

template <template <typename, class> class Table>
void test_table(const char* name)
{
    Table<int_t, std::equal_to<int_t>> t;
    assert(t.is_empty());
    assert(t.verify());

    constexpr int_t N = 3000;

    for (int_t i = 0; i < N; ++i)
    {
        assert(t.insert(i) != nullptr);
        assert(t.verify());
    }

    assert(t.size() == (nat_t)N);

    for (int_t i = 0; i < N; ++i)
        assert(t.search(i) != nullptr);
    assert(t.search(-1) == nullptr);
    assert(t.search(N) == nullptr);

    assert(t.insert(5) == nullptr); // duplicate

    nat_t count = 0;
    for (int_t x : t)
    {
        assert(x >= 0 && x < N);
        ++count;
    }
    assert(count == (nat_t)N);

    rng_t rng(2026);
    DynArray<int_t> keys;
    for (int_t i = 0; i < N; ++i)
        keys.append(i);
    for (nat_t i = keys.size() - 1; i > 0; --i)
    {
        nat_t j = random_uniform(rng, i + 1);
        swap(keys[i], keys[j]);
    }

    nat_t expected = (nat_t)N;
    for (int_t k : keys)
    {
        assert(t.remove(k));
        --expected;
        assert(t.size() == expected);
        assert(t.search(k) == nullptr);
        assert(t.verify());
    }
    assert(t.is_empty());
    assert(!t.remove(0));

    // interleaved insert/remove/search stress, including through tombstones
    Table<int_t, std::equal_to<int_t>> t2;
    DynArray<int_t> present;
    rng_t rng2(555);
    for (int_t i = 0; i < 8000; ++i)
    {
        if (present.is_empty() || random_uniform(rng2, 100) < 60)
        {
            int_t k = random_uniform(rng2, 5000);
            if (t2.search(k) == nullptr)
            {
                t2.insert(k);
                present.append(k);
            }
        }
        else
        {
            nat_t idx = random_uniform(rng2, present.size());
            int_t k = present[idx];
            assert(t2.remove(k));
            present.remove_pos(idx);
        }
        assert(t2.size() == present.size());
        assert(t2.verify());
    }
    for (int_t k : present)
        assert(t2.search(k) != nullptr);

    cout << name << ": Everything ok!\n";
}

int main()
{
    test_table<LinearHashTable>("LinearHashTable");
    test_table<QuadraticHashTable>("QuadraticHashTable");
    test_table<DoubleHashingTable>("DoubleHashingTable");

    // Every open-addressing flavor must also work as HashSet's pluggable
    // HashTableType, the same way LHashTable (separate chaining) does.
    HashSet<int_t, std::equal_to<int_t>, LinearHashTable> hs_linear = {1, 2, 3,
                                                                       4};
    assert(hs_linear.equal({1, 2, 3, 4}));

    HashSet<int_t, std::equal_to<int_t>, QuadraticHashTable> hs_quad = {1, 2, 3,
                                                                        4};
    assert(hs_quad.equal({1, 2, 3, 4}));

    HashSet<int_t, std::equal_to<int_t>, DoubleHashingTable> hs_double = {1, 2,
                                                                          3, 4};
    assert(hs_double.equal({1, 2, 3, 4}));

    cout << "All open-addressing tables ok!\n";
    return 0;
}
