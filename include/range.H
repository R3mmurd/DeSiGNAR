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

# ifndef DSGRANGE_H
# define DSGRANGE_H

# include <types.H>
# include <math.H>

namespace Designar
{

  template <typename T>
  class Range
  {
    static_assert(std::is_arithmetic<T>::value,
		  "Template argument must be an arithmetic type");
    
    T __first;
    T __last;
    T __step;

  public:
    Range(T _first, T _last, T _step = T(1))
      : __first(_first), __last(_last), __step(_step)
    {
      if (__first > __last)
	throw std::range_error("First value cannot be greater than last value");

      if (num_equal(__step, T(0)))
	throw std::logic_error("Step cannot be zero");
    }

    Range(T _last)
      : Range(T(0), _last, T(1))
    {
      // empty
    }

    T first() const
    {
      return __first;
    }
    
    T last() const
    {
      return __last;
    }
    
    T step() const
    {
      return __step;
    }

    bool operator == (const Range & r) const
    {
      return num_equal(__first, r.__first) and num_equal(__last, r.__last)
	and num_equal(__step, r.__step);
    }

    bool operator != (const Range & r) const
    {
      return not (*this == r);
    }

    class Iterator
    {
      const Range & r;
      T             c;
      
    public: 
      Iterator(const Range & _r)
	: r(_r), c(r.first())
      {
	// empty
      }

      bool has_current() const
      {
	return not num_equal(c, r.last());
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
	  throw std::overflow_error("There is not current element");

	c = std::min(c + r.step(), r.last());
      }

      void reset_last()
      {
	c = r.last();
      }

      T operator * () const
      {
	return get_current();
      }

      void operator ++ ()
      {
	next();
      }

      bool operator == (const Iterator & it) const
      {
	return num_equal(c, it.c) and r == it.r;
      }

      bool operator != (const Iterator & it) const
      {
	return not (*this == it);
      }
    };

    Iterator begin() const
    {
      return Iterator(*this);
    }

    Iterator end() const
    {
      Iterator it(*this);
      it.reset_last();
      return it;
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

# endif // DSGRANGE_H
