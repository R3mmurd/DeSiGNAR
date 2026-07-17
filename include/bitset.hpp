/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <array.hpp>

namespace Designar
{

    /** A single byte (8 bits), bit-indexed from 0 (least significant, "b1"
        in to_string()'s naming) to 7 (most significant, "b8"). Stored as a
        plain `unsigned char` and manipulated with explicit mask/shift
        operations.

        Earlier versions represented the 8 bits as eight adjacent
        `unsigned int : 1` bitfields and read/wrote them by reinterpreting
        `this` as an `unsigned char *`. The C++ standard leaves the bit
        order and padding of adjacent bitfields implementation-defined, so
        that approach silently assumed a specific compiler/ABI bit layout;
        on an implementation that packs bitfields differently, every
        operation here (set, to_num, shifts, masks) would have produced
        bit-reversed or corrupted values without ever crashing. Operating
        directly on a single `unsigned char` with `<<`/`>>`/`&`/`|` has
        fully-defined behavior regardless of platform. */
    class Byte
    {
        unsigned char byte;

    public:
        Byte();

        Byte(bool, bool, bool, bool, bool, bool, bool, bool);

        Byte(int);

        Byte(const Byte& b);

        bool get_bit(unsigned char) const;

        void set_bit(unsigned char, bool);

        void flip();

        void set(int);

        int to_num() const;

        operator int() const;

        std::string to_string() const;

        operator std::string() const;

        Byte& operator=(const Byte&);

        Byte& operator=(int);

        Byte operator<<(nat_t);

        void operator<<=(nat_t);

        Byte operator>>(nat_t);

        void operator>>=(nat_t);

        Byte operator&(nat_t);

        void operator&=(nat_t);

        Byte operator|(nat_t);

        void operator|=(nat_t);

        Byte operator~();

        bool operator==(int) const;

        bool operator!=(int) const;

        bool operator<(int) const;

        bool operator<=(int) const;

        bool operator>(int) const;

        bool operator>=(int) const;
    };

    class DynBitSet
    {
    public:
        using ItemType = bool;

        class RWProxy
        {
            DynBitSet& dbs;
            nat_t i;

        public:
            RWProxy(DynBitSet&, nat_t);

            operator bool() const;

            bool operator=(bool);
        };

    private:
        static constexpr unsigned char BIT_SIZE = 8;

        nat_t num_bits;
        DynArray<Byte> bit_array;

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

        void init(nat_t, bool);

    public:
        DynBitSet();

        DynBitSet(nat_t, bool val = false);

        DynBitSet(const DynBitSet&);

        DynBitSet(DynBitSet&&);

        DynBitSet(const std::initializer_list<bool>&);

        ~DynBitSet() = default;

        DynBitSet& operator=(const DynBitSet&);

        DynBitSet& operator=(DynBitSet&&);

        void swap(DynBitSet&);

        bool is_empty() const;

        void clear();

        nat_t size() const;

        void append(bool);

        bool remove_last();

        void set_bit(nat_t, bool);

        bool get_bit(nat_t) const;

        std::string to_string() const;

        void write(std::ostream&) const;

        void read(std::istream&);

        const RWProxy operator[](nat_t) const;

        RWProxy operator[](nat_t);

        class Iterator : public ArrayIterator<Iterator, DynBitSet, bool, true>
        {
            using Base = ArrayIterator<Iterator, DynBitSet, bool, true>;
            using Base::Base;

        public:
            RWProxy get_current();

            RWProxy get_current() const;
        };

        Iterator begin();

        Iterator begin() const;

        Iterator end();

        Iterator end() const;
    };

} // end namespace Designar