/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <set.H>
# include <sort.H>

using namespace std;
using namespace Designar;

int main()
{
  HashSet<lint_t> hash_set;

  for (lint_t i = 0; i < 10; ++i)
    assert(hash_set.insert(i + 1) != nullptr);

  assert(hash_set.insert(5) == nullptr);

  assert(hash_set.search(4) != nullptr);
  assert(hash_set.search(6) != nullptr);
  assert(hash_set.search(15) == nullptr);

  assert(hash_set.remove(6));
  assert(not hash_set.remove(15));

  assert(hash_set.search(4) != nullptr);
  assert(hash_set.search(6) == nullptr);
  assert(hash_set.search(15) == nullptr);
  
  HashSet<lint_t> another_hash_set = { 1,2,3,4,5 };

  assert(another_hash_set.size() == 5);

  assert(not another_hash_set.all([] (auto item) { return item % 2 == 0; }));
  assert(another_hash_set.exists([] (auto item) { return item % 2 == 0; }));

  auto fhash = another_hash_set.filter([] (auto item)
				       {
					 return item % 2 == 0;
				       });

  assert(sort(fhash).equal({2,4}));
  

  auto mhash = another_hash_set.map([] (auto item)
				    {
				      return item * 2;
				    });

  assert(mhash.size() == 5);
  assert(sort(mhash).equal({2,4,6,8,10}));

  lint_t sum = another_hash_set.fold(0, [] (auto item, auto acc)
					{
					  return item + acc;
					});

  assert(sum == 15);

  lint_t prod = another_hash_set.fold(1, [] (auto item, auto acc)
					 {
					   return item * acc;
					 });

  assert(prod == 120);

  HashSet<lint_t> another_hash_set_cpy = another_hash_set;

  assert(another_hash_set_cpy.size() == 5);
  assert(another_hash_set_cpy.size() == another_hash_set.size());
 
  HashSet<lint_t> another_hash_set_mv = move(another_hash_set_cpy);
  assert(another_hash_set_cpy.is_empty());

  HashSet<lint_t> hs;

  for (lint_t i = 0; i < 100000; ++i)
    hs.append(i + 1);

  assert(hs.size() == 100000);

  for (lint_t i = 0; i < 100000; ++i)
    hs.remove(i + 1);

  assert(hs.size() == 0);
  
  HashSet<lint_t> s1 = {1,2,3,4};
  HashSet<lint_t> s2 = {3,4,5,6};

  auto js1s2 = s1.join(s2).to_array();

  assert(sort(js1s2).equal({1,2,3,4,5,6}));

  auto is1s2 = s1.intersect(s2).to_array();
  assert(sort(is1s2).equal({3,4}));

  auto ds1s2 = s1.difference(s2).to_array();
  assert(sort(ds1s2).equal({1,2}));
  
  auto cs1s2 = s1.cartesian_product(s2);

  assert(sort(cs1s2, [] (auto p, auto q)
	      {
		if (p.first < q.first)
		  return true;
		
		if (p.first > q.first)
		  return false;
		
		return p.second < q.second;
	      }).equal({{1,3},{1,4},{1,5},{1,6},{2,3},{2,4},{2,5},{2,6},{3,3},{3,4},{3,5},{3,6},{4,3},{4,4},{4,5},{4,6}}));
  
  cout << "Everything ok!\n";
  
  return 0;
}
