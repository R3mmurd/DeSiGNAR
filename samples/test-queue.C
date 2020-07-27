/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <queue.H>

using namespace std;
using namespace Designar;

int main()
{
  FixedQueue<lint_t, 10> fixed_queue;

  assert(fixed_queue.is_empty());
  assert(not fixed_queue.is_full());
  assert(fixed_queue.size() == 0);
  assert(fixed_queue.get_capacity() == 10);

  fixed_queue.put(1);

  assert(fixed_queue.front() == 1);
  assert(fixed_queue.rear() == 1);
  assert(fixed_queue.size() == 1);
  assert(not fixed_queue.is_empty());
  assert(not fixed_queue.is_full());

  fixed_queue.put(5);

  assert(fixed_queue.front() == 1);
  assert(fixed_queue.rear() == 5);
  assert(fixed_queue.size() == 2);

  lint_t p = fixed_queue.get();

  assert(fixed_queue.front() == 5);
  assert(fixed_queue.rear() == 5);
  assert(fixed_queue.size() == 1);
  assert(p == 1);

  fixed_queue.put(2);
  fixed_queue.put(3);
  fixed_queue.put(4);
  fixed_queue.put(5);
  fixed_queue.put(6);
  fixed_queue.put(7);
  fixed_queue.put(8);
  fixed_queue.put(9);
  fixed_queue.put(10);

  assert(fixed_queue.is_full());

  try
    {
      fixed_queue.put(11);
      assert(false);
    }
  catch(overflow_error)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }

  fixed_queue.get();
  fixed_queue.get();
  fixed_queue.get();
  fixed_queue.get();

  FixedQueue<lint_t, 10> fixed_queue_cpy = fixed_queue;
  
  assert(fixed_queue_cpy.front() == 5);
  assert(fixed_queue_cpy.rear() == 10);
  assert(fixed_queue_cpy.size() == 6);

  FixedQueue<lint_t, 10> fixed_queue_mv = move(fixed_queue_cpy);

  assert(fixed_queue_mv.front() == 5);
  assert(fixed_queue_mv.rear() == 10);
  assert(fixed_queue_mv.size() == 6);
  assert(fixed_queue_cpy.is_empty());

  fixed_queue.get();
  fixed_queue.get();
  fixed_queue.get();
  fixed_queue.get();
  fixed_queue.get();
  fixed_queue.get();

  assert(fixed_queue.is_empty());

  try
    {
      fixed_queue.front();
      assert(false);
    }
  catch(underflow_error)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }
  
  try
    {
      fixed_queue.get();
      assert(false);
    }
  catch(underflow_error)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }

  DynQueue<lint_t> array_queue;

  assert(array_queue.is_empty());
  assert(array_queue.size() == 0);

  array_queue.put(1);

  assert(array_queue.front() == 1);
  assert(array_queue.rear() == 1);
  assert(array_queue.size() == 1);
  assert(not array_queue.is_empty());

  array_queue.put(5);

  assert(array_queue.front() == 1);
  assert(array_queue.rear() == 5);
  assert(array_queue.size() == 2);

  p = array_queue.get();

  assert(array_queue.front() == 5);
  assert(array_queue.rear() == 5);
  assert(array_queue.size() == 1);
  assert(p == 1);

  array_queue.put(2);
  array_queue.put(3);
  array_queue.put(4);
  array_queue.put(5);
  array_queue.put(6);
  array_queue.put(7);
  array_queue.put(8);
  array_queue.put(9);
  array_queue.put(10);
  array_queue.put(11);
  
  array_queue.get();
  array_queue.get();
  array_queue.get();
  array_queue.get();

  DynQueue<lint_t> array_queue_cpy = array_queue;
  
  assert(array_queue_cpy.front() == 5);
  assert(array_queue_cpy.rear() == 11);
  assert(array_queue_cpy.size() == 7);

  DynQueue<lint_t> array_queue_mv = move(array_queue_cpy);

  assert(array_queue_mv.front() == 5);
  assert(array_queue_mv.rear() == 11);
  assert(array_queue_mv.size() == 7);
  assert(array_queue_cpy.is_empty());

  array_queue.get();
  array_queue.get();
  array_queue.get();
  array_queue.get();
  array_queue.get();
  array_queue.get();
  array_queue.get();

  assert(array_queue.is_empty());

  try
    {
      array_queue.front();
      assert(false);
    }
  catch(underflow_error)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }
  
  try
    {
      array_queue.get();
      assert(false);
    }
  catch(underflow_error)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }

  DynQueue<lint_t> arr_queue;

  for (lint_t i = 0; i < 10000; ++i)
    arr_queue.put(i);

  for (lint_t i = 0; i < 10000; ++i)
    assert(arr_queue.get() == i);

  ListQueue<lint_t> list_queue;

  assert(list_queue.is_empty());
  assert(list_queue.size() == 0);

  list_queue.put(1);

  assert(list_queue.front() == 1);
  assert(list_queue.rear() == 1);
  assert(list_queue.size() == 1);
  assert(not list_queue.is_empty());

  list_queue.put(5);

  assert(list_queue.front() == 1);
  assert(list_queue.rear() == 5);
  assert(list_queue.size() == 2);

  p = list_queue.get();

  assert(list_queue.front() == 5);
  assert(list_queue.rear() == 5);
  assert(list_queue.size() == 1);
  assert(p == 1);

  list_queue.put(2);
  list_queue.put(3);
  list_queue.put(4);
  list_queue.put(5);
  list_queue.put(6);
  list_queue.put(7);
  list_queue.put(8);
  list_queue.put(9);
  list_queue.put(10);
  list_queue.put(11);
  
  list_queue.get();
  list_queue.get();
  list_queue.get();
  list_queue.get();

  ListQueue<lint_t> list_queue_cpy = list_queue;
  
  assert(list_queue_cpy.front() == 5);
  assert(list_queue_cpy.rear() == 11);
  assert(list_queue_cpy.size() == 7);

  ListQueue<lint_t> list_queue_mv = move(list_queue_cpy);

  assert(list_queue_mv.front() == 5);
  assert(list_queue_mv.rear() == 11);
  assert(list_queue_mv.size() == 7);
  assert(list_queue_cpy.is_empty());

  list_queue.get();
  list_queue.get();
  list_queue.get();
  list_queue.get();
  list_queue.get();
  list_queue.get();
  list_queue.get();

  assert(list_queue.is_empty());

  try
    {
      list_queue.front();
      assert(false);
    }
  catch(underflow_error)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }
  
  try
    {
      list_queue.get();
      assert(false);
    }
  catch(underflow_error)
    {
      assert(true);
    }
  catch(...)
    {
      assert(false);
    }
  
  cout << "Everything ok!\n";
  return 0;
}
