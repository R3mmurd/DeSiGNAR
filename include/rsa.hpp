/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file rsa.hpp
    @brief Textbook RSA public-key cryptography, built entirely on
    pieces this library already has: `miller_rabin_is_prime`
    (randomizedalgorithms.hpp) to generate two random primes, `gcd`/
    `mod_inverse`/`mod_pow` (numbertheory.hpp) for the rest of the key
    generation and the encrypt/decrypt operations themselves.

    @warning This is the textbook construction — no OAEP or any other
    padding scheme, no protection against the many known chosen-
    ciphertext/timing/small-exponent attacks real RSA implementations
    must defend against, and key sizes small enough to stay within
    mod_pow()'s documented safe-modulus range (comfortably under
    2^32) rather than the thousands of bits a real RSA modulus needs.
    Exactly the same "education/legacy-format-compat only, not for real
    security use" framing already given to MD5/SHA-1 in hash.hpp — use
    SHA-256 there and a real, audited cryptography library (not this
    one) for anything that actually needs to keep a secret.
    @ingroup Algorithms
*/

#pragma once

#include <stdexcept>

#include <numbertheory.hpp>
#include <randomizedalgorithms.hpp>

namespace Designar
{
    template <typename T>
    struct RSAKeyPair
    {
        T n; // modulus (public)
        T e; // public exponent
        T d; // private exponent
    };

    namespace detail
    {
        /** A random prime in `[low, high)`, found by repeated sampling
            and testing with miller_rabin_is_prime() — the same "keep
            trying random candidates until one passes" approach that
            algorithm itself uses for its own witnesses, appropriate
            here since primes are dense enough (by the prime number
            theorem, roughly 1 in every ln(n) integers near `n` is
            prime) that this terminates quickly for any reasonable
            range. */
        template <typename T>
        T random_prime(rng_t& rng, T low, T high, nat_t miller_rabin_rounds)
        {
            if (low < T(2))
            {
                low = T(2);
            }

            while (true)
            {
                T candidate = low + random_uniform<T>(rng, high - low);

                if (miller_rabin_is_prime(candidate, miller_rabin_rounds, rng))
                {
                    return candidate;
                }
            }
        }
    } // end namespace detail

    /** Generates an RSA key pair: two distinct random primes `p`/`q` in
        `[prime_low, prime_high)`, `n = p*q`, a public exponent `e`
        (65537 if it is coprime to `phi(n) = (p-1)*(q-1)` and smaller
        than it — the conventional real-world default — otherwise the
        smallest odd value that is), and the matching private exponent
        `d = e^-1 mod phi(n)` via mod_inverse(). `prime_high` defaults
        to comfortably keep `n = p*q` under mod_pow()'s documented safe-
        modulus limit (~2^32) — see this file's own @warning about key
        sizes. */
    template <typename T>
    RSAKeyPair<T> rsa_generate_keypair(rng_t& rng, T prime_low = T(1000),
                                       T prime_high = T(50000),
                                       nat_t miller_rabin_rounds = 25)
    {
        T p = detail::random_prime(rng, prime_low, prime_high,
                                   miller_rabin_rounds);
        T q = detail::random_prime(rng, prime_low, prime_high,
                                   miller_rabin_rounds);

        while (q == p)
        {
            q = detail::random_prime(rng, prime_low, prime_high,
                                     miller_rabin_rounds);
        }

        T n = p * q;
        T phi = (p - T(1)) * (q - T(1));

        T e = T(65537);

        if (e >= phi || gcd(e, phi) != T(1))
        {
            e = T(3);

            while (gcd(e, phi) != T(1))
            {
                e += T(2);
            }
        }

        T d = mod_inverse(e, phi);

        return RSAKeyPair<T>{n, e, d};
    }

    /** `message` must be strictly less than `n` (RSA only ever encrypts
        a single "block" smaller than the modulus — chunking a longer
        message into multiple such blocks is the caller's
        responsibility, same as any textbook block-cipher-style
        primitive). */
    template <typename T>
    T rsa_encrypt(T message, T n, T e)
    {
        if (message < T(0) || message >= n)
        {
            throw std::domain_error(
                "rsa_encrypt: message must be in [0, n)");
        }

        return mod_pow(message, e, n);
    }

    template <typename T>
    T rsa_decrypt(T ciphertext, T n, T d)
    {
        return mod_pow(ciphertext, d, n);
    }

} // end namespace Designar
