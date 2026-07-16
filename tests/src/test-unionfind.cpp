/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <unionfind.hpp>
#include <random.hpp>

using namespace Designar;
using namespace std;

int main()
{
  DisjointSet<int_t> ds;
  for (int_t i = 0; i < 100; ++i)
    ds.make_set(i);

  for (int_t i = 0; i < 100; ++i)
    assert(ds.find(i) == i);

  ds.union_sets(1, 2);
  assert(ds.same_set(1, 2));
  assert(!ds.same_set(1, 3));

  ds.union_sets(2, 3);
  assert(ds.same_set(1, 3));
  assert(ds.same_set(2, 3));

  // union a bunch of pairs and verify connectivity via naive union-find semantics
  rng_t rng(4242);
  DynArray<pair<int_t, int_t>> unions;
  for (int_t i = 0; i < 300; ++i)
  {
    int_t a = random_uniform(rng, 100);
    int_t b = random_uniform(rng, 100);
    ds.union_sets(a, b);
    unions.append({a, b});
  }

  // brute-force reference: build connectivity via repeated closure
  DynArray<int_t> ref(100, 0);
  for (int_t i = 0; i < 100; ++i)
    ref[i] = i;

  auto ref_find = [&](int_t x)
  {
    while (ref[x] != x)
      x = ref[x];
    return x;
  };

  bool changed = true;
  while (changed)
  {
    changed = false;
    for (auto& pr : unions)
    {
      int_t ra = ref_find(pr.first);
      int_t rb = ref_find(pr.second);
      if (ra != rb)
      {
        ref[ra] = rb;
        changed = true;
      }
    }
  }

  for (int_t i = 0; i < 100; ++i)
    for (int_t j = 0; j < 100; ++j)
      assert(ds.same_set(i, j) == (ref_find(i) == ref_find(j)));

  // make_set is a no-op on existing key
  ds.make_set(1);
  assert(ds.num_keys() == 100);

  bool threw = false;
  try
  {
    ds.find(9999);
  }
  catch (const domain_error&)
  {
    threw = true;
  }
  assert(threw);

  cout << "DisjointSet: Everything ok!\n";
  return 0;
}
