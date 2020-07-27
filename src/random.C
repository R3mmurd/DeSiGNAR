/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
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

  bool flip(rng_t & rng, real_t p)
  {
    return random_Bernoulli(rng, p);
  }

  nat_t throw_dice(rng_t & rng, nat_t num_faces)
  {
    return random_uniform(rng, num_faces) + 1;
  }
  
} // end namespace Designar
