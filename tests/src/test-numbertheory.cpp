/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <numbertheory.hpp>

using namespace Designar;

int main()
{
    // gcd
    assert(gcd(48, 18) == 6);
    assert(gcd(18, 48) == 6);
    assert(gcd(-48, 18) == 6);
    assert(gcd(48, -18) == 6);
    assert(gcd(0, 5) == 5);
    assert(gcd(5, 0) == 5);
    assert(gcd(0, 0) == 0);
    assert(gcd(17, 5) == 1);
    assert(gcd(1071, 462) == 21);

    // extended_gcd: Bezout's identity must actually hold, not just the
    // gcd value.
    {
        int_t x, y;
        int_t g = extended_gcd(int_t(35), int_t(15), x, y);
        assert(g == gcd(int_t(35), int_t(15)));
        assert(35 * x + 15 * y == g);
    }
    {
        int_t x, y;
        int_t g = extended_gcd(int_t(240), int_t(46), x, y);
        assert(g == gcd(int_t(240), int_t(46)));
        assert(240 * x + 46 * y == g);
    }
    {
        int_t x, y;
        int_t g = extended_gcd(int_t(1071), int_t(462), x, y);
        assert(g == 21);
        assert(1071 * x + 462 * y == g);
    }

    // mod_pow
    assert(mod_pow(int_t(2), int_t(10), int_t(1000)) == 24); // 1024 % 1000
    assert(mod_pow(int_t(3), int_t(0), int_t(7)) == 1);      // x^0 == 1
    assert(mod_pow(int_t(5), int_t(3), int_t(13)) == 125 % 13);
    assert(mod_pow(int_t(4), int_t(13), int_t(497)) == 445); // textbook example

    // Fermat's little theorem: a^(p-1) == 1 (mod p) for prime p not
    // dividing a.
    assert(mod_pow(int_t(2), int_t(1000000006), int_t(1000000007)) == 1);

    // mod_inverse
    assert(mod_inverse(int_t(3), int_t(11)) == 4);  // 3*4 == 12 == 1 (mod 11)
    assert(mod_inverse(int_t(10), int_t(17)) == 12); // 10*12 == 120 == 1 (mod 17)

    for (int_t a = 1; a < 11; ++a)
    {
        int_t inv = mod_inverse(a, int_t(11));
        assert((a * inv) % 11 == 1);
    }

    bool threw = false;

    try
    {
        mod_inverse(int_t(2), int_t(4)); // gcd(2,4) == 2, no inverse
    }
    catch (const std::domain_error&)
    {
        threw = true;
    }

    assert(threw);

    return 0;
}
