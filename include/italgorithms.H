/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGITALGORITHMS_H
# define DSGITALGORITHMS_H

# include <types.H>

namespace Designar
{
  template <typename T> class DynArray;
  template <typename T> class SLList;
  
  template <typename RetT, class It>
  RetT * nth_ptr_it(const It &, const It &, nat_t);

  template <typename RetT, class It>
  RetT & nth_it(const It &, const It &, nat_t);

  template <class It, class Op>
  void for_each_it(const It &, const It &, Op &);

  template <class It, class Op>
  void for_each_it(const It &, const It &, Op && op = Op());

  template <class It, class Op>
  void enum_for_each_it(const It &, const It &, Op &);

  template <class It, class Op>
  void enum_for_each_it(const It &, const It &, Op && op = Op());

  template <class It, class Op>
  void for_each_pair_it(const It &, const It &, Op &);

  template <class It, class Op>
  void for_each_pair_it(const It &, const It &, Op && op = Op());

  template <class ContainerRet, class It, class Pred>
  ContainerRet filter_it(const It &, const It &, Pred &);

  template <class ContainerRet, class It, class Pred>
  ContainerRet filter_it(const It &, const It &, Pred && pred = Pred());

  template <class ContainerRet, class It, class Op>
  ContainerRet map_it(const It &, const It &, Op &);

  template <class ContainerRet, class It, class Op>
  ContainerRet map_it(const It &, const It &, Op && op = Op());

  template <class ContainerRet, class It, class Op, class Pred>
  ContainerRet map_if_it(const It &, const It &, Op &, Pred &);
  
  template <class ContainerRet, class It, class Op, class Pred>
  ContainerRet map_if_it(const It &, const It &, Op &, Pred && pred = Pred());
  
  template <class ContainerRet, class It, class Op, class Pred>
  ContainerRet map_if_it(const It &, const It &, Op &&, Pred &);
  
  template <class ContainerRet, class It, class Op, class Pred>
  ContainerRet map_if_it(const It &, const It &, Op && op = Op(),
			 Pred && pred = Pred());

  template <typename RetT, class It, class Pred>
  RetT * search_ptr_it(const It &, const It &, Pred &);

  template <typename RetT, class It, class Pred>
  RetT * search_ptr_it(const It &, const It &, Pred && pred = Pred());

  template <class It, class Pred>
  bool all_it(const It &, const It &, Pred &);

  template <class It, class Pred>
  bool all_it(const It &, const It &, Pred && pred = Pred());

  template <class It, class Pred>
  bool exists_it(const It &, const It &, Pred &);

  template <class It, class Pred>
  bool exists_it(const It &, const It &, Pred && pred = Pred());

  template <class It, class Pred>
  bool none_it(const It &, const It &, Pred &);

  template <class It, class Pred>
  bool none_it(const It &, const It &, Pred && pred = Pred());

  template <typename RetT, class It, class Op>
  RetT fold_it(const It &, const It &, const RetT &, Op &);

  template <typename RetT, class It, class Op>
  RetT fold_it(const It &, const It &, const RetT &, Op && op = Op());

  template <typename RetT, class It, class Op>
  RetT fold_it(const It &, const It &, RetT &&, Op &);

  template <typename RetT, class It, class Op>
  RetT fold_it(const It &, const It &, RetT &&, Op && op = Op());

  template <class It, class Pred>
  bool remove_first_if_it(const It &, const It &, Pred &);
  
  template <class It, class Pred>
  bool remove_first_if_it(const It &, const It &, Pred && pred = Pred());

  template <class It, class Pred>
  void remove_if_it(const It &, const It &, Pred &);

  template <class It, class Pred>
  void remove_if_it(const It &, const It &, Pred && pred = Pred());
  
  template <class It1, class It2, class Eq>
  bool equal_it(const It1 &, const It1 &, const It2 &, const It2 &, Eq &);
  
  template <class It1, class It2, class Eq>
  bool equal_it(const It1 &, const It1 &,
		const It2 &, const It2 &, Eq && eq = Eq());
  
  template <class It, class Cmp>
  bool is_sorted_it(const It &, const It &, Cmp &);
  
