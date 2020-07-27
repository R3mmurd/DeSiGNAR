/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <list.H>
# include <set.H>
# include <sort.H>
# include <random.H>

using namespace std;
using namespace Designar;

int main()
{
  DynArray<lint_t> array;

  assert(array.is_empty());

  array.insert(2);

  assert(not array.is_empty());
  assert(array.size() == 1);
  assert(array.get_first() == 2);
  assert(array.get_last() == 2);

  array.insert(1);

  assert(not array.is_empty());
  assert(array.size() == 2);
  assert(array.get_first() == 1);
  assert(array.get_last() == 2);

  array.append(3);

  assert(not array.is_empty());
  assert(array.size() == 3);
  assert(array.get_first() == 1);
  assert(array.get_last() == 3);

  for (nat_t i = 0; i < 10; ++i)
    array.append(array.get_last() + 1);

  assert(not array.is_empty());
  assert(array.size() == 13);
  assert(array.get_first() == 1);
  assert(array.get_last() == 13);

  array.remove_first();
  assert(not array.is_empty());
  assert(array.size() == 12);
  assert(array.get_first() == 13);
  assert(array.get_last() == 12);

  array.remove_last();
  assert(not array.is_empty());
  assert(array.size() == 11);
  assert(array.get_first() == 13);
  assert(array.get_last() == 11);

  array.clear();

  assert(array.is_empty());

  for (lint_t i = 0; i < 5; ++i)
    array.append(i + 1);

  assert(not array.is_empty());
  assert(array.size() == 5);
  assert(array.get_first() == 1);
  assert(array.get_last() == 5);

  auto farray = array.filter([] (const auto & item)
			     {
			       return item % 2 == 0;
			     });

  assert(farray.equal({2,4}));

  auto marray = array.map([] (const auto & item)
			  {
			    return item * 2;
			  });

  assert(marray.equal({2,4,6,8,10}));

  auto sum = array.fold(0, [] (const auto & item, const auto & acc)
			{
			  return item + acc;
			});

  assert(sum == 15);

  auto prod = array.fold(1, [] (const auto & item, const auto & acc)
			 {
			   return item * acc;
			 });

  assert(prod == 120);
  
  assert(not array.all([] (const auto & item) { return item % 2 == 0; }));
  assert(farray.all([] (const auto & item) { return item % 2 == 0; }));
  assert(marray.all([] (const auto & item) { return item % 2 == 0; }));

  assert(array.exists([] (const auto & item) { return item % 2 == 0; }));
  assert(farray.exists([] (const auto & item) { return item % 2 == 0; }));
  assert(marray.exists([] (const auto & item) { return item % 2 == 0; }));

  assert(not array.none([] (const auto & item) { return item % 2 == 0; }));
  assert(not farray.none([] (const auto & item) { return item % 2 == 0; }));
  assert(not marray.none([] (const auto & item) { return item % 2 == 0; }));

  assert(not array.none([] (const auto & item) { return item % 2 != 0; }));
  assert(farray.none([] (const auto & item) { return item % 2 != 0; }));
  assert(marray.none([] (const auto & item) { return item % 2 != 0; }));

  for (nat_t i = 0; i < array.size(); ++i)
    assert(array.nth(i) == i + 1);

  assert(array.search_ptr([] (const auto & item) { return item == 3; })
	 != nullptr);

  assert(array.search_ptr([] (const auto & item) { return item > 5; })
	 == nullptr);

  DynArray<lint_t> another_array = {1,2,3,4,5,6,7,8,9,10};

  assert(another_array.equal({1,2,3,4,5,6,7,8,9,10}));


  SLList<lint_t> sllist = another_array.to_list();

  assert(sllist.equal({1,2,3,4,5,6,7,8,9,10}));

  DynArray<lint_t> arr;

  for (lint_t i = 0; i < 10000; ++i)
    arr.append(i);

  assert(arr.size() == 10000);
  assert(arr.get_first() == 0);
  assert(arr.get_last() == 9999);

  for (lint_t i = 0; i < 10000; ++i)
    arr.remove_last();

  assert(arr.is_empty());

  ArraySet<lint_t> unsorted_array_set;

  assert(unsorted_array_set.insert(3) != nullptr);
  assert(unsorted_array_set.insert(4) != nullptr);
  assert(unsorted_array_set.append(1) != nullptr);
  assert(unsorted_array_set.append(5) != nullptr);
  assert(unsorted_array_set.append(6) != nullptr);
  assert(unsorted_array_set.append(7) != nullptr);

  assert(unsorted_array_set.append(6) == nullptr);
  assert(unsorted_array_set.insert(6) == nullptr);

  assert(unsorted_array_set.equal({3,4,1,5,6,7}));

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
  assert(not unsorted_array_set.remove(0));
  assert(not unsorted_array_set.remove(9));

  assert(unsorted_array_set.equal({3,4,1,5,7}));
  
  SortedArraySet<lint_t> sorted_array_set;

  assert(sorted_array_set.insert(3) != nullptr);
  assert(sorted_array_set.insert(4) != nullptr);
  assert(sorted_array_set.append(1) != nullptr);
  assert(sorted_array_set.append(5) != nullptr);
  assert(sorted_array_set.append(6) != nullptr);
  assert(sorted_array_set.append(7) != nullptr);

  assert(sorted_array_set.append(6) == nullptr);
  assert(sorted_array_set.insert(6) == nullptr);

  assert(sorted_array_set.equal({1,3,4,5,6,7}));

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
  assert(not sorted_array_set.remove(0));
  assert(not sorted_array_set.remove(9));

  assert(sorted_array_set.equal({1,3,4,5,7}));

  ArraySet<lint_t> s1 = {1,2,3,4};
  ArraySet<lint_t> s2 = {3,4,5,6};

  assert(s1.equal({1,2,3,4}));
  assert(s2.equal({3,4,5,6}));

  assert(s1.join(s2).equal({1,2,3,4,5,6}));

  assert(s1.intersect(s2).equal({3,4}));

  assert(s1.difference(s2).equal({1,2}));

  assert(s2.difference(s1).equal({5,6}));
  
  assert(s1.cartesian_product(s2).equal({{1,3},{1,4},{1,5},{1,6},{2,3},{2,4},{2,5},{2,6},{3,3},{3,4},{3,5},{3,6},{4,3},{4,4},{4,5},{4,6}}));
			
  assert(s1.zip(s2).equal({{1,3},{2,4},{3,5},{4,6}}));

  rng_t rng(time(nullptr));

  DynArray<lint_t> aa;

  for (lint_t i = 0; i < 20; ++i)
    aa.append(random_uniform(rng, 100));

  insertion_sort(aa);
  assert(aa.is_sorted());

  aa.clear();

  for (lint_t i = 0; i < 1000; ++i)
    aa.append(random_uniform(rng, 100));

  auto sa = sort(aa);
  assert(sa.is_sorted());
  assert(not aa.is_sorted());

  inline_sort(aa);
  assert(aa.is_sorted());

  assert(reverse(aa).is_sorted([](auto x, auto y) { return x > y; }));
  
  cout << "Everything ok!\n";
  
  return 0;
}
