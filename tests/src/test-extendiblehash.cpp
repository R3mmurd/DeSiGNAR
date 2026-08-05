/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>
#include <set>
#include <extendiblehash.hpp>

using namespace std;
using namespace Designar;

int main()
{
    ExtendibleHashTable<int> t;
    assert(t.is_empty());

    std::set<int> reference;

    for (int i = 0; i < 500; ++i)
    {
        int v = i * 37 + 5;
        reference.insert(v);
        t.insert(v);
    }

    assert(t.size() == reference.size());
    assert(t.get_global_depth() > 0); // must have split/doubled at least once

    for (int v : reference)
    {
        assert(t.has(v));
    }

    assert(!t.has(-999999));

    // duplicate insert returns nullptr, does not change size
    int first_key = *reference.begin();
    nat_t size_before = t.size();
    assert(t.insert(first_key) == nullptr);
    assert(t.size() == size_before);

    // remove half
    int removed = 0;

    for (auto it = reference.begin(); it != reference.end();)
    {
        if (removed % 2 == 0)
        {
            assert(t.remove(*it));
            it = reference.erase(it);
        }
        else
        {
            ++it;
        }

        ++removed;
    }

    assert(t.size() == reference.size());

    for (int v : reference)
    {
        assert(t.has(v));
    }

    assert(!t.remove(-999999)); // removing an absent key returns false

    // copy/move semantics
    ExtendibleHashTable<int> copy(t);
    assert(copy.size() == t.size());

    for (int v : reference)
    {
        assert(copy.has(v));
    }

    ExtendibleHashTable<int> moved(std::move(copy));
    assert(moved.size() == t.size());

    // clear()
    ExtendibleHashTable<int> cleared(t);
    cleared.clear();
    assert(cleared.is_empty());
    assert(cleared.get_global_depth() == 0);

    cout << "Everything ok!\n";

    return 0;
}
