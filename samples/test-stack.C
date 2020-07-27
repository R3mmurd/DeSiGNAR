/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <stack.H>

using namespace std;
using namespace Designar;

int main()
{
  FixedStack<lint_t, 10> fixed_stack;

  assert(fixed_stack.is_empty());
  assert(not fixed_stack.is_full());
  assert(fixed_stack.size() == 0);
  assert(fixed_stack.get_capacity() == 10);

  fixed_stack.push(1);

  assert(fixed_stack.top() == 1);
  assert(fixed_stack.size() == 1);
  assert(not fixed_stack.is_empty());
  assert(not fixed_stack.is_full());

  fixed_stack.push(5);

  assert(fixed_stack.top() == 5);
  assert(fixed_stack.size() == 2);

  lint_t p = fixed_stack.pop();

  assert(fixed_stack.top() == 1);
  assert(fixed_stack.size() == 1);
  assert(p == 5);

  fixed_stack.push(2);
  fixed_stack.push(3);
  fixed_stack.push(4);
  fixed_stack.push(5);
  fixed_stack.push(6);
  fixed_stack.push(7);
  fixed_stack.push(8);
  fixed_stack.push(9);
  fixed_stack.push(10);

  assert(fixed_stack.is_full());

  try
    {
      fixed_stack.push(11);
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


  fixed_stack.popn(4);

  assert(fixed_stack.top() == 6);
  assert(fixed_stack.size() == 6);

  FixedStack<lint_t, 10> fixed_stack_cpy = fixed_stack;
  
  assert(fixed_stack_cpy.top() == 6);
  assert(fixed_stack_cpy.size() == 6);

  FixedStack<lint_t, 10> fixed_stack_mv = move(fixed_stack_cpy);

  assert(fixed_stack_mv.top() == 6);
  assert(fixed_stack_mv.size() == 6);
  assert(fixed_stack_cpy.is_empty());

  fixed_stack.popn(6);

  assert(fixed_stack.is_empty());

  try
    {
      fixed_stack.top();
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
      fixed_stack.pop();
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

  DynStack<lint_t> array_stack;
  
  assert(array_stack.is_empty());
  assert(array_stack.size() == 0);

  array_stack.push(1);

  assert(array_stack.top() == 1);
  assert(array_stack.size() == 1);
  assert(not array_stack.is_empty());

  array_stack.push(5);

  assert(array_stack.top() == 5);
  assert(array_stack.size() == 2);

  p = array_stack.pop();

  assert(array_stack.top() == 1);
  assert(array_stack.size() == 1);
  assert(p == 5);

  array_stack.push(2);
  array_stack.push(3);
  array_stack.push(4);
  array_stack.push(5);
  array_stack.push(6);
  array_stack.push(7);
  array_stack.push(8);
  array_stack.push(9);
  array_stack.push(10);
  array_stack.push(11);

  assert(array_stack.size() == 11);
  
  array_stack.popn(4);

  assert(array_stack.top() == 7);
  assert(array_stack.size() == 7);
  
  DynStack<lint_t> array_stack_cpy = array_stack;
   
  assert(array_stack_cpy.top() == 7);
  assert(array_stack_cpy.size() == 7);
  
  DynStack<lint_t> array_stack_mv = move(array_stack_cpy);
  
  assert(array_stack_mv.top() == 7);
  assert(array_stack_mv.size() == 7);
  assert(array_stack_cpy.is_empty());
  
  array_stack.popn(7);

  assert(array_stack.is_empty());
  
  try
    {
      array_stack.top();
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
      array_stack.pop();
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
  
  DynStack<lint_t> arr_stack;

  for (lint_t i = 0; i < 10000; ++i)
    arr_stack.push(i + 1);

  for (lint_t i = 0; i < 10000; ++i)
    assert(arr_stack.pop() == 10000 - i);
  
  ListStack<lint_t> list_stack;
  
  assert(list_stack.is_empty());
  assert(list_stack.size() == 0);

  list_stack.push(1);

  assert(list_stack.top() == 1);
  assert(list_stack.size() == 1);
  assert(not list_stack.is_empty());

  list_stack.push(5);

  assert(list_stack.top() == 5);
  assert(list_stack.size() == 2);

  p = list_stack.pop();

  assert(list_stack.top() == 1);
  assert(list_stack.size() == 1);
  assert(p == 5);

  list_stack.push(2);
  list_stack.push(3);
  list_stack.push(4);
  list_stack.push(5);
  list_stack.push(6);
  list_stack.push(7);
  list_stack.push(8);
  list_stack.push(9);
  list_stack.push(10);
  list_stack.push(11);

  assert(list_stack.size() == 11);
  
  list_stack.popn(4);

  assert(list_stack.top() == 7);
  assert(list_stack.size() == 7);
  
  ListStack<lint_t> list_stack_cpy = list_stack;
   
  assert(list_stack_cpy.top() == 7);
  assert(list_stack_cpy.size() == 7);
  
  ListStack<lint_t> list_stack_mv = move(list_stack_cpy);

  assert(list_stack_mv.top() == 7);
  assert(list_stack_mv.size() == 7);
  assert(list_stack_cpy.is_empty());
  
  list_stack.popn(7);

  assert(list_stack.is_empty());

  try
    {
      list_stack.top();
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
      list_stack.pop();
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
