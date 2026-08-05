/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file deque.hpp
    @brief FixedDeque/DynDeque: a double-ended queue — push/pop at both
    ends — built on the exact same circular-buffer technique as
    FixedQueue/DynQueue (queue.hpp), just allowing the front index to
    move backward (wrapping) as well as the rear index to move forward.
    @ingroup DataStructures
*/

#pragma once

#include <array.hpp>

#include <new>

namespace Designar
{

    template <typename T, nat_t CAP = 100>
    class FixedDeque
    {
        T array[CAP];
        nat_t num_items;
        nat_t f;
        nat_t r;

        void copy_deque(const FixedDeque&);

        void swap(FixedDeque& q)
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

        FixedDeque() : num_items(0), f(0), r(CAP - 1)
        {
            // empty
        }

        FixedDeque(const FixedDeque& q) : num_items(q.num_items)
        {
            copy_deque(q);
        }

        FixedDeque(FixedDeque&& q) : FixedDeque()
        {
            swap(q);
        }

        FixedDeque& operator=(const FixedDeque& q)
        {
            if (this == &q)
            {
                return *this;
            }

            num_items = q.num_items;
            f = q.f;
            r = q.r;
            copy_deque(q);

            return *this;
        }

        FixedDeque& operator=(FixedDeque&& q)
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
                throw std::underflow_error("Deque is empty");
            }

            return array[f];
        }

        const T& front() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            return array[f];
        }

        T& back()
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            return array[r];
        }

        const T& back() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            return array[r];
        }

        T& push_back(const T& item)
        {
            if (is_full())
            {
                throw std::overflow_error("Deque is full");
            }

            r = (r + 1) % CAP;
            array[r] = item;
            ++num_items;
            return array[r];
        }

        T& push_back(T&& item)
        {
            if (is_full())
            {
                throw std::overflow_error("Deque is full");
            }

            r = (r + 1) % CAP;
            array[r] = std::move(item);
            ++num_items;
            return array[r];
        }

        T& push_front(const T& item)
        {
            if (is_full())
            {
                throw std::overflow_error("Deque is full");
            }

            f = (f + CAP - 1) % CAP;
            array[f] = item;
            ++num_items;
            return array[f];
        }

        T& push_front(T&& item)
        {
            if (is_full())
            {
                throw std::overflow_error("Deque is full");
            }

            f = (f + CAP - 1) % CAP;
            array[f] = std::move(item);
            ++num_items;
            return array[f];
        }

        T pop_back()
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            T ret_val = std::move(array[r]);
            r = (r + CAP - 1) % CAP;
            --num_items;
            return ret_val;
        }

        T pop_front()
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            T ret_val = std::move(array[f]);
            f = (f + 1) % CAP;
            --num_items;
            return ret_val;
        }
    };

    template <typename T, nat_t CAP>
    void FixedDeque<T, CAP>::copy_deque(const FixedDeque& q)
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

    /** Owns its storage directly, exactly like DynQueue — see that
        class's own doc comment (queue.hpp) for why: `T` need not be
        default-constructible or copy-assignable, only move/copy-
        constructible. `push_front`/`push_back` both placement-construct
        into a slot `resize_up()` has already guaranteed is raw; `pop_
        front`/`pop_back` destroy the slot they vacate before `resize_
        down()` runs. */
    template <typename T>
    class DynDeque
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

        void swap(DynDeque& q)
        {
            std::swap(cap, q.cap);
            std::swap(num_items, q.num_items);
            std::swap(f, q.f);
            std::swap(r, q.r);
            std::swap(array_ptr, q.array_ptr);
        }

        void copy_deque(const DynDeque&);

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

        DynDeque()
            : cap(MIN_SIZE),
              num_items(0),
              f(0),
              r(MIN_SIZE - 1),
              array_ptr(allocate(MIN_SIZE))
        {
            // empty
        }

        DynDeque(const DynDeque& q)
            : cap(q.cap),
              num_items(0),
              f(0),
              r(cap - 1),
              array_ptr(allocate(q.cap))
        {
            copy_deque(q);
        }

        DynDeque(DynDeque&& q)
            : cap(0), num_items(0), f(0), r(0), array_ptr(nullptr)
        {
            swap(q);
        }

        ~DynDeque()
        {
            destroy_all();
            deallocate(array_ptr);
        }

        DynDeque& operator=(const DynDeque& q)
        {
            if (this == &q)
            {
                return *this;
            }

            DynDeque tmp(q);
            swap(tmp);

            return *this;
        }

        DynDeque& operator=(DynDeque&& q)
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
                throw std::underflow_error("Deque is empty");
            }

            return array_ptr[f];
        }

        const T& front() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            return array_ptr[f];
        }

        T& back()
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            return array_ptr[r];
        }

        const T& back() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            return array_ptr[r];
        }

        T& push_back(const T& item)
        {
            r = (r + 1) % cap;
            new (array_ptr + r) T(item);
            ++num_items;
            resize_up();
            return array_ptr[r];
        }

        T& push_back(T&& item)
        {
            r = (r + 1) % cap;
            new (array_ptr + r) T(std::move(item));
            ++num_items;
            resize_up();
            return array_ptr[r];
        }

        T& push_front(const T& item)
        {
            f = (f + cap - 1) % cap;
            new (array_ptr + f) T(item);
            ++num_items;
            resize_up();
            return array_ptr[f];
        }

        T& push_front(T&& item)
        {
            f = (f + cap - 1) % cap;
            new (array_ptr + f) T(std::move(item));
            ++num_items;
            resize_up();
            return array_ptr[f];
        }

        T pop_back()
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
            }

            T ret_val = std::move(array_ptr[r]);
            array_ptr[r].~T();
            r = (r + cap - 1) % cap;
            --num_items;
            resize_down();
            return ret_val;
        }

        T pop_front()
        {
            if (is_empty())
            {
                throw std::underflow_error("Deque is empty");
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
    void DynDeque<T>::copy_deque(const DynDeque& q)
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
    void DynDeque<T>::resize(nat_t sz)
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

} // end namespace Designar
