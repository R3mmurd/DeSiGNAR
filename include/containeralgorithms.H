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

# ifndef CONTAINERALGORITHMS_H
# define CONTAINERALGORITHMS_H

# include <rangealgorithms.H>

namespace Designar
{
  template<class ContainerType, typename T>
  class ContainerAlgorithms
  {
    ContainerType & me()
    {
      return *static_cast<ContainerType *>(this);
    }

    const ContainerType & const_me() const
    {
      return *static_cast<const ContainerType *>(this);
    }

  public:
    ContainerAlgorithms()
    {
      // empty
    }

    ContainerAlgorithms(const ContainerAlgorithms &)
    {
      // empty
    }
    
    ContainerAlgorithms(ContainerAlgorithms &&)
    {
      // empty
    }

    ContainerAlgorithms & operator = (const ContainerAlgorithms &)
    {
      return *this;
    }

    ContainerAlgorithms & operator = (ContainerAlgorithms &&)
    {
      return *this;
    }

    T * nth_ptr(nat_t);

    T & nth(nat_t);

    const T & nth(nat_t) const;

    template <class Op>
    void for_each(Op &) const;

    template <class Op>
    void for_each(Op && op = Op()) const;

    template <class ContainerRet, class Pred>
    ContainerRet filter(Pred &) const;

    template <class ContainerRet = SLList<T>, class Pred>
    ContainerRet filter(Pred && pred = Pred()) const;

    template <typename RetT, class ContainerRet, class Op>
    ContainerRet map(Op &) const;

    template <typename RetT = T, class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map(Op && op = Op()) const;

    template <typename RetT, class ContainerRet, class Op, class Pred>
    ContainerRet map_if(Op &, Pred &) const;

    template <typename RetT = T, class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_if(Op &, Pred && pred = Pred()) const;

    template <typename RetT = T, class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_if(Op &&, Pred &) const;

    template <typename RetT = T, class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_if(Op && op = Op(), Pred && pred = Pred()) const;
    
    template <typename RetT, class Op>
    RetT fold(const RetT &, Op &) const;

    template <typename RetT = T, class Op>
    RetT fold(const RetT &, Op && op = Op()) const;

    template <typename RetT = T, class Op>
    RetT fold(RetT &&, Op &) const;

    template <typename RetT = T, class Op>
    RetT fold(RetT &&, Op && op = Op()) const;

    template <class Pred>
    bool all(Pred &) const;

    template <class Pred>
    bool all(Pred && pred = Pred()) const;

    template <class Pred>
    bool exists(Pred &) const;

    template <class Pred>
    bool exists(Pred && pred = Pred()) const;

    template <class Pred>
    T * search_ptr(Pred &) const;

    template <class Pred>
    T * search_ptr(Pred && pred = Pred()) const;

    template <class Pred>
    bool remove_first_if(Pred &);

    template <class Pred>
    bool remove_first_if(Pred && pred = Pred());

    template <class Pred>
    void remove_if(Pred &);

    template <class Pred>
    void remove_if(Pred && pred = Pred());

    template <class Eq>
    bool equal(const ContainerType &, Eq &) const;

    template <class Eq = std::equal_to<T>>
    bool equal(const ContainerType &, Eq && eq = Eq()) const;

    template <class Cmp>
    bool is_sorted(Cmp &) const;

    template <class Cmp = std::less<T>>
    bool is_sorted(Cmp && cmp = Cmp()) const;

    template <class ContainerType2 = ContainerType>
    SLList<std::pair<T, typename ContainerType2::KeyType>>
      zip(const ContainerType2 &) const;

    template <class ContainerType2 = ContainerType>
    SLList<std::pair<T, typename ContainerType2::KeyType>>
      zip_eq(const ContainerType2 &) const;

    template <class ContainerType2 = ContainerType>
    SLList<std::pair<T, typename ContainerType2::KeyType>>
      zip_left(const ContainerType2 &) const;

    template <class ContainerType2 = ContainerType>
    SLList<std::pair<T, typename ContainerType2::KeyType>>
      zip_right(const ContainerType2 &) const;
      
    DynArray<T> to_array() const;

    SLList<T> to_list() const;
  };

  template <class ContainerType, typename T>
  T * ContainerAlgorithms<ContainerType, T>::nth_ptr(nat_t i)
  {
    return nth_ptr_it<T>(me().begin(), me().end(), i);
  }

