/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <array.hpp>
#include <list.hpp>
#include <sort.hpp>
#include <random.hpp>

using namespace std;
using namespace Designar;

template <class SortFn>
void test_on_edge_cases(SortFn sort_fn, const char* name)
{
    // Empty and single-element arrays must not crash.
    {
        DynArray<int_t> a;
        sort_fn(a);
        assert(a.is_empty());
    }

    {
        DynArray<int_t> a = {42};
        sort_fn(a);
        assert(a.is_sorted());
    }

    // Already sorted, reverse-sorted, and all-duplicate inputs are the
    // classic edge cases every one of these algorithms needs to handle
    // without degenerating incorrectly (as opposed to just slowly).
    {
        DynArray<int_t> a = {1, 2, 3, 4, 5};
        sort_fn(a);
        assert(a.is_sorted());
        assert(a.equal({1, 2, 3, 4, 5}));
    }

    {
        DynArray<int_t> a = {5, 4, 3, 2, 1};
        sort_fn(a);
        assert(a.is_sorted());
        assert(a.equal({1, 2, 3, 4, 5}));
    }

    {
        DynArray<int_t> a = {7, 7, 7, 7, 7};
        sort_fn(a);
        assert(a.is_sorted());
        assert(a.equal({7, 7, 7, 7, 7}));
    }

    // Random inputs of various sizes, including ones spanning
    // QuicksortThreshold for the algorithms (merge_sort, quicksort) that
    // switch strategy above it.
    {
        rng_t rng(4242);

        for (int_t trial = 0; trial < 30; ++trial)
        {
            nat_t n = random_uniform(rng, 200) + 1;
            DynArray<int_t> a;

            for (nat_t i = 0; i < n; ++i)
            {
                a.append(random_uniform(rng, 100));
            }

            sort_fn(a);
            assert(a.is_sorted());
        }
    }

    cout << name << ": Everything ok!\n";
}

