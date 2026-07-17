/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file threadpool.hpp
    @brief ThreadPool: a fixed-size pool of worker threads that pull
    submitted tasks off a shared queue — built directly on top of this
    library's own ConcurrentQueue (queue.hpp) rather than reimplementing
    task-queue synchronization from scratch.
    @ingroup Concurrency
*/

#pragma once

#include <thread>
#include <functional>
#include <future>

#include <queue.hpp>
#include <array.hpp>

namespace Designar
{
    /** Every worker thread runs the same loop: block on `tasks.get()`
        (which is exactly ConcurrentQueue's job) until a task arrives, run
        it, repeat. Shutdown uses the classic "poison pill" technique
        rather than an atomic stop flag a worker could check: a worker
        blocked inside `tasks.get()` cannot observe a flag changing
        elsewhere, so the destructor instead pushes one no-op sentinel
        task per worker (each recognized by an empty `std::function`) —
        guaranteeing every worker wakes up with *something* to pull off
        the queue, sees it is the shutdown sentinel, and exits. */
    class ThreadPool
    {
        ConcurrentQueue<std::function<void()>> tasks;
        DynArray<std::thread> workers;

        void worker_loop()
        {
            while (true)
            {
                std::function<void()> task = tasks.get();

                if (!task) // the shutdown sentinel: a default-constructed,
                           // empty std::function
                {
                    break;
                }

                task();
            }
        }

    public:
        /** `num_threads == 0` is treated as "use
            std::thread::hardware_concurrency(), or 1 if that cannot be
            determined" — never as "a pool with zero workers", which would
            accept submissions that could then never run. */
        explicit ThreadPool(nat_t num_threads = 0)
        {
            if (num_threads == 0)
            {
                unsigned int hw = std::thread::hardware_concurrency();
                num_threads = hw > 0 ? hw : 1;
            }

            workers.reserve(num_threads);

            for (nat_t i = 0; i < num_threads; ++i)
            {
                workers.append(std::thread([this] { worker_loop(); }));
            }
        }

        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        ~ThreadPool()
        {
            for (nat_t i = 0; i < workers.size(); ++i)
            {
                tasks.put(std::function<void()>()); // one shutdown sentinel per
                                                    // worker
            }

            for (std::thread& w : workers)
            {
                w.join();
            }
        }

        nat_t size() const
        {
            return workers.size();
        }

        /** Submits `f` (any callable taking no arguments) to run on whichever
            worker picks it up next, returning a `std::future` for its result
            — or its exception, if it throws: like `std::async`, the
            exception is captured and re-thrown from `future::get()` rather
            than escaping into the worker thread (which would otherwise call
            `std::terminate`). */
        template <class F>
        auto submit(F&& f) -> std::future<std::invoke_result_t<F>>
        {
            using ReturnType = std::invoke_result_t<F>;

            auto task = std::make_shared<std::packaged_task<ReturnType()>>(
                std::forward<F>(f));
            std::future<ReturnType> result = task->get_future();

            tasks.put([task] { (*task)(); });

            return result;
        }
    };

} // end namespace Designar
