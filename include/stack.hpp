/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file stack.hpp
    @brief Three stack (LIFO) implementations with different storage
    strategies: fixed-capacity array, growable array, and linked list.
    @ingroup DataStructures
*/

#pragma once

#include <array.hpp>
#include <list.hpp>

namespace Designar
{

    /** A stack backed by a fixed-size C array embedded directly in the
        object, with capacity `CAP` fixed at compile time. It performs no
        heap allocation at all, so it is the cheapest and most predictable
        option when an upper bound on the number of items is known in
        advance, but `push` throws once that hard ceiling is reached
        instead of growing like DynStack or ListStack. */
    template <typename T, nat_t CAP = 100>
    class FixedStack
    {
        T array[CAP];
        nat_t num_items;

        void copy_stack(const FixedStack&);

        void swap(FixedStack& s)
        {
            std::swap(array, s.array);
            std::swap(num_items, s.num_items);
        }

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

        FixedStack() : num_items(0)
        {
            // empty
        }

        FixedStack(const FixedStack& s) : num_items(s.num_items)
        {
            copy_stack(s);
        }

        FixedStack(FixedStack&& s) : FixedStack()
        {
            swap(s);
        }

        FixedStack& operator=(const FixedStack& s)
        {
            if (this == &s)
            {
                return *this;
            }

            num_items = s.num_items;
            copy_stack(s);

            return *this;
        }

        FixedStack& operator=(FixedStack&& s)
        {
            swap(s);
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

        void clear()
        {
            num_items = 0;
        }

        nat_t size() const
        {
            return num_items;
        }

        nat_t get_capacity() const
        {
            return CAP;
        }

        T& push(const T& item)
        {
            if (is_full())
            {
                throw std::overflow_error("Stack is full");
            }

            array[num_items++] = item;
            return array[num_items - 1];
        }

        T& push(T&& item)
        {
            if (is_full())
            {
                throw std::overflow_error("Stack is full");
            }

            array[num_items++] = std::move(item);
            return array[num_items - 1];
        }

        T& top()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return array[num_items - 1];
        }

        const T& top() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return array[num_items - 1];
        }

        T& base()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return array[0];
        }

        const T& base() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return array[0];
        }

        T pop()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return std::move(array[--num_items]);
        }

        void popn(nat_t n)
        {
            if (n > num_items)
            {
                throw std::underflow_error("n is to large");
            }

            num_items -= n;
        }
    };

    template <typename T, nat_t CAP>
    void FixedStack<T, CAP>::copy_stack(const FixedStack& s)
    {
        for (nat_t i = 0; i < num_items; ++i)
        {
            array[i] = s.array[i];
        }
    }

    /** A stack backed by a DynArray<T> that grows on demand: unlike
        FixedStack there is no compile-time capacity ceiling, and unlike
        ListStack there is no per-element heap allocation, giving push/pop
        amortized O(1) cost and the good cache locality of contiguous
        storage. The tradeoff is that occasional reallocation-and-copy
        steps triggered by growth can make an individual `push` call more
        expensive than usual. */
    template <typename T>
    class DynStack : private DynArray<T>
    {
        using BaseArray = DynArray<T>;

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

        DynStack() : BaseArray()
        {
            // empty
        }

        DynStack(nat_t cap) : BaseArray(cap)
        {
            // empty
        }

        DynStack(const DynStack& s) : BaseArray(s)
        {
            // empty
        }

        DynStack(DynStack&& s) : BaseArray(std::forward<DynStack<T>>(s))
        {
            // empty
        }

        DynStack& operator=(const DynStack& s)
        {
            if (this == &s)
            {
                return *this;
            }

            (BaseArray&)* this = s;
            return *this;
        }

        DynStack& operator=(DynStack&& s)
        {
            BaseArray::swap(s);
            return *this;
        }

        bool is_empty() const
        {
            return BaseArray::is_empty();
        }

        void clear()
        {
            BaseArray::clear();
        }

        nat_t size() const
        {
            return BaseArray::size();
        }

        T& push(const T& item)
        {
            BaseArray::append(item);
            return BaseArray::get_last();
        }

        T& push(T&& item)
        {
            BaseArray::append(std::forward<T>(item));
            return BaseArray::get_last();
        }

        T& top()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseArray::get_last();
        }

        const T& top() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseArray::get_last();
        }

        T& base()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseArray::get_first();
        }

        const T& base() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseArray::get_first();
        }

        T pop()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseArray::remove_last();
        }

        void popn(nat_t);
    };

    template <typename T>
    void DynStack<T>::popn(nat_t n)
    {
        if (n > size())
        {
            throw std::underflow_error("n is to large");
        }

        while (n-- > 0)
        {
            pop();
        }
    }

    /** A stack backed by a singly linked list (SLList<T>): every push
        allocates one new node and every pop frees one, so there is no
        capacity limit and no reallocation-driven latency spike the way
        DynStack occasionally has — push/pop are always O(1), not just
        amortized. The cost is a heap allocation per element and worse
        cache locality than the contiguous storage used by FixedStack and
        DynStack. */
    template <typename T>
    class ListStack : private SLList<T>
    {
        using BaseList = SLList<T>;

    public:
        using ItemType = T;
        using KeyType = T;
        using DataType = T;
        using ValueType = T;
        using SizeType = nat_t;

        ListStack() : BaseList()
        {
            // empty
        }

        ListStack(const ListStack& s) : BaseList(s)
        {
            // empty
        }

        ListStack(ListStack&& s) : BaseList(std::forward<ListStack<T>>(s))
        {
            // empty
        }

        ListStack& operator=(const ListStack& s)
        {
            if (this == &s)
            {
                return *this;
            }

            (BaseList&)* this = s;
            return *this;
        }

        ListStack& operator=(ListStack&& s)
        {
            BaseList::swap(s);
            return *this;
        }

        bool is_empty() const
        {
            return BaseList::is_empty();
        }

        void clear()
        {
            BaseList::clear();
        }

        nat_t size() const
        {
            return BaseList::size();
        }

        T& push(const T& item)
        {
            return BaseList::insert(item);
        }

        T& push(T&& item)
        {
            return BaseList::insert(std::forward<T>(item));
        }

        T& top()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseList::get_first();
        }

        const T& top() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseList::get_first();
        }

        T& base()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseList::get_last();
        }

        const T& base() const
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseList::get_last();
        }

        T pop()
        {
            if (is_empty())
            {
                throw std::underflow_error("Stack is empty");
            }

            return BaseList::remove_first();
        }

        void popn(nat_t);
    };

    template <typename T>
    void ListStack<T>::popn(nat_t n)
    {
        if (n > size())
        {
            throw std::underflow_error("n is to large");
        }

        while (n-- > 0)
        {
            pop();
        }
    }

} // end namespace Designar
