/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGSORT_H
# define DSGSORT_H

# include <array.H>
# include <list.H>

namespace Designar
{

  template <typename T,
	    template <typename> class ArrayType,
	    class Cmp = std::less<T>>
  lint_t binary_search(const ArrayType<T> &, const T &,
		       lint_t, lint_t, Cmp &);

  template <typename T,
	    template <typename> class ArrayType,
	    class Cmp = std::less<T>>
  lint_t sequential_search(const ArrayType<T> &, const T &,
			   lint_t, lint_t, Cmp &);

  template <class ArrayType, class Cmp>
  void insertion_sort(ArrayType &, lint_t, lint_t, Cmp &);

  template <typename ArrayType, class Cmp>
  lint_t partition(ArrayType &, lint_t, lint_t, Cmp &);

  template <typename ArrayType, class Cmp>
  void quicksort(ArrayType &, lint_t, lint_t, Cmp &);

  template <typename T, class Cmp>
  void sift_up(T *, nat_t, nat_t, Cmp &);

  template <typename T, class Cmp>
  void sift_down(T *, nat_t, nat_t, Cmp &);

  template <typename T, class Cmp>
  std::tuple<NodeSLList<T>, typename NodeSLList<T>::Node *, NodeSLList<T>>
  partition(NodeSLList<T> &, Cmp &);

  template <typename T, class Cmp>
  void quicksort(NodeSLList<T> &, Cmp &);

  template <class Cmp>
  std::tuple<DL, DL *, DL> partition(DL &, Cmp &);

  template <class Cmp>
  void quicksort(DL &, Cmp &);

  template <class ArrayType>
  ArrayType reverse(const ArrayType &);

  template <class SRCL, class TGTL>
  TGTL reverse(const SRCL &);

  template <typename T, template <typename> class ArrayType, class Cmp>
  lint_t binary_search(const ArrayType<T> & a, const T & k,
		       lint_t l, lint_t r, Cmp & cmp)
  {
    if (l > r)
      return l;
    
    lint_t m = (l + r) / 2;
    
    if (cmp(k, a.at(m)))
      return binary_search(a, k, l, m - 1, cmp);
    else if (cmp(a.at(m), k))
      return binary_search(a, k, m + 1, r, cmp);
    
    return m;
  }

  template <typename T,
	    template <typename> class ArrayType,
	    class Cmp = std::less<T>>
  inline lint_t binary_search(const ArrayType<T> & a, T & k,
			      lint_t l, lint_t r, Cmp && cmp = Cmp())
  {
    return binary_search<T, ArrayType, Cmp>(a, k, l, r, cmp);
  }

  template <typename T,
	    template <typename> class ArrayType,
	    class Cmp = std::less<T>>
  inline lint_t binary_search(const ArrayType<T> & a, const T & k,
			      Cmp & cmp)
  {
    return binary_search<T, ArrayType, Cmp>(a, k, 0, a.size() - 1, cmp);
  }

  template <typename T,
	    template <typename> class ArrayType,
	    class Cmp = std::less<T>>
  inline lint_t binary_search(const ArrayType<T> & a, const T & k,
			      Cmp && cmp = Cmp())
  {
    return binary_search<T, ArrayType, Cmp>(a, k, cmp);
  }

  template <typename T, template <typename> class ArrayType, class Cmp>
  lint_t sequential_search(const ArrayType<T> & a, const T & k,
			   lint_t l, lint_t r, Cmp & cmp)
  {
    lint_t i = l;
    
    while (i <= r and not cmp(k, a.at(i)))
      ++i;
    
    return i;
  }

  template <typename T,
	    template <typename> class ArrayType,
	    class Cmp = std::less<T>>
  lint_t sequential_search(const ArrayType<T> & a, const T & k,
			   lint_t l, lint_t r, Cmp && cmp = Cmp())
  {
    return sequential_search<T, ArrayType, Cmp>(a, k, l, r, cmp);
  }

  template <typename T,
	    template <typename> class ArrayType,
	    class Cmp = std::less<T>>
  lint_t sequential_search(const ArrayType<T> & a, const T & k, Cmp & cmp)
  {
    return sequential_search<T, ArrayType, Cmp>(a, k, 0, a.size() - 1, cmp);
  }

  template <typename T,
	    template <typename> class ArrayType,
	    class Cmp = std::less<T>>
  lint_t sequential_search(const ArrayType<T> & a, const T & k,
			   Cmp && cmp = Cmp())
  {
    return sequential_search<T, ArrayType, Cmp>(a, k, cmp);
  }
   
