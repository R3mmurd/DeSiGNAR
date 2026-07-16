/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <bitset.hpp>

namespace Designar
{

  Byte::Byte()
      : byte(0)
  {
    // Empty
  }

  Byte::Byte(bool _b1, bool _b2, bool _b3, bool _b4,
             bool _b5, bool _b6, bool _b7, bool _b8)
      : byte(static_cast<unsigned char>((_b1 ? 1 : 0) << 0 |
                                        (_b2 ? 1 : 0) << 1 |
                                        (_b3 ? 1 : 0) << 2 |
                                        (_b4 ? 1 : 0) << 3 |
                                        (_b5 ? 1 : 0) << 4 |
                                        (_b6 ? 1 : 0) << 5 |
                                        (_b7 ? 1 : 0) << 6 |
                                        (_b8 ? 1 : 0) << 7))
  {
    // Empty
  }

  Byte::Byte(int num)
  {
    set(num);
  }

  Byte::Byte(const Byte& b)
      : byte(b.byte)
  {
    // Empty
  }

  bool Byte::get_bit(unsigned char num_bit) const
  {
    if (num_bit > 7)
    {
      throw std::overflow_error("Invalid bit number");
    }

    return (byte >> num_bit) & 1;
  }

  void Byte::set_bit(unsigned char num_bit, bool value)
  {
    if (num_bit > 7)
    {
      throw std::overflow_error("Invalid bit number");
    }

    if (value)
    {
      byte |= static_cast<unsigned char>(1u << num_bit);
    }
    else
    {
      byte &= static_cast<unsigned char>(~(1u << num_bit));
    }
  }

  void Byte::flip()
  {
    *this = ~*this;
  }

  void Byte::set(int num)
  {
    byte = static_cast<unsigned char>(num);
  }

  int Byte::to_num() const
  {
    return byte;
  }

  Byte::operator int() const
  {
    return to_num();
  }

  std::string Byte::to_string() const
  {
    std::string str = "";

    for (int i = 7; i >= 0; --i)
    {
      str += get_bit(static_cast<unsigned char>(i)) ? '1' : '0';
    }

    return str;
  }

  Byte::operator std::string() const
  {
    return to_string();
  }

  Byte& Byte::operator=(const Byte& b)
  {
    if (&b == this)
    {
      return *this;
    }

    byte = b.byte;

    return *this;
  }

  Byte& Byte::operator=(int num)
  {
    set(num);
    return *this;
  }

  Byte Byte::operator<<(nat_t s)
  {
    Byte ret;
    ret.byte = static_cast<unsigned char>(byte << s);
    return ret;
  }

  void Byte::operator<<=(nat_t s)
  {
    *this = *this << s;
  }

  Byte Byte::operator>>(nat_t s)
  {
    Byte ret;
    ret.byte = static_cast<unsigned char>(byte >> s);
    return ret;
  }

  void Byte::operator>>=(nat_t s)
  {
    *this = *this >> s;
  }

  Byte Byte::operator&(nat_t c)
  {
    Byte ret;
    ret.byte = static_cast<unsigned char>(byte & c);
    return ret;
  }

  void Byte::operator&=(nat_t c)
  {
    *this = *this & c;
  }

  Byte Byte::operator|(nat_t c)
  {
    Byte ret;
    ret.byte = static_cast<unsigned char>(byte | c);
    return ret;
  }

  void Byte::operator|=(nat_t c)
  {
    *this = *this | c;
  }

  Byte Byte::operator~()
  {
    Byte ret;
    ret.byte = static_cast<unsigned char>(~byte);
    return ret;
  }

  bool Byte::operator==(int c) const
  {
    return byte == c;
  }

  bool Byte::operator!=(int c) const
  {
    return byte != c;
  }

  bool Byte::operator<(int c) const
  {
    return byte < c;
  }

  bool Byte::operator<=(int c) const
  {
    return byte <= c;
  }

  bool Byte::operator>(int c) const
  {
    return byte > c;
  }

  bool Byte::operator>=(int c) const
  {
    return byte >= c;
  }

  DynBitSet::RWProxy::RWProxy(DynBitSet& _dbs, nat_t _i)
      : dbs(_dbs), i(_i)
  {
    // empty
  }

