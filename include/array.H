/*
  This file is part of Designar Library.
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

# ifndef DSGARRAY_H
# define DSGARRAY_H

# include <setalgorithms.H>
# include <containeralgorithms.H>

namespace Designar
{
  template <typename T>
  class FixedArray
  {
    nat_t cap;
    T   * array_ptr;

    void init(const T &);

    void copy(const FixedArray &);

  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;

    nat_t data_to_pos(T & data)
    {
      return nat_t(&data) - nat_t(array_ptr);
    }

    FixedArray()
      : cap(0), array_ptr(nullptr)
    {
      // empty
    }

    FixedArray(nat_t c)
      : cap(c), array_ptr(new T[cap])
    {
      // empty
    }

    FixedArray(nat_t c, const T & init_value)
      : FixedArray(c)
    {
      init(init_value);
    }

    FixedArray(const FixedArray & a)
      : FixedArray(a.cap)
    {
      copy(a);
    }

    FixedArray(FixedArray && a)
      : FixedArray()
    {
      swap(a);
    }

    FixedArray(const std::initializer_list<T> & l)
      : cap(l.size()), array_ptr(new T[cap])
    {
      nat_t i = 0;
      
      for (const T & item : l)
	array_ptr[i++] = item;
    }

    ~FixedArray()
    {
      delete [] array_ptr;
    }

    FixedArray & operator = (const FixedArray & a)
    {
      if (this == &a)
	return *this;

      delete []array_ptr;
      cap = a.cap;
      array_ptr = new T[cap];
      copy(a);
      return *this;
    }

    FixedArray & operator = (FixedArray && a)
    {
      swap(a);
      return *this;
    }

    void swap(FixedArray & a)
    {
      std::swap(cap, a.cap);
      std::swap(array_ptr, a.array_ptr);
    }

    void resize(nat_t);

    nat_t get_capacity() const
    {
      return cap;
    }

    nat_t size() const
    {
      return get_capacity();
    }

    T & get_item(nat_t i)
    {
      if (i >= cap)
	throw std::out_of_range("Index out of range");

      return array_ptr[i];
    }

    const T & get_item(nat_t i) const
    {
      if (i >= cap)
	throw std::out_of_range("Index out of range");

      return array_ptr[i];
    }

    void set_item(nat_t i, const T & value)
    {
      if (i >= cap)
	throw std::out_of_range("Index out of range");

      array_ptr[i] = value;
    }

    void set_item(nat_t i, T && value)
    {
      if (i >= cap)
	throw std::out_of_range("Index out of range");

      array_ptr[i] = std::move(value);
    }

    T & operator [] (nat_t i)
    {
      return get_item(i);
    }

    const T & operator [] (nat_t i) const
    {
      return get_item(i);
    }
  };

  template <typename T>
  void FixedArray<T>::init(const T & init_value)
  {
    for (nat_t i = 0; i < cap; ++i)
      array_ptr[i] = init_value;
  }
  
  template <typename T>
  void FixedArray<T>::copy(const FixedArray & a)
  {
    if (std::is_pod<T>::value)
      memcpy(array_ptr, a.array_ptr, sizeof(T) * cap);
    else
      for (nat_t i = 0; i < cap; ++i)
	array_ptr[i] = a.array_ptr[i];
  }

  template <typename T>
  void FixedArray<T>::resize(nat_t c)
  {
    if (c == cap)
      return;
    
    T * new_array_ptr = new T[c];

    nat_t sz = std::min(c, cap);

    for (nat_t i = 0; i < sz; ++i)
      new_array_ptr[i] = array_ptr[i];

    delete [] array_ptr;
    cap = c;
    array_ptr = new_array_ptr;
  }

  template <typename T>
  class DynArray : private FixedArray<T>,
		public ContainerAlgorithms<DynArray<T>, T>
  {
    using BaseArray = FixedArray<T>;
    
  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;
    
  private:
    static constexpr nat_t  MIN_SIZE      = 32;
    static constexpr real_t RESIZE_FACTOR = 0.4;
    
    nat_t num_items;
    
    void copy_array(const DynArray &);
    
    void resize_up()
    {
      if (num_items < BaseArray::get_capacity())
	return;
      
      assert(BaseArray::get_capacity() * (1 + RESIZE_FACTOR) > num_items);
      
      BaseArray::resize(BaseArray::get_capacity() * (1 + RESIZE_FACTOR));
    }
    
    void resize_down()
    {
      if (num_items > BaseArray::get_capacity() * RESIZE_FACTOR or
	  BaseArray::get_capacity() == MIN_SIZE)
	return;
      
      assert(BaseArray::get_capacity() * (1 - RESIZE_FACTOR) > num_items);
      
      nat_t new_cap = std::max<real_t>(BaseArray::get_capacity() *
				       (1 - RESIZE_FACTOR), MIN_SIZE);
      
      BaseArray::resize(new_cap);
    }
    
  public:
    DynArray(nat_t cap)
      : BaseArray(cap), num_items(0)
    {
      // empty
    }
    
    DynArray()
      : DynArray(MIN_SIZE)
    {
      // empty
    }
    
    DynArray(const DynArray & a)
      : BaseArray(a.get_capacity()), num_items(a.num_items)
    {
      copy_array(a);
    }
    
    DynArray(DynArray && a)
      : BaseArray(), num_items(0)
    {
      swap(a);
    }
    
    DynArray(const std::initializer_list<T> &);
    
    void swap(DynArray & a)
    {
      BaseArray::swap(a);
      std::swap(num_items, a.num_items);
    }
    
    nat_t get_capacity() const
    {
      return BaseArray::get_capacity();
    }
    
    nat_t size() const
    {
      return num_items;
    }
    
    bool is_empty() const
    {
      return num_items == 0;
    }
    
    void clear()
    {
      num_items = 0;
      
      if (BaseArray::get_capacity() != MIN_SIZE)
	{
	  DynArray new_array(MIN_SIZE);
	  BaseArray::swap(new_array);
	}
    }
    
    T & get_first()
    {
      if (num_items == 0)
	throw std::underflow_error("Array is empty");
      
      return BaseArray::get_item(0);
    }
    
    const T & get_first() const
    {
      if (num_items == 0)
	throw std::underflow_error("Array is empty");
      
      return BaseArray::get_item(0);
    }
    
    T & get_last()
    {
      if (num_items == 0)
	throw std::underflow_error("Array is empty");
      
      return BaseArray::get_item(num_items - 1);
    }
    
    const T & get_last() const
    {
      if (num_items == 0)
	throw std::underflow_error("Array is empty");
      
      return BaseArray::get_item(num_items - 1);
    }
    
    T & insert(nat_t, const T &);
    
    T & insert(nat_t, T &&);
    
    T & insert(const T & item)
    {
      return insert(0, item);
    }
    
    T & insert(T && item)
    {
      return insert(0, std::forward<T>(item));
    }
    
    
    T & append(const T & item)
    {
      BaseArray::set_item(num_items++, item);
      resize_up();
      return BaseArray::get_item(num_items - 1);
    }
    
    T & append(T && item)
    {
      BaseArray::set_item(num_items++, std::forward<T>(item));
      resize_up();
      return BaseArray::get_item(num_items - 1);
    }
    
    T remove_pos(nat_t);
    
    T remove(T & item)
    {
      nat_t i = BaseArray::data_to_pos(item);
      
      if (i >= num_items)
	throw std::logic_error("Item does not belong to array");
      
      return remove_pos(i);
    }
    
    T remove_first()
    {
      if (num_items == 0)
	throw std::underflow_error("Array is empty");
      
      return remove_pos(0);
    }
    
    T remove_last()
    {
      T ret_val = std::move(BaseArray::get_item(--num_items));
      resize_down();
      return ret_val;
    }
    
    DynArray & operator = (const DynArray & a)
    {
      if (this == &a)
	return *this;
      
      (DynArray &) *this = a;
      num_items = a.num_items;
      return *this;
    }
    
    DynArray & operator = (DynArray && a)
    {
      swap(a);
      return *this;
    }
    
    T & select(nat_t i)
    {
      if (i >= num_items)
	throw std::out_of_range("Index is out of range");
      
      return BaseArray::get_item(i);
    }
    
    const T & select(nat_t i) const
    {
      if (i >= num_items)
	throw std::out_of_range("Index is out of range");
      
      return BaseArray::get_item(i);
    }
    
    T & operator [] (nat_t i)
    {
      return select(i);
    }
    
    const T & operator [] (nat_t i) const
    {
      return select(i);
    }
    
    class Iterator
    {
      DynArray * array_ptr;
      nat_t curr;
      
    public:
      Iterator()
	: array_ptr(nullptr), curr(0)
      {
	// empty
      }
      
      Iterator(const DynArray & a)
	: array_ptr(&const_cast<DynArray &>(a)), curr(0)
      {
	// empty
      }
      
      Iterator(const DynArray & a, nat_t c)
	: array_ptr(&const_cast<DynArray &>(a)), curr(c)
      {
	// empty
      }
      
      Iterator(const Iterator & it)
	: array_ptr(it.array_ptr), curr(it.curr)
      {
	// empty
      }
      
      Iterator(Iterator && it)
	: Iterator()
      {
	swap(it);
      }
      
      Iterator & operator = (const Iterator & it)
      {
	if (this == &it)
	  return *this;
	
	array_ptr = it.array_ptr;
	curr = it.curr;
	return *this;
      }
      
      Iterator & operator = (Iterator && it)
      {
	swap(it);
	return *this;
      }
      
      void swap(Iterator & it)
      {
	std::swap(array_ptr, it.array_ptr);
	std::swap(curr, it.curr);
      }
      
      bool has_current() const
      {
	return curr < array_ptr->size();
      }
      
      T & get_current()
      {
	return (*array_ptr)[curr];
      }
      
      const T & get_current() const
      {
	return (*array_ptr)[curr];
      }
      
      void next()
      {
	if (not has_current())
	  throw std::overflow_error("There is not next element");
	
	++curr;
      }
      
      void prev()
      {
	if (not has_current())
	  throw std::underflow_error("There is not prev element");
	
	--curr;
      }
      
      void reset_first()
      {
	curr = 0;
      }
      
      void reset_last()
      {
	curr = array_ptr->size() - 1;
      }
      
      nat_t get_position() const
      {
	return curr;
      }
      
      T del()
      {
	if (not has_current())
	  throw std::logic_error("There is not current element");
	
	return array_ptr->remove_pos(curr);
      }
      
      void operator ++ ()
      {
	next();
      }
      
      void operator -- ()
      {
	prev();
      }
      
      T & operator * ()
      {
	return get_current();
      }
      
      const T & operator * () const
      {
	return get_current();
      }
      
      bool operator == (const Iterator & it) const
      {
	return array_ptr == it.array_ptr and curr == it.curr;
      }
      
      bool operator != (const Iterator & it) const
      {
	return not (*this == it);
      }
    };
    
    Iterator begin()
    {
      return Iterator(*this);
    }
    
    Iterator begin() const
    {
      return Iterator(*this);
    }
    
    Iterator end()
    {
      return Iterator(*this, num_items);
    }
    
    Iterator end() const
    {
      return Iterator(*this, num_items);
    }
  };
  
  template <typename T>
  DynArray<T>::DynArray(const std::initializer_list<T> & l)
    : DynArray(l.size())
  {
    for (const T & item : l)
      append(item);
  }
  
  template <typename T>
  void DynArray<T>::copy_array(const DynArray & a)
  {
    for (nat_t i = 0; i < num_items; ++i)
      BaseArray::set_item(i, a.get_item(i));
  }

  template <typename T> 
  T & DynArray<T>::insert(nat_t pos, const T & item)
  {
    BaseArray::set_item(num_items, item);
    
    for (nat_t i = num_items; i > pos; --i)
      std::swap(BaseArray::get_item(i), BaseArray::get_item(i - 1));
    
    ++num_items;
    
    resize_up();

    return BaseArray::get_item(pos);
  }
  
  template <typename T>
  T & DynArray<T>::insert(nat_t pos, T && item)
  {
    BaseArray::set_item(num_items, std::forward<T>(item));
    
    for (nat_t i = num_items; i > pos; --i)
      std::swap(BaseArray::get_item(i), BaseArray::get_item(i - 1));
    
    ++num_items;
    
    resize_up();

    return BaseArray::get_item(pos);
  }

  template <typename T>
  T DynArray<T>::remove_pos(nat_t pos)
  {
    if (pos >= num_items)
      throw std::out_of_range("Index is out of range");
    
    T ret_val = std::move(BaseArray::get_item(pos));
    
    --num_items;
    
    for (nat_t i = pos; i < num_items; ++i)
      std::swap(BaseArray::get_item(i), BaseArray::get_item(i + 1));
    
    resize_down();
    
    return ret_val;;
  }
  
  template <typename Key, class Cmp = std::less<Key>>
    class ArraySet : public ContainerAlgorithms<ArraySet<Key, Cmp>, Key>,
		     public SetAlgorithms<ArraySet, Key, Cmp>
    {
    public:
      using ItemType  = Key;
      using KeyType   = Key;
      using DataType  = Key;
      using ValueType = Key;
      using SizeType  = nat_t;

      enum class SortingFlag { UNSORTED, SORTED };

      DynArray<Key> array;
      Cmp         & cmp;
      SortingFlag   flag;

      bool not_equal_key(const Key & e1, const Key & e2) const
      {
	return cmp(e1, e2) or cmp(e2, e1);
      }

      bool equal_key(const Key & e1, const Key & e2) const
      {
	return not not_equal_key(e1, e2);
      }

      Key * insert_sorted(const Key &);

      Key * insert_sorted(Key &&);

      lint_t binary_search(const Key &, lint_t, lint_t) const;

      lint_t sequential_search(const Key &, lint_t, lint_t) const;

    public:
      ArraySet(nat_t cap, SortingFlag f, Cmp & _cmp)
	: array(cap), cmp(_cmp), flag(f)
      {
	// empty
      }

      ArraySet(SortingFlag f, Cmp && _cmp = Cmp())
	: array(), cmp(_cmp), flag(f)
      {
	// empty
      }

      ArraySet(nat_t cap, Cmp && _cmp = Cmp())
	: ArraySet(cap, SortingFlag::UNSORTED, _cmp)
      {
	// empty
      }

      ArraySet(Cmp && _cmp = Cmp())
	: array(), cmp(_cmp), flag(SortingFlag::UNSORTED)
      {
	// empty
      }

      ArraySet(const ArraySet & a)
	: array(a.array), cmp(a.cmp), flag(a.flag)
      {
	// empty
      }

      ArraySet(ArraySet && a)
	: ArraySet()
      {
	swap(a);
      }

      ArraySet(const std::initializer_list<Key> &);

      void swap(ArraySet & a)
      {
	array.swap(a.array);
	std::swap(cmp, a.cmp);
	std::swap(flag, a.flag);
      }

      Cmp & get_cmp()
      {
	return cmp;
      }

      const Cmp & get_cmp() const
      {
	return cmp;
      }

      bool is_sorted() const
      {
	if (flag == SortingFlag::SORTED)
	  return true;
	
	return array.template is_sorted<Cmp>(cmp);
      }
      
      bool is_empty() const
      {
	return array.is_empty();
      }

      nat_t size() const
      {
	return array.size();
      }

      void clear()
      {
	array.clear();
      }

      SortingFlag get_sorting_flag() const
      {
	return flag;
      }

      Key * insert(const Key & item)
      {
	if (flag == SortingFlag::UNSORTED)
	  {
	    lint_t pos = sequential_search(item, 0, lint_t(this->size()) - 1);
	    
	    if (pos < this->size())
	      return nullptr;
	  
	    return &array.append(item);
	  }
	else
	  return insert_sorted(item);
      }

      Key * insert(Key && item)
      {
	if (flag == SortingFlag::UNSORTED)
	  {
	    lint_t pos = sequential_search(item, 0, lint_t(this->size()) - 1);
	    
	    if (pos < this->size())
	      return nullptr;
	  
	    return &array.append(std::forward<Key>(item));
	  }
	else
	  return insert_sorted(std::forward<Key>(item));
      }

      Key * append(const Key & k)
      {
	return insert(k);
      }
      
      Key * append(Key && k)
      {
	return insert(std::forward<Key>(k));
      }

      Key * search_or_insert(const Key & item)
      {
	if (flag == SortingFlag::UNSORTED)
	  {
	    lint_t pos = sequential_search(item, 0, lint_t(this->size()) - 1);
	    
	    if (pos < this->size())
	      return &array[pos];
	  
	    return &array.append(item);
	  }
	else
	  {
	    lint_t pos = binary_search(item, 0, this->size() - 1);
	    
	    if (pos == this->size())
	      return &array.append(item);
	    
	    if (equal_key(item, array.select(pos)))
	      return &array[pos];
	    
	    return &array.insert(pos, item);
	  }
      }

      Key * search_or_insert(Key && item)
      {
	if (flag == SortingFlag::UNSORTED)
	  {
	    lint_t pos = sequential_search(item, 0, lint_t(this->size()) - 1);
	    
	    if (pos < this->size())
	      return &array[pos];
	  
	    return &array.append(std::forward<Key>(item));
	  }
	else
	  {
	    lint_t pos = binary_search(item, 0, this->size() - 1);
	    
	    if (pos == this->size())
	      return &array.append(std::forward<Key>(item));
	    
	    if (equal_key(item, array.select(pos)))
	      return &array[pos];
	    
	    return &array.insert(pos, std::forward<Key>(item));
	  }
      }      

      Key * search(const Key & item)
      {
	lint_t pos;

	if (flag == SortingFlag::UNSORTED)
	  pos = sequential_search(item, 0, lint_t(this->size()) - 1);
	else
	  pos = binary_search(item, 0, lint_t(this->size()) - 1);

	if (pos >= this->size() or not_equal_key(item, array.select(pos)))
	  return nullptr;

	return &array.select(pos);
      }

      const Key * search(const Key & item) const
      {
	lint_t pos;

	if (flag == SortingFlag::UNSORTED)
	  pos = sequential_search(item, 0, lint_t(this->size()) - 1);
	else
	  pos = binary_search(item, 0, lint_t(this->size()) - 1);

	if (pos >= this->size() or not_equal_key(item, array.select(pos)))
	  return nullptr;

	return &array.select(pos);
      }

      Key & find(const Key & item)
      {
	Key * result = search(item);
	if (result == nullptr)
	  throw std::domain_error("Item does not exist");
	return *result;
      }

      const Key & find(const Key & item) const
      {
	Key * result = search(item);
	if (result == nullptr)
	  throw std::domain_error("Item does not exist");
	return *result;
      }

      Key remove_pos(nat_t pos)
      {
	return array.remove_pos(pos);
      }

      bool remove(const Key & item)
      {
	lint_t pos;
	
	if (flag == SortingFlag::UNSORTED)
	  pos = sequential_search(item, 0, lint_t(this->size()) - 1);
	else
	  pos = binary_search(item, 0, lint_t(this->size()) - 1);

	if (pos >= this->size() or not_equal_key(array.select(pos), item))
	  return false;

	array.remove_pos(pos);
	return true;
      }

      Key & select(nat_t i)
      {
	return array.select(i);
      }

      const Key & select(nat_t i) const
      {
	return array.select(i);
      }

      Key & operator [] (nat_t i)
      {
	return array[i];
      }

      const Key & operator [] (nat_t i) const
      {
	return array[i];
      }

      class Iterator
      {
        typename DynArray<Key>::Iterator it;

      public:
	Iterator()
	  : it()
	{
	  // empty
	}
	
	Iterator(const ArraySet & a)
	  : it(a.array)
	{
	  // empty
	}

	Iterator(const ArraySet & a, nat_t c)
	  : it(a.array, c)
	{
	  // empty
	}

	Iterator(const Iterator & itor)
	  : it(itor.it)
	{
	  // empty
	}

	Iterator(Iterator && itor)
	  : Iterator()
	{
	  swap(itor);
	}

	Iterator & operator = (const Iterator & itor)
	{
	  if (this == &it)
	    return *this;

	  it = itor.it;
	  return *this;
	}

	Iterator & operator = (Iterator && itor)
	{
	  swap(itor);
	  return *this;
	}

	void swap(Iterator & itor)
	{
	  std::swap(it, itor.it);
	}
      
	bool has_current() const
	{
	  return it.has_current();
	}
	
	Key & get_current()
	{
	  return it.get_current();
	}

	const Key & get_current() const
	{
	  return it.get_current();
	}

	void next()
	{
	  it.next();
	}

	void prev()
	{
	  it.prev();
	}

	void reset_first()
	{
	  it.reset_first();
	}

	void reset_last()
	{
	  it.reset_last();
	}

	nat_t get_position() const
	{
	  return it.get_position();
	}

	Key del()
	{
	  return it.del();
	}

	void operator ++ ()
	{
	  next();
	}

	void operator -- ()
	{
	  prev();
	}

	Key & operator * ()
	{
	  return get_current();
	}

	const Key & operator * () const
	{
	  return get_current();
	}
	
	bool operator == (const Iterator & itor) const
	{
	  return it == itor.it;
	}

	bool operator != (const Iterator & itor) const
	{
	  return not (*this == itor);
	}
      };

      Iterator begin()
      {
	return Iterator(*this);
      }
      
      Iterator begin() const
      {
	return Iterator(*this);
      }
      
      Iterator end()
      {
	return Iterator(*this, size());
      }
      
      Iterator end() const
      {
	return Iterator(*this, size());
      }
    };

  template <typename Key, class Cmp>
  Key * ArraySet<Key, Cmp>::insert_sorted(const Key & item)
  {
    lint_t pos = binary_search(item, 0, this->size() - 1);

    if (pos == this->size())
      return &array.append(item);

    if (equal_key(item, array.select(pos)))
      return nullptr;

    return &array.insert(pos, item);
  }

  template <typename Key, class Cmp>
  Key * ArraySet<Key, Cmp>::insert_sorted(Key && item)
  {
    lint_t pos = binary_search(item, 0, this->size() - 1);

    if (pos == this->size())
      return &array.append(std::forward<Key>(item));

    if (equal_key(item, array.select(pos)))
      return nullptr;

    return &array.insert(pos, std::forward<Key>(item));
  }

  template <typename Key, class Cmp>
  lint_t ArraySet<Key, Cmp>::
  binary_search(const Key & k, lint_t l, lint_t r) const
  {
    if (l > r)
      return l;

    lint_t m = (l + r) / 2;

    if (cmp(k, array.select(m)))
      return binary_search(k, l, m - 1);
    else if (cmp(array.select(m), k))
      return binary_search(k, m + 1, r);

    return m;
  }

  template <typename Key, class Cmp>
  lint_t ArraySet<Key, Cmp>::
  sequential_search(const Key & k, lint_t l, lint_t r) const
  {
    lint_t i = l;
    while (i <= r and not_equal_key(k, array.select(i)))
      ++i;
    return i;
  }

  template <typename Key, class Cmp>
  ArraySet<Key, Cmp>::ArraySet(const std::initializer_list<Key> & l)
    : ArraySet(l.size())
  {
    for (const Key & item : l)
      append(item);
  }

  template <typename T, nat_t N = 2>
  class MultiDimArray
  {
    static_assert(N >= 2, "N must be greater than 1");
    
    class Slice
    {
      nat_t sz;              // size of linear array
      FixedArray<nat_t> exts;  // extents
      FixedArray<nat_t> strs;  // strides
      
      void init();
      
      void swap(Slice & another)
      {
	std::swap(sz, another.sz);
	std::swap(exts, another.exts);
	std::swap(strs, another.strs);
      }

    public:
      Slice()
	: sz(1), exts(N), strs(N)
      {
	init();
      };

      Slice(const Slice & slice)
	: sz(slice.sz), exts(slice.exts), strs(slice.strs)
      {
	// empty
      }

      Slice(Slice && slice)
	: Slice()
      {
	swap(slice);
      }

      Slice(const std::initializer_list<nat_t> & l)
	: sz(1), exts(l), strs(l.size())
      {
	init();
      }

      template <typename... Dims>
      Slice(Dims... dims)
	: sz(1), exts({nat_t(dims)...}), strs(sizeof...(Dims)) 
      {
	static_assert(sizeof...(Dims) == N, "");
	static_assert(AllAreConvertible<nat_t, Dims...>::value, "");
	init();
      }

      const FixedArray<nat_t> & extents() const
      {
	return exts;
      }

      const FixedArray<nat_t> & strides() const
      {
	return strs;
      }

      nat_t size() const
      {
	return sz;
      }

      template <typename... Dims>
      nat_t operator () (Dims...);
      
      Slice & operator = (const Slice & slice)
      {
	if (&slice == this)
	  return *this;
	
	sz = slice.sz;
	exts = slice.exts;
	strs = slice.strs;
	
	return *this;
      }
      
      Slice & operator = (Slice && slice)
      {
	swap(slice);
	return *this;
      }
    };

  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;

  private:
    Slice       slice;
    FixedArray<T> array;

    void swap(MultiDimArray & another)
    {
      std::swap(slice, another.slice);
      std::swap(array, another.array);
    }

  public:
    MultiDimArray() = default;
    
    template <typename... Dims>
    MultiDimArray(Dims... dims)
      : slice(dims...), array(slice.size())
    {
      // empty
    }

    MultiDimArray(const MultiDimArray & mda)
      : slice(mda.slice), array(mda.array)
    {
      // empty
    }

    MultiDimArray(MultiDimArray && mda)
      : MultiDimArray()
    {
      swap(mda);
    }

    nat_t size(nat_t i) const
    {
      slice.extents()[i];
    }

    template <typename... Dims>
    T & get(Dims... dims)
    {
      nat_t idx = slice(dims...);
      if (idx >= slice.size())
	throw std::overflow_error("Invalid position");
      return array[idx];
    }

    template <typename... Dims>
    const T & get(Dims... dims) const
    {
      nat_t idx = slice(dims...);
      if (idx >= slice.size())
	throw std::overflow_error("Invalid position");
      return array[idx];
    }

    template <typename... Dims>
    T & at(Dims... dims)
    {
      return get(dims...);
    }

    template <typename... Dims>
    const T & at(Dims... dims) const
    {
      return get(dims...);
    }

    template <typename... Dims>
    T & operator () (Dims... dims)
    {
      return get(dims...);
    }

    template <typename... Dims>
    const T & operator () (Dims... dims) const
    {
      return get(dims...);
    }
    
    MultiDimArray & operator = (const MultiDimArray & mda)
    {
      if (&mda == this)
	return *this;
      
      slice = mda.slice;
      array = mda.array;
      
      return *this;
    }
    
    MultiDimArray & operator = (MultiDimArray && mda)
    {
      swap(mda);
      return *this;
    }
  };

  template <typename T, nat_t N>
  void MultiDimArray<T, N>::Slice::init()
  {
    for (nat_t i = 0; i < N; ++i)
      sz *= exts[i];
    
    strs[N - 1] = 1;
    
    for (nat_t i = N - 1; i > 0; --i)
      strs[i - 1] = strs[i] * exts[i];
  }

  template <typename T, nat_t N>
  template <typename... Dims>
  nat_t MultiDimArray<T, N>::Slice::operator () (Dims... dims)
  {
    static_assert(sizeof...(Dims) == N, "");
    static_assert(AllAreConvertible<nat_t, Dims...>::value, "");
    FixedArray<nat_t> arr{nat_t(dims)...};

    nat_t ret = 0;
    
    for (size_t i = 0; i < N; ++i)
      ret += arr[i] * strs[i];

    return ret;
  }
  

  template <class ArrayType, class Cmp>
  inline void insertion_sort(ArrayType & a, lint_t l, lint_t r, Cmp & cmp)
  {
    for (lint_t i = l + 1; i <= r; ++i)
      {
	typename ArrayType::DataType data = std::move(a[i]);

	lint_t j = i;

	for ( ; j > l and cmp(data, a[j - 1]); --j)
	  a[j] = std::move(a[j - 1]);

	a[j] = std::move(data);
      }
  }

  template <class ArrayType,
	    class Cmp = std::less<typename ArrayType::DataType>> inline
  void insertion_sort(ArrayType & a, lint_t l, lint_t r, Cmp && cmp = Cmp())
  {
    insertion_sort<ArrayType, Cmp>(a, l, r, cmp);
  }

  template <class ArrayType, class Cmp>
  inline void insertion_sort(ArrayType & a, lint_t size, Cmp & cmp)
  {
    insertion_sort(a, 0, size - 1, cmp);
  }

  template <class ArrayType,
	    class Cmp = std::less<typename ArrayType::DataType>>
  inline void insertion_sort(ArrayType & a, lint_t size, Cmp && cmp = Cmp())
  {
    insertion_sort<ArrayType, Cmp>(a, size, cmp);
  }

  template <class ArrayType, class Cmp>
  inline void insertion_sort(ArrayType & a, Cmp & cmp)
  {
    insertion_sort(a, a.size(), cmp);
  }

  template <class ArrayType,
	    class Cmp = std::less<typename ArrayType::DataType>>
  inline void insertion_sort(ArrayType & a, Cmp && cmp = Cmp())
  {
    insertion_sort<ArrayType, Cmp>(a, cmp);
  }
  
  template <typename ArrayType, class Cmp>
  inline lint_t select_pivot(ArrayType & a, lint_t l, lint_t r, Cmp & cmp)
  {
    assert(l <= r);
    
    lint_t m = (l + r) / 2;

    lint_t partial_min = cmp(a[l], a[m]) ? l : m;

    return cmp(a[partial_min], a[r]) ? partial_min : r;    
  }

  template <typename ArrayType, class Cmp>
  inline lint_t partition(ArrayType & a, lint_t l, lint_t r, Cmp & cmp)
  {
    int pivot = select_pivot(a, l, r, cmp);
    
    std::swap(a[pivot], a[r]);
    
    lint_t i = l - 1;
    lint_t j = r;

    while (i <= j)
      {
	while (cmp(a[++i], a[r]));

	while (cmp(a[r], a[--j]))
	  if (j == l)
	    break;

	if (i >= j)
	  break;

	std::swap(a[i], a[j]);
      }

    std::swap(a[i], a[r]);

    return i;
  }

  template <typename ArrayType, class Cmp>
  inline void quicksort(ArrayType & a, lint_t l, lint_t r, Cmp & cmp)
  {
    if (l >= r)
      return;
    
    if (r - l + 1 <= QuicksortThreshold)
      {
	insertion_sort(a, l, r, cmp);
	return;
      }

    lint_t pivot = partition(a, l, r, cmp);

    if (pivot - l < r - pivot)
      {
	quicksort(a, l, pivot - 1, cmp);
	quicksort(a, pivot + 1, r, cmp);
      }
    else
      {
	quicksort(a, pivot + 1, r, cmp);
	quicksort(a, l, pivot - 1, cmp);
      }
  }

  template <class ArrayType,
	    class Cmp = std::less<typename ArrayType::DataType>> inline
  void quicksort(ArrayType & a, lint_t l, lint_t r, Cmp && cmp = Cmp())
  {
    quicksort<ArrayType, Cmp>(a, l, r, cmp);
  }

  template <class ArrayType, class Cmp>
  inline void quicksort(ArrayType & a, lint_t size, Cmp & cmp)
  {
    quicksort(a, 0, size - 1, cmp);
  }

  template <class ArrayType,
	    class Cmp = std::less<typename ArrayType::DataType>>
  inline void quicksort(ArrayType & a, lint_t size, Cmp && cmp = Cmp())
  {
    quicksort<ArrayType, Cmp>(a, size, cmp);
  }

  template <class ArrayType, class Cmp>
  inline void quicksort(ArrayType & a, Cmp & cmp)
  {
    quicksort(a, a.size(), cmp);
  }

  template <class ArrayType,
	    class Cmp = std::less<typename ArrayType::DataType>>
  inline void quicksort(ArrayType & a, Cmp && cmp = Cmp())
  {
    quicksort<ArrayType, Cmp>(a, cmp);
  }
    
} // end namespace Designar

# endif // DSGARRAY_H