  template <class ArrayType, class Cmp>
  void insertion_sort(ArrayType & a, lint_t l, lint_t r, Cmp & cmp)
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
  lint_t partition(ArrayType & a, lint_t l, lint_t r, Cmp & cmp)
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
  void quicksort(ArrayType & a, lint_t l, lint_t r, Cmp & cmp)
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

  template <typename T, class Cmp = std::less<T>>
  inline void quicksort(FixedArray<T> & a, Cmp & cmp)
  {
    quicksort<FixedArray<T>, Cmp>(a, cmp);
  }

  template <typename T, class Cmp = std::less<T>>
  inline void quicksort(FixedArray<T> & a, Cmp && cmp = Cmp())
  {
    quicksort<T, Cmp>(a, cmp);
  }

  template <typename T, class Cmp = std::less<T>>
  inline void quicksort(DynArray<T> & a, Cmp & cmp)
  {
    quicksort<DynArray<T>, Cmp>(a, cmp);
  }

  template <typename T, class Cmp = std::less<T>>
  inline void quicksort(DynArray<T> & a, Cmp && cmp = Cmp())
  {
    quicksort<T, Cmp>(a, cmp);
  }

  template <typename T, class Cmp>
  void sift_up(T * a, nat_t l, nat_t r, Cmp & cmp)
  {
    nat_t i = r;

    nat_t u = i / 2;

    while (u >= l and cmp(a[i], a[u]))
      {
	std::swap(a[i], a[u]);
	i = u;
	u = i / 2;
      }
  }

  template <typename T, class Cmp>
  inline void sift_up(T * a, nat_t l, nat_t r, Cmp && cmp = Cmp())
  {
    return sift_up<T, Cmp>(a, l, r, cmp);
  }

  template <typename T, class Cmp>
  void sift_down(T * a, nat_t l, nat_t r, Cmp & cmp)
  {
    nat_t i = l;

    nat_t c = i * 2;

    while (c <= r)
      {
	if (c < r)
	  if (cmp(a[c + 1], a[c]))
	    ++c;

	if (not cmp(a[c], a[i]))
	  break;

	std::swap(a[c], a[i]);
	i = c;
	c = i * 2;
      }
  }

  template <typename T, class Cmp>
  inline void sift_down(T * a, nat_t l, nat_t r, Cmp && cmp = Cmp())
  {
    return sift_down<T, Cmp>(a, l, r, cmp);
  }

  template <typename T, class Cmp>
  std::tuple<NodeSLList<T>, typename NodeSLList<T>::Node *, NodeSLList<T>>
  partition(NodeSLList<T> & l, Cmp & cmp)
  {
    auto pivot = l.remove_first();

    NodeSLList<T> ls, gs;

    while (not l.is_empty())
      {
	auto p = l.remove_first();

	if (cmp(p->get_item(), pivot->get_item()))
	  ls.append(p);
	else
	  gs.append(p);
      }

    return std::make_tuple(std::move(ls), pivot, std::move(gs));
  }

  template <typename T, class Cmp>
  void quicksort(NodeSLList<T> & l, Cmp & cmp)
  {
    if (l.is_unitarian_or_empty())
      return;

    auto part = partition(l, cmp);

    quicksort(std::get<0>(part), cmp);
    quicksort(std::get<2>(part), cmp);

    l.concat(std::get<0>(part));
    l.append(std::get<1>(part));
    l.concat(std::get<2>(part));
  }

  template <typename T, class Cmp = std::less<T>>
  inline void quicksort(NodeSLList<T> & l, Cmp && cmp = Cmp())
  {
    quicksort<T, Cmp>(l, cmp);
  }

  template <class Cmp>
  std::tuple<DL, DL *, DL> partition(DL & l, Cmp & cmp)
  {
    auto pivot = l.remove_next();

    DL ls, gs;

    while (not l.is_empty())
      {
	auto p = l.remove_next();

	if (cmp(p, pivot))
	  ls.insert_prev(p);
	else
	  gs.insert_prev(p);
      }

    return std::make_tuple(std::move(ls), pivot, std::move(gs));
  }

  template <class Cmp>
  void quicksort(DL & l, Cmp & cmp)
  {
    if (l.is_unitarian_or_empty())
      return;

    auto part = partition(l, cmp);

    quicksort(std::get<0>(part), cmp);
    quicksort(std::get<2>(part), cmp);

    l.concat(std::get<0>(part));
    l.insert_prev(std::get<1>(part));
    l.concat(std::get<2>(part));
  }

