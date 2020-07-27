/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <random.H>

using namespace Designar;

int main()
{
  constexpr nat_t NUM_EXP = 1000000;
  
  constexpr lint_t min_i = -300;
  constexpr lint_t max_i =  300;

  constexpr nat_t min_n = 100;
  constexpr nat_t max_n = 700;

  constexpr real_t min_r = -300.;
  constexpr real_t max_r =  300.;

  constexpr nat_t dice_1 = 10;
  constexpr nat_t dice_2 = 20;

  rng_seed_t seed = time(nullptr);

  rng_t rng_i(seed);
  rng_t rng_n(seed);
  rng_t rng_r(seed);
  rng_t rng_d(seed);

  for (nat_t i = 0; i < NUM_EXP; ++i)
    {
      auto r_i = random_uniform(rng_i, max_i);
      assert(r_i >= 0 and r_i < max_i);
      r_i =  random_uniform(rng_i, min_i, max_i);
      assert(r_i >= min_i and r_i < max_i);

      auto r_n = random_uniform(rng_n, max_n);
      assert(r_n >= 0 and r_n < max_n);
      r_n = random_uniform(rng_n, min_n, max_n);
      assert(r_n >= min_n and r_n < max_n);

      auto r_r = random_uniform(rng_r, max_r);
      assert(r_r >= 0. and r_r < max_r);
      r_r =  random_uniform(rng_r, min_r, max_r);
      assert(r_r >= min_r and r_r < max_r);

      auto dice_result_0 = throw_dice(rng_d);
      assert(dice_result_0 <= DEFAULT_DICE_NUM_FACES);

      auto dice_result_1 = throw_dice(rng_d, dice_1);
      assert(dice_result_1 <= dice_1);

      auto dice_result_2 = throw_dice(rng_d, dice_2);
      assert(dice_result_2 <= dice_2);
    }

  cout << "Everything ok\n";
  return 0;
}
