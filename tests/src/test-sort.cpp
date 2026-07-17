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
  test_on_edge_cases([](DynArray<int_t>& a)
                     { selection_sort(a); }, "selection_sort");

  test_on_edge_cases([](DynArray<int_t>& a)
                     { bubble_sort(a); }, "bubble_sort");

  test_on_edge_cases([](DynArray<int_t>& a)
                     { shell_sort(a); }, "shell_sort");

  test_on_edge_cases([](DynArray<int_t>& a)
                     { merge_sort(a); }, "merge_sort (array)");

  test_on_edge_cases([](DynArray<int_t>& a)
                     { heap_sort(a); }, "heap_sort");

  // A custom comparator (descending order) must work for every
  // algorithm, not just the default std::less.
  {
    DynArray<int_t> a = {5, 3, 1, 4, 2};
    auto desc = [](int_t x, int_t y)
    { return x > y; };

    selection_sort(a, desc);
    assert(a.equal({5, 4, 3, 2, 1}));
  }

  {
    DynArray<int_t> a = {5, 3, 1, 4, 2};
    auto desc = [](int_t x, int_t y)
    { return x > y; };

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

    merge_sort(a, [](const Item& x, const Item& y)
               { return x.key < y.key; });

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

      for (SLNode<int_t>* p = list.get_first(); p != nullptr; p = p->get_next(), ++i)
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

  cout << "Everything ok!\n";
  return 0;
}
