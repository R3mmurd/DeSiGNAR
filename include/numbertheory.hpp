/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file numbertheory.hpp
    @brief Elementary number theory (CLRS ch. 31): the greatest common
    divisor, the extended Euclidean algorithm (Bezout coefficients),
    modular exponentiation, modular inverse, and the Chinese remainder
    theorem — the building blocks randomizedalgorithms.hpp's
    Miller-Rabin primality test (and rsa.hpp's textbook RSA) are built
    on, and generally useful anywhere else modular arithmetic comes up.
    @ingroup Algorithms
*/

#pragma once

#include <stdexcept>

#include <array.hpp>
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

    /** The Chinese remainder theorem: given `x mod moduli[i] ==
        remainders[i]` for every `i`, with the `moduli` pairwise
        coprime, returns the unique `x` in `[0, product of moduli)`
        satisfying every congruence at once — via the standard explicit
        construction (`x = sum(remainders[i] * M_i * (M_i^-1 mod
        moduli[i]))`, where `M_i` is the product of every modulus
        *except* `moduli[i]`), reducing modulo the running product `M`
        after every term to keep intermediate values from growing
        needlessly, though the same plain-`T`-multiplication overflow
        caveat as mod_pow()/mod_inverse() still applies for large
        moduli. Throws if `remainders`/`moduli` are empty, mismatched in
        size, or any pair of moduli isn't actually coprime (the case
        this construction assumes away). */
    template <typename T>
    T chinese_remainder_theorem(const DynArray<T>& remainders,
                                const DynArray<T>& moduli)
    {
        if (remainders.is_empty() || remainders.size() != moduli.size())
        {
            throw std::invalid_argument(
                "chinese_remainder_theorem: remainders and moduli must be "
                "non-empty and the same size");
        }

        T big_m = T(1);

        for (nat_t i = 0; i < moduli.size(); ++i)
        {
            for (nat_t j = i + 1; j < moduli.size(); ++j)
            {
                if (gcd(moduli[i], moduli[j]) != T(1))
                {
                    throw std::invalid_argument(
                        "chinese_remainder_theorem: moduli must be pairwise "
                        "coprime");
                }
            }

            big_m *= moduli[i];
        }

        T x = T(0);

        for (nat_t i = 0; i < moduli.size(); ++i)
        {
            T m_i = big_m / moduli[i];
            T y_i = mod_inverse(m_i % moduli[i], moduli[i]);
            T term = ((m_i % big_m) * (y_i % big_m)) % big_m;
            term = (term * (remainders[i] % big_m)) % big_m;
            x = (x + term) % big_m;
        }

        return ((x % big_m) + big_m) % big_m;
    }

} // end namespace Designar
