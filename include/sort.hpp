/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file sort.hpp
    @brief Searching and sorting: binary/sequential search, and the
    classic comparison sorts (insertion, selection, bubble, shell,
    merge, quick, heap) plus the non-comparison counting/radix sorts,
    on both arrays and linked lists where each algorithm's own shape
    makes that a natural fit.
    @ingroup Sorting
*/

#pragma once

#include <type_traits>

#include <array.hpp>
#include <list.hpp>
#include <typetraits.hpp>

namespace Designar
{

    template <typename T, template <typename> class ArrayType,
              class Cmp = std::less<T>>
    int_t binary_search(const ArrayType<T>&, const T&, int_t, int_t, Cmp&);

    template <typename T, template <typename> class ArrayType,
              class Cmp = std::less<T>>
    int_t sequential_search(const ArrayType<T>&, const T&, int_t, int_t, Cmp&);

    template <class ArrayType, class Cmp>
    void gapped_insertion_pass(ArrayType&, int_t, int_t, int_t, Cmp&);

    template <class ArrayType, class Cmp>
    void insertion_sort(ArrayType&, int_t, int_t, Cmp&);

    template <class ArrayType, class Cmp>
    void selection_sort(ArrayType&, int_t, int_t, Cmp&);

    template <class ArrayType, class Cmp>
    void bubble_sort(ArrayType&, int_t, int_t, Cmp&);

    template <class ArrayType, class Cmp>
    void shell_sort(ArrayType&, int_t, int_t, Cmp&);

    template <typename ArrayType, class Cmp>
    int_t partition(ArrayType&, int_t, int_t, Cmp&);

    template <typename ArrayType, class Cmp>
    void quicksort(ArrayType&, int_t, int_t, Cmp&);

    template <class ArrayType, class Cmp>
    void merge(ArrayType&, int_t, int_t, int_t, Cmp&);

    template <class ArrayType, class Cmp>
    void merge_sort(ArrayType&, int_t, int_t, Cmp&);

    template <class ArrayType, class Cmp>
    void sift_up(ArrayType&, int_t, nat_t, nat_t, Cmp&);

    template <class ArrayType, class Cmp>
    void sift_down(ArrayType&, int_t, nat_t, nat_t, Cmp&);

    template <class ArrayType, class Cmp>
    void heap_sort(ArrayType&, int_t, int_t, Cmp&);

    template <class ArrayType>
    void counting_sort(ArrayType&, int_t, int_t, nat_t);

    template <class ArrayType>
    void radix_sort(ArrayType&, int_t, int_t);

    template <typename T, class Cmp>
    std::tuple<NodeSLList<T>, typename NodeSLList<T>::Node*, NodeSLList<T>>
    partition(NodeSLList<T>&, Cmp&);

    template <typename T, class Cmp>
    void quicksort(NodeSLList<T>&, Cmp&);

    template <typename T, class Cmp>
    NodeSLList<T> merge(NodeSLList<T>&, NodeSLList<T>&, Cmp&);

    template <typename T, class Cmp>
    void merge_sort(NodeSLList<T>&, Cmp&);

    template <class Cmp>
    std::tuple<DL, DL*, DL> partition(DL&, Cmp&);

    template <class Cmp>
    void quicksort(DL&, Cmp&);

    template <class ArrayType>
    ArrayType reverse(const ArrayType&);

    template <class SRCL, class TGTL>
    TGTL reverse(const SRCL&);

    template <typename T, template <typename> class ArrayType, class Cmp>
    int_t binary_search(const ArrayType<T>& a, const T& k, int_t l, int_t r,
                        Cmp& cmp)
    {
        if (l > r)
        {
            return l;
        }

        int_t m = (l + r) / 2;

        if (cmp(k, a.at(m)))
        {
            return binary_search(a, k, l, m - 1, cmp);
        }
        else if (cmp(a.at(m), k))
        {
            return binary_search(a, k, m + 1, r, cmp);
        }

        return m;
    }

    template <typename T, template <typename> class ArrayType,
              class Cmp = std::less<T>>
    inline int_t binary_search(const ArrayType<T>& a, T& k, int_t l, int_t r,
                               Cmp&& cmp = Cmp())
    {
        return binary_search<T, ArrayType, Cmp>(a, k, l, r, cmp);
    }

    template <typename T, template <typename> class ArrayType,
              class Cmp = std::less<T>>
    inline int_t binary_search(const ArrayType<T>& a, const T& k, Cmp& cmp)
    {
        return binary_search<T, ArrayType, Cmp>(a, k, 0, a.size() - 1, cmp);
    }

    template <typename T, template <typename> class ArrayType,
              class Cmp = std::less<T>>
    inline int_t binary_search(const ArrayType<T>& a, const T& k,
                               Cmp&& cmp = Cmp())
    {
        return binary_search<T, ArrayType, Cmp>(a, k, cmp);
    }

