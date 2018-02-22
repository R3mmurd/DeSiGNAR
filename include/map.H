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

# ifndef DSGMAP_H
# define DSGMAP_H

# include <array.H>
# include <tree.H>
# include <hash.H>

namespace Designar
{
  template <typename Key, typename Value>
  using MapKey = std::pair<Key, Value>;

  template <typename Key, typename Value>
  using MapItem = MapKey<Key, Value>;

  template <typename... Args>
  auto map_key(Args &&... args) ->
    decltype(std::make_pair(std::forward<Args>(args)...))
  {
    return std::make_pair(std::forward<Args>(args)...);
  }

  template <typename... Args>
  auto map_item(Args &&... args) ->
    decltype(std::make_pair(std::forward<Args>(args)...))
  {
    return std::make_pair(std::forward<Args>(args)...);
  }

  template <typename Key, typename Value>
  Key & key(MapKey<Key, Value> & item)
  {
    return item.first;
  }

  template <typename Key, typename Value>
  const Key & key(const MapKey<Key, Value> & item)
  {
    return item.first;
  }

  template <typename Key, typename Value>
  Key key(MapKey<Key, Value> && item)
  {
    return item.first;
  }

  template <typename Key, typename Value>
  Key & key(MapKey<Key, Value> * item_ptr)
  {
    return key(*item_ptr);
  }

  template <typename Key, typename Value>
  Value & value(MapKey<Key, Value> & item)
  {
    return item.second;
  }

  template <typename Key, typename Value>
  const Value & value(const MapKey<Key, Value> & item)
  {
    return item.second;
  }

  template <typename Key, typename Value>
  Value value(MapKey<Key, Value> && item)
  {
    return item.second;
  }

  template <typename Key, typename Value>
  Value & value(MapKey<Key, Value> * item_ptr)
  {
    return value(*item_ptr);
  }
  
  template <typename Key, typename Value, class Cmp>
  class CmpWrapper
  {
    Cmp & cmp;
    
  public:
    CmpWrapper(Cmp & _cmp)
      : cmp(_cmp)
    {
      // empty
    }

    CmpWrapper(Cmp && _cmp = Cmp())
      : CmpWrapper(_cmp)
    {
      // empty
    }

    CmpWrapper(const CmpWrapper & cw)
      : cmp(cw.cmp)
    {
      // empty
    }

    CmpWrapper(CmpWrapper && cw)
      : CmpWrapper()
    {
      std::swap(cmp, cw.cmp);
    }

    Cmp & get_cmp()
    {
      return cmp;
    }

    const Cmp & get_cmp() const
    {
      return cmp;
    }

    CmpWrapper & operator = (const CmpWrapper & cw)
    {
      if (this == &cw)
	return *this;

      cmp = cw.cmp;
      return *this;
    }

    CmpWrapper & operator = (CmpWrapper && cw)
    {
      std::swap(cmp, cw.cmp);
      return *this;
    }
    
    bool operator () (const MapKey<Key, Value> & p,
		      const MapKey<Key, Value> & q) const
    {
      return cmp(p.first, q.first);
    }
  };

  template <template <typename, class> class Set,
	    typename Key, typename Value, class Cmp>
  class GenMap : public Set<MapKey<Key, Value>, CmpWrapper<Key, Value, Cmp>>
  {
    using BaseSet        = Set<MapKey<Key, Value>, CmpWrapper<Key, Value, Cmp>>;
    using MapKey         = MapKey<Key, Value>;

    using BaseSet::BaseSet;

  public:
    using KeyType   = Key;
    using ValueType = Value;
    using SizeType  = nat_t;
    using CmpType   = Cmp;

    Cmp & get_cmp()
    {
      return BaseSet::get_cmp().get_cmp();
    }
    
    const Cmp & get_cmp() const
    {
      return BaseSet::get_cmp().get_cmp();
    }
    
