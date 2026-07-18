/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <numbertheory.hpp>

using namespace Designar;

int main()
{
    cout << "gcd(1071, 462) = " << gcd(int_t(1071), int_t(462)) << endl;

    int_t x, y;
    int_t g = extended_gcd(int_t(1071), int_t(462), x, y);
    cout << "extended_gcd(1071, 462): gcd = " << g << ", 1071*(" << x
         << ") + 462*(" << y << ") = " << (1071 * x + 462 * y) << endl;

    cout << "mod_pow(4, 13, 497) = " << mod_pow(int_t(4), int_t(13), int_t(497))
         << endl;

    int_t inv = mod_inverse(int_t(3), int_t(11));
    cout << "mod_inverse(3, 11) = " << inv << " (check: 3*" << inv
         << " mod 11 = " << (3 * inv) % 11 << ")" << endl;

    return 0;
}
