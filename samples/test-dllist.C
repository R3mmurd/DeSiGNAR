/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <array.H>
# include <list.H>
# include <sort.H>
# include <random.H>

using namespace std;
using namespace Designar;

int main()
{
  DLList<lint_t> list;

  assert(list.is_empty());

  list.insert(2);

  assert(not list.is_empty());
  assert(list.size() == 1);
  assert(list.get_first() == 2);
  assert(list.get_last() == 2);

  list.insert(1);

  assert(not list.is_empty());
  assert(list.size() == 2);
  assert(list.get_first() == 1);
  assert(list.get_last() == 2);

  list.append(3);

  assert(not list.is_empty());
  assert(list.size() == 3);
  assert(list.get_first() == 1);
  assert(list.get_last() == 3);

  for (nat_t i = 0; i < 10; ++i)
    list.append(list.get_last() + 1);

  assert(not list.is_empty());
  assert(list.size() == 13);
  assert(list.get_first() == 1);
  assert(list.get_last() == 13);

  list.clear();

  assert(list.is_empty());

  for (lint_t i = 0; i < 5; ++i)
    list.append(i + 1);

  assert(not list.is_empty());
  assert(list.size() == 5);
  assert(list.get_first() == 1);
  assert(list.get_last() == 5);
  
  assert(list.equal({1,2,3,4,5}));

  auto flist = list.filter([] (const auto & item) { return item % 2 == 0; });

  assert(flist.equal({2,4}));

  auto mlist = list.map([] (const auto & item) { return item * 2; });

  assert(mlist.equal({2,4,6,8,10}));

  auto sum = list.fold(0, [] (const auto & item, const auto & acc)
		       {
			 return item + acc;
		       });

  assert(sum == 15);

  auto prod = list.fold(1, [] (const auto & item, const auto & acc)
			{
			  return item * acc;
			});

  assert(prod == 120);

  assert(not list.all([] (const auto & item) { return item % 2 == 0; }));
  assert(flist.all([] (const auto & item) { return item % 2 == 0; }));
  assert(mlist.all([] (const auto & item) { return item % 2 == 0; }));

  assert(list.exists([] (const auto & item) { return item % 2 == 0; }));
  assert(flist.exists([] (const auto & item) { return item % 2 == 0; }));
  assert(mlist.exists([] (const auto & item) { return item % 2 == 0; }));

  assert(not list.none([] (const auto & item) { return item % 2 == 0; }));
  assert(not flist.none([] (const auto & item) { return item % 2 == 0; }));
  assert(not mlist.none([] (const auto & item) { return item % 2 == 0; }));

  assert(not list.none([] (const auto & item) { return item % 2 != 0; }));
  assert(flist.none([] (const auto & item) { return item % 2 != 0; }));
  assert(mlist.none([] (const auto & item) { return item % 2 != 0; }));

  DLList<lint_t> list_cpy = list;

  assert(list_cpy.size() == 5);
  assert(list.equal({1,2,3,4,5}));
  assert(list_cpy.equal({1,2,3,4,5}));

  DLList<lint_t> list_mv = move(list_cpy);

  assert(list_cpy.size() == 0);
  assert(list_mv.size() == 5);

  assert(list_cpy.equal({}));
  assert(list_mv.equal({1,2,3,4,5}));
	 
  for (nat_t i = 0; i < list.size(); ++i)
    assert(list.nth(i) == i + 1);

  assert(list.search_ptr([] (const auto & item) { return item == 3; })
	 != nullptr);

  assert(list.search_ptr([] (const auto & item) { return item > 5; })
	 == nullptr);

  DLList<lint_t> another_list = {1,2,3,4,5,6,7,8,9,10};

  for (nat_t i = 0; i < another_list.size(); ++i)
    assert(another_list[i] == i + 1);

  try
    {
      another_list[10];
      assert(false);
    }
  catch(out_of_range)
    {
    }
  catch(...)
    {
      assert(false);
    }

  
  SLList<lint_t> sllist = another_list.to_list();

  assert(sllist.equal({1,2,3,4,5,6,7,8,9,10}));

  DynArray<lint_t> array = another_list.to_array();

  assert(array.equal({1,2,3,4,5,6,7,8,9,10}));

  rng_t rng(get_random_seed());

  DLList<lint_t> ll;

  for (lint_t i = 0; i < 20; ++i)
    ll.append(random_uniform(rng, 100));


  auto sl = sort(ll);
  assert(sl.is_sorted());
  assert(not ll.is_sorted());

  inline_sort(ll);
  assert(ll.is_sorted());

  assert(reverse(ll).is_sorted([](auto x, auto y) { return x > y; }));
  
  cout << "Everything ok!\n";
  
  return 0;
}