    Value * insert(const Key & k, const Value & v)
    {
      MapKey p = map_key(k, v);
      
      MapKey * result = BaseSet::insert(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * insert(Key && k, const Value & v)
    {
      MapKey p = map_key(std::forward<Key>(k), v);
      
      MapKey * result = BaseSet::insert(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * insert(const Key & k, Value && v)
    {
      MapKey p = map_key(k, std::forward<Value>(v));
      
      MapKey * result = BaseSet::insert(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * insert(Key && k, Value && v)
    {
      MapKey p = map_key(std::forward<Key>(k), std::forward<Value>(v));
      
      MapKey * result = BaseSet::insert(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * append(const Key & k, const Value & v)
    {
      return insert(k, v);
    }
    
    Value * append(Key && k, const Value & v)
    {
      return insert(std::forward<Key>(k), v);
    }
    
    Value * append(const Key & k, Value && v)
    {
      return insert(k, std::forward<Value>(v));
    }
    
    Value * append(Key && k, Value && v)
    {
      return insert(std::forward<Key>(k), std::forward<Value>(v));
    }
    
    Value * search(const Key & k)
    {
      MapKey p = map_key(k, Value());
      
      MapKey * result = BaseSet::search(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * search(Key && k)
    {
      MapKey p = map_key(std::forward<Key>(k), Value());
      
      MapKey * result = BaseSet::search(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * search_or_insert(const Key & k)
    {
      MapKey p = map_key(k, Value());
      return &BaseSet::search_or_insert(std::move(p))->second;
    }
    
    Value * search_or_insert(Key && k)
    {
      MapKey p = map_key(std::forward<Key>(k), Value());
      return &BaseSet::search_or_insert(std::move(p))->second;
    }
    
    Value & find(const Key & k)
    {
      MapKey p = map_key(k, Value());
      return BaseSet::find(std::move(p)).second;
    }
    
    const Value & find(const Key & k) const
    {
      MapKey p = map_key(k, Value());
      return BaseSet::find(std::move(p)).second;
    }
    
    Value & find(Key && k)
    {
      MapKey p = map_key(std::forward<Key>(k), Value());
      return BaseSet::find(std::move(p)).second;
    }
    
    const Value & find(Key && k) const
    {
      MapKey p = map_key(std::forward<Key>(k), Value());
      return BaseSet::find(std::move(p)).second;
    }
    
    bool remove(const Key & k)
    {
      MapKey p = map_key(k, Value());
      return BaseSet::remove(p);
    }
    
    Value & operator [] (const Key & k)
    {
      return *search_or_insert(k);
    }
    
    const Value & operator [] (const Key & k) const
    {
      return *search_or_insert(k);
    }
    
    Value & operator [] (Key && k)
    {
      return *search_or_insert(std::forward<Key>(k));
    }
    
    const Value & operator [] (Key && k) const
    {
      return *search_or_insert(std::forward<Key>(k));
    }
  };

  template <typename Key, typename Value, class Cmp = std::less<Key>>
  class ArrayMap : public GenMap<ArraySet, Key, Value, Cmp>
  {
    using BaseArray     = ArraySet<MapKey<Key, Value>,
				   CmpWrapper<Key, Value, Cmp>>;
    using BaseMap        = GenMap<ArraySet, Key, Value, Cmp>;
    using CmpWrapperType = CmpWrapper<Key, Value, Cmp>;
    using MapKey         = MapKey<Key, Value>;

  public:
    ArrayMap(nat_t cap, Cmp & _cmp)
      : BaseMap(cap, CmpWrapperType(_cmp))
    {
      // empty
    }
    
    ArrayMap(Cmp && _cmp = Cmp())
      : BaseMap(CmpWrapperType(std::forward<Cmp>(_cmp)))
    {
      // empty
    }
    
    ArrayMap(nat_t cap, Cmp && _cmp = Cmp())
      : BaseMap(cap, CmpWrapperType(std::forward<Cmp>(_cmp)))
    {
      // empty
    }
    
    ArrayMap(const std::initializer_list<MapKey> & l)
      : BaseMap(l)
    {
      // empty
    }

    ArrayMap(const ArrayMap & map)
      : BaseMap(map)
    {
      // empty
    }
    
    ArrayMap(ArrayMap && map)
      : ArrayMap()
    {
      BaseMap::swap(map);
    }
  };

  template <typename Key, typename Value, class Cmp = std::less<Key>>
  class TreeMap : public GenMap<TreeSet, Key, Value, Cmp>
  {
    using BaseTree       = TreeSet<MapKey<Key, Value>,
				   CmpWrapper<Key, Value, Cmp>>;
    using BaseMap        = GenMap<TreeSet, Key, Value, Cmp>;
    using CmpWrapperType = CmpWrapper<Key, Value, Cmp>;
    using MapKey         = MapKey<Key, Value>;

  public:
    TreeMap(rng_seed_t seed, Cmp & _cmp)
      : BaseMap(seed, CmpWrapperType(_cmp))
    {
      // empty
    }

    TreeMap(Cmp & _cmp)
      : BaseMap(CmpWrapperType(_cmp))
    {
      // empty
    }
    
    TreeMap(Cmp && _cmp = Cmp())
      : TreeMap(_cmp)
    {
      // empty
    }

    TreeMap(rng_seed_t seed, Cmp && _cmp = Cmp())
      : TreeMap(seed, _cmp)
    {
      // empty
    }

    TreeMap(const std::initializer_list<MapKey> & l)
      : BaseMap(l)
    {
      // empty
    }

    TreeMap(const TreeMap & map)
      : BaseMap(map)
    {
      // empty
    }

    TreeMap(TreeMap && map)
      : TreeMap()
    {
      BaseMap::swap(map);
    }
  };

  template<typename Key, typename Value, typename Fct>
  inline nat_t hash_fct_wrapper(Fct fct, const MapKey<Key, Value> & p)
  {
    return fct(p.first);
  }
  
  template <typename Key,
	    typename Value,
	    class Cmp = std::equal_to<Key>>
  class HashMap : public GenMap<HashSet, Key, Value, Cmp>
  {
    using BaseMap        = GenMap<HashSet, Key, Value, Cmp>;
    using HashFctPtr     = nat_t (*) (const Key &);
    using HashFctType    = std::function<nat_t(const Key &)>;
    using CmpWrapperType = CmpWrapper<Key, Value, Cmp>;
    using MapKey         = MapKey<Key, Value>;
    using BaseHash       = HashSet<MapKey, CmpWrapperType>;
    
    HashFctType fct;

  public:
    HashMap(nat_t size, Cmp & _cmp, HashFctPtr _fct)
      : BaseMap(size, CmpWrapperType(_cmp),
		std::bind(hash_fct_wrapper<Key,Value,HashFctType>, _fct,
			  std::placeholders::_1)), fct(_fct)
    {
      // empty
    }
    
    HashMap(nat_t size, Cmp && _cmp = Cmp(), HashFctPtr fct = &super_fast_hash)
      : HashMap(size, _cmp, fct)
    {
      // empty
    }
    
    HashMap(Cmp & _cmp, HashFctPtr _fct)
      : BaseMap(CmpWrapperType(_cmp),
		std::bind(hash_fct_wrapper<Key,Value,HashFctType>, _fct,
			  std::placeholders::_1)), fct(_fct)
    {
      // empty
    }

    HashMap(Cmp && _cmp = Cmp(), HashFctPtr fct = &super_fast_hash)
      : HashMap(_cmp, fct)
    {
	// empty
    }

    HashMap(const std::initializer_list<MapKey> &);

    HashMap(const HashMap & map)
      : BaseMap(map), fct(map.fct)
    {
      // empty
    }

    HashMap(HashMap && map)
      : HashMap()
    {
      swap(map);
    }

    void swap(HashMap & map)
    {
      BaseMap::swap(map);
      std::swap(fct, map.fct);
    }

    HashFctType & get_hash_fct()
    {
      return fct;
    }

    const HashFctType & get_hash_fct() const
    {
      return fct;
    }
  };
  
  template <typename Key, typename Value, class Cmp>
  HashMap<Key, Value, Cmp>::HashMap(const std::initializer_list<MapKey> & l)
    : HashMap(l.size())
  {
    for (const auto & item : l)
      BaseHash::append(item);
  }
  
} // end namespace Designar

# endif // DSGMAP_H
