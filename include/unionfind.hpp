/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file unionfind.hpp
    @brief DisjointSet: a union-find / disjoint-set-union data structure
    with union-by-rank and path halving, generic over any hashable Key
    type (rather than the classic textbook presentation over plain
    integer indices 0..n-1).
    @ingroup DataStructures
*/

#pragma once

#include <map.hpp>

namespace Designar
{
    /** Maintains a partition of a universe of keys into disjoint sets,
        supporting near-constant-amortized-time make_set()/find()/
        union_sets() via the classic union-by-rank + path-halving
        combination (either alone gives O(lg n) amortized; together they
        give the (inverse-Ackermann) O(alpha(n)) bound covered in every
        algorithms course).

        Unlike the textbook presentation (which operates on plain integer
        indices 0..n-1 and leaves mapping application-level identifiers to
        indices as an exercise for the reader), this maps arbitrary Key
        values to internal indices itself via a HashMap, so it can be used
        directly with whatever domain the caller is partitioning (graph
        nodes, string identifiers, ...) — the same spirit as Kruskal's
        algorithm in graphalgorithms.hpp, which needs exactly this
        structure internally (there, over Node<GT>* pointers) but builds
        it in an ad hoc way rather than exposing it as a reusable type. */
    template <typename Key, class Cmp = std::equal_to<Key>>
    class DisjointSet
    {
        HashMap<Key, nat_t, Cmp> index_of;
        DynArray<nat_t> parent;
        DynArray<nat_t> rnk;
        DynArray<Key> key_of;

        nat_t find_root(nat_t i)
        {
            while (parent[i] != i)
            {
                parent[i] = parent[parent[i]]; // path halving
                i = parent[i];
            }

            return i;
        }

        nat_t index_of_existing(const Key& k) const
        {
            const nat_t* idx = index_of.search(k);

            if (idx == nullptr)
            {
                throw std::domain_error(
                    "Key does not belong to this DisjointSet");
            }

            return *idx;
        }

    public:
        DisjointSet() = default;

        /** Adds `k` as a new singleton set, if it is not already present
            (present keys are left untouched, matching the classic
            make_set() being a no-op on an already-known element). */
        void make_set(const Key& k)
        {
            if (index_of.search(k) != nullptr)
            {
                return;
            }

            nat_t idx = parent.size();
            index_of.insert(k, idx);
            parent.append(idx);
            rnk.append(0);
            key_of.append(k);
        }

        bool contains(const Key& k) const
        {
            return index_of.search(k) != nullptr;
        }

        nat_t num_keys() const
        {
            return parent.size();
        }

        /** Returns the representative key of the set containing `k`. Two
            keys are in the same set exactly when find() returns the same
            representative for both (see same_set()). */
        const Key& find(const Key& k)
        {
            return key_of[find_root(index_of_existing(k))];
        }

        bool same_set(const Key& a, const Key& b)
        {
            return find_root(index_of_existing(a)) ==
                   find_root(index_of_existing(b));
        }

        /** Merges the sets containing `a` and `b` (a no-op if they are
            already the same set). The smaller-rank tree's root is attached
            under the larger-rank tree's root, which is what keeps every
            tree's height at O(lg n) even without path compression's help. */
        void union_sets(const Key& a, const Key& b)
        {
            nat_t ra = find_root(index_of_existing(a));
            nat_t rb = find_root(index_of_existing(b));

            if (ra == rb)
            {
                return;
            }

            if (rnk[ra] < rnk[rb])
            {
                std::swap(ra, rb);
            }

            parent[rb] = ra;

            if (rnk[ra] == rnk[rb])
            {
                ++rnk[ra];
            }
        }
    };

} // end namespace Designar
