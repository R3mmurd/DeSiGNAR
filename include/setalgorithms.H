/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGSETALGORITHMS_H
# define DSGSETALGORITHMS_H

# include <types.H>
# include <typetraits.H>

namespace Designar
{
  template <class T> class SLList;
  
  template <class SetType, typename Key>
  class SetAlgorithms
  {
    
    SetType & me()
    {
      return static_cast<SetType *>(this);
    }

    const SetType & const_me() const
    {
      return *static_cast<const SetType *>(this);
    }

  public:
    bool contains(const Key & k) const
    {
      return const_me().search(k) != nullptr;
    }

    bool has(const Key & k) const
    {
      return contains(k);
    }

    static SetType join(const SetType & s1, const SetType & s2)
    {
      SetType ret_val = s1;

      s2.for_each([&ret_val] (const Key & item)
		  {
		    ret_val.append(item);
		  });

      return ret_val;
    }

    SetType join(const SetType & s) const
    {
      return join(const_me(), s);
    }

    static SetType intersect(const SetType & s1, const SetType & s2)
    {
      if (s1.size() < s2.size())
	return s1.template filter<SetType>([&s2] (const Key & item)
					   {
					     return s2.contains(item);
					   });
      
      return s2.template filter<SetType>([&s1] (const Key & item)
					   {
					     return s1.contains(item);
					   });
    }

    SetType intersect(const SetType & s) const
    {
      return intersect(const_me(), s);
    }

    static SetType difference(const SetType & s1, const SetType & s2)
    {
      return s1.template filter<SetType>([&s2] (const Key & item)
					 {
					   return not s2.contains(item);
					 });
    }

    SetType difference(const SetType & s) const
    {
      return difference(const_me(), s);
    }

    template <class SetType2 = SetType>
    static SLList<std::pair<Key, typename SetType2::KeyType>>
      cartesian_product(const SetType & s1, const SetType2 & s2)
    {
      using Key2 = typename SetType2::KeyType;
      
      SLList<std::pair<Key, Key2>> ret_val;

      s1.for_each([&ret_val, &s2] (const Key & item1)
		  {
		    s2.for_each([&ret_val, &item1] (const Key2 & item2)
				{
				  ret_val.append(std::make_pair(item1, item2));
				});
		  });
      
      return ret_val;
    }
    
    template <class SetType2 = SetType>
    SLList<std::pair<Key, typename SetType2::KeyType>>
    cartesian_product(const SetType2 & s) const
    {
      return cartesian_product<SetType2>(const_me(), s);
    }
  };
  
} // end namespace Designar

# endif // DSGSETALGORITHMS_H