  template <class ContainerType, typename T>
  T & ContainerAlgorithms<ContainerType, T>::nth(nat_t i)
  {
    return nth_it<T>(me().begin(), me().end(), i);
  }

  template <class ContainerType, typename T>
  const T & ContainerAlgorithms<ContainerType, T>::nth(nat_t i) const
  {
    return nth_it(const_me().begin(), const_me().end(), i);
  }
  
  template <class ContainerType, typename T>
  template <class Op>
  void ContainerAlgorithms<ContainerType, T>::for_each(Op & op) const
  {
    for_each_it(const_me().begin(), const_me().end(), op);
  }

  template <class ContainerType, typename T>
  template <class Op>
  void ContainerAlgorithms<ContainerType, T>::for_each(Op && op) const
  {
    for_each<Op>(op);
  }

  template <class ContainerType, typename T>
  template <class ContainerRet, class Pred>
  ContainerRet ContainerAlgorithms<ContainerType, T>::filter(Pred & pred) const
  {
    return filter_it<ContainerRet>(const_me().begin(), const_me().end(), pred);
  }
  
  template <class ContainerType, typename T>
  template <class ContainerRet, class Pred>
  ContainerRet ContainerAlgorithms<ContainerType, T>::filter(Pred && pred) const
  {
    return filter<ContainerRet, Pred>(pred);
  }

  template <class ContainerType, typename T>
  template <typename RetT, class ContainerRet, class Op>
  ContainerRet ContainerAlgorithms<ContainerType, T>::map(Op & op) const
  {
    return map_it<ContainerRet>(const_me().begin(), const_me().end(), op);
  }
  
  template <class ContainerType, typename T>
  template <typename RetT, class ContainerRet, class Op>
  ContainerRet ContainerAlgorithms<ContainerType, T>::map(Op && op) const
  {
    return map<RetT, ContainerRet, Op>(op);
  }

