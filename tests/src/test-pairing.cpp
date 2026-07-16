/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <pairingheap.hpp>
#include <random.hpp>
#include <array.hpp>

using namespace Designar;
using namespace std;

int main()
{
  PairingHeap<int_t> h;
  assert(h.is_empty());

  constexpr int_t N = 3000;
  rng_t rng(2026);
  DynArray<int_t> values;
  for (int_t i = 0; i < N; ++i)
    values.append(random_uniform(rng, 1000000));

  for (int_t v : values)
    h.insert(v);

  assert(h.size() == (nat_t)N);

  // extract_min repeatedly must yield non-decreasing order (heap property)
  int_t prev = std::numeric_limits<int_t>::min();
  nat_t count = 0;
  while (!h.is_empty())
  {
    int_t m = h.extract_min();
    assert(m >= prev);
    prev = m;
    ++count;
  }
  assert(count == (nat_t)N);
  assert(h.is_empty());

  // decrease_key test
  PairingHeap<int_t> h2;
  auto* n10 = h2.insert(10);
  auto* n20 = h2.insert(20);
  auto* n30 = h2.insert(30);
  (void)n30;
  assert(h2.get_min() == 10);

  h2.decrease_key(n20, 5);
  assert(h2.get_min() == 5);

  h2.decrease_key(n10, 1);
  assert(h2.get_min() == 1);

  bool threw = false;
  try
  {
    h2.decrease_key(n20, 100);
  }
  catch (const domain_error&)
  {
    threw = true;
  }
  assert(threw);

  // Dijkstra-like usage pattern: insert many, repeatedly decrease keys, drain in order
  PairingHeap<int_t> h3;
  DynArray<PairingHeap<int_t>::Node*> handles;
  for (int_t i = 0; i < 200; ++i)
    handles.append(h3.insert(1000 + i));

  for (int_t i = 0; i < 200; ++i)
    assert(handles[i]->get_key() == 1000 + i); // exercise accessor

  for (int_t i = 199; i >= 0; --i)
    h3.decrease_key(handles[i], i); // decreasing in reverse order, all distinct & smaller

  int_t prev3 = -1;
  while (!h3.is_empty())
  {
    int_t m = h3.extract_min();
    assert(m > prev3);
    prev3 = m;
  }

  PairingHeap<int_t> h4;
  for (int_t i = 0; i < 100; ++i)
    h4.insert(i);
  PairingHeap<int_t> h5 = h4; // copy
  h5.insert(-1);
  assert(h5.get_min() == -1);
  assert(h4.get_min() == 0);

  PairingHeap<int_t> h6 = std::move(h5);
  assert(h6.get_min() == -1);

  cout << "PairingHeap: Everything ok!\n";
  return 0;
}
