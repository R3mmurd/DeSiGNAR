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

# ifndef DSGQUEUE_H
# define DSGQUEUE_H

# include <array.H>
# include <list.H>

namespace Designar
{

  template <typename T, nat_t cap = 100>
  class FixedQueue
  {
    T     array[cap];
    nat_t num_items;
    nat_t f;
    nat_t r;

    void copy_queue(const FixedQueue &);

    void swap(FixedQueue & q)
    {
      std::swap(array, q.array);
      std::swap(num_items, q.num_items);
      std::swap(f, q.f);
      std::swap(r, q.r);
    }

  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;

    FixedQueue()
      : num_items(0), f(0), r(cap - 1)
    {
      // empty
    }

    FixedQueue(const FixedQueue & q)
      : num_items(q.num_items)
    {
      copy_queue(q);
    }

    FixedQueue(FixedQueue && q)
      : FixedQueue()
    {
      swap(q);
    }

    FixedQueue & operator = (const FixedQueue & q)
    {
      if (this == &q)
	return *this;

      num_items = q.num_items;
      f = q.f;
      r = q.r;
      copy_queue(q);

      return *this;
    }

    FixedQueue & operator = (FixedQueue && q)
    {
      swap(q);
      return *this;
    }

    bool is_empty() const
    {
      return num_items == 0;
    }

    bool is_full() const
    {
      return num_items == cap;
    }

    nat_t size() const
    {
      return num_items;
    }

    nat_t get_capacity() const
    {
      return cap;
    }

    void clear()
    {
      num_items = 0;
      f = 0;
      r = cap - 1;
    }

    T & front()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return array[f];
    }
    
