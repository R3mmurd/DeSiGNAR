/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file numbertheory.hpp
    @brief Elementary number theory (CLRS ch. 31): the greatest common
    divisor, the extended Euclidean algorithm (Bezout coefficients),
    modular exponentiation, and modular inverse — the building blocks
    randomizedalgorithms.hpp's Miller-Rabin primality test is built on,
    and generally useful anywhere else modular arithmetic comes up.
    @ingroup Algorithms
*/

#pragma once

#include <stdexcept>

#include <types.hpp>

namespace Designar
{
    /** The greatest common divisor of `a` and `b` via the Euclidean
        algorithm — always returned non-negative regardless of the sign
        of the inputs, matching the usual mathematical convention
        (`gcd(a, b) == gcd(|a|, |b|)`). */
    template <typename T>
    T gcd(T a, T b)
    {
        if (a < T(0))
        {
            a = -a;
        }

        if (b < T(0))
        {
            b = -b;
        }

        while (b != T(0))
        {
            T t = b;
            b = a % b;
            a = t;
        }

        return a;
    }

    /** The extended Euclidean algorithm: returns `gcd(a, b)` and sets
        `x`/`y` such that `a*x + b*y == gcd(a, b)` (Bezout's identity) —
        the standard recursive formulation, unwinding the coefficients
        on the way back up from `extended_gcd(b, a % b, ...)`. This is
        what mod_inverse() below is built on: when `gcd(a, m) == 1`,
        `x` here is exactly `a`'s inverse mod `m` (mod `m` and reduced
        into range). */
    template <typename T>
    T extended_gcd(T a, T b, T& x, T& y)
    {
        if (b == T(0))
        {
            x = T(1);
            y = T(0);
            return a;
        }

        T x1, y1;
        T g = extended_gcd(b, a % b, x1, y1);
        x = y1;
        y = x1 - (a / b) * y1;
        return g;
    }

    /** `base^exp mod modulus`, via exponentiation by squaring (the same
        technique math.hpp's fast_integral_pow() uses, applied under a
        modulus) — `O(lg exp)` multiplications instead of `O(exp)`.

        @warning Uses plain `T` multiplication for `(a * b) % modulus`
        internally, which silently overflows if `modulus` is large
        enough that squaring a value just under it doesn't fit in `T`
        (safe for `T = nat_t`/`int_t` — 64-bit — as long as `modulus`
        stays under roughly `2^32`; for real cryptographic-scale moduli,
        a big-number type is needed instead, which this library does not
        provide). Fine for miller_rabin_is_prime() (randomizedalgorithms.hpp)
        and any other teaching-scale use. */
    template <typename T>
    T mod_pow(T base, T exp, T modulus)
    {
        if (modulus == T(1))
        {
            return T(0);
        }

        T result = T(1);
        base %= modulus;

        while (exp > T(0))
        {
            if (exp % T(2) == T(1))
            {
                result = (result * base) % modulus;
            }

            exp /= T(2);
            base = (base * base) % modulus;
        }

        return result;
    }

    /** The modular inverse of `a` mod `modulus` — the unique `x` in
        `[0, modulus)` with `(a * x) % modulus == 1` — via
        extended_gcd(). Throws `std::domain_error` if `a` and `modulus`
        aren't coprime (`gcd(a, modulus) != 1`), since no inverse exists
        then. */
    template <typename T>
    T mod_inverse(T a, T modulus)
    {
        T x, y;
        T g = extended_gcd(a, modulus, x, y);

        if (g != T(1))
        {
            throw std::domain_error(
                "mod_inverse: no inverse exists (a and modulus are not "
                "coprime)");
        }

        return ((x % modulus) + modulus) % modulus;
    }

} // end namespace Designar
