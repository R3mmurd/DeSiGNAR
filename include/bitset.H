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

# ifndef DSGBITSET_H
# define DSGBITSET_H

# include <array.H>

namespace Designar
{

  class Byte
  {
    unsigned int b1 : 1;
    unsigned int b2 : 1;
    unsigned int b3 : 1;
    unsigned int b4 : 1;
    unsigned int b5 : 1;
    unsigned int b6 : 1;
    unsigned int b7 : 1;
    unsigned int b8 : 1;

  public:
    Byte();

    Byte(bool, bool, bool, bool, bool, bool, bool, bool);

    Byte(int);
    
    bool get_bit(unsigned char) const;
    
    void set_bit(unsigned char, bool);
    
    void flip();

    void set(int);

    int to_num() const;
    
    operator int() const;

    std::string to_string() const;
    
    operator std::string() const;

    Byte & operator = (const Byte &);
    
    Byte & operator = (int);

    Byte operator << (nat_t);
    
    void operator <<= (nat_t);
    
    Byte operator >> (nat_t);

    void operator >>= (nat_t);

    Byte operator & (nat_t);
    
    void operator &= (nat_t);

    Byte operator | (nat_t);

    void operator |= (nat_t);

    Byte operator ~ ();

    bool operator == (int) const;
    
    bool operator != (int) const;

    bool operator < (int) const;

    bool operator <= (int) const;

    bool operator > (int) const;

    bool operator >= (int) const;
  };

  class DynBitSet
  {
  public:
    class RWProxy
    {
      DynBitSet & dbs;
      nat_t       i;

    public:
      RWProxy(DynBitSet &, nat_t);
      
      operator bool() const;

      bool operator = (bool);
    };
    
  private:
    static constexpr unsigned char BIT_SIZE = 8;
    
    nat_t           num_bits;
    DynArray<Byte>  bit_array;

    static nat_t which_byte(nat_t num_bit)
    {
      return num_bit / 8;
    }

    static nat_t which_bit_in_byte(nat_t num_bit)
    {
      return num_bit % 8;
    }

    static nat_t how_many_bytes(nat_t num_bits)
    {
      return which_byte(num_bits - 1) + 1;
    }

    void init(nat_t);

  public:
    DynBitSet();

    DynBitSet(nat_t);

    DynBitSet(const DynBitSet &);

    DynBitSet(DynBitSet &&);

    DynBitSet(const std::initializer_list<bool> &);

    ~DynBitSet() = default;

    DynBitSet & operator = (const DynBitSet &);

    DynBitSet & operator = (DynBitSet &&);
    
    void swap(DynBitSet &);
    
    bool is_empty() const;

    void clear();

    nat_t size() const;

    void push(bool);

    bool pop();

    void set_bit(nat_t, bool);

    bool get_bit(nat_t) const;

    std::string to_string() const;

    void write(std::ostream &) const;

    void read(std::istream &);

    const RWProxy operator [] (nat_t) const;

    RWProxy operator [] (nat_t);
  };
  
} // end namespace Designar

# endif // DSGBITSET
