/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <algorithm>
#include <cassert>
#include <iostream>
#include <limits>
#include <vector>
#include <fibonacciheap.hpp>

using namespace std;
using namespace Designar;

int main()
{
    FibonacciHeap<int> h;
    assert(h.is_empty());

    std::vector<int> vals;

    for (int i = 0; i < 2000; ++i)
    {
        vals.push_back(int((nat_t(i) * 2654435761u) % 50021));
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

    // decrease_key + cascading-cut stress: strictly increasing keys, so
    // "the first 100 extracted" is unambiguously "the first 100
    // inserted" — needed to know which handles remain live afterward.
    FibonacciHeap<int> h2;
    std::vector<FibonacciHeap<int>::Node*> handles2;
    std::vector<int> vals2;

    for (int i = 0; i < 500; ++i)
    {
        int v = 100000 + i;
        vals2.push_back(v);
        handles2.push_back(h2.insert(v));
    }

    for (int i = 0; i < 100; ++i)
    {
        assert(h2.extract_min() == vals2[i]);
    }

    for (nat_t i = 100; i < handles2.size(); i += 3)
    {
        int new_val = vals2[i] - 200000;
        h2.decrease_key(handles2[i], new_val);
        vals2[i] = new_val;
    }

    assert(h2.get_min() == *std::min_element(vals2.begin() + 100, vals2.end()));

    int prev = std::numeric_limits<int>::min();
    nat_t count = 0;

    while (!h2.is_empty())
    {
        int v = h2.extract_min();
        assert(v >= prev);
        prev = v;
        ++count;
    }

    assert(count == vals2.size() - 100);

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

    // copy/move semantics
    FibonacciHeap<int> h3;

    for (int v : {5, 3, 8, 1, 9, 2, 7})
    {
        h3.insert(v);
    }

    FibonacciHeap<int> h4(h3);
    assert(h4.size() == h3.size());

    FibonacciHeap<int> h5(std::move(h4));

    prev = std::numeric_limits<int>::min();

    while (!h5.is_empty())
    {
        int v = h5.extract_min();
        assert(v >= prev);
        prev = v;
    }

    assert(h3.size() == 7); // h3 still usable after being copied from

    cout << "Everything ok!\n";

    return 0;
}
