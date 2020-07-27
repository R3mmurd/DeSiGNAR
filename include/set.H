/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGSET_H
# define DSGSET_H

# include <array.H>
# include <tree.H>
# include <hash.H>
# include <sort.H>

namespace Designar
{
  template <typename Key, class Cmp = std::less<Key>>
  struct NotEqualKey
  {
    Cmp & cmp;

    NotEqualKey(Cmp & c)
      : cmp(c)
    {
      // empty
    }

    NotEqualKey(Cmp && c = Cmp())
      : cmp(c)
    {
      // empty
    }

    bool operator () (const Key & a, const Key & b) const
    {
      return cmp(a, b) or cmp(b, a);
    }
  };

  template <typename Key, class Cmp = std::less<Key>>
  struct EqualKey
  {
    Cmp & cmp;

    EqualKey(Cmp & c)
      : cmp(c)
    {
      // empty
    }

    EqualKey(Cmp && c = Cmp())
      : cmp(c)
    {
      // empty
    }

    bool operator () (const Key & a, const Key & b) const
    {
      return not NotEqualKey<Key, Cmp>(cmp)(a, b);
    }
  };

  template <typename Key, class Cmp>
  class SortedArraySetOp
  {
    DynArray<Key> & array;
    Cmp           & cmp;


  protected:
    NotEqualKey<Key, Cmp> not_equal_key;
    EqualKey<Key, Cmp>    equal_key;

    lint_t search(const Key & k, lint_t l, lint_t r) const
    {
      return binary_search(array, k, l, r, cmp);
    }

  public:
    SortedArraySetOp(DynArray<Key> & a, Cmp & c)
      : array(a), cmp(c), not_equal_key(cmp), equal_key(cmp)
    {
      // empty
    }
    
    bool is_sorted() const
    {
      return true;
    }

    Key * insert(const Key & item)
    {
      lint_t pos = search(item, 0, array.size() - 1);
      
      if (pos == array.size())
	return &array.append(item);
      
      if (equal_key(item, array.at(pos)))
	return nullptr;
      
      return &array.insert(pos, item);
    }
    
    Key * insert(Key && item)
    {
      lint_t pos = search(item, 0, array.size() - 1);
      
      if (pos == array.size())
	return &array.append(std::forward<Key>(item));
      
      if (equal_key(item, array.at(pos)))
	return nullptr;
      
      return &array.insert(pos, std::forward<Key>(item));
    }
    
    Key * insert_dup(const Key & item)
    {
      lint_t pos = search(item, 0, array.size() - 1);
      
      if (pos == array.size())
	return &array.append(item);
      
      return &array.insert(pos, item);
    }
    
    Key * insert_dup(Key && item)
    {
      lint_t pos = search(item, 0, array.size() - 1);
      
      if (pos == array.size())
	return &array.append(std::forward<Key>(item));
      
      return &array.insert(pos, std::forward<Key>(item));
    }
    
    Key * search_or_insert(const Key & item)
    {
      lint_t pos = search(item, 0, array.size() - 1);
      
      if (pos == array.size())
	return &array.append(item);
      
      if (equal_key(item, array.at(pos)))
	return &array[pos];
      
      return &array.insert(pos, item);
    }
    
    Key * search_or_insert(Key && item)
    {
      lint_t pos = search(item, 0, array.size() - 1);
      
      if (pos == array.size())
	return &array.append(std::forward<Key>(item));
      
      if (equal_key(item, array.at(pos)))
	return &array[pos];
      
      return &array.insert(pos, std::forward<Key>(item));
    }

    Key remove_pos(nat_t pos)
    {
      return array.remove_pos_closing_breach(pos);
    }

    const Key & select(nat_t i)
    {
      return array.at(i);
    }
    
    nat_t position(const Key & item)
    {
      return search(item, 0, array.size() - 1);
    }
  };

  template <typename Key, class Cmp>
  class UnsortedArraySetOp
  {
    DynArray<Key> & array;
    Cmp           & cmp;

  protected:
    NotEqualKey<Key, Cmp> not_equal_key;
    EqualKey<Key, Cmp>    equal_key;
    