int main()
{
    test_on_edge_cases([](DynArray<int_t>& a) { selection_sort(a); },
                       "selection_sort");

    test_on_edge_cases([](DynArray<int_t>& a) { bubble_sort(a); },
                       "bubble_sort");

    test_on_edge_cases([](DynArray<int_t>& a) { shell_sort(a); }, "shell_sort");

    test_on_edge_cases([](DynArray<int_t>& a) { merge_sort(a); },
                       "merge_sort (array)");

    test_on_edge_cases([](DynArray<int_t>& a) { heap_sort(a); }, "heap_sort");

    // A custom comparator (descending order) must work for every
    // algorithm, not just the default std::less.
    {
        DynArray<int_t> a = {5, 3, 1, 4, 2};
        auto desc = [](int_t x, int_t y) { return x > y; };

        selection_sort(a, desc);
        assert(a.equal({5, 4, 3, 2, 1}));
    }

    {
        DynArray<int_t> a = {5, 3, 1, 4, 2};
        auto desc = [](int_t x, int_t y) { return x > y; };

        heap_sort(a, desc);
        assert(a.equal({5, 4, 3, 2, 1}));
    }

    // merge_sort is stable: equal keys must keep their relative input
    // order. Sort pairs by key only and check the tag sequence among
    // equal keys is preserved.
    {
        struct Item
        {
            int_t key;
            int_t tag;
        };

        DynArray<Item> a = {{1, 0}, {2, 0}, {1, 1}, {2, 1}, {1, 2}};

        merge_sort(a,
                   [](const Item& x, const Item& y) { return x.key < y.key; });

        DynArray<int_t> key1_tags;

        for (nat_t i = 0; i < a.size(); ++i)
        {
            if (a[i].key == 1)
            {
                key1_tags.append(a[i].tag);
            }
        }

        assert(key1_tags.equal({0, 1, 2}));
    }

    // merge_sort on a linked list (NodeSLList, via ::split()).
    {
        rng_t rng(99);

        for (int_t trial = 0; trial < 10; ++trial)
        {
            NodeSLList<int_t> list;
            DynArray<int_t> expected;

            nat_t n = random_uniform(rng, 100) + 1;

            for (nat_t i = 0; i < n; ++i)
            {
                int_t value = random_uniform(rng, 100);
                list.append(new SLNode<int_t>(value));
                expected.append(value);
            }

            merge_sort(list);
            inline_sort(expected);

            nat_t i = 0;

            for (SLNode<int_t>* p = list.get_first(); p != nullptr;
                 p = p->get_next(), ++i)
            {
                assert(p->get_item() == expected[i]);
            }

            assert(i == n);

            while (!list.is_empty())
            {
                delete list.remove_first();
            }
        }

        cout << "merge_sort (NodeSLList): Everything ok!\n";
    }

    // counting_sort / radix_sort: non-comparison sorts over non-negative
    // integer keys.
    {
        rng_t rng(7);

        for (int_t trial = 0; trial < 30; ++trial)
        {
            nat_t n = random_uniform(rng, 200) + 1;
            DynArray<nat_t> a;

            for (nat_t i = 0; i < n; ++i)
            {
                a.append(random_uniform(rng, 500));
            }

            DynArray<nat_t> b = a;

            counting_sort(a, nat_t(500));
            assert(a.is_sorted());

            radix_sort(b);
            assert(b.is_sorted());

            assert(a.equal(b));
        }

        DynArray<nat_t> empty;
        counting_sort(empty, nat_t(10));
        assert(empty.is_empty());

        DynArray<nat_t> single = {42};
        radix_sort(single);
        assert(single.equal({42}));

        cout << "counting_sort/radix_sort: Everything ok!\n";
    }

    // sift_up()/sift_down() directly — heap_sort()/FixedHeap/DynHeap all
    // exercise these already, but only ever with offset=0 (FixedHeap,
    // DynHeap) or via heap_sort()'s own internal call (offset=l). Testing
    // them here directly, with an explicit non-zero offset and both a raw
    // C array and a DynArray, covers the generalization itself: both
    // ArrayType kinds must work, and the 1-based l/r positions must be
    // interpreted relative to `offset`, not to the start of the whole
    // underlying storage.
    {
        // Raw array, offset 0 — heapify a 7-element min-heap by repeated
        // sift_up(), then verify the classic min-heap invariant
        // (parent <= both children) holds at every node.
        {
            int_t raw[7] = {5, 3, 8, 1, 9, 2, 7};

            for (nat_t i = 2; i <= 7; ++i)
            {
                sift_up(raw, 0, nat_t(1), i, std::less<int_t>());
            }

            for (nat_t i = 1; i <= 7; ++i)
            {
                nat_t left = 2 * i;
                nat_t right = 2 * i + 1;

                if (left <= 7)
                {
                    assert(raw[i - 1] <= raw[left - 1]);
                }

                if (right <= 7)
                {
                    assert(raw[i - 1] <= raw[right - 1]);
                }
            }
        }

        // DynArray, non-zero offset — heapify only the sub-range [3, 9] of
        // a larger array (elements at indices 0..2 are untouched sentinels
        // and must stay exactly as they were), mirroring exactly how
        // heap_sort() uses a non-zero offset for a [l, r] slice.
        {
            DynArray<int_t> a = {-1, -1, -1, 5, 3, 8, 1, 9, 2, 7};
            int_t offset = 3;
            nat_t n = 7;

            for (nat_t i = 2; i <= n; ++i)
            {
                sift_up(a, offset, nat_t(1), i, std::less<int_t>());
            }

            assert(a[0] == -1 && a[1] == -1 && a[2] == -1);

            for (nat_t i = 1; i <= n; ++i)
            {
                nat_t left = 2 * i;
                nat_t right = 2 * i + 1;

                if (left <= n)
                {
                    assert(a[offset + int_t(i) - 1] <=
                           a[offset + int_t(left) - 1]);
                }

                if (right <= n)
                {
                    assert(a[offset + int_t(i) - 1] <=
                           a[offset + int_t(right) - 1]);
                }
            }

            // sift_down() from the root after replacing it: must restore the
            // min-heap invariant without touching anything outside [offset,
            // offset + n).
            a[offset] = 100;
            sift_down(a, offset, nat_t(1), n, std::less<int_t>());

            assert(a[0] == -1 && a[1] == -1 && a[2] == -1);

            for (nat_t i = 1; i <= n; ++i)
            {
                nat_t left = 2 * i;
                nat_t right = 2 * i + 1;

                if (left <= n)
                {
                    assert(a[offset + int_t(i) - 1] <=
                           a[offset + int_t(left) - 1]);
                }

                if (right <= n)
                {
                    assert(a[offset + int_t(i) - 1] <=
                           a[offset + int_t(right) - 1]);
                }
            }
        }

        cout << "sift_up/sift_down: Everything ok!\n";
    }

    cout << "Everything ok!\n";
    return 0;
}
