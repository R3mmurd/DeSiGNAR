/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <types.hpp>

#include <iterator>

namespace Designar
{
  template <bool B, typename T, typename F>
  using RetType = typename std::conditional<B, T, F>::type;

  /** The nested typedefs std::iterator_traits<Iterator> looks for, so any
      concrete iterator built on this CRTP hierarchy works directly with
      `<algorithm>`/`<numeric>` (std::find, std::sort, std::accumulate,
      std::distance, etc.) without the caller having to do anything extra.
      iterator_category itself is declared separately at each of
      ForwardIterator/BidirectionalIterator/RandomAccessIterator below
      (each hides this class's declaration for any concrete iterator
      that derives from it), since that is the one trait that actually
      varies by capability; value_type/difference_type/pointer/reference
      only depend on T and RET_CPY, which are the same at every level. */
  template <class Iterator, typename T, bool RET_CPY = false>
  class BasicIterator
  {
  protected:
    Iterator& me()
    {
      return *static_cast<Iterator*>(this);
    }

    const Iterator& const_me() const
    {
      return *static_cast<const Iterator*>(this);
    }

  public:
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = int_t;
    using pointer = T*;
    using reference = RetType<RET_CPY, T, T&>;

    bool has_curr() const
    {
      return const_me().has_current();
    }

    RetType<RET_CPY, T, T&> get_curr()
    {
      return me().get_current();
    }

    RetType<RET_CPY, T, const T&> get_curr() const
    {
      return const_me().get_current();
    }

    RetType<RET_CPY, T, T&> operator*()
    {
      return me().get_current();
    }

    RetType<RET_CPY, T, const T&> operator*() const
    {
      return const_me().get_current();
    }

    T* operator->()
    {
      return &me().get_current();
    }

    bool operator==(const Iterator& it) const
    {
      return const_me().get_location() == it.get_location();
    }

    bool operator!=(const Iterator& it) const
    {
      return const_me().get_location() != it.get_location();
    }
  };

  template <class Iterator, typename T, bool RET_CPY = false>
  class ForwardIterator : public BasicIterator<Iterator, T, RET_CPY>
  {
    using Base = BasicIterator<Iterator, T, RET_CPY>;

  public:
    using iterator_category = std::forward_iterator_tag;

    Iterator& operator++()
    {
      Base::me().next();
      return Base::me();
    }

    Iterator operator++(int)
    {
      Iterator ret_val = Base::me();
      Base::me().next();
      return ret_val;
    }
  };

  template <class Iterator, typename T, bool RET_CPY = false>
  class BidirectionalIterator : public ForwardIterator<Iterator, T, RET_CPY>
  {
    using Base = ForwardIterator<Iterator, T, RET_CPY>;

  public:
    using iterator_category = std::bidirectional_iterator_tag;

    Iterator& operator--()
    {
      Base::me().prev();
      return Base::me();
    }

    Iterator operator--(int)
    {
      Iterator ret_val = Base::me();
      Base::me().prev();
      return ret_val;
    }
  };

  template <class Iterator, typename T, bool RET_CPY = false>
  class RandomAccessIterator : public BidirectionalIterator<Iterator, T, RET_CPY>
  {
    using Base = BidirectionalIterator<Iterator, T, RET_CPY>;

  public:
    using iterator_category = std::random_access_iterator_tag;
    using difference_type = typename Base::difference_type;

    RetType<RET_CPY, T, T&> operator[](nat_t i)
    {
      Base::me().move_to(i);
      return Base::me().get_current();
    }

    Iterator& operator+=(nat_t p)
    {
      Base::me().next_n(p);
      return Base::me();
    }

    Iterator operator+(nat_t p)
    {
      Iterator it = Base::me();
      it += p;
      return it;
    }

    Iterator& operator-=(nat_t p)
    {
      Base::me().prev_n(p);
      return Base::me();
    }

    Iterator operator-(nat_t p)
    {
      Iterator it = Base::me();
      it -= p;
      return it;
    }

    /** Distance between two iterators into the same sequence — what
        std::distance() (and, in turn, algorithms like std::sort() that
        are specialized for random-access iterators) needs in order to
        compute the O(1) distance a forward/bidirectional iterator would
        otherwise have to walk to find. Signed, since `it - other` is
        negative when `it` comes before `other`. */
    difference_type operator-(const Iterator& it) const
    {
      return difference_type(Base::const_me().get_location()) -
             difference_type(it.get_location());
    }

    bool operator<(const Iterator& it) const
    {
      return Base::const_me().get_location() < it.get_location();
    }

    bool operator<=(const Iterator& it) const
    {
      return Base::const_me().get_location() <= it.get_location();
    }

    bool operator>(const Iterator& it) const
    {
      return Base::const_me().get_location() > it.get_location();
    }

    bool operator>=(const Iterator& it) const
    {
      return Base::const_me().get_location() >= it.get_location();
    }
  };

  template <class It>
  It next_it(const It& it)
  {
    It ret_val = it;
    ret_val.next();
    return ret_val;
  }

  template <class It>
  It next_it_c(const It& it)
  {
    It ret_val = it;
    ret_val.next();

    if (!ret_val.has_current())
    {
      ret_val.reset_first();
    }

    return ret_val;
  }

  template <class It>
  It prev_it(const It& it)
  {
    It ret_val = it;
    ret_val.prev();
    return ret_val;
  }
} // end namespace Designar
