/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGHASH_H
# define DSGHASH_H

# include <array.H>
# include <list.H>
# include <containeralgorithms.H>
# include <setalgorithms.H>
# include <iterator.H>

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__)	\
  || defined(_MSC_VER) || defined (__BORLANDC__) || defined (__TURBOC__)
#define get16bits(d) (*((const uint16_t *) (d)))
#endif

#if !defined (get16bits)
#define get16bits(d) ((((uint32_t)(((const uint8_t *)(d))[1])) << 8)	\
		      +(uint32_t)(((const uint8_t *)(d))[0]) )
#endif

namespace Designar
{
  nat_t super_fast_hash(void *, nat_t);

  inline nat_t super_fast_hash(const char * key)
  {
    return super_fast_hash((void *) key, strlen(key));
  }

  inline nat_t super_fast_hash(const std::string & key)
  {
    return super_fast_hash((void *) key.c_str(), key.size());
  }

  template <typename Key>
  inline nat_t super_fast_hash(const Key & key)
  {
    return super_fast_hash((void *) &key, sizeof(key));
  }

  template <typename First, typename Second>
  inline nat_t super_fast_hash(const std::pair<First, Second> & p)
  {
    return super_fast_hash<First>(p.first) ^ super_fast_hash<Second>(p.second);
  }

