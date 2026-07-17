/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <bitset.hpp>
#include <random.hpp>

using namespace std;
using namespace Designar;

int main()
{
    DynBitSet bs;
    assert(bs.is_empty());
    assert(bs.size() == 0);

    bs.append(true);
    assert(bs.size() == 1);
    assert(!bs.is_empty());

    assert(bs[0]);
    bs[0] = false;
    assert(!bs[0]);

    bs.remove_last();
    assert(bs.is_empty());
    assert(bs.size() == 0);

    DynBitSet bs1(64);
    assert(bs1.size() == 64);
    for (auto i = 0; i < 64; ++i)
        assert(!bs1[i]);

    DynBitSet bs2(64, true);
    assert(bs2.size() == 64);
    for (auto i = 0; i < 64; ++i)
        assert(bs2[i]);

    DynBitSet bs3{0, 1, 1, 1, 0, 0};
    assert(bs3.size() == 6);
    assert(bs3[0] == 0);
    assert(bs3[1] == 0);
    assert(bs3[2] == 1);
    assert(bs3[3] == 1);
    assert(bs3[4] == 1);
    assert(bs3[5] == 0);

    assert(bs3.to_string() == "011100");

    // Regression: DynBitSet::remove_last() used to compute
    // which_byte(num_bits - 1) without checking num_bits == 0 first; since
    // num_bits is unsigned, that underflowed to a huge value and indexed
    // far out of bounds of the backing array (see bitset.cpp). It must now
    // throw a clean std::underflow_error instead.
    {
        DynBitSet empty_bs;
        assert(empty_bs.is_empty());

        bool threw = false;
        try
        {
            empty_bs.remove_last();
        }
        catch (const underflow_error&)
        {
            threw = true;
        }
        assert(threw);
    }

    // Regression: Byte used to store its 8 bits as adjacent
    // `unsigned int : 1` bitfields and read/write them by reinterpreting
    // `this` as an `unsigned char *`, which silently assumes a specific
    // compiler/ABI bit layout that the standard does not guarantee. It is
    // now a plain `unsigned char` manipulated with explicit mask/shift
    // operations; these checks pin down the expected (LSB-first, matching
    // to_string()'s existing convention) bit numbering.
    {
        Byte b(0);
        assert(b.to_num() == 0);
        assert(b.to_string() == "00000000");

        b.set_bit(0, true);
        b.set_bit(3, true);
        b.set_bit(7, true);

        assert(b.get_bit(0));
        assert(!b.get_bit(1));
        assert(b.get_bit(3));
        assert(b.get_bit(7));
        assert(b.to_num() == (1 | (1 << 3) | (1 << 7)));
        assert(b.to_string() == "10001001");

        Byte c(0b00001111);
        assert(c.to_num() == 0b00001111);
        assert((c << nat_t(2)).to_num() == ((0b00001111 << 2) & 0xFF));
        assert((c >> nat_t(2)).to_num() == (0b00001111 >> 2));
        assert((c & nat_t(0b00000101)).to_num() == 0b00000101);
        assert((c | nat_t(0b11110000)).to_num() == 0b11111111);
        assert((~c).to_num() == (~0b00001111 & 0xFF));

        Byte d(255);
        d.flip();
        assert(d.to_num() == 0);
    }

    cout << "Everything ok!\n";
    return 0;
}
