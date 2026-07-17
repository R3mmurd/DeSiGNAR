/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <btree.hpp>
#include <random.hpp>

using namespace Designar;
using namespace std;

int main()
{
    BTree<int_t> t;
    assert(t.is_empty());
    assert(t.verify());

    constexpr int_t N = 5000;
    for (int_t i = 0; i < N; ++i)
    {
        assert(t.insert(i) != nullptr);
        assert(t.verify());
    }

    assert(t.size() == (nat_t)N);
    assert(t.min() == 0);
    assert(t.max() == N - 1);
    assert(t.insert(5) == nullptr); // duplicate

    for (int_t i = 0; i < N; ++i)
        assert(t.search(i) != nullptr);
    assert(t.search(-1) == nullptr);
    assert(t.search(N) == nullptr);

    int_t prev = -1;
    nat_t count = 0;
    t.for_each_inorder(
        [&](int_t x)
        {
            assert(x > prev);
            prev = x;
            ++count;
        });
    assert(count == (nat_t)N);

    rng_t rng(20260716);
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

    // interleaved insert/remove stress with different min-degree values
    {
        BTree<int_t, std::less<int_t>, 2>
            t2; // smallest legal degree, most splitting/merging
        DynArray<int_t> present;
        rng_t rng2(99);

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

            assert(t2.verify());
            assert(t2.size() == present.size());
        }

        for (int_t k : present)
            assert(t2.search(k) != nullptr);
    }

    BTree<int_t> t3;
    for (int_t i = 0; i < 300; ++i)
        t3.insert(i);

    BTree<int_t> t4 = t3; // copy
    t4.insert(99999);
    assert(t4.search(99999) != nullptr);
    assert(t3.search(99999) == nullptr);

    BTree<int_t> t5 = std::move(t4);
    assert(t5.search(99999) != nullptr);

    cout << "BTree: Everything ok!\n";
    return 0;
}
