/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <metaheuristic.hpp>

using namespace Designar;

int main()
{
    rng_t rng(20260718);

    // roulette_wheel_select: an all-weight-on-one-index population must
    // always pick that index.
    {
        DynArray<real_t> w = {1.0, 0.0, 0.0};

        for (int i = 0; i < 100; ++i)
        {
            assert(roulette_wheel_select(w, rng) == 0);
        }
    }

    // roulette_wheel_select: all-zero weights must not throw/crash, and
    // must return a valid index.
    {
        DynArray<real_t> zero = {0.0, 0.0, 0.0};

        for (int i = 0; i < 100; ++i)
        {
            nat_t idx = roulette_wheel_select(zero, rng);
            assert(idx < 3);
        }
    }

    // roulette_wheel_select: over many draws, an index's selection
    // frequency should be roughly proportional to its weight.
    {
        DynArray<real_t> w = {1.0, 3.0};
        nat_t count1 = 0;
        constexpr nat_t TRIALS = 4000;

        for (nat_t i = 0; i < TRIALS; ++i)
        {
            if (roulette_wheel_select(w, rng) == 1)
            {
                ++count1;
            }
        }

        real_t ratio = real_t(count1) / TRIALS;
        assert(ratio > 0.65 && ratio < 0.85); // expected 0.75
    }

    // tournament_select: the highest-fitness index should win the
    // overwhelming majority of tournaments.
    {
        DynArray<real_t> fit = {1.0, 5.0, 2.0};
        nat_t best_count = 0;

        for (int i = 0; i < 500; ++i)
        {
            if (tournament_select(fit, 3, rng) == 1)
            {
                ++best_count;
            }
        }

        assert(best_count > 250);
    }

    // clamp_to
    assert(clamp_to(5, 0, 10) == 5);
    assert(clamp_to(-5, 0, 10) == 0);
    assert(clamp_to(15, 0, 10) == 10);
    assert(clamp_to(0, 0, 10) == 0);
    assert(clamp_to(10, 0, 10) == 10);

    return 0;
}