    template <typename T, template <typename> class ArrayType, class Cmp>
    int_t sequential_search(const ArrayType<T>& a, const T& k, int_t l, int_t r,
                            Cmp& cmp)
    {
        int_t i = l;

        while (i <= r && !cmp(k, a.at(i)))
        {
            ++i;
        }

        return i;
    }

    template <typename T, template <typename> class ArrayType,
              class Cmp = std::less<T>>
    int_t sequential_search(const ArrayType<T>& a, const T& k, int_t l, int_t r,
                            Cmp&& cmp = Cmp())
    {
        return sequential_search<T, ArrayType, Cmp>(a, k, l, r, cmp);
    }

    template <typename T, template <typename> class ArrayType,
              class Cmp = std::less<T>>
    int_t sequential_search(const ArrayType<T>& a, const T& k, Cmp& cmp)
    {
        return sequential_search<T, ArrayType, Cmp>(a, k, 0, a.size() - 1, cmp);
    }

    template <typename T, template <typename> class ArrayType,
              class Cmp = std::less<T>>
    int_t sequential_search(const ArrayType<T>& a, const T& k,
                            Cmp&& cmp = Cmp())
    {
        return sequential_search<T, ArrayType, Cmp>(a, k, cmp);
    }

    /** The shared core of insertion_sort() and shell_sort(): moves a[i]
        leftward `gap` positions at a time until it reaches its correct
        place (per `cmp`) among the elements `gap` apart from it.
        insertion_sort() is exactly this with gap=1 — a single pass over
        immediate neighbors; shell_sort() runs this same pass once per
        gap in a decreasing sequence ending in 1, so its last pass is
        an ordinary insertion sort over an already-mostly-sorted array
        (the earlier, larger-gap passes having moved far-out-of-place
        elements most of the way home cheaply). */
    template <class ArrayType, class Cmp>
    void gapped_insertion_pass(ArrayType& a, int_t l, int_t r, int_t gap,
                               Cmp& cmp)
    {
        for (int_t i = l + gap; i <= r; ++i)
        {
            typename ArrayType::DataType data = std::move(a[i]);

            int_t j = i;

            for (; j >= l + gap && cmp(data, a[j - gap]); j -= gap)
            {
                a[j] = std::move(a[j - gap]);
            }

            a[j] = std::move(data);
        }
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void gapped_insertion_pass(ArrayType& a, int_t l, int_t r, int_t gap,
                                      Cmp&& cmp = Cmp())
    {
        gapped_insertion_pass<ArrayType, Cmp>(a, l, r, gap, cmp);
    }

    template <class ArrayType, class Cmp>
    void insertion_sort(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        gapped_insertion_pass(a, l, r, 1, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void insertion_sort(ArrayType& a, int_t l, int_t r,
                               Cmp&& cmp = Cmp())
    {
        insertion_sort<ArrayType, Cmp>(a, l, r, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void insertion_sort(ArrayType& a, int_t size, Cmp& cmp)
    {
        insertion_sort(a, 0, size - 1, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void insertion_sort(ArrayType& a, int_t size, Cmp&& cmp = Cmp())
    {
        insertion_sort<ArrayType, Cmp>(a, size, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void insertion_sort(ArrayType& a, Cmp& cmp)
    {
        insertion_sort(a, a.size(), cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void insertion_sort(ArrayType& a, Cmp&& cmp = Cmp())
    {
        insertion_sort<ArrayType, Cmp>(a, cmp);
    }

    /** O(n^2), and — unlike insertion_sort/bubble_sort — never adaptive
        (it always scans the whole unsorted remainder for a minimum, even
        on an already-sorted array), but it makes at most n-1 swaps total,
        the fewest of any of the O(n^2) sorts here — worth teaching
        alongside insertion_sort specifically for that contrast (fewest
        writes vs. fewest comparisons on nearly-sorted input). */
    template <class ArrayType, class Cmp>
    void selection_sort(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        for (int_t i = l; i < r; ++i)
        {
            int_t min_pos = i;

            for (int_t j = i + 1; j <= r; ++j)
            {
                if (cmp(a[j], a[min_pos]))
                {
                    min_pos = j;
                }
            }

            if (min_pos != i)
            {
                std::swap(a[i], a[min_pos]);
            }
        }
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void selection_sort(ArrayType& a, int_t l, int_t r,
                               Cmp&& cmp = Cmp())
    {
        selection_sort<ArrayType, Cmp>(a, l, r, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void selection_sort(ArrayType& a, int_t size, Cmp& cmp)
    {
        selection_sort(a, 0, size - 1, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void selection_sort(ArrayType& a, int_t size, Cmp&& cmp = Cmp())
    {
        selection_sort<ArrayType, Cmp>(a, size, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void selection_sort(ArrayType& a, Cmp& cmp)
    {
        selection_sort(a, a.size(), cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void selection_sort(ArrayType& a, Cmp&& cmp = Cmp())
    {
        selection_sort<ArrayType, Cmp>(a, cmp);
    }

    /** O(n^2); the one classic comparison sort here that is adaptive in
        the strongest sense — it detects a fully-sorted array in a single
        O(n) pass (the `swapped` flag below) and stops, rather than merely
        running faster on nearly-sorted input the way insertion_sort
        does. */
    template <class ArrayType, class Cmp>
    void bubble_sort(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        for (int_t i = l; i < r; ++i)
        {
            bool swapped = false;

            for (int_t j = l; j < r - (i - l); ++j)
            {
                if (cmp(a[j + 1], a[j]))
                {
                    std::swap(a[j], a[j + 1]);
                    swapped = true;
                }
            }

            if (!swapped)
            {
                break;
            }
        }
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void bubble_sort(ArrayType& a, int_t l, int_t r, Cmp&& cmp = Cmp())
    {
        bubble_sort<ArrayType, Cmp>(a, l, r, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void bubble_sort(ArrayType& a, int_t size, Cmp& cmp)
    {
        bubble_sort(a, 0, size - 1, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void bubble_sort(ArrayType& a, int_t size, Cmp&& cmp = Cmp())
    {
        bubble_sort<ArrayType, Cmp>(a, size, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void bubble_sort(ArrayType& a, Cmp& cmp)
    {
        bubble_sort(a, a.size(), cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void bubble_sort(ArrayType& a, Cmp&& cmp = Cmp())
    {
        bubble_sort<ArrayType, Cmp>(a, cmp);
    }

    /** O(n log^2 n) to O(n^1.5) depending on the gap sequence — this uses
        the simplest classic one (repeatedly halving the gap), which is
        easy to reason about but not the asymptotically best known choice.
        Literally insertion_sort generalized: each pass is
        gapped_insertion_pass() (insertion_sort()'s own core, above) run
        with a larger-than-1 gap, and the final pass (gap=1) is exactly
        insertion_sort() itself — over an array the earlier, larger-gap
        passes have already moved close to sorted, which is what makes
        that last full insertion-sort pass fast in practice despite its
        own O(n^2) worst case. */
    template <class ArrayType, class Cmp>
    void shell_sort(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        int_t n = r - l + 1;

        for (int_t gap = n / 2; gap > 0; gap /= 2)
        {
            gapped_insertion_pass(a, l, r, gap, cmp);
        }
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void shell_sort(ArrayType& a, int_t l, int_t r, Cmp&& cmp = Cmp())
    {
        shell_sort<ArrayType, Cmp>(a, l, r, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void shell_sort(ArrayType& a, int_t size, Cmp& cmp)
    {
        shell_sort(a, 0, size - 1, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void shell_sort(ArrayType& a, int_t size, Cmp&& cmp = Cmp())
    {
        shell_sort<ArrayType, Cmp>(a, size, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void shell_sort(ArrayType& a, Cmp& cmp)
    {
        shell_sort(a, a.size(), cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void shell_sort(ArrayType& a, Cmp&& cmp = Cmp())
    {
        shell_sort<ArrayType, Cmp>(a, cmp);
    }

    template <typename ArrayType, class Cmp>
    inline int_t select_pivot(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        assert(l <= r);

        int_t m = (l + r) / 2;

        int_t partial_min = cmp(a[l], a[m]) ? l : m;

        return cmp(a[partial_min], a[r]) ? partial_min : r;
    }

    template <typename ArrayType, class Cmp>
    int_t partition(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        int pivot = select_pivot(a, l, r, cmp);

        std::swap(a[pivot], a[r]);

        int_t i = l - 1;
        int_t j = r;

        while (i <= j)
        {
            while (cmp(a[++i], a[r]))
            {
                ;
            }

            while (cmp(a[r], a[--j]))
            {
                if (j == l)
                {
                    break;
                }
            }

            if (i >= j)
            {
                break;
            }

            std::swap(a[i], a[j]);
        }

        std::swap(a[i], a[r]);

        return i;
    }

    template <typename ArrayType, class Cmp>
    void quicksort(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        if (l >= r)
        {
            return;
        }

        if (r - l + 1 <= QuicksortThreshold)
        {
            insertion_sort(a, l, r, cmp);
            return;
        }

        int_t pivot = partition(a, l, r, cmp);

        if (pivot - l < r - pivot)
        {
            quicksort(a, l, pivot - 1, cmp);
            quicksort(a, pivot + 1, r, cmp);
        }
        else
        {
            quicksort(a, pivot + 1, r, cmp);
            quicksort(a, l, pivot - 1, cmp);
        }
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void quicksort(ArrayType& a, int_t l, int_t r, Cmp&& cmp = Cmp())
    {
        quicksort<ArrayType, Cmp>(a, l, r, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void quicksort(ArrayType& a, int_t size, Cmp& cmp)
    {
        quicksort(a, 0, size - 1, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void quicksort(ArrayType& a, int_t size, Cmp&& cmp = Cmp())
    {
        quicksort<ArrayType, Cmp>(a, size, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void quicksort(ArrayType& a, Cmp& cmp)
    {
        quicksort(a, a.size(), cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void quicksort(ArrayType& a, Cmp&& cmp = Cmp())
    {
        quicksort<ArrayType, Cmp>(a, cmp);
    }

    template <typename T, class Cmp = std::less<T>>
    inline void quicksort(FixedArray<T>& a, Cmp& cmp)
    {
        quicksort<FixedArray<T>, Cmp>(a, cmp);
    }

    template <typename T, class Cmp = std::less<T>>
    inline void quicksort(FixedArray<T>& a, Cmp&& cmp = Cmp())
    {
        quicksort<T, Cmp>(a, cmp);
    }

    template <typename T, class Cmp = std::less<T>>
    inline void quicksort(DynArray<T>& a, Cmp& cmp)
    {
        quicksort<DynArray<T>, Cmp>(a, cmp);
    }

    template <typename T, class Cmp = std::less<T>>
    inline void quicksort(DynArray<T>& a, Cmp&& cmp = Cmp())
    {
        quicksort<T, Cmp>(a, cmp);
    }

    /** Merges the two already-sorted runs `a[l..m]` and `a[m+1..r]` into
        one sorted `a[l..r]`, via a temporary buffer — the one classic
        comparison sort in this file that is not in-place, in exchange for
        being the one that is stable and has a guaranteed O(n log n)
        worst case (quicksort's worst case is O(n^2); merge_sort's never
        is, regardless of input order or pivot choice). */
    template <class ArrayType, class Cmp>
    void merge(ArrayType& a, int_t l, int_t m, int_t r, Cmp& cmp)
    {
        using T = typename ArrayType::DataType;

        DynArray<T> tmp;

        int_t i = l;
        int_t j = m + 1;

        while (i <= m && j <= r)
        {
            if (cmp(a[j], a[i]))
            {
                tmp.append(std::move(a[j++]));
            }
            else
            {
                tmp.append(std::move(a[i++]));
            }
        }

        while (i <= m)
        {
            tmp.append(std::move(a[i++]));
        }

        while (j <= r)
        {
            tmp.append(std::move(a[j++]));
        }

        for (nat_t k = 0; k < tmp.size(); ++k)
        {
            a[l + int_t(k)] = std::move(tmp[k]);
        }
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void merge(ArrayType& a, int_t l, int_t m, int_t r,
                      Cmp&& cmp = Cmp())
    {
        merge<ArrayType, Cmp>(a, l, m, r, cmp);
    }

    template <class ArrayType, class Cmp>
    void merge_sort(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        if (l >= r)
        {
            return;
        }

        if (r - l + 1 <= QuicksortThreshold)
        {
            insertion_sort(a, l, r, cmp);
            return;
        }

        int_t m = l + (r - l) / 2;

        merge_sort(a, l, m, cmp);
        merge_sort(a, m + 1, r, cmp);
        merge(a, l, m, r, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void merge_sort(ArrayType& a, int_t l, int_t r, Cmp&& cmp = Cmp())
    {
        merge_sort<ArrayType, Cmp>(a, l, r, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void merge_sort(ArrayType& a, int_t size, Cmp& cmp)
    {
        merge_sort(a, 0, size - 1, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void merge_sort(ArrayType& a, int_t size, Cmp&& cmp = Cmp())
    {
        merge_sort<ArrayType, Cmp>(a, size, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void merge_sort(ArrayType& a, Cmp& cmp)
    {
        merge_sort(a, a.size(), cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void merge_sort(ArrayType& a, Cmp&& cmp = Cmp())
    {
        merge_sort<ArrayType, Cmp>(a, cmp);
    }

    /** Generic over any type indexable via operator[] — a raw `T*` (which
        is how FixedHeap's fixed-size C array reaches this) and a proper
        container reference such as `DynArray<Key>&` (how DynHeap and
        heap_sort() below reach it) both qualify uniformly, so this one
        implementation replaces what used to be two near-identical
        copies: this function itself (raw-pointer-only) and DynHeap's own
        private static sift_up(BaseArray&, ...).

        `l`/`r` are 1-based (matching the classic heap formulas
        parent(i)=i/2, child(i)=i*2, which only stay this clean with
        1-based indices); `offset` is the *real* index that conceptual
        position 1 corresponds to (0 for a heap occupying an entire
        array, as FixedHeap/DynHeap do; the sub-range's own start index
        for heap_sort() below, which runs this over a `[l, r]` slice of a
        larger array) — every access is `a[offset + x - 1]`, not `a[x]`.
        An earlier version of this function took 1-based indices
        literally by having callers pass `array - 1` as the base pointer
        instead of an offset; forming a pointer to the element *before*
        the start of an array is undefined behavior in C++ (only "one
        past the end" is well-defined) regardless of whether it is ever
        dereferenced, and UBSan's bounds instrumentation correctly
        flagged it. Folding the offset into the index expression instead
        keeps the exact same 1-based formulas without ever forming an
        out-of-bounds pointer — or needing a pointer at all, which is what
        lets this same function serve both T* and a container reference. */
    template <class ArrayType, class Cmp>
    void sift_up(ArrayType& a, int_t offset, nat_t l, nat_t r, Cmp& cmp)
    {
        nat_t i = r;

        nat_t u = i / 2;

        while (u >= l &&
               cmp(a[offset + int_t(i) - 1], a[offset + int_t(u) - 1]))
        {
            std::swap(a[offset + int_t(i) - 1], a[offset + int_t(u) - 1]);
            i = u;
            u = i / 2;
        }
    }

    template <class ArrayType, class Cmp>
    inline void sift_up(ArrayType& a, int_t offset, nat_t l, nat_t r,
                        Cmp&& cmp = Cmp())
    {
        return sift_up<ArrayType, Cmp>(a, offset, l, r, cmp);
    }

    /** @see sift_up() above for why every access is `a[offset + x - 1]`
        rather than `a[x]`, and for why `ArrayType` is generic. */
    template <class ArrayType, class Cmp>
    void sift_down(ArrayType& a, int_t offset, nat_t l, nat_t r, Cmp& cmp)
    {
        nat_t i = l;

        nat_t c = i * 2;

        while (c <= r)
        {
            if (c < r)
            {
                if (cmp(a[offset + int_t(c)], a[offset + int_t(c) - 1]))
                {
                    ++c;
                }
            }

            if (!cmp(a[offset + int_t(c) - 1], a[offset + int_t(i) - 1]))
            {
                break;
            }

            std::swap(a[offset + int_t(c) - 1], a[offset + int_t(i) - 1]);
            i = c;
            c = i * 2;
        }
    }

    template <class ArrayType, class Cmp>
    inline void sift_down(ArrayType& a, int_t offset, nat_t l, nat_t r,
                          Cmp&& cmp = Cmp())
    {
        return sift_down<ArrayType, Cmp>(a, offset, l, r, cmp);
    }

    /** In-place, O(n log n) worst case guaranteed (like merge_sort, unlike
        quicksort) but — unlike merge_sort — needs no extra buffer, at the
        cost of not being stable and having noticeably worse cache
        behavior in practice (the heap operations jump around the array
        rather than scanning it). Reuses sift_up()/sift_down() above,
        which is why it can be defined right here instead of needing its
        own heap machinery — but those implement a *min*-heap with respect
        to `cmp` (this library's established heap convention), and
        extracting repeatedly from a min-heap would put the *smallest*
        remaining element next to the already-sorted region each time —
        exactly backwards from what an in-place sort needs (the extracted
        extreme must go to the boundary being vacated, growing the sorted
        region from that end). Flipping cmp's argument order turns
        "smallest per cmp" into "largest per cmp", so the standard build
        max-heap / repeatedly swap the root to the shrinking array's tail
        heapsort algorithm falls out without a separate max-heap
        implementation to duplicate and maintain.

        Operates on `a` directly with `l` passed through as sift_up()/
        sift_down()'s `offset` — no address-of/pointer-arithmetic step
        needed (an earlier version of this function took `&a[l]` as a raw
        base pointer, back when sift_up()/sift_down() only accepted one;
        now that they take any indexable ArrayType plus an explicit
        offset, `a`/`l` can be passed straight through). */
    template <class ArrayType, class Cmp>
    void heap_sort(ArrayType& a, int_t l, int_t r, Cmp& cmp)
    {
        using T = typename ArrayType::DataType;

        int_t n = r - l + 1;

        if (n < 2)
        {
            return;
        }

        auto flipped = [&cmp](const T& x, const T& y) { return cmp(y, x); };

        for (nat_t i = 2; i <= nat_t(n); ++i)
        {
            sift_up(a, l, nat_t(1), i, flipped);
        }

        for (nat_t i = nat_t(n); i > 1; --i)
        {
            std::swap(a[l], a[l + int_t(i) - 1]);
            sift_down(a, l, nat_t(1), i - 1, flipped);
        }
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void heap_sort(ArrayType& a, int_t l, int_t r, Cmp&& cmp = Cmp())
    {
        heap_sort<ArrayType, Cmp>(a, l, r, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void heap_sort(ArrayType& a, int_t size, Cmp& cmp)
    {
        heap_sort(a, 0, size - 1, cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void heap_sort(ArrayType& a, int_t size, Cmp&& cmp = Cmp())
    {
        heap_sort<ArrayType, Cmp>(a, size, cmp);
    }

    template <class ArrayType, class Cmp>
    inline void heap_sort(ArrayType& a, Cmp& cmp)
    {
        heap_sort(a, a.size(), cmp);
    }

    template <class ArrayType,
              class Cmp = std::less<typename ArrayType::DataType>>
    inline void heap_sort(ArrayType& a, Cmp&& cmp = Cmp())
    {
        heap_sort<ArrayType, Cmp>(a, cmp);
    }

    /** Non-comparison sorts: instead of deciding order via `cmp`
        comparisons (subject to the Ω(n log n) lower bound every sort
        above this point is stuck with), these work directly with integer
        keys, and so can beat that bound — O(n + k) for counting_sort
        (`k` the key range), O(d*(n + k)) for radix_sort (`d` the number
        of digits) — in exchange for being far less general (integral
        DataType only, non-negative keys only, and counting_sort needs the
        key range known up front). */
    template <class ArrayType>
    void counting_sort(ArrayType& a, int_t l, int_t r, nat_t max_key)
    {
        using T = typename ArrayType::DataType;

        static_assert(std::is_integral<T>::value,
                      "counting_sort requires an integral DataType "
                      "(non-negative keys in [0, max_key])");

        if (l >= r)
        {
            return;
        }

        DynArray<nat_t> count(max_key + 1, nat_t(0));

        for (int_t i = l; i <= r; ++i)
        {
            ++count[nat_t(a[i])];
        }

        for (nat_t k = 1; k <= max_key; ++k)
        {
            count[k] += count[k - 1];
        }

        DynArray<T> output(nat_t(r - l + 1), T());

        for (int_t i = r; i >= l; --i)
        {
            nat_t key = nat_t(a[i]);
            output[--count[key]] = a[i];
        }

        for (int_t i = l; i <= r; ++i)
        {
            a[i] = std::move(output[nat_t(i - l)]);
        }
    }

    template <class ArrayType>
    inline void counting_sort(ArrayType& a, int_t size, nat_t max_key)
    {
        counting_sort(a, 0, size - 1, max_key);
    }

    template <class ArrayType>
    inline void counting_sort(ArrayType& a, nat_t max_key)
    {
        counting_sort(a, a.size(), max_key);
    }

    /** LSD (least-significant-digit) radix sort, base 10 for readability
        — a base matching the machine word size (e.g. 256) is the usual
        choice in practice, trading a larger per-pass counting array for
        fewer passes (fewer digits to process). Each pass is exactly
        counting_sort()'s algorithm specialized to one base-10 digit
        (kept as its own self-contained loop below, rather than calling
        counting_sort() directly, since the two differ slightly: the key
        here is `(a[i] / exp) % 10`, not `a[i]` itself); stability across
        passes — required for radix sort's correctness, since a later
        (more-significant-digit) pass must not disturb the relative order
        two elements were already placed in by an earlier tie — is what
        makes this correct, and is exactly why counting_sort (itself
        stable) rather than an unstable sort is the right subroutine here. */
    template <class ArrayType>
    void radix_sort(ArrayType& a, int_t l, int_t r)
    {
        using T = typename ArrayType::DataType;

        static_assert(
            std::is_integral<T>::value,
            "radix_sort requires an integral DataType (non-negative keys)");

        if (l >= r)
        {
            return;
        }

        T max_val = a[l];

        for (int_t i = l + 1; i <= r; ++i)
        {
            if (a[i] > max_val)
            {
                max_val = a[i];
            }
        }

        for (T exp = 1; max_val / exp > 0; exp *= 10)
        {
            DynArray<nat_t> count(nat_t(10), nat_t(0));

            for (int_t i = l; i <= r; ++i)
            {
                ++count[nat_t((a[i] / exp) % 10)];
            }

            for (nat_t d = 1; d < 10; ++d)
            {
                count[d] += count[d - 1];
            }

            DynArray<T> output(nat_t(r - l + 1), T());

            for (int_t i = r; i >= l; --i)
            {
                nat_t digit = nat_t((a[i] / exp) % 10);
                output[--count[digit]] = a[i];
            }

            for (int_t i = l; i <= r; ++i)
            {
                a[i] = output[nat_t(i - l)];
            }
        }
    }

    template <class ArrayType>
    inline void radix_sort(ArrayType& a, int_t size)
    {
        radix_sort(a, 0, size - 1);
    }

    template <class ArrayType>
    inline void radix_sort(ArrayType& a)
    {
        radix_sort(a, a.size());
    }

    template <typename T, class Cmp>
    std::tuple<NodeSLList<T>, typename NodeSLList<T>::Node*, NodeSLList<T>>
    partition(NodeSLList<T>& l, Cmp& cmp)
    {
        auto pivot = l.remove_first();

        NodeSLList<T> ls, gs;

        while (!l.is_empty())
        {
            auto p = l.remove_first();

            if (cmp(p->get_item(), pivot->get_item()))
            {
                ls.append(p);
            }
            else
            {
                gs.append(p);
            }
        }

        return std::make_tuple(std::move(ls), pivot, std::move(gs));
    }

    template <typename T, class Cmp>
    void quicksort(NodeSLList<T>& l, Cmp& cmp)
    {
        if (l.is_unitarian_or_empty())
        {
            return;
        }

        auto part = partition(l, cmp);

        quicksort(std::get<0>(part), cmp);
        quicksort(std::get<2>(part), cmp);

        l.concat(std::get<0>(part));
        l.append(std::get<1>(part));
        l.concat(std::get<2>(part));
    }

    template <typename T, class Cmp = std::less<T>>
    inline void quicksort(NodeSLList<T>& l, Cmp&& cmp = Cmp())
    {
        quicksort<T, Cmp>(l, cmp);
    }

    /** The classic linked-list sort — merge_sort's natural home is
        arguably a list more than an array (splitting a list in two costs
        nothing extra the way it does for an array, since there is no
        contiguous storage to divide; merging two lists back together is
        just relinking nodes, not copying into a temporary buffer the way
        array-based merge() above needs to). Uses NodeSLList::split() to
        halve `l` (round-robin, not front/back — either halving strategy
        keeps the O(n log n) bound, and round-robin is what split() this
        library already provides). */
    template <typename T, class Cmp>
    NodeSLList<T> merge(NodeSLList<T>& l, NodeSLList<T>& r, Cmp& cmp)
    {
        NodeSLList<T> result;

        while (!l.is_empty() && !r.is_empty())
        {
            if (cmp(r.get_first()->get_item(), l.get_first()->get_item()))
            {
                result.append(r.remove_first());
            }
            else
            {
                result.append(l.remove_first());
            }
        }

        while (!l.is_empty())
        {
            result.append(l.remove_first());
        }

        while (!r.is_empty())
        {
            result.append(r.remove_first());
        }

        return result;
    }

    template <typename T, class Cmp = std::less<T>>
    inline NodeSLList<T> merge(NodeSLList<T>& l, NodeSLList<T>& r,
                               Cmp&& cmp = Cmp())
    {
        return merge<T, Cmp>(l, r, cmp);
    }

    template <typename T, class Cmp>
    void merge_sort(NodeSLList<T>& l, Cmp& cmp)
    {
        if (l.is_unitarian_or_empty())
        {
            return;
        }

        NodeSLList<T> left, right;
        l.split(left, right);

        merge_sort(left, cmp);
        merge_sort(right, cmp);

        l = merge(left, right, cmp);
    }

    template <typename T, class Cmp = std::less<T>>
    inline void merge_sort(NodeSLList<T>& l, Cmp&& cmp = Cmp())
    {
        merge_sort<T, Cmp>(l, cmp);
    }

    template <class Cmp>
    std::tuple<DL, DL*, DL> partition(DL& l, Cmp& cmp)
    {
        auto pivot = l.remove_next();

        DL ls, gs;

        while (!l.is_empty())
        {
            auto p = l.remove_next();

            if (cmp(p, pivot))
            {
                ls.insert_prev(p);
            }
            else
            {
                gs.insert_prev(p);
            }
        }

        return std::make_tuple(std::move(ls), pivot, std::move(gs));
    }

    template <class Cmp>
    void quicksort(DL& l, Cmp& cmp)
    {
        if (l.is_unitarian_or_empty())
        {
            return;
        }

        auto part = partition(l, cmp);

        quicksort(std::get<0>(part), cmp);
        quicksort(std::get<2>(part), cmp);

        l.concat(std::get<0>(part));
        l.insert_prev(std::get<1>(part));
        l.concat(std::get<2>(part));
    }

    template <class Cmp>
    inline void quicksort(DL& l, Cmp&& cmp = Cmp())
    {
        quicksort<Cmp>(l, cmp);
    }

    /** @warning `cmp` is a reference bound to this constructor's own
        default-argument parameter when no explicit lvalue is supplied
        (`KeyCmp(Cmp &&_cmp = Cmp())`); the temporary is destroyed at the
        end of the constructing full-expression, so the default-argument
        path is a known dangling-reference hazard (see DefaultCmpHolder in
        typetraits.hpp for the general fix pattern used elsewhere in this
        codebase). It is intentionally left unfixed here: KeyCmp/PtrCmp are
        themselves used as the `Cmp` type parameter of one another (and of
        DistanceCmp in graphalgorithms.hpp) via Graph::sort_arcs, and that
        Cmp type can itself be a reference-holding, non-copy-assignable
        wrapper (e.g. DistanceCmp) — the "own a default instance and
        copy-assign into it" fix breaks for any such Cmp, since it has no
        usable copy-assignment operator. In practice KeyCmp/PtrCmp are
        always constructed with an explicit lvalue comparator by the
        `quicksort` free functions in this header, never via the
        default-argument path, which is why this has not surfaced as an
        observed bug. */
    template <typename T, class Cmp>
    struct KeyCmp
    {
        Cmp& cmp;

        KeyCmp(Cmp& _cmp) : cmp(_cmp)
        {
            // empty
        }

        KeyCmp(Cmp&& _cmp = Cmp()) : cmp(_cmp)
        {
            // empty
        }

        bool operator()(DL* l, DL* r) const
        {
            return cmp(static_cast<DLNode<T>*>(l)->get_item(),
                       static_cast<DLNode<T>*>(r)->get_item());
        }
    };

    /** @see KeyCmp above. */
    template <typename T, class Cmp>
    struct PtrCmp
    {
        Cmp& cmp;

        PtrCmp(Cmp& _cmp) : cmp(_cmp)
        {
            // empty
        }

        PtrCmp(Cmp&& _cmp = Cmp()) : cmp(_cmp)
        {
            // empty
        }

        bool operator()(const T& a, const T& b) const
        {
            return cmp(const_cast<T*>(&a), const_cast<T*>(&b));
        }
    };

    template <typename T, class Cmp>
    inline void quicksort(DLNode<T>& l, Cmp& cmp)
    {
        KeyCmp<T, Cmp> key_cmp(cmp);
        quicksort<KeyCmp<T, Cmp>>(l, key_cmp);
    }

    template <typename T, class Cmp = std::less<T>>
    inline void quicksort(DLNode<T>& l, Cmp&& cmp = Cmp())
    {
        quicksort<T, Cmp>(l, cmp);
    }

    template <typename T, class Cmp>
    inline void quicksort(DLList<T>& l, Cmp& cmp)
    {
        KeyCmp<T, Cmp> key_cmp(cmp);
        quicksort<KeyCmp<T, Cmp>>(l, key_cmp);
    }

    template <typename T, class Cmp = std::less<T>>
    inline void quicksort(DLList<T>& l, Cmp&& cmp = Cmp())
    {
        quicksort<T, Cmp>(l, cmp);
    }

    template <typename SeqType,
              class Cmp = std::less<typename SeqType::ItemType>>
    inline SeqType sort(const SeqType& s, Cmp& cmp)
    {
        SeqType ret_val = s;
        quicksort<typename SeqType::ItemType, Cmp>(ret_val, cmp);
        return ret_val;
    }

    template <typename SeqType,
              class Cmp = std::less<typename SeqType::ItemType>>
    inline SeqType sort(const SeqType& s, Cmp&& cmp = Cmp())
    {
        return sort<SeqType, Cmp>(s, cmp);
    }

    template <typename SeqType,
              class Cmp = std::less<typename SeqType::ItemType>>
    inline void inline_sort(SeqType& s, Cmp& cmp)
    {
        quicksort<typename SeqType::ItemType, Cmp>(s, cmp);
    }

    template <typename SeqType,
              class Cmp = std::less<typename SeqType::ItemType>>
    inline void inline_sort(SeqType& s, Cmp&& cmp = Cmp())
    {
        inline_sort<SeqType, Cmp>(s, cmp);
    }

    template <class ArrayType>
    ArrayType reverse(const ArrayType& a)
    {
        ArrayType ret_val;

        for (nat_t i = a.size(); i > 0; --i)
        {
            ret_val.append(a[i - 1]);
        }

        return ret_val;
    }

    template <class SRCL, class TGTL>
    TGTL reverse(const SRCL& l)
    {
        TGTL ret_val;

        for (auto item : l)
        {
            ret_val.insert(item);
        }

        return ret_val;
    }

    template <typename T>
    inline FixedArray<T> reverse(const FixedArray<T>& a)
    {
        return reverse<FixedArray<T>>(a);
    }

    template <typename T>
    inline DynArray<T> reverse(const DynArray<T>& a)
    {
        return reverse<DynArray<T>>(a);
    }

    template <typename T>
    inline SLList<T> reverse(const SLList<T>& l)
    {
        return reverse<SLList<T>, SLList<T>>(l);
    }

    template <typename T>
    inline DLList<T> reverse(const DLList<T>& l)
    {
        return reverse<DLList<T>, DLList<T>>(l);
    }

} // end namespace Designar