  template <class It, class Cmp>
  bool is_sorted_it(const It &, const It &, Cmp && cmp = Cmp());
  
  template <typename T1, typename T2, class It1, class It2>
  SLList<std::pair<T1, T2>>
    zip_it(const It1 &, const It1 &, const It2 &, const It2 &);
    
  template <typename T1, typename T2, class It1, class It2>
  SLList<std::pair<T1, T2>>
    zip_eq_it(const It1 &, const It1 &, const It2 &, const It2 &);

  template <typename T1, typename T2, class It1, class It2>
  SLList<std::pair<T1, T2>>
    zip_left_it(const It1 &, const It1 &, const It2 &, const It2 &);

  template <typename T1, typename T2, class It1, class It2>
  SLList<std::pair<T1, T2>>
    zip_right_it(const It1 &, const It1 &, const It2 &, const It2 &);

  template <class ContainerType, class It>
  ContainerType to_container(const It &, const It &);
  
  template <typename T, class It>
  SLList<T> to_list_it(const It &, const It &);

  template <typename T, class It>
  DynArray<T> to_array_it(const It &, const It &);
  
  template <typename RetT, class It>
  RetT * nth_ptr_it(const It & a, const It & b, nat_t pos)
  {
    for (It i = a; i != b; ++i)
      {
	if (pos == 0)
	  return &*i;
	--pos;
      }

    return nullptr;
  }

  template <typename RetT, class It>
  RetT & nth_it(const It & a, const It & b, nat_t pos)
  {
    for (It i = a; i != b; ++i)
      {
	if (pos == 0)
	  return *i;
	--pos;
      }

    throw std::overflow_error("Index is to large");
  }
  
  template <class It, class Op>
  void for_each_it(const It & a, const It & b, Op & op)
  {
    for (It i = a; i != b; ++i)
      op(*i);
  }

  template <class It, class Op>
  void for_each_it(const It & a, const It & b, Op && op)
  {
    for_each_it<It, Op>(a, b, op);
  }

  template <class It, class Op>
  void enum_for_each_it(const It & a, const It & b, Op & op)
  {
    nat_t p = 0;
    for (It i = a; i != b; ++i, ++p)
      op(p, *i);
  }

  template <class It, class Op>
  void enum_for_each_it(const It & a, const It & b, Op && op)
  {
    enum_for_each_it<It, Op>(a, b, op);
  }

  template <class It, class Op>
  void for_each_pair_it(const It & a, const It & b, Op & op)
  {
    It last_i = prev_it(b);
    for (It i = a; i != last_i; ++i)
      for (It j = next_it(i); j != b; ++j)
	op(*i, *j);
  }

  template <class It, class Op>
  void for_each_pair_it(const It & a, const It & b, Op && op)
  {
    for_each_pair_it<It, Op>(a, b, op);
  }

  template <class ContainerRet, class It, class Pred>
  ContainerRet filter_it(const It & a, const It & b, Pred & pred)
  {
    ContainerRet ret;
    
    for (It i = a; i != b; ++i)
      if (pred(*i))
	ret.append(*i);

    return ret;
  }

  template <class ContainerRet, class It, class Pred>
  ContainerRet filter_it(const It & a, const It & b, Pred && pred)
  {
    return filter_it<ContainerRet, It, Pred>(a, b, pred);
  }

  template <class ContainerRet, class It, class Op>
  ContainerRet map_it(const It & a, const It & b, Op & op)
  {
    ContainerRet ret;

    for (It i = a; i != b; ++i)
      ret.append(op(*i));

    return ret;
  }

  template <class ContainerRet, class It, class Op>
  ContainerRet map_it(const It & a, const It & b, Op && op)
  {
    return map_it<ContainerRet, It, Op>(a, b, op);
  }

  template <class ContainerRet, class It, class Op, class Pred>
  ContainerRet map_if_it(const It & a, const It & b, Op & op, Pred & pred)
  {
    ContainerRet ret;

    for (It i = a; i != b; ++i)
      if (pred(*i))
	ret.append(op(*i));

    return ret;
  }
  
