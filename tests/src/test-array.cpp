/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <list.hpp>
#include <set.hpp>
#include <sort.hpp>
#include <random.hpp>

using namespace std;
using namespace Designar;

int main()
{
    DynArray<int_t> array;

    assert(array.is_empty());

    array.insert(2);

    assert(!array.is_empty());
    assert(array.size() == 1);
    assert(array.get_first() == 2);
    assert(array.get_last() == 2);

    array.insert(1);

    assert(!array.is_empty());
    assert(array.size() == 2);
    assert(array.get_first() == 1);
    assert(array.get_last() == 2);

    array.append(3);

    assert(!array.is_empty());
    assert(array.size() == 3);
    assert(array.get_first() == 1);
    assert(array.get_last() == 3);

    for (nat_t i = 0; i < 10; ++i)
        array.append(array.get_last() + 1);

    assert(!array.is_empty());
    assert(array.size() == 13);
    assert(array.get_first() == 1);
    assert(array.get_last() == 13);

    array.remove_first();
    assert(!array.is_empty());
    assert(array.size() == 12);
    assert(array.get_first() == 13);
    assert(array.get_last() == 12);

    array.remove_last();
    assert(!array.is_empty());
    assert(array.size() == 11);
    assert(array.get_first() == 13);
    assert(array.get_last() == 11);

    array.clear();

    assert(array.is_empty());

    for (int_t i = 0; i < 5; ++i)
        array.append(i + 1);

    assert(!array.is_empty());
    assert(array.size() == 5);
    assert(array.get_first() == 1);
    assert(array.get_last() == 5);

    auto farray = array.filter([](const auto& item) { return item % 2 == 0; });

    assert(farray.equal({2, 4}));

    auto marray = array.map([](const auto& item) { return item * 2; });

    assert(marray.equal({2, 4, 6, 8, 10}));

    auto sum = array.fold(0, [](const auto& item, const auto& acc)
                          { return item + acc; });

    assert(sum == 15);

    auto prod = array.fold(1, [](const auto& item, const auto& acc)
                           { return item * acc; });

    assert(prod == 120);

    assert(!array.all([](const auto& item) { return item % 2 == 0; }));
    assert(farray.all([](const auto& item) { return item % 2 == 0; }));
    assert(marray.all([](const auto& item) { return item % 2 == 0; }));

    assert(array.exists([](const auto& item) { return item % 2 == 0; }));
    assert(farray.exists([](const auto& item) { return item % 2 == 0; }));
    assert(marray.exists([](const auto& item) { return item % 2 == 0; }));

    assert(!array.none([](const auto& item) { return item % 2 == 0; }));
    assert(!farray.none([](const auto& item) { return item % 2 == 0; }));
    assert(!marray.none([](const auto& item) { return item % 2 == 0; }));

    assert(!array.none([](const auto& item) { return item % 2 != 0; }));
    assert(farray.none([](const auto& item) { return item % 2 != 0; }));
    assert(marray.none([](const auto& item) { return item % 2 != 0; }));

    for (nat_t i = 0; i < array.size(); ++i)
        assert(array.nth(i) == i + 1);

    assert(array.search_ptr([](const auto& item) { return item == 3; }) !=
           nullptr);

    assert(array.search_ptr([](const auto& item) { return item > 5; }) ==
           nullptr);

    DynArray<int_t> another_array = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    assert(another_array.equal({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

    SLList<int_t> sllist = another_array.to_list();

    assert(sllist.equal({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

    DynArray<int_t> arr;

    for (int_t i = 0; i < 10000; ++i)
        arr.append(i);

    assert(arr.size() == 10000);
    assert(arr.get_first() == 0);
    assert(arr.get_last() == 9999);

    for (int_t i = 0; i < 10000; ++i)
        arr.remove_last();

    assert(arr.is_empty());

    ArraySet<int_t> unsorted_array_set;

    assert(unsorted_array_set.insert(3) != nullptr);
    assert(unsorted_array_set.insert(4) != nullptr);
    assert(unsorted_array_set.append(1) != nullptr);
    assert(unsorted_array_set.append(5) != nullptr);
    assert(unsorted_array_set.append(6) != nullptr);
    assert(unsorted_array_set.append(7) != nullptr);

    assert(unsorted_array_set.append(6) == nullptr);
    assert(unsorted_array_set.insert(6) == nullptr);

    assert(unsorted_array_set.equal({3, 4, 1, 5, 6, 7}));

    assert(unsorted_array_set.search(3) != nullptr);
    assert(unsorted_array_set.search(4) != nullptr);
    assert(unsorted_array_set.search(1) != nullptr);
    assert(unsorted_array_set.search(5) != nullptr);
    assert(unsorted_array_set.search(6) != nullptr);
    assert(unsorted_array_set.search(7) != nullptr);
    assert(unsorted_array_set.search(0) == nullptr);
    assert(unsorted_array_set.search(2) == nullptr);
    assert(unsorted_array_set.search(9) == nullptr);

    assert(unsorted_array_set.remove(6));
    assert(!unsorted_array_set.remove(0));
    assert(!unsorted_array_set.remove(9));

    assert(unsorted_array_set.equal({3, 4, 1, 5, 7}));

    SortedArraySet<int_t> sorted_array_set;

    assert(sorted_array_set.insert(3) != nullptr);
    assert(sorted_array_set.insert(4) != nullptr);
    assert(sorted_array_set.append(1) != nullptr);
    assert(sorted_array_set.append(5) != nullptr);
    assert(sorted_array_set.append(6) != nullptr);
    assert(sorted_array_set.append(7) != nullptr);

    assert(sorted_array_set.append(6) == nullptr);
    assert(sorted_array_set.insert(6) == nullptr);

    assert(sorted_array_set.equal({1, 3, 4, 5, 6, 7}));

    assert(sorted_array_set.search(3) != nullptr);
    assert(sorted_array_set.search(4) != nullptr);
    assert(sorted_array_set.search(1) != nullptr);
    assert(sorted_array_set.search(5) != nullptr);
    assert(sorted_array_set.search(6) != nullptr);
    assert(sorted_array_set.search(7) != nullptr);
    assert(sorted_array_set.search(0) == nullptr);
    assert(sorted_array_set.search(2) == nullptr);
    assert(sorted_array_set.search(9) == nullptr);

    assert(sorted_array_set.remove(6));
    assert(!sorted_array_set.remove(0));
    assert(!sorted_array_set.remove(9));

    assert(sorted_array_set.equal({1, 3, 4, 5, 7}));

    // Regression: GenArraySet's default comparator constructor used to bind
    // its `Cmp &cmp` member straight to a temporary/default-argument that
    // is destroyed at the end of the constructor call, leaving `cmp`
    // dangling for the entire lifetime of the set (see set.hpp /
    // typetraits.hpp's DefaultCmpHolder). Exercising a set built with no
    // explicit comparator (i.e. via the fixed default-argument path) is
    // itself the regression check; a stateful external comparator on top
    // of that also verifies the intentional reference-sharing design still
    // works (the set observes changes to the caller's comparator object).
    {
        ArraySet<int_t> default_cmp_set;

        for (int_t i = 0; i < 50; ++i)
            default_cmp_set.append(i);

        assert(default_cmp_set.size() == 50);

        struct ReverseAfterN
        {
            nat_t count = 0;
            nat_t flip_at;

            ReverseAfterN(nat_t n = 0) : flip_at(n)
            {
                // empty
            }

            bool operator()(int_t a, int_t b)
            {
                ++count;
                return count <= flip_at ? a < b : a > b;
            }
        };

        ReverseAfterN cmp(
            0); // always "a > b" (count starts at 0, immediately > flip_at)
        SortedArraySet<int_t, ReverseAfterN> stateful_set(32, cmp);

        stateful_set.append(3);
        stateful_set.append(1);
        stateful_set.append(2);

        assert(cmp.count >
               0); // the set actually used *our* external cmp object
    }

    ArraySet<int_t> s1 = {1, 2, 3, 4};
    ArraySet<int_t> s2 = {3, 4, 5, 6};

    assert(s1.equal({1, 2, 3, 4}));
    assert(s2.equal({3, 4, 5, 6}));

    assert(s1.join(s2).equal({1, 2, 3, 4, 5, 6}));

    assert(s1.intersect(s2).equal({3, 4}));

    assert(s1.difference(s2).equal({1, 2}));

    assert(s2.difference(s1).equal({5, 6}));

    assert(s1.cartesian_product(s2).equal({{1, 3},
                                           {1, 4},
                                           {1, 5},
                                           {1, 6},
                                           {2, 3},
                                           {2, 4},
                                           {2, 5},
                                           {2, 6},
                                           {3, 3},
                                           {3, 4},
                                           {3, 5},
                                           {3, 6},
                                           {4, 3},
                                           {4, 4},
                                           {4, 5},
                                           {4, 6}}));

    assert(s1.zip(s2).equal({{1, 3}, {2, 4}, {3, 5}, {4, 6}}));

    rng_t rng(time(nullptr));

    DynArray<int_t> aa;

    for (int_t i = 0; i < 20; ++i)
        aa.append(random_uniform(rng, 100));

    insertion_sort(aa);
    assert(aa.is_sorted());

    aa.clear();

    for (int_t i = 0; i < 1000; ++i)
        aa.append(random_uniform(rng, 100));

    auto sa = sort(aa);
    assert(sa.is_sorted());
    assert(!aa.is_sorted());

    inline_sort(aa);
    assert(aa.is_sorted());

    assert(reverse(aa).is_sorted([](auto x, auto y) { return x > y; }));

    // Regression: FixedArray::item_to_pos() used to compute a byte offset
    // instead of an element index, so remove(T&) threw "Item does not
    // belong to array" for any T with sizeof(T) > 1 (see array.hpp).
    {
        DynArray<pair<int_t, int_t>> pair_array;

        pair_array.append({1, 10});
        auto& mid = pair_array.append({2, 20});
        pair_array.append({3, 30});

        assert(pair_array.size() == 3);

        pair<int_t, int_t> removed = pair_array.remove(mid);

        assert(removed.first == 2 && removed.second == 20);
        assert(pair_array.size() == 2);
    }

    // Regression: DynArray's growth formula (cap * 1.4, truncated) used to
    // stall for small capacities (0, 1, 2), leaving the array permanently
    // full and making the very next append()/insert() throw
    // std::out_of_range instead of growing (see array.hpp).
    {
        DynArray<int_t> tiny(0);
        tiny.append(1);
        tiny.append(2);
        tiny.append(3);
        assert(tiny.equal({1, 2, 3}));

        DynArray<int_t> one(1);
        one.append(1);
        one.append(2);
        assert(one.equal({1, 2}));

        DynArray<int_t> two(2);
        two.append(1);
        two.append(2);
        two.append(3);
        assert(two.equal({1, 2, 3}));
    }

    // Regression: DynArray(n, init_val) left num_items == capacity with no
    // spare room, so the next append() threw std::out_of_range instead of
    // growing like it does for any other array that just became full (see
    // array.hpp).
    {
        DynArray<int_t> filled(5, 42);
        assert(filled.size() == 5);

        for (int_t i = 0; i < 5; ++i)
            assert(filled[i] == 42);

        filled.append(7);
        assert(filled.size() == 6);
        assert(filled.get_last() == 7);
    }

    cout << "Everything ok!\n";

    return 0;
}
