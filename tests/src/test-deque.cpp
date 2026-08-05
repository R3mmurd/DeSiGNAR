/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>
#include <string>
#include <deque.hpp>

using namespace std;
using namespace Designar;

namespace
{
    template <class D>
    void test_deque()
    {
        D d;
        assert(d.is_empty());
        d.push_back(1);
        d.push_back(2);
        d.push_front(0);
        d.push_front(-1);
        // deque now: -1 0 1 2
        assert(d.size() == 4);
        assert(d.front() == -1);
        assert(d.back() == 2);
        assert(d.pop_front() == -1);
        assert(d.pop_back() == 2);
        assert(d.size() == 2);
        assert(d.front() == 0);
        assert(d.back() == 1);
        assert(d.pop_front() == 0);
        assert(d.pop_back() == 1);
        assert(d.is_empty());

        bool threw = false;

        try
        {
            d.pop_front();
        }
        catch (const std::underflow_error&)
        {
            threw = true;
        }

        assert(threw);

        // stress: many push_back/push_front interleaved (exercises
        // DynDeque's resize path and FixedDeque's circular wraparound)
        D d2;

        for (int i = 0; i < 200; ++i)
        {
            if (i % 2 == 0)
            {
                d2.push_back(i);
            }
            else
            {
                d2.push_front(-i);
            }
        }

        assert(d2.size() == 200);

        while (!d2.is_empty())
        {
            d2.pop_back();
        }

        assert(d2.is_empty());
    }
} // end anonymous namespace

int main()
{
    test_deque<DynDeque<int>>();
    test_deque<FixedDeque<int, 300>>();

    // move-only-friendly usage (std::string is move-constructible; the
    // point is that neither Fixed/DynDeque require T to be default-
    // constructible for the deque itself to be declared).
    DynDeque<std::string> ds;
    ds.push_back(std::string("hello"));
    ds.push_front(std::string("world"));
    assert(ds.front() == "world");
    assert(ds.back() == "hello");

    // copy/move semantics
    DynDeque<int> a;
    a.push_back(1);
    a.push_back(2);
    a.push_front(0);

    DynDeque<int> b(a);
    assert(b.size() == a.size());
    assert(b.front() == 0);
    assert(b.back() == 2);

    DynDeque<int> c(std::move(b));
    assert(c.size() == 3);

    cout << "Everything ok!\n";

    return 0;
}
