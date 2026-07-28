/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <vector>
#include <binomialheap.hpp>

using namespace std;
using namespace Designar;

int main()
{
    BinomialHeap<int> h;
    assert(h.is_empty());

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

    // decrease_key
    BinomialHeap<int> h2;
    auto* n10 = h2.insert(10);
    h2.insert(5);
    h2.insert(20);
    h2.insert(15);
    assert(h2.get_min() == 5);
    h2.decrease_key(n10, 1);
    assert(h2.get_min() == 1);

    bool threw = false;

    try
    {
        h2.decrease_key(n10, 100); // increasing a key must be rejected
    }
    catch (const std::domain_error&)
    {
        threw = true;
    }

    assert(threw);

    // copy/move semantics
    BinomialHeap<int> h3(h2);
    assert(h3.size() == h2.size());

    BinomialHeap<int> h4(std::move(h3));

    int prev = std::numeric_limits<int>::min();

    while (!h4.is_empty())
    {
        int v = h4.extract_min();
        assert(v >= prev);
        prev = v;
    }

    assert(h2.size() == 4);

    cout << "Everything ok!\n";

    return 0;
}
