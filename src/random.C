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

# include <random.H>

namespace Designar
{

  rng_seed_t get_random_seed()
  {
    return std::chrono::system_clock::now().time_since_epoch().count() %
      rng_t::max();
  }

  real_t random(rng_t & rng)
  {
    return std::generate_canonical<real_t, NUM_BITS>(rng);
  }

  bool random_Bernoulli(rng_t & rng, real_t p)
  {
    return random(rng) < p;
  }

  nat_t random_binomial(rng_t & rng, nat_t n, real_t p)
  {
    return std::binomial_distribution<nat_t>(n, p)(rng);
  }

  nat_t throw_dice(rng_t & rng, nat_t num_faces)
  {
    return random_uniform(rng, num_faces) + 1;
  }
  
} // end namespace Designar
