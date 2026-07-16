/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <array.hpp>
#include <list.hpp>
#include <sort.hpp>
#include <random.hpp>

using namespace std;
using namespace Designar;

int main()
{
  SLList<int_t> list;

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

  assert(list.size() == 0);

  assert(list.is_empty());

  for (int_t i = 0; i < 5; ++i)
    list.append(i + 1);

  assert(not list.is_empty());
  assert(list.size() == 5);
  assert(list.get_first() == 1);
  assert(list.get_last() == 5);

  assert(list.equal({1, 2, 3, 4, 5}));

  auto flist = list.filter([](const auto& item)
                           { return item % 2 == 0; });

  assert(flist.equal({2, 4}));

  auto mlist = list.map([](const auto& item)
                        { return item * 2; });

  assert(mlist.equal({2, 4, 6, 8, 10}));

  auto sum = list.fold(0, [](const auto& item, const auto& acc)
                       { return item + acc; });

  assert(sum == 15);

  auto prod = list.fold(1, [](const auto& item, const auto& acc)
                        { return item * acc; });

  assert(prod == 120);

  assert(not list.all([](const auto& item)
                      { return item % 2 == 0; }));
  assert(flist.all([](const auto& item)
                   { return item % 2 == 0; }));
  assert(mlist.all([](const auto& item)
                   { return item % 2 == 0; }));

  assert(list.exists([](const auto& item)
                     { return item % 2 == 0; }));
  assert(flist.exists([](const auto& item)
                      { return item % 2 == 0; }));
  assert(mlist.exists([](const auto& item)
                      { return item % 2 == 0; }));

  assert(not list.none([](const auto& item)
                       { return item % 2 == 0; }));
  assert(not flist.none([](const auto& item)
                        { return item % 2 == 0; }));
  assert(not mlist.none([](const auto& item)
                        { return item % 2 == 0; }));

  assert(not list.none([](const auto& item)
                       { return item % 2 != 0; }));
  assert(flist.none([](const auto& item)
                    { return item % 2 != 0; }));
  assert(mlist.none([](const auto& item)
                    { return item % 2 != 0; }));

  for (nat_t i = 0; i < list.size(); ++i)
    assert(list.nth(i) == i + 1);

  assert(list.search_ptr([](const auto& item)
                         { return item == 3; }) != nullptr);

  assert(list.search_ptr([](const auto& item)
                         { return item > 5; }) == nullptr);

  SLList<int_t> list_cpy = list;

  assert(list_cpy.size() == 5);

  assert(list_cpy.equal({1, 2, 3, 4, 5}));

  SLList<int_t> list_mv = std::move(list_cpy);

  assert(list_cpy.size() == 0);
  assert(list_mv.size() == 5);

  assert(list_mv.equal({1, 2, 3, 4, 5}));
  assert(list_cpy.equal({}));

  SLList<int_t> another_list = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

  for (nat_t i = 0; i < another_list.size(); ++i)
    assert(another_list[i] == i + 1);

  try
  {
    another_list[10];
    assert(false);
  }
  catch (const out_of_range&)
  {
  }
  catch (...)
  {
    assert(false);
  }

  assert(another_list.equal({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

  DynArray<int_t> array = another_list.to_array();

  assert(array.equal({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}));

  assert(another_list.zip({1, 2, 3, 4, 5}).equal({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}}));

  assert(another_list.zip_left({1, 2, 3, 4, 5}).equal({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 1}, {7, 2}, {8, 3}, {9, 4}, {10, 5}}));

  assert(another_list.zip_right({1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12}).equal({{1, 1}, {2, 2}, {3, 3}, {4, 4}, {5, 5}, {6, 6}, {7, 7}, {8, 8}, {9, 9}, {10, 10}, {1, 11}, {2, 12}}));

  rng_t rng(get_random_seed());

  SLList<int_t> ll;

  for (int_t i = 0; i < 20; ++i)
    ll.append(random_uniform(rng, 100));

  auto sl = sort(ll);
  assert(sl.is_sorted());
  assert(not ll.is_sorted());

  inline_sort(ll);
  assert(ll.is_sorted());

  assert(reverse(ll).is_sorted([](auto x, auto y)
                               { return x > y; }));

  // NodeSLList::split(): round-robin distributes `this`'s nodes between
  // `l` and `r` (l gets the odd one out on odd-length lists). This is a
  // NodeSLList-level operation (like insert(Node*)/append(Node*)/
  // remove_first()), so it is exercised directly on NodeSLList here
  // rather than through SLList, whose own num_items bookkeeping split()
  // does not update.
  {
    auto collect = [](NodeSLList<int_t>& l)
    {
      DynArray<int_t> items;
      for (SLNode<int_t>* n = l.get_first(); n != nullptr; n = n->get_next())
      {
        items.append(n->get_item());
      }
      return items;
    };

    // Even length: splits evenly, nothing left over.
    {
      NodeSLList<int_t> src, l, r;
      for (int_t v : {1, 2, 3, 4})
      {
        src.append(new SLNode<int_t>(v));
      }

      src.split(l, r);

      assert(src.is_empty());
      assert(collect(l).equal({1, 3}));
      assert(collect(r).equal({2, 4}));

      // Regression: append(this->remove_first()) on the second half of
      // each pair used to run unconditionally, so on the last pair of an
      // odd-length list it appended a null node, corrupting `r`'s tail
      // (leaving it null while its head still pointed at a real node)
      // and crashing the next real append. Appending one more real node
      // to each after split() must still work.
      l.append(new SLNode<int_t>(100));
      r.append(new SLNode<int_t>(200));
      assert(collect(l).equal({1, 3, 100}));
      assert(collect(r).equal({2, 4, 200}));

      while (!l.is_empty())
      {
        delete l.remove_first();
      }

      while (!r.is_empty())
      {
        delete r.remove_first();
      }
    }

    // Odd length: the odd node out goes to `l`, not `r`.
    {
      NodeSLList<int_t> src, l, r;
      for (int_t v : {1, 2, 3, 4, 5})
      {
        src.append(new SLNode<int_t>(v));
      }

      src.split(l, r);

      assert(src.is_empty());
      assert(collect(l).equal({1, 3, 5}));
      assert(collect(r).equal({2, 4}));

      // Same post-split corruption check as above, on the odd-length
      // case that used to trigger append(nullptr) on `r`.
      r.append(new SLNode<int_t>(200));
      assert(collect(r).equal({2, 4, 200}));

      while (!l.is_empty())
      {
        delete l.remove_first();
      }

      while (!r.is_empty())
      {
        delete r.remove_first();
      }
    }

    cout << "NodeSLList::split(): Everything ok!\n";
  }

  cout << "Everything ok!\n";

  return 0;
}
