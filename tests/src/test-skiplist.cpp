/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <skiplist.hpp>
#include <random.hpp>

using namespace Designar;
using namespace std;

int main()
{
    SkipList<int_t> sl;
    assert(sl.is_empty());

    constexpr int_t N = 3000;
    for (int_t i = 0; i < N; ++i)
        assert(sl.insert(i) != nullptr);

    assert(sl.size() == (nat_t)N);
    assert(sl.min() == 0);
    assert(sl.max() == N - 1);
    assert(sl.insert(5) == nullptr); // duplicate

    for (int_t i = 0; i < N; ++i)
        assert(sl.search(i) != nullptr);
    assert(sl.search(-1) == nullptr);
    assert(sl.search(N) == nullptr);

    int_t prev = -1;
    nat_t count = 0;
    for (int_t x : sl)
    {
        assert(x > prev);
        prev = x;
        ++count;
    }
    assert(count == (nat_t)N);

    rng_t rng(31415);
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
        assert(sl.remove(k));
        --expected;
        assert(sl.size() == expected);
        assert(sl.search(k) == nullptr);
    }
    assert(sl.is_empty());
    assert(!sl.remove(0));

    SkipList<int_t> sl2;
    for (int_t i = 0; i < 200; ++i)
        sl2.insert(i);

    SkipList<int_t> sl3 = sl2;
    sl3.insert(99999);
    assert(sl3.search(99999) != nullptr);
    assert(sl2.search(99999) == nullptr);

    SkipList<int_t> sl4 = std::move(sl3);
    assert(sl4.search(99999) != nullptr);

    // header (the one Node built via the level-only constructor) used to
    // unconditionally default-construct a Key, even though its key is
    // never read by anything in this file — fixed via std::optional<Key>.
    // insert()/append()/search_or_insert() also gained Key&& overloads,
    // matching every other tree/set in this library. Exercised end to
    // end with a Key that has no default constructor and is move-only.
    {
        struct NoDefaultKey
        {
            int_t value;

            NoDefaultKey() = delete;

            explicit NoDefaultKey(int_t v) : value(v)
            {
                // empty
            }

            NoDefaultKey(const NoDefaultKey&) = delete;
            NoDefaultKey& operator=(const NoDefaultKey&) = delete;
            NoDefaultKey(NoDefaultKey&&) = default;
            NoDefaultKey& operator=(NoDefaultKey&&) = default;

            bool operator<(const NoDefaultKey& o) const
            {
                return value < o.value;
            }
        };

        SkipList<NoDefaultKey> sl5;

        for (int_t i = 0; i < 100; ++i)
        {
            sl5.insert(NoDefaultKey(i));
        }

        assert(sl5.search(NoDefaultKey(50)) != nullptr);
        assert(sl5.search(NoDefaultKey(9999)) == nullptr);
        assert(sl5.remove(NoDefaultKey(50)));
        assert(sl5.search(NoDefaultKey(50)) == nullptr);

        cout << "SkipList: non-default-constructible, move-only Key "
                "Everything ok!\n";
    }

    cout << "SkipList: Everything ok!\n";
    return 0;
}