    const T & front() const
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return array[f];
    }

    T & rear()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return array[r];
    }
    
    const T & rear() const
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return array[r];
    }

    T & put(const T & item)
    {
      if (is_full())
	throw std::overflow_error("Queue is full");
      
      r = (r + 1) % cap;
      array[r] = item;
      ++num_items;
      return array[r];
    }

    T & put(T && item)
    {
      if (is_full())
	throw std::overflow_error("Queue is full");
      
      r = (r + 1) % cap;
      array[r] = std::move(item);
      ++num_items;
      return array[r];
    }

    T get()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      T ret_val = std::move(array[f]);
      f = (f + 1) % cap;
      --num_items;
      return ret_val;
    }
  };

  template <typename T, nat_t cap>
  void FixedQueue<T, cap>::copy_queue(const FixedQueue & q)
  {
    nat_t ii = q.f;
    
    for (nat_t i = 0; i < num_items; ++i)
      {
	array[i] = q.array[ii];
	ii = (ii + 1) % cap;
      }

    f = 0;
    r = num_items - 1;
  }

  template <typename T>
  class DynQueue : private FixedArray<T>
  {
    using BaseArray = FixedArray<T>;
    
    static constexpr nat_t MIN_SIZE = 32;
    static constexpr real_t RESIZE_FACTOR = 0.4;

    nat_t num_items;
    nat_t f;
    nat_t r;

    void copy_queue(const DynQueue &);

    void swap(DynQueue & q)
    {
      BaseArray::swap(q);
      std::swap(num_items, q.num_items);
      std::swap(f, q.f);
      std::swap(r, q.r);
    }

    void resize(nat_t);
    
    void resize_up()
    {
      if (num_items < BaseArray::get_capacity())
	return;
      
      assert(BaseArray::get_capacity() * (1 + RESIZE_FACTOR) > num_items);

      resize(BaseArray::get_capacity() * (1 + RESIZE_FACTOR));
    }

    void resize_down()
    {
      if (num_items > BaseArray::get_capacity() * RESIZE_FACTOR or
	  BaseArray::get_capacity() == MIN_SIZE)
	return;

      assert(BaseArray::get_capacity() * (1 - RESIZE_FACTOR) > num_items);

      resize(BaseArray::get_capacity() * (1 - RESIZE_FACTOR));
    }

  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;

    DynQueue()
      : BaseArray(MIN_SIZE), num_items(0), f(0), r(MIN_SIZE - 1)
    {
      // empty
    }

    DynQueue(const DynQueue & q)
      : BaseArray(q.get_capacity()), num_items(q.num_items)
    {
      copy_queue(q);
    }

    DynQueue(DynQueue && q)
      : BaseArray(), num_items(0), f(0), r(0)
    {
      swap(q);
    }

    DynQueue & operator = (const DynQueue & q)
    {
      if (this == &q)
	return *this;

      BaseArray arr(q.get_capacity());
      BaseArray::swap(arr);
      num_items = q.num_items;
      copy_queue(q);
      
      return *this;
    }

    DynQueue & operator = (DynQueue && q)
    {
      swap(q);
      return *this;
    }

    bool is_empty() const
    {
      return num_items == 0;
    }

    nat_t size() const
    {
      return num_items;
    }

    void clear()
    {
      num_items = 0;
      
      if (BaseArray::get_capacity() != MIN_SIZE)
	{
	  BaseArray new_array(MIN_SIZE);
	  BaseArray::swap(new_array);
	}

      f = 0;
      r = MIN_SIZE - 1;
    }

    T & front()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseArray::get_item(f);
    }
    
    const T & front() const
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseArray::get_item(f);
    }

    T & rear()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseArray::get_item(r);
    }
    
    const T & rear() const
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseArray::get_item(r);
    }

    T & put(const T & item)
    {
      r = (r + 1) % BaseArray::get_capacity();
      BaseArray::set_item(r, item);
      ++num_items;
      resize_up();
      return BaseArray::get_item(r);
    }

    T & put(T && item)
    {
      r = (r + 1) % BaseArray::get_capacity();
      BaseArray::set_item(r, std::forward<T>(item));
      ++num_items;
      resize_up();
      return BaseArray::get_item(r);
    }

    T get()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      T ret_val = std::move(BaseArray::get_item(f));
      f = (f + 1) % BaseArray::get_capacity();
      --num_items;
      resize_down();
      return ret_val;
    }
  };
  
  template <typename T>
  void DynQueue<T>::copy_queue(const DynQueue & q)
  {
    nat_t ii = q.f;
    
    for (nat_t i = 0; i < num_items; ++i)
      {
	BaseArray::set_item(i, q.get_item(ii));
	ii = (ii + 1) % BaseArray::get_capacity();
      }
    
    f = 0;
    r = num_items - 1;
  }

  template <typename T>
  void DynQueue<T>::resize(nat_t sz)
  {
    if (sz < MIN_SIZE)
      sz = MIN_SIZE;

    BaseArray new_array(sz);

    nat_t ii = f;
    
    for (nat_t i = 0; i < num_items; ++i)
      {
	new_array.set_item(i, BaseArray::get_item(ii));
	ii = (ii + 1) % BaseArray::get_capacity();
      }

    BaseArray::swap(new_array);

    f = 0;
    r = num_items - 1;
  }

  template <typename T>
  class ListQueue : private SLList<T>
  {
    using BaseList = SLList<T>;
    
  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;

    ListQueue()
      : BaseList()
    {
      // empty
    }

    ListQueue(const ListQueue & q)
      : BaseList(q)
    {
      // empty
    }

    ListQueue(ListQueue && q)
      : BaseList(std::forward<ListQueue<T>>(q))
    {
      // empty
    }

    ListQueue & operator = (const ListQueue & q)
    {
      if (this == &q)
	return *this;
      
      (BaseList &) *this = q;
      return *this;
    }
    
    ListQueue & operator = (ListQueue && q)
    {
      BaseList::swap(q);
      return *this;
    }

    bool is_empty() const
    {
      return BaseList::is_empty();
    }

    nat_t size() const
    {
      return BaseList::size();
    }

    void clear()
    {
      BaseList::clear();
    }

    T & front()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseList::get_first();
    }
    
    const T & front() const
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseList::get_first();
    }

    T & rear()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseList::get_last();
    }
    
    const T & rear() const
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseList::get_last();
    }

    T & put(const T & item)
    {
      return BaseList::append(item);
    }

    T & put(T && item)
    {
      return BaseList::append(std::forward<T>(item));
    }

    T get()
    {
      if (is_empty())
	throw std::underflow_error("Queue is empty");

      return BaseList::remove_first();
    }
  };
  
} // end namespace Designar

# endif // DSGQUEUE_H