  template <typename Key,
	    class Cmp = std::equal_to<Key>>
  class LHashTable: private FixedArray<DLList<Key>>,
		    public ContainerAlgorithms<LHashTable<Key, Cmp>, Key>,
		    public SetAlgorithms<LHashTable<Key, Cmp>, Key>
  {
    using List      = DLList<Key>;
    using BaseArray = FixedArray<List>;
    
  public:
    using ItemType    = Key;
    using KeyType     = Key;
    using DataType    = Key;
    using ValueType   = Key;
    using SizeType    = nat_t;
    using CmpType     = Cmp;
    using HashFctPtr  = nat_t (*) (const Key &);
    using HashFctType = std::function<nat_t(const Key &)>;
    
    static constexpr nat_t  DFT_SIZE        = 32;
    static constexpr real_t DFT_LOWER_ALPHA = 0.25;
    static constexpr real_t DFT_UPPER_ALPHA = 0.75;
    
  private:
    nat_t       num_items;
    Cmp       & cmp;
    HashFctType hash_fct;
    real_t      lower_alpha;
    real_t      upper_alpha;
    
    void clear_lists();
    
    void resize(nat_t);
    
    
    Key * search_in_list(List & list, const Key & k)
    {
      return list.search_ptr([&k, this](const auto & item)
			     {
			       return cmp(k, item);
			     });
    }
    
    const Key * search_in_list(const List & list, const Key & k) const
    {
      return list.search_ptr([&k, this](const auto & item)
	      {
		return cmp(k, item);
	      });
    }
    
    nat_t h(const Key & item) const
    {
      return hash_fct(item) % BaseArray::get_capacity();
    }
    
  public:    
    LHashTable(nat_t size, Cmp & _cmp, HashFctType fct,
	       real_t _lower_alpha, real_t _upper_alpha)
      : BaseArray(size), num_items(0), cmp(_cmp), hash_fct(fct),
      lower_alpha(_lower_alpha), upper_alpha(_upper_alpha)
    {
      // empty
    }
    
    LHashTable(nat_t size, Cmp & _cmp, HashFctType fct)
      : LHashTable(size, _cmp, fct, DFT_LOWER_ALPHA, DFT_UPPER_ALPHA)
    {
      // empty
    }
    
    LHashTable(nat_t size, Cmp && _cmp, HashFctType fct)
	: LHashTable(size, _cmp, fct, DFT_LOWER_ALPHA, DFT_UPPER_ALPHA)
    {
      // empty
    }
    
    LHashTable(nat_t size, Cmp & _cmp, HashFctPtr fct = &super_fast_hash)
      : LHashTable(size, _cmp, fct, DFT_LOWER_ALPHA, DFT_UPPER_ALPHA)
    {
      // empty
    }

    LHashTable(nat_t size, Cmp && _cmp = Cmp(),
	       HashFctPtr fct = &super_fast_hash)
      : LHashTable(size, _cmp, fct, DFT_LOWER_ALPHA, DFT_UPPER_ALPHA)
    {
      // empty
    }
      
    LHashTable(Cmp & _cmp, HashFctType fct)
      : LHashTable(DFT_SIZE, _cmp, fct, DFT_LOWER_ALPHA, DFT_UPPER_ALPHA)
    {
      // empty
    }
    
    LHashTable(Cmp && _cmp, HashFctType fct)
      : LHashTable(DFT_SIZE, _cmp, fct, DFT_LOWER_ALPHA, DFT_UPPER_ALPHA)
    {
      // empty
    }

    LHashTable(Cmp & _cmp, HashFctPtr fct = &super_fast_hash)
      : LHashTable(DFT_SIZE, _cmp, fct, DFT_LOWER_ALPHA, DFT_UPPER_ALPHA)
    {
      // empty
    }

    LHashTable(Cmp && _cmp = Cmp(), HashFctPtr fct = &super_fast_hash)
      : LHashTable(_cmp, fct)
    {
      // empty
    }
      
    LHashTable(const LHashTable & h)
      : BaseArray(h), num_items(h.num_items), cmp(h.cmp), hash_fct(h.hash_fct),
      lower_alpha(h.lower_alpha), upper_alpha(h.upper_alpha)
    {
      // empty
    }

    LHashTable(LHashTable && h)
      : LHashTable()
    {
      swap(h);
    }

    LHashTable(const std::initializer_list<Key> &);
    
    LHashTable & operator = (const LHashTable & h)
    {
      if (this == &h)
	return *this;
      
      (BaseArray &) *this = h;
      num_items = h.num_items;
      cmp = h.cmp;
      hash_fct = h.hash_fct;
      lower_alpha = h.lower_alpha;
      upper_alpha = h.upper_alpha;
      
      return *this;
    }
    
    LHashTable & operator = (LHashTable && h)
    {
      swap(h);
      return *this;
    }
    
    void swap(LHashTable & h)
    {
      BaseArray::swap(h);
      std::swap(num_items, h.num_items);
      std::swap(cmp, h.cmp);
      std::swap(hash_fct, h.hash_fct);
      std::swap(lower_alpha, h.lower_alpha);
      std::swap(upper_alpha, h.upper_alpha);
    }
    
    Cmp & get_cmp()
    {
      return cmp;
    }
    
    const Cmp & get_cmp() const
    {
      return cmp;
    }
    
    const HashFctType & get_hash_fct() const
    {
      return hash_fct;
    }
    
    real_t get_lower_alpha() const
    {
      return lower_alpha;
    }

    real_t get_upper_alpha() const
    {
      return upper_alpha;
    }
    
    void set_lower_alpha(real_t value)
    {
      lower_alpha = value;
    }
    
    void set_upper_alpha(real_t value)
    {
      upper_alpha = value;
    }
    
    void reset_alpha_values()
    {
      lower_alpha = DFT_LOWER_ALPHA;
      upper_alpha = DFT_UPPER_ALPHA;
    }
    
    real_t alpha() const
    {
      return real_t(num_items) / real_t(BaseArray::get_capacity());
    }
    
    bool is_empty() const
    {
      return num_items == 0;
    }

    nat_t size() const
    {
      return num_items;
    }

    nat_t N() const
    {
      return size();
    }

    nat_t M() const
    {
      return BaseArray::get_capacity();
    }

    void clear()
    {
      clear_lists();
      num_items = 0;
      BaseArray::clear();
    }

    Key * insert(const Key & item)
    {
      nat_t i = h(item);

      List & list = BaseArray::at(i);

      if (not list.is_empty() and search_in_list(list, item) != nullptr)
	return nullptr;

      if (alpha() < upper_alpha)
	{
	  ++num_items;
	  return &list.append(item);
	}
      
      resize(BaseArray::get_capacity() * 2);
      i = h(item);
      ++num_items;
      return &BaseArray::at(i).insert(item);
    }

    Key * insert(Key && item)
    {
      nat_t i = h(item);
      
      List & list = BaseArray::at(i);

      if (not list.is_empty() and search_in_list(list, item) != nullptr)
	return nullptr;

      if (alpha() < upper_alpha)
	{
	  ++num_items;
	  return &list.insert(std::forward<Key>(item));
	}
      
      resize(BaseArray::get_capacity() * 2);
      i = h(item);
      ++num_items;
      return &BaseArray::at(i).insert(std::forward<Key>(item));
    }

    Key * insert_dup(const Key & item)
    {
      nat_t i = h(item);

      List & list = BaseArray::at(i);

      if (alpha() < upper_alpha)
	{
	  ++num_items;
	  return &list.append(item);
	}
      
      resize(BaseArray::get_capacity() * 2);
      i = h(item);
      ++num_items;
      return &BaseArray::at(i).insert(item);
    }

    Key * insert_dup(Key && item)
    {
      nat_t i = h(item);
      
      List & list = BaseArray::at(i);

      if (alpha() < upper_alpha)
	{
	  ++num_items;
	  return &list.insert(std::forward<Key>(item));
	}
      
      resize(BaseArray::get_capacity() * 2);
      i = h(item);
      ++num_items;
      return &BaseArray::at(i).insert(std::forward<Key>(item));
    }

    Key * append(const Key & k)
    {
      return insert(k);
    }

    Key * append(Key && k)
    {
      return insert(std::forward<Key>(k));
    }

    Key * append_dup(const Key & k)
    {
      return insert_dup(k);
    }

    Key * append_dup(Key && k)
    {
      return insert_dup(std::forward<Key>(k));
    }

    Key * search(const Key & k)
    {
      nat_t i = h(k);

      List & list = BaseArray::at(i);
      
      if (list.is_empty())
	return nullptr;

      return search_in_list(list, k);
    }

    const Key * search(const Key & k) const
    {
      nat_t i = h(k);

      const List & list = BaseArray::at(i);
      
      if (list.is_empty())
	return nullptr;

      return search_in_list(list, k);
    }

    Key * search_or_insert(const Key & item)
    {
      nat_t i = h(item);

      List & list = BaseArray::at(i);

      if (not list.is_empty())
	{
	  Key * result = search_in_list(list, item);

	  if (result != nullptr)
	    return result;
	}

      if (alpha() < upper_alpha)
	{
	  ++num_items;
	  return &list.insert(item);
	}
      
      resize(BaseArray::get_capacity() * 2);
      i = h(item);
      ++num_items;
      return &BaseArray::at(i).insert(item);
    }

    Key * search_or_insert(Key && item)
    {
      nat_t i = h(item);
      
      List & list = BaseArray::at(i);

      if (not list.is_empty())
	{
	  Key * result = search_in_list(list, item);

	  if (result != nullptr)
	    return result;
	}

      if (alpha() < upper_alpha)
	{
	  ++num_items;
	  return &list.insert(std::forward<Key>(item));
	}
      
      resize(BaseArray::get_capacity() * 2);
      i = h(item);
      ++num_items;
      return &BaseArray::at(i).insert(std::forward<Key>(item));
    }

    Key & find(const Key & k)
    {
      Key * ptr = search(k);

      if (ptr == nullptr)
	throw std::domain_error("Key does not exists");

      return *ptr;
    }

    const Key & find(const Key & k) const
    {
      const Key * ptr = search(k);

      if (ptr == nullptr)
	throw std::domain_error("Key does not exists");

      return *ptr;
    }

    bool remove(const Key & k)
    {
      nat_t i = h(k);
      
      List & list = BaseArray::at(i);
      
      if (list.is_empty())
	return false;

      bool result = list.remove_first_if([&k, this](const auto & item)
	      {
		return cmp(k, item);
	      });

      if (not result)
	return false;

      --num_items;

      nat_t min_size = DFT_SIZE;

      if (BaseArray::get_capacity() == min_size or alpha() > lower_alpha)
	return true;
      
      nat_t new_size = std::max(min_size, BaseArray::get_capacity() / 2);
      resize(new_size);

      return true;
    }

    class Iterator : public BidirectionalIterator<Iterator, Key>
    {
      friend class LHashTable;
      friend class BasicIterator<Iterator, Key>;
      
      LHashTable            * set_ptr;
      lint_t                  set_pos;
      typename List::Iterator list_it;
      lint_t                  pos;

      void locate_end();

      void locate_prev(nat_t);

      void locate_next(nat_t);

    protected:
      lint_t get_location() const
      {
	return pos;
      }

      Iterator(const LHashTable & h, int)
	: set_ptr(&const_cast<LHashTable &>(h)), set_pos(h.M()),
	  list_it(), pos(h.N())
      {
	locate_end();
      }
      
    public:
      Iterator()
	: set_ptr(nullptr), set_pos(-1), list_it(), pos(-1)
      {
	// empty
      }

      Iterator(const LHashTable & h)
	: set_ptr(&const_cast<LHashTable &>(h)), set_pos(0), list_it(), pos(0)
      {
	locate_next(set_pos);
      }

      Iterator(const Iterator & it)
	: set_ptr(it.set_ptr), set_pos(it.set_pos), list_it(it.list_it),
	  pos(it.pos)
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

	set_ptr = it.set_ptr;
	set_pos = it.set_pos;
	list_it = it.list_it;
	pos = it.pos;

	return *this;
      }

      Iterator & operator = (Iterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(Iterator & it)
      {
	std::swap(set_ptr, it.set_ptr);
	std::swap(set_pos, it.set_pos);
	std::swap(list_it, it.list_it);
	std::swap(pos, it.pos);
      }

      void reset_first()
      {
	pos = 0;
	locate_next(0);
      }

      void reset_last()
      {
	pos = set_ptr->N();
	locate_end();
	prev();
      }

      bool has_current() const
      {
	return set_pos >= 0 and set_pos < set_ptr->get_capacity();
      }

      Key & get_current()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	return list_it.get_current();
      }

      const Key & get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	return list_it.get_current();
      }

