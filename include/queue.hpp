/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <array.hpp>
#include <list.hpp>

#include <new>

namespace Designar
{

    template <typename T, nat_t CAP = 100>
    class FixedQueue
    {
        T array[CAP];
        nat_t num_items;
        nat_t f;
        nat_t r;

        void copy_queue(const FixedQueue&);

        void swap(FixedQueue& q)
        {
            std::swap(array, q.array);
            std::swap(num_items, q.num_items);
            std::swap(f, q.f);
            std::swap(r, q.r);
        }

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

        FixedQueue() : num_items(0), f(0), r(CAP - 1)
        {
            // empty
        }

        FixedQueue(const FixedQueue& q) : num_items(q.num_items)
        {
            copy_queue(q);
        }

        FixedQueue(FixedQueue&& q) : FixedQueue()
        {
            swap(q);
        }

        FixedQueue& operator=(const FixedQueue& q)
        {
            if (this == &q)
            {
                return *this;
            }

            num_items = q.num_items;
            f = q.f;
            r = q.r;
            copy_queue(q);

            return *this;
        }

        FixedQueue& operator=(FixedQueue&& q)
        {
            swap(q);
            return *this;
        }

        bool is_empty() const
        {
            return num_items == 0;
        }

        bool is_full() const
        {
            return num_items == CAP;
        }

        nat_t size() const
        {
            return num_items;
        }

        nat_t get_capacity() const
        {
            return CAP;
        }

        void clear()
        {
            num_items = 0;
            f = 0;
            r = CAP - 1;
        }

        T& front()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return array[f];
        }

