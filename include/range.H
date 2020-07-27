/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGRANGE_H
# define DSGRANGE_H

# include <types.H>
# include <math.H>
# include <iterator.H>
# include <containeralgorithms.H>

namespace Designar
{

  template <typename T>
  class Range : public ContainerAlgorithms<Range<T>, T>
  {
    static_assert(std::is_arithmetic<T>::value,
		  "Template argument must be an arithmetic type");
    
  public:
    using ItemType  = T;
    using KeyType   = T;
    using DataType  = T;
    using ValueType = T;
    using SizeType  = nat_t;

  private:
    T first;
    T last;
    T step;

  public:
    Range(T _first, T _last, T _step = T(1))
      : first(_first), last(_last), step(_step)
    {
      if (first > last)
	throw std::range_error("First value cannot be greater than last value");

      if (num_equal(step, T(0)))
	throw std::logic_error("Step cannot be zero");
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

    bool operator == (const Range & r) const
    {
      return num_equal(first, r.first) and num_equal(last, r.last)
	and num_equal(step, r.step);
    }

    bool operator != (const Range & r) const
    {
      return not (*this == r);
    }

    class Iterator : public RandomAccessIterator<Iterator, T, true>
    {
      friend class BasicIterator<Iterator, T, true>;
      
      const Range & r;
      T             c;
      nat_t         p;

    protected:
      nat_t get_location() const
      {
	return p;
      }      
      
    public: 
      Iterator(const Range & _r)
	: r(_r), c(r.min()), p(0)
      {
	// empty
      }

      Iterator(const Range & _r, nat_t pos)
	: r(_r), c((r.max() - r.min()) * pos), p(pos)
      {
	// empty
      }

      bool has_current() const
      {
	return c < r.max();
      }

      T get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	return c;
      }

      void next()
      {
	if (not has_current())
	  return;

	++p;
	c += r.step_size();
      }

      void next_n(nat_t n)
      {
	if (not has_current())
	  return;

	p = std::min(p + n, r.size());
	c = std::min(c + r.step_size() * n, r.size() * r.step_size());
      }

      void prev()
      {
	if (c == r.min())
	  return;

	--p;
	c -= r.step_size();
      }

      void prev_n(nat_t n)
      {
	if (n * r.step_size() > c - r.min())
	  return;

	p -= n;
	c -= n * r.step_size();
      }

      void reset_first()
      {
	c = r.min();
      }

      void reset_last()
      {
	p = r.size() - 1;
	c = p * r.step();
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

# endif // DSGRANGE__H
