/*
  This file is part of Designar.
  Copyright (C) 2017 by Alejandro J. Mujica

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Any user request of this software, write to 

  Alejandro Mujica

  aledrums@gmail.com
*/

# include <array.H>
# include <tree.H>

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

  TreeSet<lint_t> tree_mv = move(tree_cpy);
  assert(tree_cpy.is_empty());
  assert(tree_mv.size() == 4);

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
  
  cout << "Everything ok!\n";

  return 0;
}
