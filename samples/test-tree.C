/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <set.H>

using namespace std;
using namespace Designar;

int main()
{
  TreeSet<lint_t> tree = { 2,4,6,8,10 };

  assert(tree.verify());
  
  assert(tree.size() == 5);

  assert(tree.select(0) == 2);
  assert(tree.select(3) == 8);

  assert(tree.min() == 2);
  assert(tree.max() == 10);

  assert(tree.equal({2,4,6,8,10}));

  try
    {
      tree.select(5);
      assert(false);
    }
  catch(out_of_range)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }

  assert(tree[0] == 2);
  assert(tree[3] == 8);

  try
    {
      tree[5];
      assert(false);
    }
  catch(out_of_range)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }

  assert(tree.position(2) == 0);
  assert(tree.position(8) == 3);
  assert(tree.position(0) == -1);  

  TreeSet<lint_t> tree_cpy = tree;

  assert(tree_cpy.size() == 5);

  assert(tree_cpy.search(4) != nullptr);
  assert(tree_cpy.search(0) == nullptr);

  assert(tree_cpy.remove(4));
  assert(tree_cpy.size() == 4);
  assert(tree_cpy.search(4) == nullptr);
  assert(not tree_cpy.remove(0));
  assert(tree_cpy.size() == 4);
  assert(tree_cpy.verify());

  assert(tree_cpy.equal({2,6,8,10}));

  auto p = tree.split_key(5);

  assert(tree.is_empty());
  assert(get<0>(p).verify());
  assert(get<0>(p).size() == 2);
  assert(get<1>(p).verify());
  assert(get<1>(p).size() == 3);

  assert(get<0>(p).equal({2,4}));
  assert(get<1>(p).equal({6,8,10}));
  
  tree.exclusive_join(get<0>(p), get<1>(p));
  assert(get<0>(p).is_empty());
  assert(get<1>(p).is_empty());
  assert(tree.size() == 5);
  assert(tree.verify());

  assert(tree.equal({2,4,6,8,10}));

  auto q = tree.split_key(6);
  assert(get<0>(q).size() == 0);
  assert(get<1>(q).size() == 0);
  assert(tree.size() == 5);

  TreeSet<lint_t> tree_mv = move(tree_cpy);
  assert(tree_cpy.is_empty());
  assert(tree_mv.size() == 4);

  TreeSet<lint_t> tree_dup;
  for (int i = 0; i < 5; ++i)
    for (int j = 0; j < 5; ++j)
      tree_dup.insert_dup(i + 1);

  assert(tree_dup.size() == 25);
  assert(tree_dup.equal<SLList<lint_t>>(
    {1,1,1,1,1,2,2,2,2,2,3,3,3,3,3,4,4,4,4,4,5,5,5,5,5}));
  assert(tree_dup.verify_dup());

  auto pdup = tree_dup.split_key_dup(3);
  assert(tree_dup.is_empty());
  assert(get<0>(pdup).size() == 15);
  assert(get<1>(pdup).size() == 10);
  assert(get<0>(pdup).verify_dup());
  assert(get<1>(pdup).verify_dup());
  assert(get<0>(pdup).equal<SLList<lint_t>>({1,1,1,1,1,2,2,2,2,2,3,3,3,3,3}));
  assert(get<1>(pdup).equal<SLList<lint_t>>({4,4,4,4,4,5,5,5,5,5}));

  tree_dup.join_dup(get<0>(pdup), get<1>(pdup));
  assert(get<0>(pdup).is_empty());
  assert(get<1>(pdup).is_empty());
  assert(tree_dup.size() == 25);  

  auto list = tree.to_list();

  assert(list.equal({2,4,6,8,10}));

  auto array = tree.to_array();

  assert(array.equal({2,4,6,8,10}));

  assert(tree.filter([] (auto item)
		     {
		       return item % 2 == 0;
		     }).equal({2,4,6,8,10}));

  assert(tree.map([] (auto item)
		  {
		    return item * 2;
		  }).equal({4,8,12,16,20}));
	 
  assert(tree.fold(0, [] (auto item, auto acc) { return item + acc; }) == 30);

  assert(tree.fold(1, [] (auto item, auto acc) { return item * acc; }) == 3840);

  assert(tree.all([] (auto item) { return item % 2 == 0; }));
  assert(not tree.exists([] (auto item) { return item % 2 != 0; }));
  assert(tree.none([] (const auto & item) { return item % 2 != 0; }));

  TreeSet<lint_t>  ts1 = {1,2,3,4};
  TreeSet<lint_t>  ts2 = {3,4,5,6};
  ArraySet<lint_t> as2 = {3,4,5,6};

  assert(ts1.join(ts2).equal({1,2,3,4,5,6}));

  assert(ts1.intersect(ts2).equal({3,4}));

  assert(ts1.difference(ts2).equal({1,2}));

  assert(ts2.difference(ts1).equal({5,6}));

  assert(ts1.cartesian_product(ts2).equal({{1,3},{1,4},{1,5},{1,6},{2,3},{2,4},{2,5},{2,6},{3,3},{3,4},{3,5},{3,6},{4,3},{4,4},{4,5},{4,6}}));
  
  assert(ts1.cartesian_product(as2).equal({{1,3},{1,4},{1,5},{1,6},{2,3},{2,4},{2,5},{2,6},{3,3},{3,4},{3,5},{3,6},{4,3},{4,4},{4,5},{4,6}}));

  assert(ts1.zip(ts2).equal({{1,3},{2,4},{3,5},{4,6}}));

  TreeSet<int> ttt{1,2,3,4,5,6,7,8,9,10};

  ttt.remove_first_if([] (auto item) { return item > 5; });
  assert(ttt.equal({1,2,3,4,5,7,8,9,10}));

  ttt.remove_if( [] (int x) { return (x & 1); });
  assert(ttt.equal({2,4,8,10}));
  
  cout << "Everything ok!\n";

  return 0;
}