  template <class ContainerRet, class It, class Op, class Pred>
  ContainerRet map_if_it(const It & a, const It & b, Op & op, Pred && pred)
  {
    return map_if_it<ContainerRet, It, Op, Pred>(a, b, op, pred);
  }
  
  template <class ContainerRet, class It, class Op, class Pred>
  ContainerRet map_if_it(const It & a, const It & b, Op && op, Pred & pred)
  {
    return map_if_it<ContainerRet, It, Op, Pred>(a, b, op, pred);
  }
  
  template <class ContainerRet, class It, class Op, class Pred>
  ContainerRet map_if_it(const It & a, const It & b, Op && op, Pred && pred)
  {
    return map_if_it<ContainerRet, It, Op, Pred>(a, b, op, pred);
  }

  template <typename RetT, class It, class Op>
  RetT fold_it(const It & a, const It & b, const RetT & init_value, Op & op)
  {
    RetT ret_val = init_value;
    
    for (It i = a; i != b; ++i)
      ret_val = op(*i, ret_val);

    return ret_val;
  }

  template <typename RetT, class It, class Op>
  RetT fold_it(const It & a, const It & b, const RetT & init_value, Op && op)
  {
    return fold_it<RetT, It, Op>(a, b, init_value, op);
  }

  template <typename RetT, class It, class Op>
  RetT fold_it(const It & a, const It & b, RetT && init_value, Op & op)
  {
    RetT ret_val = std::move(init_value);
    
    for (It i = a; i != b; ++i)
      ret_val = op(*i, ret_val);

    return ret_val;
  }

  template <typename RetT, class It, class Op>
  RetT fold_it(const It & a, const It & b, RetT && init_value, Op && op)
  {
    return fold_it<RetT, It, Op>(a, b, std::forward<RetT>(init_value), op);
  }

  template <class It, class Pred>
  bool all_it(const It & a, const It & b, Pred & pred)
  {
    for (It i = a; i != b; ++i)
      if (not pred(*i))
	return false;
    return true;
  }

  template <class It, class Pred>
  bool all_it(const It & a, const It & b, Pred && pred)
  {
    return all_it<It, Pred>(a, b, pred);
  }

  template <class It, class Pred>
  bool exists_it(const It & a, const It & b, Pred & pred)
  {
    for (It i = a; i != b; ++i)
      if (pred(*i))
	return true;
    return false;
  }

  template <class It, class Pred>
  bool exists_it(const It & a, const It & b, Pred && pred)
  {
    return exists_it<It, Pred>(a, b, pred);
  }

  template <class It, class Pred>
  bool none_it(const It & a, const It & b, Pred & pred)
  {
    for (It i = a; i != b; ++i)
      if (pred(*i))
	return false;
    return true;
  }

  template <class It, class Pred>
  bool none_it(const It & a, const It & b, Pred && pred)
  {
    return none_it<It, Pred>(a, b, pred);
  }

  template <typename RetT, class It, class Pred>
  RetT * search_ptr_it(const It & a, const It & b, Pred & pred)
  {
    for (It i = a; i != b; ++i)
      if (pred(*i))
	return &*i;

    return nullptr;
  }

  template <typename RetT, class It, class Pred>
  RetT * search_ptr_it(const It & a, const It & b, Pred && pred)
  {
    return search_ptr_it<RetT, It, Pred>(a, b, pred);
  }

  template <class It, class Pred>
  bool remove_first_if_it(const It & a, const It & b, Pred & pred)
  {
    for (It i = a; i != b; ++i)
      if (pred(*i))
	{
	  i.del();
	  return true;
	}

    return false;
  }
  
  template <class It, class Pred>
  bool remove_first_if_it(const It & a, const It & b, Pred && pred)
  {
    return remove_first_if_it<It, Pred>(a, b, pred);
  }

  template <class It, class Pred>
  void remove_if_it(const It & a, const It & b, Pred & pred)
  {
    for (It i = a; i != b; )
      if (pred(*i))
	i.del();
      else
	++i;
  }