  template <class ContainerType, typename T>
  template <typename RetT, class ContainerRet, class Op, class Pred>
  ContainerRet
  ContainerAlgorithms<ContainerType, T>::map_if(Op & op, Pred & pred) const
  {
    return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
				   op, pred);
  }
  
  template <class ContainerType, typename T>
  template <typename RetT,class ContainerRet, class Op, class Pred>
  ContainerRet ContainerAlgorithms<ContainerType, T>::
  map_if(Op & op, Pred && pred) const
  {
    return map_if<RetT, ContainerRet, Op, Pred>(op, pred);
  }

  template <class ContainerType, typename T>
  template <typename RetT,class ContainerRet, class Op, class Pred>
  ContainerRet ContainerAlgorithms<ContainerType, T>::
  map_if(Op && op, Pred & pred) const
  {
    return map_if<RetT, ContainerRet, Op, Pred>(op, pred);
  }

  template <class ContainerType, typename T>
  template <typename RetT,class ContainerRet, class Op, class Pred>
  ContainerRet ContainerAlgorithms<ContainerType, T>::
  map_if(Op && op, Pred && pred) const
  {
    return map_if<RetT, ContainerRet, Op, Pred>(op, pred);
  }

  template <class ContainerType, typename T>
  template <typename RetT, class Op>
  RetT ContainerAlgorithms<ContainerType, T>::
  fold(const RetT & init_val, Op & op) const
  {
    return fold_it<RetT>(const_me().begin(), const_me().end(), init_val, op);
  }
  
  template <class ContainerType, typename T>
  template <typename RetT, class Op>
  RetT ContainerAlgorithms<ContainerType, T>::
  fold(const RetT & init_val, Op && op) const
  {
    return fold<RetT, Op>(init_val, op);
  }
  
  template <class ContainerType, typename T>
  template <typename RetT, class Op>
  RetT ContainerAlgorithms<ContainerType, T>::
  fold(RetT && init_val, Op & op) const
  {
    return fold_it<RetT>(const_me().begin(), const_me().end(),
			 std::forward<RetT>(init_val), op);
  }

  template <class ContainerType, typename T>
  template <typename RetT, class Op>
  RetT ContainerAlgorithms<ContainerType, T>::
  fold(RetT && init_val, Op && op) const
  {
    return fold<RetT, Op>(std::forward<RetT>(init_val), op);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  bool ContainerAlgorithms<ContainerType, T>::all(Pred & pred) const
  {
    return all_it(const_me().begin(), const_me().end(), pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  bool ContainerAlgorithms<ContainerType, T>::all(Pred && pred) const
  {
    return all<Pred>(pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  bool ContainerAlgorithms<ContainerType, T>::exists(Pred & pred) const
  {
    return exists_it(const_me().begin(), const_me().end(), pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  bool ContainerAlgorithms<ContainerType, T>::exists(Pred && pred) const
  {
    return exists<Pred>(pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  T * ContainerAlgorithms<ContainerType, T>::search_ptr(Pred & pred) const
  {
    return search_ptr_it<T>(const_me().begin(), const_me().end(), pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  T * ContainerAlgorithms<ContainerType, T>::search_ptr(Pred && pred) const
  {
    return search_ptr<Pred>(pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  bool ContainerAlgorithms<ContainerType, T>::remove_first_if(Pred & pred)
  {
    return remove_first_if_it(me().begin(), me().end(), pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  bool ContainerAlgorithms<ContainerType, T>::remove_first_if(Pred && pred)
  {
    return remove_first_if<Pred>(pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  void ContainerAlgorithms<ContainerType, T>::remove_if(Pred & pred)
  {
    remove_if_it(me().begin(), me().end(), pred);
  }

  template <class ContainerType, typename T>
  template <class Pred>
  void ContainerAlgorithms<ContainerType, T>::remove_if(Pred && pred)
  {
    remove_if<Pred>(pred);
  }

  template <class ContainerType, typename T>
  template <class Eq>
  bool ContainerAlgorithms<ContainerType, T>::
  equal(const ContainerType & c, Eq & eq) const
  {
    return equal_it(const_me().begin(), const_me().end(),
		    c.begin(), c.end(), eq);
  }

  template <class ContainerType, typename T>
  template <class Eq>
  bool ContainerAlgorithms<ContainerType, T>::
  equal(const ContainerType & c, Eq && eq) const
  {
    return equal<Eq>(c, eq);
  }

  template <class ContainerType, typename T>
  template <class Cmp>
  bool ContainerAlgorithms<ContainerType, T>::is_sorted(Cmp & cmp) const
  {
    return is_sorted_it(const_me().begin(), const_me().end(), cmp);
  }

  template <class ContainerType, typename T>
  template <class Cmp>
  bool ContainerAlgorithms<ContainerType, T>::is_sorted(Cmp && cmp) const
  {
    return is_sorted<Cmp>(cmp);
  }
  
  template <class ContainerType, typename T>
  template <class ContainerType2>
  SLList<std::pair<T, typename ContainerType2::KeyType>>
    ContainerAlgorithms<ContainerType, T>::zip(const ContainerType2 & c) const
  {
    using T2 = typename ContainerType2::KeyType;

    return zip_it<T, T2>(const_me().begin(), const_me().end(),
			 c.begin(), c.end());
  }

  template <class ContainerType, typename T>
  template <class ContainerType2>
  SLList<std::pair<T, typename ContainerType2::KeyType>>
    ContainerAlgorithms<ContainerType, T>::
    zip_eq(const ContainerType2 & c) const
  {
    using T2 = typename ContainerType2::KeyType;

    return zip_eq_it<T, T2>(const_me().begin(), const_me().end(),
			    c.begin(), c.end());    
  }

  template <class ContainerType, typename T>
  template <class ContainerType2>
  SLList<std::pair<T, typename ContainerType2::KeyType>>
    ContainerAlgorithms<ContainerType, T>::
    zip_left(const ContainerType2 & c) const
  {
    using T2 = typename ContainerType2::KeyType;

    return zip_left_it<T, T2>(const_me().begin(), const_me().end(),
			      c.begin(), c.end());
  }

  template <class ContainerType, typename T>
  template <class ContainerType2>
  SLList<std::pair<T, typename ContainerType2::KeyType>>
    ContainerAlgorithms<ContainerType, T>::
    zip_right(const ContainerType2 & c) const
  {
    using T2 = typename ContainerType2::KeyType;

    return zip_right_it<T, T2>(const_me().begin(), const_me().end(),
			       c.begin(), c.end());
  }
  
  template <class ContainerType, typename T>
  DynArray<T> ContainerAlgorithms<ContainerType, T>::to_array() const
  {
    return to_array_it<T>(const_me().begin(), const_me().end());
  }
  
  template <class ContainerType, typename T>
  SLList<T> ContainerAlgorithms<ContainerType, T>::to_list() const
  {
    return to_list_it<T>(const_me().begin(), const_me().end());
  }
  
} // end namespace Designar

# endif // CONTAINERALGORITHMS_H