  template <class Cmp>
  inline void quicksort(DL & l, Cmp && cmp = Cmp())
  {
    quicksort<Cmp>(l, cmp);
  }

  template <typename T, class Cmp>
  struct KeyCmp
  {
    Cmp & cmp;

    KeyCmp(Cmp & _cmp)
      : cmp(_cmp)
    {
      // empty
    }

    KeyCmp(Cmp && _cmp = Cmp())
      : cmp(_cmp)
    {
      // empty
    }

    bool operator () (DL * l, DL * r) const
    {
      return cmp(static_cast<DLNode<T> *>(l)->get_item(),
		 static_cast<DLNode<T> *>(r)->get_item());
    }
  };

  template <typename T, class Cmp>
  struct PtrCmp
  {
    Cmp & cmp;

    PtrCmp(Cmp & _cmp)
      : cmp(_cmp)
    {
      // empty
    }

    PtrCmp(Cmp && _cmp = Cmp())
      : cmp(_cmp)
    {
      // empty
    }

    bool operator () (const T & a, const T & b) const
    {
      return cmp(const_cast<T *>(&a), const_cast<T *>(&b));
    }
  };
  
  template <typename T, class Cmp>
  inline void quicksort(DLNode<T> & l, Cmp & cmp)
  {
    KeyCmp<T, Cmp> key_cmp(cmp);
    quicksort<KeyCmp<T, Cmp>>(l, key_cmp);
  }

  template <typename T, class Cmp = std::less<T>>
  inline void quicksort(DLNode<T> & l, Cmp && cmp = Cmp())
  {
    quicksort<T, Cmp>(l, cmp);
  }

  template <typename T, class Cmp>
  inline void quicksort(DLList<T> & l, Cmp & cmp)
  {
    KeyCmp<T, Cmp> key_cmp(cmp);
    quicksort<KeyCmp<T, Cmp>>(l, key_cmp);
  }

  template <typename T, class Cmp = std::less<T>>
  inline void quicksort(DLList<T> & l, Cmp && cmp = Cmp())
  {
    quicksort<T, Cmp>(l, cmp);
  }

  template <typename SeqType, class Cmp = std::less<typename SeqType::ItemType>>
  inline SeqType sort(const SeqType & s, Cmp & cmp)
  {
    SeqType ret_val = s;
    quicksort<typename SeqType::ItemType, Cmp>(ret_val, cmp);
    return ret_val;
  }

  template <typename SeqType, class Cmp = std::less<typename SeqType::ItemType>>
  inline SeqType sort(const SeqType & s, Cmp && cmp = Cmp())
  {
    return sort<SeqType, Cmp>(s, cmp);
  }

  template <typename SeqType, class Cmp = std::less<typename SeqType::ItemType>>
  inline void inline_sort(SeqType & s, Cmp & cmp)
  {
    quicksort<typename SeqType::ItemType, Cmp>(s, cmp);
  }

  template <typename SeqType, class Cmp = std::less<typename SeqType::ItemType>>
  inline void inline_sort(SeqType & s, Cmp && cmp = Cmp())
  {
    inline_sort<SeqType, Cmp>(s, cmp);
  }

  template <class ArrayType>
  ArrayType reverse(const ArrayType & a)
  {
    ArrayType ret_val;
    
    for (nat_t i = a.size(); i > 0; --i)
      ret_val.append(a[i - 1]);

    return ret_val;
  }

  template <class SRCL, class TGTL>
  TGTL reverse(const SRCL & l)
  {
    TGTL ret_val;

    for (auto item : l)
      ret_val.insert(item);

    return ret_val;
  }

  template <typename T>
  inline FixedArray<T> reverse(const FixedArray<T> & a)
  {
    return reverse<FixedArray<T>>(a);
  }

  template <typename T>
  inline DynArray<T> reverse(const DynArray<T> & a)
  {
    return reverse<DynArray<T>>(a);
  }

  template <typename T>
  inline SLList<T> reverse(const SLList<T> & l)
  {
    return reverse<SLList<T>, SLList<T>>(l);
  }

  template <typename T>
  inline DLList<T> reverse(const DLList<T> & l)
  {
    return reverse<DLList<T>, DLList<T>>(l);
  }

} // end namespace Designar

# endif // DSGSORT_H
