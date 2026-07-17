/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>

using namespace std;

#include <thread>
#include <vector>
#include <unordered_set>

#include <singleton.hpp>
#include <types.hpp>

using namespace Designar;

class Counter : public Singleton<Counter>
{
    friend class Singleton<Counter>;

    int_t value;

protected:
    Counter() : value(0)
    {
        // empty
    }

public:
    int_t increment()
    {
        return ++value;
    }

    int_t get() const
    {
        return value;
    }
};

int main()
{
    // Basic single-threaded behavior: same instance every time, state
    // persists across calls.
    assert(Counter::get_instance().get() == 0);
    Counter::get_instance().increment();
    assert(Counter::get_instance().get() == 1);
    assert(&Counter::get_instance() == Counter::get_ptr_instance());

    // Regression: get_ptr_instance() used to do an unsynchronized
    // "if (instance == nullptr) instance = new T();" check-then-act on a
    // std::unique_ptr member. Two threads racing on first access could
    // both see nullptr and both construct a T, with the second assignment
    // silently replacing (and leaking) the first instance -- and any
    // pointer obtained from that first instance would be left dangling.
    // get_instance()/get_ptr_instance() are now implemented via a
    // function-local static, whose initialization C++11 guarantees is
    // thread-safe (see singleton.hpp). This test cannot deterministically
    // prove the absence of a race, but running many threads that all race
    // on first access and asserting every single one of them observed the
    // *same* address is a reasonable smoke test: under the old
    // implementation this reliably (if non-deterministically) failed under
    // ThreadSanitizer, and could fail plainly by observing more than one
    // distinct address.
    class Widget : public Singleton<Widget>
    {
        friend class Singleton<Widget>;

    protected:
        Widget() = default;
    };

    constexpr nat_t NUM_THREADS = 32;

    vector<thread> threads;
    vector<Widget*> observed(NUM_THREADS, nullptr);

    for (nat_t i = 0; i < NUM_THREADS; ++i)
        threads.emplace_back([&observed, i]
                             { observed[i] = &Widget::get_instance(); });

    for (auto& t : threads)
        t.join();

    unordered_set<Widget*> distinct_addresses(observed.begin(), observed.end());
    assert(distinct_addresses.size() == 1);

    cout << "Everything ok!\n";

    return 0;
}
