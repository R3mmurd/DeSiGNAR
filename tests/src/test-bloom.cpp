/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <bloomfilter.hpp>
#include <random.hpp>

using namespace Designar;
using namespace std;

int main()
{
  nat_t n = 1000;
  real_t target_fp = 0.01;
  nat_t m = BloomFilter<int_t>::optimal_num_bits(n, target_fp);
  nat_t k = BloomFilter<int_t>::optimal_num_hashes(m, n);

  BloomFilter<int_t> bf(m, k);
  assert(bf.size() == 0);
  assert(bf.bit_count() == m);
  assert(bf.hash_count() == k);

  DynArray<int_t> inserted;
  for (int_t i = 0; i < (int_t)n; ++i)
  {
    bf.insert(i);
    inserted.append(i);
  }
  assert(bf.size() == n);

  // No false negatives, ever.
  for (int_t x : inserted)
    assert(bf.might_contain(x));

  // Empirically measure false-positive rate on keys known not to be
  // inserted; must be reasonably close to the target (loose bound, this
  // is a randomized structure, not an exact one).
  nat_t false_positives = 0;
  nat_t trials = 20000;
  for (int_t i = (int_t)n; i < (int_t)n + (int_t)trials; ++i)
    if (bf.might_contain(i))
      ++false_positives;

  real_t observed_fp = real_t(false_positives) / real_t(trials);
  cout << "observed false-positive rate: " << observed_fp
       << " (target " << target_fp << ")\n";
  assert(observed_fp < target_fp * 5); // generous margin, still catches a badly broken filter

  bf.clear();
  assert(bf.size() == 0);
  for (int_t x : inserted)
    assert(!bf.might_contain(x)); // cleared: no false claims of membership (probabilistically expected all-empty)

  bool threw = false;
  try
  {
    BloomFilter<int_t> bad(0, 1);
  }
  catch (const domain_error&)
  {
    threw = true;
  }
  assert(threw);

  cout << "BloomFilter: Everything ok!\n";
  return 0;
}
