/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file relation.hpp
    @brief A union-find (disjoint-set) structure for equivalence relations
    over indices or arbitrary values.
    @ingroup DataStructures
*/

#pragma once

#include <map.hpp>

namespace Designar
{

    /** A classic union-find (disjoint-set) data structure implementing an
        equivalence relation over the indices `0` to `n - 1`: `join(p, q)`
        merges the blocks containing `p` and `q`, and `are_connected(p, q)`
        reports whether they currently belong to the same block. Union is
        performed by size (the root of the smaller block is attached to the
        root of the larger one, tracked in `sz`), which keeps the resulting
        trees shallow; `find` walks parent links in `id` up to the root
        without path compression, so lookups cost O(log n) rather than the
        amortized O(1) achieved when both optimizations are combined. */
    class EquivalenceRelation
    {
        FixedArray<nat_t> id;
        FixedArray<nat_t> sz;
        nat_t num_blocks;

        nat_t find(nat_t) const;

    public:
        EquivalenceRelation(nat_t);

        void join(nat_t, nat_t);

        bool are_connected(nat_t, nat_t) const;

        nat_t get_num_blocks() const;

        nat_t size() const;
    };

    /** A thin adapter over EquivalenceRelation that lets callers build
        equivalence classes over arbitrary values of type `T` instead of
        raw `nat_t` indices. It keeps a hash table (`table`, a
        `HashMap<T, nat_t, Equal>`) mapping each distinct `T` value seen so
        far to the index EquivalenceRelation assigned it; `search_or_insert`
        looks up that mapping, inserting a fresh index the first time a
        value appears (throwing `std::overflow_error` once the underlying
        relation's fixed capacity is exhausted). `join` and `are_connected`
        translate their `T` arguments to indices through this table and
        then delegate to the base class. Any type usable as a `HashMap` key
        works as `T`, so `Equal` defaults to `std::equal_to<T>`. */
    template <typename T, class Equal = std::equal_to<T>>
    class TRelation : public EquivalenceRelation
    {
        using Table = HashMap<T, nat_t, Equal>;

        mutable Table table;

        nat_t search_or_insert(const T& item) const
        {
            nat_t* result = table.search(item);

            if (result == nullptr)
            {
                if (table.size() == EquivalenceRelation::size())
                {
                    throw std::overflow_error("Cannot add a new item");
                }

                result = table.insert(item, table.size());
            }

            return *result;
        }

        void init_from_list(const std::initializer_list<T>&);

    public:
        TRelation(nat_t n, Equal& eq)
            : EquivalenceRelation(n), table(n * 1.25, eq)
        {
            // empty
        }

        TRelation(nat_t n, Equal&& eq = Equal())
            : EquivalenceRelation(n), table(n * 1.25, std::forward<Equal>(eq))
        {
            // empty
        }

        TRelation(const std::initializer_list<T>& list, Equal& eq)
            : EquivalenceRelation(list.size()), table(list.size() * 1.25, eq)
        {
            init_from_list(list);
        }

        TRelation(const std::initializer_list<T>& list, Equal&& eq = Equal())
            : EquivalenceRelation(list.size()),
              table(list.size() * 1.25, std::forward<Equal>(eq))
        {
            init_from_list(list);
        }

        void join(const T& tp, const T& tq)
        {
            nat_t p = search_or_insert(tp);
            nat_t q = search_or_insert(tq);
            EquivalenceRelation::join(p, q);
        }

        bool are_connected(const T& tp, const T& tq) const
        {
            nat_t p, q;

            try
            {
                p = search_or_insert(tp);
                q = search_or_insert(tq);
            }
            catch (...)
            {
                throw std::domain_error(
                    "Both elements must belong to relation");
            }

            return EquivalenceRelation::are_connected(p, q);
        }
    };

    template <typename T, class Equal>
    void TRelation<T, Equal>::init_from_list(const std::initializer_list<T>& l)
    {
        nat_t i = 0;

        for (const T& item : l)
        {
            table.insert(item, i++);
        }
    }

} // end namespace Designar
