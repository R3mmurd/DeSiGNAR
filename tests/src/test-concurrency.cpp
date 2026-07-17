/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <atomic>
#include <vector>

#include <concurrentmap.hpp>
#include <threadpool.hpp>

using namespace Designar;
using namespace std;

int main()
{
    // ConcurrentMap: basic single-threaded correctness first
    ConcurrentMap<int_t, string> cm;
    assert(cm.is_empty());
    assert(cm.insert(1, "one"));
    assert(!cm.insert(1, "uno")); // duplicate
    assert(cm.contains(1));
    assert(*cm.get(1) == "one");
    assert(!cm.get(999).has_value());

    cm.put(1, "ONE"); // upsert
    assert(*cm.get(1) == "ONE");

    assert(cm.remove(1));
    assert(!cm.contains(1));
    assert(!cm.remove(1));

    // Multi-threaded stress: many threads inserting distinct keys concurrently.
    {
        ConcurrentMap<int_t, int_t> shared_map;
        constexpr int_t NUM_THREADS = 8;
        constexpr int_t PER_THREAD = 2000;

        vector<thread> threads;
        for (int_t t = 0; t < NUM_THREADS; ++t)
            threads.emplace_back(
                [&shared_map, t]
                {
                    for (int_t i = 0; i < PER_THREAD; ++i)
                    {
                        int_t key = t * PER_THREAD + i;
                        shared_map.put(key, key * key);
                    }
                });

        for (auto& th : threads)
            th.join();

        assert(shared_map.size() == (nat_t)(NUM_THREADS * PER_THREAD));

        for (int_t t = 0; t < NUM_THREADS; ++t)
            for (int_t i = 0; i < PER_THREAD; ++i)
            {
                int_t key = t * PER_THREAD + i;
                auto v = shared_map.get(key);
                assert(v.has_value() && *v == key * key);
            }
    }

    // ConcurrentSet basic + concurrent stress
    {
        ConcurrentSet<int_t> cs;
        assert(cs.insert(5));
        assert(!cs.insert(5));
        assert(cs.contains(5));
        assert(cs.remove(5));
        assert(!cs.contains(5));

        ConcurrentSet<int_t> shared_set;
        atomic<int_t> success_count{0};
        vector<thread> threads;
        for (int_t t = 0; t < 8; ++t)
            threads.emplace_back(
                [&]
                {
                    for (int_t i = 0; i < 1000; ++i)
                        if (shared_set.insert(i)) // many threads race to insert
                                                  // the SAME 1000 keys
                            success_count.fetch_add(1);
                });
        for (auto& th : threads)
            th.join();

        assert(shared_set.size() == 1000);
        assert(success_count.load() ==
               1000); // exactly one thread wins each key
    }

    cout << "ConcurrentMap/ConcurrentSet: Everything ok!\n";

    // ThreadPool: basic submit/result
    {
        ThreadPool pool(4);
        assert(pool.size() == 4);

        auto fut = pool.submit([] { return 40 + 2; });
        assert(fut.get() == 42);

        // many tasks, verify all run and results correct
        vector<future<int_t>> futures;
        for (int_t i = 0; i < 500; ++i)
            futures.push_back(pool.submit([i] { return i * i; }));

        for (int_t i = 0; i < 500; ++i)
            assert(futures[i].get() == i * i);

        // exception propagation
        auto bad =
            pool.submit([]() -> int_t { throw std::runtime_error("boom"); });
        bool threw = false;
        try
        {
            bad.get();
        }
        catch (const std::runtime_error& e)
        {
            threw = true;
            assert(string(e.what()) == "boom");
        }
        assert(threw);

        // side-effect task (void-ish via shared atomic counter)
        atomic<int_t> counter{0};
        vector<future<void>> void_futures;
        for (int_t i = 0; i < 200; ++i)
            void_futures.push_back(
                pool.submit([&counter] { counter.fetch_add(1); }));
        for (auto& f : void_futures)
            f.get();
        assert(counter.load() == 200);
    }

    cout << "ThreadPool: Everything ok!\n";
    return 0;
}
