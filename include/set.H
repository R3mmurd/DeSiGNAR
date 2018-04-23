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

# ifndef DSGSET_H
# define DSGSET_H

# include <array.H>
# include <tree.H>
# include <hash.H>

namespace Designar
{

  template <typename Key, class Cmp>
  class SortedArraySetOp
  {
    DynArray<Key> & array;
    Cmp           & cmp;

  protected:
    bool not_equal_key(const Key & k1, const Key & k2) const
    {
      return cmp(k1, k2) or cmp(k2, k1);
    }

    bool equal_key(const Key & k1, const Key & k2) const
    {
      return not not_equal_key(k1, k2);
    }
    
    lint_t search(const Key &, lint_t, lint_t) const;

  public:
    SortedArraySetOp(DynArray<Key> & a, Cmp & c)
      : array(a), cmp(c)
    {
      // empty
    }
    
    bool is_sorted() const
    {
      return true;
    }

    Key * insert(const Key &);
    
    Key * insert(Key &&);
    
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
  Key * SortedArraySetOp<Key, Cmp>::insert(const Key & item)
  {
    lint_t pos = search(item, 0, array.size() - 1);
    
    if (pos == array.size())
      return &array.append(item);
    
    if (equal_key(item, array.at(pos)))
      return nullptr;
    
    return &array.insert(pos, item);
  }

  template <typename Key, class Cmp>
  Key * SortedArraySetOp<Key, Cmp>::insert(Key && item)
  {
    lint_t pos = search(item, 0, array.size() - 1);

    if (pos == array.size())
      return &array.append(std::forward<Key>(item));

    if (equal_key(item, array.at(pos)))
      return nullptr;

    return &array.insert(pos, std::forward<Key>(item));
  }

  template <typename Key, class Cmp>
  lint_t SortedArraySetOp<Key, Cmp>::
  search(const Key & k, lint_t l, lint_t r) const
  {
    if (l > r)
      return l;

    lint_t m = (l + r) / 2;

    if (cmp(k, array.at(m)))
      return search(k, l, m - 1);
    else if (cmp(array.at(m), k))
      return search(k, m + 1, r);

    return m;
  }

  template <typename Key, class Cmp>
  class UnsortedArraySetOp
  {
    DynArray<Key> & array;
    Cmp           & cmp;
    
  protected:
    bool not_equal_key(const Key & k1, const Key & k2) const
    {
      return cmp(k1, k2) or cmp(k2, k1);
    }

    bool equal_key(const Key & k1, const Key & k2) const
    {
      return not not_equal_key(k1, k2);
    }
    
    lint_t search(const Key &, lint_t, lint_t) const;
    
  public:
    UnsortedArraySetOp(DynArray<Key> & a, Cmp & c)
      : array(a), cmp(c)
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

  template <typename Key, class Cmp>
  lint_t UnsortedArraySetOp<Key, Cmp>::
  search(const Key & k, lint_t l, lint_t r) const
  {
    lint_t i = l;
    
    while (i <= r and not_equal_key(k, array.at(i)))
      ++i;
    
    return i;
  }
  
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
    
    DynArray<Key> array;
    Cmp         & cmp;
    
    bool not_equal_key(const Key & e1, const Key & e2) const
    {
      return cmp(e1, e2) or cmp(e2, e1);
    }
    
    bool equal_key(const Key & e1, const Key & e2) const
    {
      return not not_equal_key(e1, e2);
    }
    
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
    
    Key * search(const Key & item)
    {
      lint_t pos = ArraySetOp::search(item, 0, lint_t(this->size()) - 1);
      
      if (pos >= this->size() or not_equal_key(item, array.at(pos)))
	return nullptr;
      
      return &array.at(pos);
    }
    
    const Key * search(const Key & item) const
    {
      lint_t pos = ArraySetOp::search(item, 0, lint_t(this->size()) - 1);
      
      if (pos >= this->size() or not_equal_key(item, array.at(pos)))
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
      
      if (pos >= this->size() or not_equal_key(array.at(pos), item))
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
