/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <types.hpp>
#include <math.hpp>
#include <iterator.hpp>
#include <containeralgorithms.hpp>

namespace Designar
{

  template <typename T>
  class Range : public ContainerAlgorithms<Range<T>, T>
  {
    static_assert(std::is_arithmetic<T>::value,
                  "Template argument must be an arithmetic type");

  public:
    using ItemType = T;
    using KeyType = T;
    using DataType = T;
    using ValueType = T;
    using SizeType = nat_t;

  private:
    T first;
    T last;
    T step;

  public:
    Range(T _first, T _last, T _step = T(1))
        : first(_first), last(_last), step(_step)
    {
      if (first > last)
      {
        throw std::range_error("First value cannot be greater than last value");
      }

      if (num_equal(step, T(0)))
      {
        throw std::logic_error("Step cannot be zero");
      }
    }

    Range(T _last)
        : Range(T(0), _last, T(1))
    {
      // empty
    }

    Range()
        : Range(std::numeric_limits<T>::max())
    {
      // empty
    }

    T min() const
    {
      return first;
    }

    T max() const
    {
      return last;
    }

    T step_size() const
    {
      return step;
    }

    nat_t size() const
    {
      return std::ceil(double(last - first) / step);
    }

    bool operator==(const Range& r) const
    {
      return num_equal(first, r.first) && num_equal(last, r.last) and num_equal(step, r.step);
    }

    bool operator!=(const Range& r) const
    {
      return !(*this == r);
    }

    class Iterator : public RandomAccessIterator<Iterator, T, true>
    {
      friend class BasicIterator<Iterator, T, true>;

      const Range& r;
      T c;
      nat_t p;

    protected:
      nat_t get_location() const
      {
        return p;
      }

    public:
      Iterator(const Range& _r)
          : r(_r), c(r.min()), p(0)
      {
        // empty
      }

      Iterator(const Range& _r, nat_t pos)
          : r(_r), c(r.min()), p(0)
      {
        move_to(pos);
      }

      bool has_current() const
      {
        return c < r.max();
      }

      T get_current() const
      {
        if (!has_current())
        {
          throw std::overflow_error("There is not current element");
        }

        return c;
      }

      void next()
      {
        if (!has_current())
        {
          return;
        }

        ++p;
        c += r.step_size();
      }

      void next_n(nat_t n)
      {
        if (!has_current())
        {
          return;
        }

        p = std::min(p + n, r.size());
        c = std::min(c + r.step_size() * n, r.size() * r.step_size());
      }

      void prev()
      {
        if (c == r.min())
        {
          return;
        }

        --p;
        c -= r.step_size();
      }

      void prev_n(nat_t n)
      {
        if (n * r.step_size() > c - r.min())
        {
          return;
        }

        p -= n;
        c -= n * r.step_size();
      }

      void reset_first()
      {
        p = 0;
        c = r.min();
      }

      /** Positions the iterator at index `new_p` (clamped to the range's
          size), recomputing the corresponding value from scratch rather
          than stepping incrementally from wherever the iterator
          currently is. Required by RandomAccessIterator::operator[](),
          which calls move_to() directly — without it, any use of
          `range[i]` on a Range/IntRange/UIntRange/RealRange fails to
          compile (a latent bug that stayed hidden because operator[] on
          these iterators was never actually exercised). */
      void move_to(nat_t new_p)
      {
        p = std::min(new_p, r.size());
        c = r.min() + p * r.step_size();
      }

      void reset_last()
      {
        move_to(r.size() - 1);
      }
    };

    Iterator begin() const
    {
      return Iterator(*this);
    }

    Iterator end() const
    {
      return Iterator(*this, size());
    }
  };

  class IntRange : public Range<long long>
  {
    using Base = Range<long long>;
    using Base::Base;
  };

  class UIntRange : public Range<nat_t>
  {
    using Base = Range<nat_t>;
    using Base::Base;
  };

  class RealRange : public Range<real_t>
  {
    using Base = Range<real_t>;
    using Base::Base;
  };

  template <typename T>
  Range<T> range(T f, T l, T s = T(1))
  {
    return Range<T>(f, l, s);
  }

  template <typename T>
  Range<T> range(T l)
  {
    return Range<T>(l);
  }

} // end namespace Designar
