/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <bitset.H>

namespace Designar
{

  Byte::Byte()
    : b1(0), b2(0), b3(0), b4(0), b5(0), b6(0), b7(0), b8(0)
  {
    // Empty
  }

  Byte::Byte(bool _b1, bool _b2, bool _b3, bool _b4,
	     bool _b5, bool _b6, bool _b7, bool _b8)
    : b1(_b1), b2(_b2), b3(_b3), b4(_b4), b5(_b5), b6(_b6), b7(_b7), b8(_b8)
  {
    // Empty
  }

  Byte::Byte(int num)
  {
    set(num);
  }
  
  Byte::Byte(const Byte & b)
    : b1(b.b1), b2(b.b2), b3(b.b3), b4(b.b4), b5(b.b5), b6(b.b6), b7(b.b7), b8(b.b8)
  {
    // Empty
  }

  bool Byte::get_bit(unsigned char num_bit) const
  {
    switch (num_bit)
      {
      case 0: return b1;
      case 1: return b2;
      case 2: return b3;
      case 3: return b4;
      case 4: return b5;
      case 5: return b6;
      case 6: return b7;
      case 7: return b8;
      default: throw std::overflow_error("Invalid bit number");
      }
  }

  void Byte::set_bit(unsigned char num_bit, bool value)
  {
    switch (num_bit)
      {
      case 0: b1 = value; break;
      case 1: b2 = value; break;
      case 2: b3 = value; break;
      case 3: b4 = value; break;
      case 4: b5 = value; break;
      case 5: b6 = value; break;
      case 6: b7 = value; break;
      case 7: b8 = value; break;
      default: throw std::overflow_error("Invalid bit number");
      }
  }

  void Byte::flip()
  {
    *this = ~*this;
  }

  void Byte::set(int num)
  {
    unsigned char * ptr = (unsigned char *) this;
    *ptr = num;
  }

  int Byte::to_num() const
  {
    unsigned char * ptr = (unsigned char *) this;
    return *ptr;
  }

  Byte::operator int() const
  {
    return to_num();
  }

  std::string Byte::to_string() const
  {
    std::string str = "";
    str += b8 == 0 ? '0' : '1';
    str += b7 == 0 ? '0' : '1';
    str += b6 == 0 ? '0' : '1';
    str += b5 == 0 ? '0' : '1';
    str += b4 == 0 ? '0' : '1';
    str += b3 == 0 ? '0' : '1';
    str += b2 == 0 ? '0' : '1';
    str += b1 == 0 ? '0' : '1';
    return str;
  }

  Byte::operator std::string() const
  {
    return to_string();
  }

  Byte & Byte::operator = (const Byte & b)
  {
    if (&b == this)
      return *this;

    b1 = b.b1;
    b2 = b.b2;
    b3 = b.b3;
    b4 = b.b4;
    b5 = b.b5;
    b6 = b.b6;
    b7 = b.b7;
    b8 = b.b8;

    return *this;
  }

  Byte & Byte::operator = (int num)
  {
    set(num);
    return *this;
  }

  Byte Byte::operator << (nat_t s)
  {
    unsigned char * ptr = (unsigned char *) this;
    Byte ret;
    unsigned char * ret_p = (unsigned char *) &ret;
    *ret_p = *ptr << s;
    return ret;
  }

  void Byte::operator <<= (nat_t s)
  {
    *this = *this << s;
  }

  Byte Byte::operator >> (nat_t s)
  {
    unsigned char * ptr = (unsigned char *) this;
    Byte ret;
    unsigned char * ret_p = (unsigned char *) &ret;
    *ret_p = *ptr >> s;
    return ret;
  }

  void Byte::operator >>= (nat_t s)
  {
    *this = *this >> s;
  }

  Byte Byte::operator & (nat_t c)
  {
    unsigned char * ptr = (unsigned char *) this;
    Byte ret;
    unsigned char * ret_p = (unsigned char *) &ret;
    *ret_p = *ptr & c;
    return ret;
  }

  void Byte::operator &= (nat_t c)
  {
    *this = *this & c;
  }


  Byte Byte::operator | (nat_t c)
  {
    unsigned char * ptr = (unsigned char *) this;
    Byte ret;
    unsigned char * ret_p = (unsigned char *) &ret;
    *ret_p = *ptr | c;
    return ret;
  }

  void Byte::operator |= (nat_t c)
  {
    *this = *this | c;
  }

  Byte Byte::operator ~ ()
  {
    unsigned char * ptr = (unsigned char *) this;
    return ~*ptr;
  }

  bool Byte::operator == (int c) const
  {
    unsigned char * ptr = (unsigned char *) this;
    return *ptr == c;
  }

  bool Byte::operator != (int c) const
  {
    unsigned char * ptr = (unsigned char *) this;
    return *ptr != c;
  }

  bool Byte::operator < (int c) const
  {
    unsigned char * ptr = (unsigned char *) this;
    return *ptr < c;
  }

  bool Byte::operator <= (int c) const
  {
    unsigned char * ptr = (unsigned char *) this;
    return *ptr <= c;
  }

  bool Byte::operator > (int c) const
  {
    unsigned char * ptr = (unsigned char *) this;
    return *ptr > c;
  }

  bool Byte::operator >= (int c) const
  {
    unsigned char * ptr = (unsigned char *) this;
    return *ptr >= c;
  }