  protected:
    lint_t search(const Key & k, lint_t l, lint_t r) const
    {
      return sequential_search(array, k, l, r, equal_key);
    }
    
  public:
    UnsortedArraySetOp(DynArray<Key> & a, Cmp & c)
      : array(a), cmp(c), not_equal_key(cmp), equal_key(cmp)
    {
      // empty
    }
    
    bool is_sorted() const
    {
      return array.template is_sorted<Cmp>(cmp);
    }

    Key * insert(const Key & item)
    {
      lint_t pos = search(item, 0, lint_t(array.size()) - 1);
      
      if (pos < array.size())
	return nullptr;
      
      return &array.append(item);
    }
    
    Key * insert(Key && item)
    {
      lint_t pos = search(item, 0, lint_t(array.size()) - 1);
      
      if (pos < array.size())
	return nullptr;
      
      return &array.append(std::forward<Key>(item));
    }

    Key * insert_dup(const Key & item)
    {
      return &array.append(item);
    }
    
    Key * insert_dup(Key && item)
    {
      return &array.append(std::forward<Key>(item));
    }

    Key * search_or_insert(const Key & item)
    {
      lint_t pos = search(item, 0, lint_t(array.size()) - 1);
      
      if (pos < array.size())
	return &array[pos];
      
      return &array.append(item);
    }
    
    Key * search_or_insert(Key && item)
    {
      lint_t pos = search(item, 0, lint_t(array.size()) - 1);
      
      if (pos < array.size())
	return &array[pos];
      
      return &array.append(std::forward<Key>(item));
    }

    Key remove_pos(nat_t pos)
    {
      return array.remove_pos(pos);
    }

    const Key & select(nat_t i)
    {
      quicksort(array, 0, array.size() - 1, cmp);
      return array.at(i);
    }
    
    nat_t position(const Key & item)
    {
      quicksort(array, 0, array.size() - 1, cmp);
      return search(item, 0, array.size() - 1);
    }
  };
  
