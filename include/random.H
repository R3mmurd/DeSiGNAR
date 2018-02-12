/*
  This file is part of Designar.
  Copyright (C) 2017 by Alejandro J. Mujica

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Any user request of this software, write to 

  Alejandro Mujica

  aledrums@gmail.com
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
