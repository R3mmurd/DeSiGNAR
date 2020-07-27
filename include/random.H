/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGRANDOM_H
# define DSGRANDOM_H

# include <types.H>

namespace Designar
{

  constexpr lint_t NUM_BITS               = 64;
  constexpr real_t DEFAULT_P              = 0.5;
  constexpr nat_t  DEFAULT_DICE_NUM_FACES = 6;

  rng_seed_t get_random_seed();

  real_t random(rng_t &);

  template <typename T>
  T random_uniform(rng_t &, T);

  template <typename T>
  T random_uniform(rng_t &, T, T);

  bool random_Bernoulli(rng_t &, real_t p = DEFAULT_P);

  nat_t random_binomial(rng_t &, nat_t, real_t p = DEFAULT_P);

  bool flip(rng_t &, real_t p = DEFAULT_P);

  nat_t throw_dice(rng_t &, nat_t num_faces = DEFAULT_DICE_NUM_FACES);

  template <typename T>
  T random_uniform(rng_t & rng, T max)
  {
    static_assert(std::is_arithmetic<T>::value,
		  "Template argument must be an arithmetic type");

    return random(rng) * max;
  }

  template <typename T>
  T random_uniform(rng_t & rng, T l, T r)
  {
    static_assert(std::is_arithmetic<T>::value,
		  "Template argument must be an arithmetic type");
    
    return random_uniform<T>(rng, r - l) + l;
  }
  
} // end namespace Designar

# endif // DSGRANDOM_H
