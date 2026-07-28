/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <vector>
#include <leftistheap.hpp>

using namespace std;
using namespace Designar;

int main()
{
    LeftistHeap<int> h;
    assert(h.is_empty());

    bool threw = false;

    try
    {
        h.get_min();
    }
    catch (const std::underflow_error&)
    {
        threw = true;
    }

    assert(threw);

    std::vector<int> vals;

    for (int i = 0; i < 1000; ++i)
    {
        vals.push_back(int((nat_t(i) * 2654435761u) % 10007));
    }

    for (int v : vals)
    {
        h.insert(v);
    }

    assert(h.size() == vals.size());

    std::vector<int> sorted_vals = vals;
    std::sort(sorted_vals.begin(), sorted_vals.end());

    for (int expected : sorted_vals)
    {
        assert(h.get_min() == expected);
        assert(h.extract_min() == expected);
    }

    assert(h.is_empty());

    // copy/move semantics
    LeftistHeap<int> h2;

    for (int v : {5, 3, 8, 1, 9, 2})
    {
        h2.insert(v);
    }

    LeftistHeap<int> h3(h2);
    assert(h3.size() == h2.size());

    LeftistHeap<int> h4(std::move(h3));
    assert(h4.size() == 6);

    int prev = std::numeric_limits<int>::min();

    while (!h4.is_empty())
    {
        int v = h4.extract_min();
        assert(v >= prev);
        prev = v;
    }

    // h2 (the original, unaffected by copying from) is still usable
    assert(h2.size() == 6);

    cout << "Everything ok!\n";

    return 0;
}
