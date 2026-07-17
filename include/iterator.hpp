/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <iterator>

#include <types.hpp>

namespace Designar
{
    template <bool B, typename T, typename F>
    using RetType = typename std::conditional<B, T, F>::type;

    /** The nested typedefs std::iterator_traits<Iterator> looks for, so any
        concrete iterator built on this CRTP hierarchy works directly with
        `<algorithm>`/`<numeric>` (std::find, std::sort, std::accumulate,
        std::distance, etc.) without the caller having to do anything extra.
        iterator_category itself is declared separately at each of
        ForwardIterator/BidirectionalIterator/RandomAccessIterator below
        (each hides this class's declaration for any concrete iterator
        that derives from it), since that is the one trait that actually
        varies by capability; value_type/difference_type/pointer/reference
        only depend on T and RET_CPY, which are the same at every level. */
    template <class Iterator, typename T, bool RET_CPY = false>
    class BasicIterator
    {
    protected:
        Iterator& me()
        {
            return *static_cast<Iterator*>(this);
        }

        const Iterator& const_me() const
        {
            return *static_cast<const Iterator*>(this);
        }

        /** Whether *this (the iterator object) is const-qualified is a
            completely different question from whether what it points to is
            mutable — every mutable STL iterator (std::vector<T>::iterator
            included) keeps operator*()/operator->()/operator+ etc. as const
            member functions precisely so a `const Iterator` local variable,
            or a `const Iterator&` parameter (both idioms <algorithm>
            implementations use routinely, e.g. libc++/MSVC STL's sort()) can
            still dereference-and-write through it. get_current() itself is
            genuinely overloaded on constness (returning T& vs const T&) for
            when *this legitimately is a const *container*'s begin()/end()
            result — but that path already returns a plain (non-const)
            Iterator by value, never a const-qualified Iterator object, so
            stripping the qualification here to reach the mutable overload of
            get_current() from a const member function is safe: nothing about
            this class's actual state is being violated, only the two
            distinct meanings of "const" that C++ layers onto the same
            keyword. */
        Iterator& mutable_me() const
        {
            return const_cast<Iterator&>(const_me());
        }

    public:
        using iterator_category = std::input_iterator_tag;
        using value_type = T;
        using difference_type = int_t;
        using pointer = T*;
        using reference = RetType<RET_CPY, T, T&>;

        bool has_curr() const
        {
            return const_me().has_current();
        }

        RetType<RET_CPY, T, T&> get_curr() const
        {
            return mutable_me().get_current();
        }

        RetType<RET_CPY, T, T&> operator*() const
        {
            return mutable_me().get_current();
        }

        T* operator->() const
        {
            return &mutable_me().get_current();
        }

        bool operator==(const Iterator& it) const
        {
            return const_me().get_location() == it.get_location();
        }

        bool operator!=(const Iterator& it) const
        {
            return const_me().get_location() != it.get_location();
        }
    };

    template <class Iterator, typename T, bool RET_CPY = false>
    class ForwardIterator : public BasicIterator<Iterator, T, RET_CPY>
    {
        using Base = BasicIterator<Iterator, T, RET_CPY>;

    public:
        using iterator_category = std::forward_iterator_tag;

        Iterator& operator++()
        {
            Base::me().next();
            return Base::me();
        }

        Iterator operator++(int)
        {
            Iterator ret_val = Base::me();
            Base::me().next();
            return ret_val;
        }
    };

    template <class Iterator, typename T, bool RET_CPY = false>
    class BidirectionalIterator : public ForwardIterator<Iterator, T, RET_CPY>
    {
        using Base = ForwardIterator<Iterator, T, RET_CPY>;

    public:
        using iterator_category = std::bidirectional_iterator_tag;

        Iterator& operator--()
        {
            Base::me().prev();
            return Base::me();
        }

        Iterator operator--(int)
        {
            Iterator ret_val = Base::me();
            Base::me().prev();
            return ret_val;
        }
    };

    template <class Iterator, typename T, bool RET_CPY = false>
    class RandomAccessIterator
        : public BidirectionalIterator<Iterator, T, RET_CPY>
    {
        using Base = BidirectionalIterator<Iterator, T, RET_CPY>;

    public:
        using iterator_category = std::random_access_iterator_tag;
        using difference_type = typename Base::difference_type;

        /** `a[n]` must be equivalent to `*(a + n)` — it must not move `a`
            itself — so this works on a throwaway copy rather than moving
            *this via move_to(), the way an earlier version of this operator
            did. */
        RetType<RET_CPY, T, T&> operator[](nat_t i) const
        {
            Iterator it = Base::const_me();
            it.move_to(i);
            return it.get_current();
        }

        Iterator& operator+=(nat_t p)
        {
            Base::me().next_n(p);
            return Base::me();
        }

        /** const: makes a copy of *this and advances the copy, so — unlike
            operator+=() above, which genuinely must mutate *this and so
            must stay non-const — this never needs *this to be non-const. */
        Iterator operator+(nat_t p) const
        {
            Iterator it = Base::const_me();
            it += p;
            return it;
        }

        Iterator& operator-=(nat_t p)
        {
            Base::me().prev_n(p);
            return Base::me();
        }

        /** @see operator+() above — same reasoning for why this is const. */
        Iterator operator-(nat_t p) const
        {
            Iterator it = Base::const_me();
            it -= p;
            return it;
        }

        /** Distance between two iterators into the same sequence — what
            std::distance() (and, in turn, algorithms like std::sort() that
            are specialized for random-access iterators) needs in order to
            compute the O(1) distance a forward/bidirectional iterator would
            otherwise have to walk to find. Signed, since `it - other` is
            negative when `it` comes before `other`. */
        difference_type operator-(const Iterator& it) const
        {
            return difference_type(Base::const_me().get_location()) -
                   difference_type(it.get_location());
        }

        bool operator<(const Iterator& it) const
        {
            return Base::const_me().get_location() < it.get_location();
        }

        bool operator<=(const Iterator& it) const
        {
            return Base::const_me().get_location() <= it.get_location();
        }

        bool operator>(const Iterator& it) const
        {
            return Base::const_me().get_location() > it.get_location();
        }

        bool operator>=(const Iterator& it) const
        {
            return Base::const_me().get_location() >= it.get_location();
        }
    };

    template <class It>
    It next_it(const It& it)
    {
        It ret_val = it;
        ret_val.next();
        return ret_val;
    }

    template <class It>
    It next_it_c(const It& it)
    {
        It ret_val = it;
        ret_val.next();

        if (!ret_val.has_current())
        {
            ret_val.reset_first();
        }

        return ret_val;
    }

    template <class It>
    It prev_it(const It& it)
    {
        It ret_val = it;
        ret_val.prev();
        return ret_val;
    }
} // end namespace Designar