  DynBitSet::RWProxy::RWProxy(DynBitSet & _dbs, nat_t _i)
    : dbs(_dbs), i(_i)
  {
    // empty
  }

  DynBitSet::RWProxy::operator bool() const
  {
    return dbs.get_bit(i);
  }

  bool DynBitSet::RWProxy::operator = (bool value)
  {
    dbs.set_bit(i, value);
    return value;
  }

  void DynBitSet::init(nat_t nb, bool val)
  {
    for (nat_t i = 0; i < nb; ++i)
      append(val);
  }

  DynBitSet::DynBitSet()
    : num_bits(0), bit_array()
  {
    // empty
  }

  DynBitSet::DynBitSet(nat_t nb, bool val)
    : DynBitSet()
  {
    init(nb, val);
  }

  DynBitSet::DynBitSet(const DynBitSet & dbs) 
    : num_bits(dbs.num_bits), bit_array(dbs.bit_array)
  {
    // empty
  }

  DynBitSet::DynBitSet(DynBitSet && dbs)
    : DynBitSet()
  {
    swap(dbs);
  }

  DynBitSet::DynBitSet(const std::initializer_list<bool> & l)
    : DynBitSet(l.size())
  {
    nat_t i = l.size() - 1;
  
    for (bool item : l)
      set_bit(i--, item);
  }

  DynBitSet & DynBitSet::operator = (const DynBitSet & dbs)
  {
    if (this == &dbs)
      return *this;

    num_bits = dbs.num_bits;
    bit_array = dbs.bit_array;
      
    return *this;
  }

  DynBitSet & DynBitSet::operator = (DynBitSet && dbs)
  {
    swap(dbs);
    return *this;
  }

  void DynBitSet::swap(DynBitSet & dbs)
  {
    std::swap(num_bits, dbs.num_bits);
    bit_array.swap(dbs.bit_array);
  }

  bool DynBitSet::is_empty() const
  {
    return num_bits == 0;
  }

  void DynBitSet::clear()
  {
    num_bits = 0;
    bit_array.clear();
  }

  nat_t DynBitSet::size() const
  {
    return num_bits;
  }

  void DynBitSet::append(bool value)
  {
    nat_t byte_num = which_byte(num_bits);
      
    Byte & byte = byte_num < bit_array.size() ?
			     bit_array[byte_num] : bit_array.append(Byte());
    byte.set_bit(which_bit_in_byte(num_bits), value);
    ++num_bits;
  }

  bool DynBitSet::remove_last()
  {
    nat_t byte_num = which_byte(num_bits - 1);
    Byte & byte = bit_array[byte_num];
    bool ret_val = byte.get_bit(which_bit_in_byte(num_bits - 1));
    --num_bits;
    return ret_val;
  }

  void DynBitSet::set_bit(nat_t i, bool value)
  {
    if (i >= num_bits)
      throw std::out_of_range("Index out of range");

    Byte & byte = bit_array[which_byte(i)];
    byte.set_bit(which_bit_in_byte(i), value);
  }

  bool DynBitSet::get_bit(nat_t i) const
  {
    if (i >= num_bits)
      throw std::out_of_range("Index out of range");

    const Byte & byte = bit_array[which_byte(i)];
    return byte.get_bit(which_bit_in_byte(i));
  }

  std::string DynBitSet::to_string() const
  {
    std::string ret;

    for (nat_t i = num_bits, j = 0; i > 0; --i, ++j)
      ret.push_back(get_bit(i - 1) ? '1' : '0');

    return ret;
  }
  
  void DynBitSet::write(std::ostream & out) const
  {
    out << bit_array.size() << ' ' << num_bits << ' ';

    for (nat_t i = 0; i < bit_array.size(); ++i)
      out << bit_array[i].to_num() << ' ';
    out << '\n';
  }
  
  void DynBitSet::read(std::istream & in)
  {
    nat_t num_bytes;
    in >> num_bytes >> num_bits;

    for (nat_t i = 0; i < num_bytes; ++i)
      {
	int c;
	in >> c;
	bit_array.append(Byte(c));
      }
  }

  const DynBitSet::RWProxy DynBitSet::operator [] (nat_t i) const
  {
    return RWProxy(const_cast<DynBitSet &>(*this), i);
  }

  DynBitSet::RWProxy DynBitSet::operator [] (nat_t i)
  {
    return RWProxy(*this, i);
  }

  DynBitSet::RWProxy DynBitSet::Iterator::get_current()
  {
    if (not Base::has_current())
      throw std::overflow_error("There is not current element");
    
    return (*array_ptr)[curr];
  }

  DynBitSet::RWProxy DynBitSet::Iterator::get_current() const
  {
    if (not Base::has_current())
	throw std::overflow_error("There is not current element");
    
    return (*array_ptr)[curr];
  }

  DynBitSet::Iterator DynBitSet::begin()
  {
    return Iterator(*this);
  }

  DynBitSet::Iterator DynBitSet::begin() const
  {
    return Iterator(*this);
  }
  
  DynBitSet::Iterator DynBitSet::end() 
  {
    return Iterator(*this, num_bits);
  }
  
  DynBitSet::Iterator DynBitSet::end() const 
  {
    return Iterator(*this, num_bits);
  }
  
} // end namespace Designar