      void next()
      {
	if (not has_current())
	  return;

	++pos;

	list_it.next();

	if (not list_it.has_current())
	  locate_next(set_pos + 1);
      }

      void prev()
      {
	if (pos == 0)
	  return;

	--pos;

	if (set_pos == set_ptr->M() or list_it == set_ptr->at(set_pos).begin())
	  locate_prev(set_pos);

	list_it.prev();
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
      return Iterator(*this, 0);
    }
    
    Iterator end() const
    {
      return Iterator(*this, 0);
    }
  };

  template <typename Key, class Cmp>
  void LHashTable<Key, Cmp>::clear_lists()
  {
    for (nat_t i = 0; i < BaseArray::get_capacity(); ++i)
      BaseArray::at(i).clear();
  }
    
  template <typename Key, class Cmp>
  void LHashTable<Key, Cmp>::resize(nat_t sz)
  {
    LHashTable new_hash_set(sz, cmp, hash_fct, lower_alpha, upper_alpha);

    for (Key & item : *this)
      new_hash_set.append(std::move(item));

    swap(new_hash_set);
  }

  template <typename Key, class Cmp>
  LHashTable<Key, Cmp>::LHashTable(const std::initializer_list<Key> & l)
    : LHashTable()
  {
    for (const Key & item : l)
      append(item);
  }

  template <typename Key, class Cmp>
  void LHashTable<Key, Cmp>::Iterator::locate_end()
  {
    while (set_pos > 0 and set_ptr->at(set_pos - 1).is_empty())
      --set_pos;
    
    if (set_pos > 0)
      list_it = set_ptr->at(set_pos - 1).end();
  }

  template <typename Key, class Cmp>
  void LHashTable<Key, Cmp>::Iterator::locate_next(nat_t i)
  {
    if (set_ptr->is_empty())
      return;
    
    while (i < set_ptr->get_capacity() and
	   set_ptr->at(i).is_empty())
      ++i;

    if (i < set_ptr->get_capacity())
      {
	assert(not set_ptr->at(i).is_empty());
	list_it = set_ptr->at(i).begin();
      }

    set_pos = i;
  }

  template <typename Key, class Cmp>
  void LHashTable<Key, Cmp>::Iterator::locate_prev(nat_t i)
  {
    if (set_ptr->is_empty())
      return;
    
    while (i > 0 and set_ptr->at(i - 1).is_empty())
      --i;
	   
    if (i > 0)
      {
	assert(not set_ptr->at(i - 1).is_empty());
	list_it = set_ptr->at(i - 1).end();
      }

    set_pos = i - 1;
  }
  
} // end namespace Designar

# endif // DSGHASH_H
