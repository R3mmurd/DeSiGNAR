/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGMAP_H
# define DSGMAP_H

# include <set.H>

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


  template <typename Key, typename Value, class Cmp, class BaseSet>
  class GenMap :
    public BaseSet
  {
    using Item = MapKey<Key, Value>;
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
      Item p = map_key(k, v);
      
      Item * result = BaseSet::insert(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * insert(Key && k, const Value & v)
    {
      Item p = map_key(std::forward<Key>(k), v);
      
      Item * result = BaseSet::insert(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * insert(const Key & k, Value && v)
    {
      Item p = map_key(k, std::forward<Value>(v));
      
      Item * result = BaseSet::insert(std::move(p));
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * insert(Key && k, Value && v)
    {
      Item p = map_key(std::forward<Key>(k), std::forward<Value>(v));
      
      Item * result = BaseSet::insert(std::move(p));
      
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
      Item p = map_key(k, Value());
      
      Item * result = BaseSet::search(p);
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    Value * search(Key && k)
    {
      Item p = map_key(std::forward<Key>(k), Value());
      
      Item * result = BaseSet::search(p);
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    const Value * search(const Key & k) const
    {
      Item p = map_key(k, Value());
      
      const Item * result = BaseSet::search(p);
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }
    
    const Value * search(Key && k) const
    {
      Item p = map_key(std::forward<Key>(k), Value());
      
      const Item * result = BaseSet::search(p);
      
      if (result == nullptr)
	return nullptr;
      
      return &result->second;
    }

    Value * search_or_insert(const Key & k, const Value & v)
    {
      Item p = map_key(k, v);
      return &BaseSet::search_or_insert(std::move(p))->second;
    }
    
    Value * search_or_insert(Key && k, const Value & v)
    {
      Item p = map_key(std::forward<Key>(k), v);
      return &BaseSet::search_or_insert(std::move(p))->second;
    }

    Value * search_or_insert(const Key & k, Value && v)
    {
      Item p = map_key(k, std::forward<Value>(v));
      return &BaseSet::search_or_insert(std::move(p))->second;
    }
    
    Value * search_or_insert(Key && k, Value && v)
    {
      Item p = map_key(std::forward<Key>(k), std::forward<Value>(v));
      return &BaseSet::search_or_insert(std::move(p))->second;
    }
    
    Value * search_or_insert(const Key & k)
    {
      return search_or_insert(k, Value());
    }
    
    Value * search_or_insert(Key && k)
    {
      return search_or_insert(std::forward<Key>(k), Value());;
    }
    
    Value & find(const Key & k)
    {
      Item p = map_key(k, Value());
      return BaseSet::find(std::move(p)).second;
    }
    
    const Value & find(const Key & k) const
    {
      Item p = map_key(k, Value());
      return BaseSet::find(std::move(p)).second;
    }
    
    Value & find(Key && k)
    {
      Item p = map_key(std::forward<Key>(k), Value());
      return BaseSet::find(std::move(p)).second;
    }
    
    const Value & find(Key && k) const
    {
      Item p = map_key(std::forward<Key>(k), Value());
      return BaseSet::find(std::move(p)).second;
    }

    bool has(const Key & k) const
    {
      Item p = map_key(k, Value());
      return BaseSet::has(std::move(p));
    }

    bool has(Key && k) const
    {
      Item p = map_key(std::forward<Key>(k), Value());
      return BaseSet::has(std::move(p));
    }
    
    bool remove(const Key & k)
    {
      Item p = map_key(k, Value());
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

  template <typename Key, typename Value, class Cmp = std::less<Key>,
	    template <typename, class> class ArrayType = UnsortedArraySet>
  class ArrayMap : public GenMap<Key, Value, Cmp,
				 ArraySet<MapKey<Key, Value>,
					  CmpWrapper<Key, Value, Cmp>,
					  ArrayType>>
  {
    using CmpWrapperType = CmpWrapper<Key, Value, Cmp>;
    using Item           = MapKey<Key, Value>;
    using BaseArray      = ArraySet<Item, CmpWrapperType, ArrayType>;
    using BaseMap        = GenMap<Key, Value, Cmp, BaseArray>;

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
    
    ArrayMap(const std::initializer_list<Item> & l)
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

    ArrayMap & operator = (const ArrayMap & m)
    {
      if (this == &m)
	return *this;

      (BaseMap &) *this = m;
      return *this;
    }

    ArrayMap & operator = (ArrayMap && m)
    {
      BaseMap::swap(m);
      return *this;
    }
  };

  template <typename Key, typename Value, class Cmp = std::less<Key>,
	    template <typename, class> class TreeType = RankedTreap>
  class TreeMap : public GenMap<Key, Value, Cmp,
				TreeSet<MapKey<Key, Value>,
					CmpWrapper<Key, Value, Cmp>,
					TreeType>>
  {
    using CmpWrapperType = CmpWrapper<Key, Value, Cmp>;
    using Item           = MapKey<Key, Value>;
    using BaseTree       = TreeSet<Item, CmpWrapperType, TreeType>;
    using BaseMap        = GenMap<Key, Value, Cmp, BaseTree>;
    

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

    TreeMap(const std::initializer_list<Item> & l)
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

    TreeMap & operator = (const TreeMap & m)
    {
      if (this == &m)
	return *this;

      (BaseMap &) *this = m;
      return *this;
    }

    TreeMap & operator = (TreeMap && m)
    {
      BaseMap::swap(m);
      return *this;
    }
  };

  template<typename Key, typename Value, typename Fct>
  inline nat_t hash_fct_wrapper(Fct fct, const MapKey<Key, Value> & p)
  {
    return fct(p.first);
  }
  
  template <typename Key, typename Value, class Cmp = std::equal_to<Key>,
	    template <typename, class> class HashTableType = LHashTable>
  class HashMap : public GenMap<Key, Value, Cmp,
				HashSet<MapKey<Key, Value>,
					CmpWrapper<Key, Value, Cmp>,
					HashTableType>>
  {
    using CmpWrapperType = CmpWrapper<Key, Value, Cmp>;
    using Item           = MapKey<Key, Value>;
    using BaseHash       = HashSet<Item, CmpWrapperType, LHashTable>;
    using BaseMap        = GenMap<Key, Value, Cmp, BaseHash>;
    using HashFctPtr     = nat_t (*) (const Key &);
    using HashFctType    = std::function<nat_t(const Key &)>;
    
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

    HashMap(const std::initializer_list<Item> &);

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

    HashMap & operator = (const HashMap & m)
    {
      if (this == &m)
	return *this;

      (BaseMap &) *this = m;
      fct = m.fct;
      return *this;
    }

    HashMap & operator = (HashMap && m)
    {
      swap(m);
      return *this;
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
  
  template <typename Key, typename Value, class Cmp,
	    template <typename, class> class HashTableType>
  HashMap<Key, Value, Cmp, HashTableType>::
  HashMap(const std::initializer_list<Item> & l)
    : HashMap(l.size())
  {
    for (const auto & item : l)
      BaseHash::append(item);
  }
  
} // end namespace Designar

# endif // DSGMAP_H