  DynBitSet::RWProxy::operator bool() const
  {
    return dbs.get_bit(i);
  }

  bool DynBitSet::RWProxy::operator=(bool value)
  {
    dbs.set_bit(i, value);
    return value;
  }

  void DynBitSet::init(nat_t nb, bool val)
  {
    for (nat_t i = 0; i < nb; ++i)
    {
      append(val);
    }
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

  DynBitSet::DynBitSet(const DynBitSet& dbs)
      : num_bits(dbs.num_bits), bit_array(dbs.bit_array)
  {
    // empty
  }

  DynBitSet::DynBitSet(DynBitSet&& dbs)
      : DynBitSet()
  {
    swap(dbs);
  }

  DynBitSet::DynBitSet(const std::initializer_list<bool>& l)
      : DynBitSet(l.size())
  {
    nat_t i = l.size() - 1;

    for (bool item : l)
    {
      set_bit(i--, item);
    }
  }

  DynBitSet& DynBitSet::operator=(const DynBitSet& dbs)
  {
    if (this == &dbs)
    {
      return *this;
    }

    num_bits = dbs.num_bits;
    bit_array = dbs.bit_array;

    return *this;
  }

  DynBitSet& DynBitSet::operator=(DynBitSet&& dbs)
  {
    swap(dbs);
    return *this;
  }

  void DynBitSet::swap(DynBitSet& dbs)
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

    Byte& byte = byte_num < bit_array.size() ? bit_array[byte_num] : bit_array.append(Byte());
    // which_bit_in_byte() is num_bit % 8, always in [0, 7]; Byte::set_bit
    // takes unsigned char (a bit position within one byte can never need
    // more range than that) rather than nat_t, so the narrowing here is
    // deliberate and always value-preserving.
    byte.set_bit(static_cast<unsigned char>(which_bit_in_byte(num_bits)), value);
    ++num_bits;
  }

  bool DynBitSet::remove_last()
  {
    if (num_bits == 0)
    {
      throw std::underflow_error("DynBitSet is empty");
    }

    nat_t byte_num = which_byte(num_bits - 1);
    Byte& byte = bit_array[byte_num];
    bool ret_val = byte.get_bit(static_cast<unsigned char>(which_bit_in_byte(num_bits - 1)));
    --num_bits;
    return ret_val;
  }

  void DynBitSet::set_bit(nat_t i, bool value)
  {
    if (i >= num_bits)
    {
      throw std::out_of_range("Index out of range");
    }

    Byte& byte = bit_array[which_byte(i)];
    byte.set_bit(static_cast<unsigned char>(which_bit_in_byte(i)), value);
  }

  bool DynBitSet::get_bit(nat_t i) const
  {
    if (i >= num_bits)
    {
      throw std::out_of_range("Index out of range");
    }

    const Byte& byte = bit_array[which_byte(i)];
    return byte.get_bit(static_cast<unsigned char>(which_bit_in_byte(i)));
  }

  std::string DynBitSet::to_string() const
  {
    std::string ret;

    for (nat_t i = num_bits, j = 0; i > 0; --i, ++j)
    {
      ret.push_back(get_bit(i - 1) ? '1' : '0');
    }

    return ret;
  }

  void DynBitSet::write(std::ostream& out) const
  {
    out << bit_array.size() << ' ' << num_bits << ' ';

    for (nat_t i = 0; i < bit_array.size(); ++i)
    {
      out << bit_array[i].to_num() << ' ';
    }
    out << '\n';
  }

  void DynBitSet::read(std::istream& in)
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

  const DynBitSet::RWProxy DynBitSet::operator[](nat_t i) const
  {
    return RWProxy(const_cast<DynBitSet&>(*this), i);
  }

  DynBitSet::RWProxy DynBitSet::operator[](nat_t i)
  {
    return RWProxy(*this, i);
  }

  DynBitSet::RWProxy DynBitSet::Iterator::get_current()
  {
    if (!Base::has_current())
    {
      throw std::overflow_error("There is not current element");
    }

    return (*array_ptr)[curr];
  }

  DynBitSet::RWProxy DynBitSet::Iterator::get_current() const
  {
    if (!Base::has_current())
    {
      throw std::overflow_error("There is not current element");
    }

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