  template <typename Key, class Cmp, class ArraySetOp>
  class GenArraySet :
    public ArraySetOp,
    public ContainerAlgorithms<GenArraySet<Key, Cmp, ArraySetOp>, Key>,
    public SetAlgorithms<GenArraySet<Key, Cmp, ArraySetOp>, Key>
  {   
  public:
    using ItemType  = Key;
    using KeyType   = Key;
    using DataType  = Key;
    using ValueType = Key;
    using SizeType  = nat_t;
    
    DynArray<Key>         array;
    Cmp                 & cmp;
    
  public:
    GenArraySet(nat_t cap, Cmp & _cmp)
      : ArraySetOp(array, _cmp), array(cap), cmp(_cmp)
    {
      // empty
    }
    
    GenArraySet(Cmp && _cmp = Cmp())
      : ArraySetOp(array, _cmp), array(), cmp(_cmp)
    {
      // empty
    }
    
    GenArraySet(nat_t cap, Cmp && _cmp = Cmp())
      : GenArraySet(cap, _cmp)
    {
      // empty
    }
    
    GenArraySet(const GenArraySet & a)
      : ArraySetOp(array, a.cmp), array(a.array), cmp(a.cmp)
    {
      // empty
    }
    
    GenArraySet(GenArraySet && a)
      : GenArraySet()
    {
      swap(a);
    }
    
    GenArraySet(const std::initializer_list<Key> &);

    GenArraySet & operator = (const GenArraySet & a)
    {
      if (&a == this)
	return *this;

      array = a.array;
      cmp = a.cmp;

      return *this;
    }

    GenArraySet & operator = (GenArraySet && a)
    {
      swap(a);
      return *this;
    }
    
    void swap(GenArraySet & a)
    {
      array.swap(a.array);
      std::swap(cmp, a.cmp);
    }
    
    Cmp & get_cmp()
    {
      return cmp;
    }
    
    const Cmp & get_cmp() const
    {
      return cmp;
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
    
    Key * append(const Key & k)
    {
      return ArraySetOp::insert(k);
    }
    
    Key * append(Key && k)
    {
      return ArraySetOp::insert(std::forward<Key>(k));
    }

    Key * append_dup(const Key & k)
    {
      return ArraySetOp::insert_dup(k);
    }
    
    Key * append_dup(Key && k)
    {
      return ArraySetOp::insert_dup(std::forward<Key>(k));
    }
    
    Key * search(const Key & item)
    {
      lint_t pos = ArraySetOp::search(item, 0, lint_t(this->size()) - 1);
      
      if (pos >= this->size() or ArraySetOp::not_equal_key(item, array.at(pos)))
	return nullptr;
      
      return &array.at(pos);
    }
    
    const Key * search(const Key & item) const
    {
      lint_t pos = ArraySetOp::search(item, 0, lint_t(this->size()) - 1);
      
      if (pos >= this->size() or ArraySetOp::not_equal_key(item, array.at(pos)))
	return nullptr;
      
      return &array.at(pos);
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
      const Key * result = search(item);
      if (result == nullptr)
	throw std::domain_error("Item does not exist");
      return *result;
    }

    bool remove(const Key & item)
    {
      lint_t pos = ArraySetOp::search(item, 0, lint_t(this->size()) - 1);
      
      if (pos >= this->size() or ArraySetOp::not_equal_key(array.at(pos), item))
	return false;
      
      ArraySetOp::remove_pos(pos);
      return true;
    }
    
    Key & operator [] (nat_t i)
    {
      return array[i];
    }
    
    const Key & operator [] (nat_t i) const
    {
      return array[i];
    }
    
    class Iterator : public DynArray<Key>::Iterator
    {
      using Base = typename DynArray<Key>::Iterator;
      
    public:
      Iterator()
	: Base()
      {
	// empty
      }
      
      Iterator(const GenArraySet & a)
	: Base(a.array)
      {
	// empty
      }
      
      Iterator(const GenArraySet & a, nat_t c)
	: Base(a.array, c)
      {
	// empty
      }
      
      Iterator(const Iterator & itor)
	: Base(itor)
      {
	// empty
      }
      
      Iterator(Iterator && itor)
	: Iterator()
      {
	Base::swap(itor);
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

  template <typename Key, class Cmp, class ArraySetOp>
  GenArraySet<Key, Cmp, ArraySetOp>::
  GenArraySet(const std::initializer_list<Key> & l)
    : GenArraySet(l.size())
  {
    for (const Key & item : l)
      append(item);
  } 

  template <typename Key, class Cmp = std::less<Key>>
  class UnsortedArraySet : public GenArraySet<Key, Cmp,
					      UnsortedArraySetOp<Key, Cmp>>
  {
    using Base = GenArraySet<Key, Cmp, UnsortedArraySetOp<Key, Cmp>>;
    using Base::Base;
  };

  template <typename Key, class Cmp = std::less<Key>>
  class SortedArraySet :
    public GenArraySet<Key, Cmp, SortedArraySetOp<Key, Cmp>>
  {
    using Base = GenArraySet<Key, Cmp, SortedArraySetOp<Key, Cmp>>;
    using Base::Base;
  };

  template <typename Key, class Cmp = std::less<Key>,
	    template <typename, class> class ArrayType = UnsortedArraySet>
  class ArraySet : public ArrayType<Key, Cmp>
  {
    using Base = ArrayType<Key, Cmp>;
    using Base::Base;

  public:
    using ContainerType = ArrayType<Key, Cmp>;
  };

  template <typename Key, class Cmp = std::less<Key>,
	    template <typename, class> class TreeType = RankedTreap>
  class TreeSet : public TreeType<Key, Cmp>
  {
    using Base = TreeType<Key, Cmp>;
    using Base::Base;

  public:
    using ContainerType = TreeType<Key, Cmp>;
  };

  template <typename Key, class Cmp = std::equal_to<Key>,
	    template <typename, class> class HashTableType = LHashTable>
  class HashSet : public HashTableType<Key, Cmp>
  {
    using Base = HashTableType<Key, Cmp>;
    using Base::Base;
    
  public:
    using ContainerType = HashTableType<Key, Cmp>;
  };
    

} // end namespace DeSIGNAR

# endif // DSGSET_H
