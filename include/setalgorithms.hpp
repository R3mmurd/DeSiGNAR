/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file setalgorithms.hpp
    @brief CRTP mixin that gives any set-like container the standard
    set-algebra operations for free.
    @ingroup Algorithms
*/

#pragma once

#include <types.hpp>
#include <typetraits.hpp>

namespace Designar
{
    template <class T>
    class SLList;

    /** A CRTP mixin analogous to ContainerAlgorithms
        (containeralgorithms.hpp): a set-like container inherits from
        `SetAlgorithms<SetType, Key>` (passing itself as `SetType`) to
        gain the standard set-algebra operations — membership testing,
        union, intersection, difference, cartesian product — without
        implementing any of them itself. The private `me()` /
        `const_me()` helpers downcast `this` back to `SetType*` /
        `const SetType*` so the operations below can call the
        container's own `search`, `size`, `for_each`, and `filter`.

        Every set-like container in the library — ArraySet, the
        hash-table-backed sets, the tree-backed sets — derives from this
        class and thereby shares one implementation of these operations
        instead of each reimplementing them. */
    template <class SetType, typename Key>
    class SetAlgorithms
    {

        SetType& me()
        {
            return static_cast<SetType*>(this);
        }

        const SetType& const_me() const
        {
            return *static_cast<const SetType*>(this);
        }

    public:
        /** @brief Membership test. */
        bool contains(const Key& k) const
        {
            return const_me().search(k) != nullptr;
        }

        /** @brief Alias for contains(). */
        bool has(const Key& k) const
        {
            return contains(k);
        }

        /** @brief Set union: elements belonging to either s1 or s2. */
        static SetType join(const SetType& s1, const SetType& s2)
        {
            SetType ret_val = s1;

            s2.for_each([&ret_val](const Key& item) { ret_val.append(item); });

            return ret_val;
        }

        SetType join(const SetType& s) const
        {
            return join(const_me(), s);
        }

        /** @brief Set intersection: elements belonging to both s1 and
            s2. */
        static SetType intersect(const SetType& s1, const SetType& s2)
        {
            if (s1.size() < s2.size())
            {
                return s1.template filter<SetType>(
                    [&s2](const Key& item) { return s2.contains(item); });
            }

            return s2.template filter<SetType>([&s1](const Key& item)
                                               { return s1.contains(item); });
        }

        SetType intersect(const SetType& s) const
        {
            return intersect(const_me(), s);
        }

        /** @brief Set difference: elements of s1 not present in s2. */
        static SetType difference(const SetType& s1, const SetType& s2)
        {
            return s1.template filter<SetType>([&s2](const Key& item)
                                               { return !s2.contains(item); });
        }

        SetType difference(const SetType& s) const
        {
            return difference(const_me(), s);
        }

        /** @brief Cartesian product: every pair combining an element of
            s1 with an element of s2. */
        template <class SetType2 = SetType>
        static SLList<std::pair<Key, typename SetType2::KeyType>>
        cartesian_product(const SetType& s1, const SetType2& s2)
        {
            using Key2 = typename SetType2::KeyType;

            SLList<std::pair<Key, Key2>> ret_val;

            s1.for_each(
                [&ret_val, &s2](const Key& item1)
                {
                    s2.for_each(
                        [&ret_val, &item1](const Key2& item2)
                        { ret_val.append(std::make_pair(item1, item2)); });
                });

            return ret_val;
        }

        template <class SetType2 = SetType>
        SLList<std::pair<Key, typename SetType2::KeyType>>
        cartesian_product(const SetType2& s) const
        {
            return cartesian_product<SetType2>(const_me(), s);
        }
    };

} // end namespace Designar