  template <class It, class Pred>
  void remove_if_it(const It & a, const It & b, Pred && pred)
  {
    remove_if_it<It, Pred>(a, b, pred);
  }
  
  template <class It1, class It2, class Eq>
  bool equal_it(const It1 & a1, const It1 & b1,
		const It2 & a2, const It2 & b2, Eq & eq)
  {
    It1 i1 = a1;
    It2 i2 = a2;

    for ( ; i1 != b1 and i2 != b2; ++i1, ++i2)
      if (not eq(*i1, *i2))
	return false;

    return (i1 == b1 and i2 == b2);
  }
  
  template <class It1, class It2, class Eq>
  bool equal_it(const It1 & a1, const It1 & b1,
		const It2 & a2, const It2 & b2, Eq && eq)
  {
    return equal_it<It1, It2, Eq>(a1, b1, a2, b2, eq);
  }
  
  template <class It, class Cmp>
  bool is_sorted_it(const It & a, const It & b, Cmp & cmp)
  {
    if (a == b)
      return true;

    It i = a;
    It j = a;
    ++j;

    for ( ; j != b; ++i, ++j)
      if (cmp(*j, *i))
	return false;

    return true;
  }
  
  template <class It, class Cmp>
  bool is_sorted_it(const It & a, const It & b, Cmp && cmp)
  {
    return is_sorted_it<It, Cmp>(a, b, cmp);
  }

  template <typename T1, typename T2, class It1, class It2>
  SLList<std::pair<T1, T2>>
    zip_it(const It1 & a1, const It1 & b1, const It2 & a2, const It2 & b2)
  {
    SLList<std::pair<T1, T2>> ret;

    It1 i1 = a1;
    It2 i2 = a2;

    for ( ; i1 != b1 and i2 != b2; ++i1, ++i2)
      ret.append(std::make_pair(*i1, *i2));

    return ret;
  }
  
  template <typename T1, typename T2, class It1, class It2>
  SLList<std::pair<T1, T2>>
    zip_eq_it(const It1 & a1, const It1 & b1, const It2 & a2, const It2 & b2)
  {
    SLList<std::pair<T1, T2>> ret;

    It1 i1 = a1;
    It2 i2 = a2;

    for ( ; i1 != b1 and i2 != b2; ++i1, ++i2)
      ret.append(std::make_pair(*i1, *i2));

    if (i1 != b1 or i2 != b2)
      throw std::length_error("Container sizes mismatch");

    return ret;
  }

  template <typename T1, typename T2, class It1, class It2>
  SLList<std::pair<T1, T2>>
    zip_left_it(const It1 & a1, const It1 & b1, const It2 & a2, const It2 & b2)
  {
    SLList<std::pair<T1, T2>> ret;

    It1 i1 = a1;
    It2 i2 = a2;

    for ( ; i1 != b1; ++i1)
      {
	ret.append(std::make_pair(*i1, *i2));

	++i2;
	
	if (i2 == b2)
	  i2 = a2;
      }

    return ret;
  }

  template <typename T1, typename T2, class It1, class It2>
  SLList<std::pair<T1, T2>>
    zip_right_it(const It1 & a1, const It1 & b1, const It2 & a2, const It2 & b2)
  {
    SLList<std::pair<T1, T2>> ret;

    It1 i1 = a1;
    It2 i2 = a2;

    for ( ; i2 != b2; ++i2)
      {
	ret.append(std::make_pair(*i1, *i2));

	++i1;

	if (i1 == b1)
	  i1 = a1;
      }

    return ret;
  }

  template <class ContainerType, class It>
  ContainerType to_container(const It & a, const It & b)
  {
    ContainerType ret;
    
    for (It i = a; i != b; ++i)
      ret.append(*i);
    
    return ret;
  }

  template <typename T, class It>
  SLList<T> to_list_it(const It & a, const It & b)
  {
    return to_container<SLList<T>, It>(a, b);
  }

  template <typename T, class It>
  DynArray<T> to_array_it(const It & a, const It & b)
  {
    return to_container<DynArray<T>, It>(a, b);
  }

} // end namespace Designar

# endif // DSGITALGORITHMS_H
