/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <types.hpp>

namespace Designar
{

  constexpr int_t NUM_BITS = 64;
  constexpr real_t DEFAULT_P = 0.5;
  constexpr nat_t DEFAULT_DICE_NUM_FACES = 6;

  /** Returns a seed for `rng_t` (a `std::mt19937_64` Mersenne Twister)
      derived from the current wall-clock time.

      @warning This module is **not** cryptographically secure and must
      never be used to generate anything whose unpredictability matters
      for security (session tokens, password-reset codes, capability
      URLs, shuffle orders an adversary must not predict, etc.). Seeding
      from wall-clock time means the seed space an attacker must search
      is bounded by how precisely they can guess the process's start
      time — often just a few thousand candidates — after which the
      *entire* future output sequence of the generator is reproducible.
      It is fine for simulations, games, and any non-adversarial use. If
      a security-sensitive use ever arises, seed from `std::random_device`
      (or an OS CSPRNG) instead. */
  rng_seed_t get_random_seed();

  real_t random(rng_t&);

  template <typename T>
  T random_uniform(rng_t&, T);

  template <typename T>
  T random_uniform(rng_t&, T, T);

  bool random_Bernoulli(rng_t&, real_t p = DEFAULT_P);

  nat_t random_binomial(rng_t&, nat_t, real_t p = DEFAULT_P);

  bool flip(rng_t&, real_t p = DEFAULT_P);

  nat_t throw_dice(rng_t&, nat_t num_faces = DEFAULT_DICE_NUM_FACES);

  template <typename T>
  T random_uniform(rng_t& rng, T max)
  {
    static_assert(std::is_arithmetic<T>::value,
                  "Template argument must be an arithmetic type");

    return random(rng) * max;
  }

  template <typename T>
  T random_uniform(rng_t& rng, T l, T r)
  {
    static_assert(std::is_arithmetic<T>::value,
                  "Template argument must be an arithmetic type");

    return random_uniform<T>(rng, r - l) + l;
  }

} // end namespace Designar