        const T& front() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return array[f];
        }

        T& rear()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return array[r];
        }

        const T& rear() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return array[r];
        }

        T& put(const T& item)
        {
            if (is_full())
            {
                throw std::overflow_error("Queue is full");
            }

            r = (r + 1) % CAP;
            array[r] = item;
            ++num_items;
            return array[r];
        }

        T& put(T&& item)
        {
            if (is_full())
            {
                throw std::overflow_error("Queue is full");
            }

            r = (r + 1) % CAP;
            array[r] = std::move(item);
            ++num_items;
            return array[r];
        }

        T get()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            T ret_val = std::move(array[f]);
            f = (f + 1) % CAP;
            --num_items;
            return ret_val;
        }
    };

    template <typename T, nat_t CAP>
    void FixedQueue<T, CAP>::copy_queue(const FixedQueue& q)
    {
        nat_t ii = q.f;

        for (nat_t i = 0; i < num_items; ++i)
        {
            array[i] = q.array[ii];
            ii = (ii + 1) % CAP;
        }

        f = 0;
        r = num_items - 1;
    }

    /** Owns its storage directly, exactly like DynArray (array.hpp) —
        only `num_items` of the `cap` slots are ever live (the circular
        range starting at `f`), growth/shrink move-constructs survivors
        into the new buffer and destroys them in the old one, and
        put()/get() placement-construct/destroy rather than assigning
        into or out of pre-existing slots. This is what lets DynQueue hold
        a `T` with no default constructor at all or a move-only `T`,
        unlike the previous FixedArray-backed version (FixedArray
        default- or copy-constructs every one of its slots up front,
        requiring `T` to be default-constructible just to declare an empty
        queue). FixedArray itself is untouched — it is still exactly right
        for FixedQueue above, which really does want every slot eagerly
        live. */
    template <typename T>
    class DynQueue
    {
        static constexpr nat_t MIN_SIZE = 32;
        static constexpr real_t RESIZE_FACTOR = 0.4;

        nat_t cap;
        nat_t num_items;
        nat_t f;
        nat_t r;
        T* array_ptr;

        static T* allocate(nat_t n)
        {
            return n == 0 ? nullptr
                          : static_cast<T*>(::operator new(sizeof(T) * n));
        }

        static void deallocate(T* p)
        {
            ::operator delete(p);
        }

        void destroy_all()
        {
            nat_t idx = f;

            for (nat_t i = 0; i < num_items; ++i)
            {
                array_ptr[idx].~T();
                idx = (idx + 1) % cap;
            }
        }

        void swap(DynQueue& q)
        {
            std::swap(cap, q.cap);
            std::swap(num_items, q.num_items);
            std::swap(f, q.f);
            std::swap(r, q.r);
            std::swap(array_ptr, q.array_ptr);
        }

        void copy_queue(const DynQueue&);

        void resize(nat_t);

        void resize_up()
        {
            if (num_items < cap)
            {
                return;
            }

            assert(cap * (1 + RESIZE_FACTOR) > num_items);

            resize(nat_t(cap * (1 + RESIZE_FACTOR)));
        }

        void resize_down()
        {
            if (num_items > cap * RESIZE_FACTOR or cap == MIN_SIZE)
            {
                return;
            }

            assert(cap * (1 - RESIZE_FACTOR) > num_items);

            resize(nat_t(cap * (1 - RESIZE_FACTOR)));
        }

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

        DynQueue()
            : cap(MIN_SIZE),
              num_items(0),
              f(0),
              r(MIN_SIZE - 1),
              array_ptr(allocate(MIN_SIZE))
        {
            // empty
        }

        /** `num_items` starts at 0 (not `q.num_items`) and is incremented
            one element at a time by copy_queue() as each copy actually
            succeeds — see DynArray's copy constructor for why (exception
            safety: if some element's copy constructor throws partway
            through, `num_items` still accurately reflects how many were
            constructed, so ~DynQueue() destroys exactly those). */
        DynQueue(const DynQueue& q)
            : cap(q.cap),
              num_items(0),
              f(0),
              r(cap - 1),
              array_ptr(allocate(q.cap))
        {
            copy_queue(q);
        }

        DynQueue(DynQueue&& q)
            : cap(0), num_items(0), f(0), r(0), array_ptr(nullptr)
        {
            swap(q);
        }

        ~DynQueue()
        {
            destroy_all();
            deallocate(array_ptr);
        }

        DynQueue& operator=(const DynQueue& q)
        {
            if (this == &q)
            {
                return *this;
            }

            DynQueue tmp(q);
            swap(tmp);

            return *this;
        }

        DynQueue& operator=(DynQueue&& q)
        {
            swap(q);
            return *this;
        }

        bool is_empty() const
        {
            return num_items == 0;
        }

        nat_t size() const
        {
            return num_items;
        }

        nat_t get_capacity() const
        {
            return cap;
        }

        void clear()
        {
            destroy_all();
            num_items = 0;

            if (cap != MIN_SIZE)
            {
                deallocate(array_ptr);
                cap = MIN_SIZE;
                array_ptr = allocate(cap);
            }

            f = 0;
            r = MIN_SIZE - 1;
        }

        T& front()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return array_ptr[f];
        }

        const T& front() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return array_ptr[f];
        }

        T& rear()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return array_ptr[r];
        }

        const T& rear() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return array_ptr[r];
        }

        /** Always placement-constructs into `array_ptr[r]`, never
            assigns: resize_up() keeps capacity strictly ahead of
            num_items after every put(), so the slot about to be written
            is always raw (never a leftover live object from an earlier
            get()). */
        T& put(const T& item)
        {
            r = (r + 1) % cap;
            new (array_ptr + r) T(item);
            ++num_items;
            resize_up();
            return array_ptr[r];
        }

        T& put(T&& item)
        {
            r = (r + 1) % cap;
            new (array_ptr + r) T(std::move(item));
            ++num_items;
            resize_up();
            return array_ptr[r];
        }

        T get()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            T ret_val = std::move(array_ptr[f]);
            array_ptr[f].~T();
            f = (f + 1) % cap;
            --num_items;
            resize_down();
            return ret_val;
        }
    };

    template <typename T>
    void DynQueue<T>::copy_queue(const DynQueue& q)
    {
        nat_t ii = q.f;

        for (; num_items < q.num_items; ++num_items)
        {
            new (array_ptr + num_items) T(q.array_ptr[ii]);
            ii = (ii + 1) % q.cap;
        }

        f = 0;
        r = num_items - 1;
    }

    template <typename T>
    void DynQueue<T>::resize(nat_t sz)
    {
        if (sz < MIN_SIZE)
        {
            sz = MIN_SIZE;
        }

        T* new_ptr = allocate(sz);

        nat_t ii = f;

        for (nat_t i = 0; i < num_items; ++i)
        {
            new (new_ptr + i) T(std::move(array_ptr[ii]));
            array_ptr[ii].~T();
            ii = (ii + 1) % cap;
        }

        deallocate(array_ptr);
        array_ptr = new_ptr;
        cap = sz;

        f = 0;
        r = num_items - 1;
    }

    template <typename T>
    class ListQueue : private SLList<T>
    {
        using BaseList = SLList<T>;

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

        ListQueue() : BaseList()
        {
            // empty
        }

        ListQueue(const ListQueue& q) : BaseList(q)
        {
            // empty
        }

        ListQueue(ListQueue&& q) : BaseList(std::forward<ListQueue<T>>(q))
        {
            // empty
        }

        ListQueue& operator=(const ListQueue& q)
        {
            if (this == &q)
            {
                return *this;
            }

            (BaseList&)* this = q;
            return *this;
        }

        ListQueue& operator=(ListQueue&& q)
        {
            BaseList::swap(q);
            return *this;
        }

        bool is_empty() const
        {
            return BaseList::is_empty();
        }

        nat_t size() const
        {
            return BaseList::size();
        }

        void clear()
        {
            BaseList::clear();
        }

        T& front()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return BaseList::get_first();
        }

        const T& front() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return BaseList::get_first();
        }

        T& rear()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return BaseList::get_last();
        }

        const T& rear() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return BaseList::get_last();
        }

        T& put(const T& item)
        {
            return BaseList::append(item);
        }

        T& put(T&& item)
        {
            return BaseList::append(std::forward<T>(item));
        }

        T get()
        {
            if (is_empty())
            {
                throw std::underflow_error("Queue is empty");
            }

            return BaseList::remove_first();
        }
    };

    template <typename T, class Queue = ListQueue<T>>
    class ConcurrentQueue
    {
        std::mutex mtx;
        std::condition_variable cond_var;
        Queue queue;

    public:
        /** Unlike every other queue in this header, put() returns void
            instead of a reference to the inserted item. The other queues
            (FixedQueue, DynQueue, ListQueue) can safely hand back a
            reference because they are single-threaded: the reference stays
            valid for as long as the caller holds on to the queue. Here, the
            lock protecting `queue` is released as soon as put() returns —
            that is the whole point of a lock_guard scoped to the function
            body — so a reference into the queue's internal storage (e.g. a
            Queue node) would be exposed to the caller with no lock held.
            A consumer thread blocked in get() can wake up, dequeue, and free
            that exact node before the producer ever touches the returned
            reference: a cross-thread use-after-free. Returning void keeps
            the API honest about what it can actually guarantee. */
        void put(const T& item)
        {
            std::lock_guard<std::mutex> lck(mtx);
            queue.put(item);
            cond_var.notify_one();
        }

        void put(T&& item)
        {
            std::lock_guard<std::mutex> lck(mtx);
            queue.put(std::move(item));
            cond_var.notify_one();
        }

        T get()
        {
            std::unique_lock<std::mutex> lck(mtx);
            cond_var.wait(lck, [this] { return !queue.is_empty(); });
            return queue.get();
        }

        nat_t size() const
        {
            std::lock_guard<std::mutex> lck(const_cast<std::mutex&>(mtx));
            return queue.size();
        }

        bool is_empty() const
        {
            std::lock_guard<std::mutex> lck(const_cast<std::mutex&>(mtx));
            return queue.is_empty();
        }
    };

} // end namespace Designar
