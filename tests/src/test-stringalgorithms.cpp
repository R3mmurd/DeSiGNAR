/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <stringalgorithms.hpp>
#include <string>
#include <random.hpp>

using namespace Designar;
using namespace std;

int main()
{
  {
    string text = "ababcababcabc";
    string pattern = "abc";
    auto m1 = kmp_search(text, pattern);
    auto m2 = rabin_karp_search(text, pattern);
    assert(m1.size() == m2.size());
    for (nat_t i = 0; i < m1.size(); ++i)
      assert(m1[i] == m2[i]);

    // manual check
    assert(m1.size() == 3);
    assert(m1[0] == 2);
    assert(m1[1] == 7);
    assert(m1[2] == 10);
  }

  {
    // overlapping matches
    string text = "aaaaa";
    string pattern = "aa";
    auto m = kmp_search(text, pattern);
    assert(m.size() == 4); // positions 0,1,2,3
  }

  {
    string text = "hello world";
    string pattern = "xyz";
    assert(kmp_search(text, pattern).is_empty());
    assert(rabin_karp_search(text, pattern).is_empty());
  }

  {
    string text = "abc";
    string pattern = "";
    assert(kmp_search(text, pattern).is_empty());
  }

  // Cross-check KMP vs Rabin-Karp vs naive brute force on random strings.
  rng_t rng(2026);
  for (int_t trial = 0; trial < 200; ++trial)
  {
    nat_t tlen = random_uniform(rng, 30) + 1;
    nat_t plen = random_uniform(rng, 6) + 1;
    string text, pattern;
    for (nat_t i = 0; i < tlen; ++i)
      text += char('a' + random_uniform(rng, 3)); // small alphabet -> lots of matches
    for (nat_t i = 0; i < plen; ++i)
      pattern += char('a' + random_uniform(rng, 3));

    DynArray<nat_t> brute;
    if (pattern.size() <= text.size())
      for (nat_t i = 0; i + pattern.size() <= text.size(); ++i)
        if (text.compare(i, pattern.size(), pattern) == 0)
          brute.append(i);

    auto kmp_result = kmp_search(text, pattern);
    auto rk_result = rabin_karp_search(text, pattern);

    assert(kmp_result.size() == brute.size());
    assert(rk_result.size() == brute.size());
    for (nat_t i = 0; i < brute.size(); ++i)
    {
      assert(kmp_result[i] == brute[i]);
      assert(rk_result[i] == brute[i]);
    }
  }

  // edit distance
  assert(edit_distance(string("kitten"), string("sitting")) == 3);
  assert(edit_distance(string(""), string("abc")) == 3);
  assert(edit_distance(string("abc"), string("")) == 3);
  assert(edit_distance(string("abc"), string("abc")) == 0);
  assert(edit_distance(string("flaw"), string("lawn")) == 2);

  cout << "StringAlgorithms: Everything ok!\n";
  return 0;
}
