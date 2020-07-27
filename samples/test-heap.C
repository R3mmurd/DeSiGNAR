/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <heap.H>
# include <random.H>

using namespace std;

using namespace Designar;

int main()
{
  FixedHeap<lint_t> fixed_heap;
  lint_t min_int = 0;

  assert(fixed_heap.is_empty());
  assert(not fixed_heap.is_full());
  assert(fixed_heap.get_capacity() == 100);

  fixed_heap.insert(3);
  fixed_heap.insert(22);
  fixed_heap.insert(18);
  fixed_heap.insert(10);
  fixed_heap.insert(1);
  fixed_heap.insert(42);
  fixed_heap.insert(15);
  fixed_heap.insert(9);
  fixed_heap.insert(0);

  min_int = std::numeric_limits<lint_t>::min();
  
  while (not fixed_heap.is_empty())
    {
      lint_t item = fixed_heap.get();
      assert(min_int <= item);
      min_int = item;
    }

  DynHeap<lint_t> array_heap;

  assert(array_heap.is_empty());

  array_heap.insert(3);
  array_heap.insert(22);
  array_heap.insert(18);
  array_heap.insert(10);
  array_heap.insert(1);
  array_heap.insert(42);
  array_heap.insert(15);
  array_heap.insert(9);
  array_heap.insert(0);

  min_int = std::numeric_limits<lint_t>::min();
  
  while (not array_heap.is_empty())
    {
      lint_t item = array_heap.get();
      assert(min_int <= item);
      min_int = item;
    }

  for (lint_t i = 0; i < 10000; ++i)
    array_heap.insert(i);

  for (lint_t i = 0; i < 10000; ++i)
    assert(array_heap.get() == i);

  LHeap<lint_t> lheap;
  
  lheap.insert(3);
  lheap.insert(22);
  lheap.insert(18);
  lheap.insert(10);
  lheap.insert(1);
  lheap.insert(42);
  lheap.insert(15);
  lheap.insert(9);
  lheap.insert(0);

  min_int = std::numeric_limits<lint_t>::min();
  
  while (not lheap.is_empty())
    {
      lint_t item = lheap.get();
      assert(min_int <= item);
      min_int = item;
    }

  constexpr nat_t max_num_items_to_remove = 100;
  nat_t current_item = 0;

  FixedArray<lint_t *> to_remove(max_num_items_to_remove);

  rng_t rng(time(nullptr));
  
  for (lint_t i = 0; i < 100000; ++i)
    {
      auto & item = lheap.insert(i);

      if (current_item == max_num_items_to_remove)
	continue;
      
      auto r = random(rng);

      if (r > 0.5)
	to_remove[current_item++] = &const_cast<lint_t &>(item);
    }

  for (nat_t i = 0; i < current_item; ++i)
    lheap.remove(*to_remove[i]);
  
  for (lint_t i = 0; i < 100000 - current_item; ++i)
    lheap.get();
  
  cout << "Everything ok!\n";
  
  return 0;
}
