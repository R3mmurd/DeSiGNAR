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

# ifndef DSGITERATOR_H
# define DSGITERATOR_H

# include <types.H>

namespace Designar
{
  template <bool B, typename T, typename F>
  using RetType = typename std::conditional<B, T, F>::type;
  
  template <class Iterator, typename T, bool RET_CPY = false>
  class BasicIterator
  {
  protected:
    Iterator & me()
    {
      return *static_cast<Iterator *>(this);
    }
  
    const Iterator & const_me() const
    {
      return *static_cast<const Iterator *>(this);
    }

  public:
    bool has_curr() const
    {
      return const_me().has_current();
    }
    
    RetType<RET_CPY, T, T &> get_curr()
    {
      return me().get_current();
    }

    RetType<RET_CPY, T, const T &> get_curr() const
    {
      return const_me().get_current();
    }
    
    RetType<RET_CPY, T, T &> operator * ()
    {
      return me().get_current();
    }

    RetType<RET_CPY, T, const T &> operator * () const
    {
      return const_me().get_current();
    }

    T * operator -> ()
    {
      return &me().get_current();
    }
  
    bool operator == (const Iterator & it) const
    {
      return const_me().get_location() == it.get_location();
    }

    bool operator != (const Iterator & it) const
    {
      return const_me().get_location() != it.get_location();
    }
  };

  template <class Iterator, typename T, bool RET_CPY = false>
  class ForwardIterator : public BasicIterator<Iterator, T, RET_CPY>
  {
    using Base = BasicIterator<Iterator, T, RET_CPY>;
    
  public:
    Iterator & operator ++ ()
    {
      Base::me().next();
      return Base::me();
    }

    Iterator operator ++ (int)
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
    Iterator & operator -- ()
    {
      Base::me().prev();
      return Base::me();
    }

    Iterator operator -- (int)
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
    RetType<RET_CPY, T, T &> operator [] (nat_t i)
    {
      Base::me().move_to(i);
      return Base::mes().get_current();
    }
    
    Iterator & operator += (nat_t p)
    {
      Base::me().next_n(p);
      return Base::me();
    }

    Iterator operator + (nat_t p)
    {
      Iterator it = Base::me();
      it += p;
      return it;
    }

    Iterator & operator -= (nat_t p)
    {
      Base::me().prev_n(p);
      return Base::me();
    }

    Iterator operator - (nat_t p)
    {
      Iterator it = Base::me();
      it -= p;
      return it;
    }

    bool operator < (const Iterator & it) const
    {
      return Base::const_me().get_location() < it.get_location();
    }
    
    bool operator <= (const Iterator & it) const
    {
      return Base::const_me().get_location() <= it.get_location();
    }
    
    bool operator > (const Iterator & it) const
    {
      return Base::const_me().get_location() > it.get_location();
    }
    
    bool operator >= (const Iterator & it) const
    {
      return Base::const_me().get_location() >= it.get_location();
    }
  };

} // end namespace Designar
  
# endif // DSGITERATOR_H
