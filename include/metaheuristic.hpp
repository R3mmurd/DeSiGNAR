/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file metaheuristic.hpp
    @brief Shared building blocks reused across this library's
    metaheuristics (genetic.hpp, antcolony.hpp, beecolony.hpp,
    particleswarm.hpp, simulatedannealing.hpp): the two selection
    strategies (fitness-proportional and tournament) that pick "which
    candidate(s) to favor" from a scored population, and a clamp helper
    for keeping a continuous value inside a search-space bound. Each
    algorithm's own construction/update rules stay in its own file —
    only what's genuinely identical across all of them lives here.
    @ingroup ArtificialIntelligence
*/

#pragma once

#include <types.hpp>
#include <array.hpp>
#include <random.hpp>

namespace Designar
{
    /** Fitness-proportional ("roulette wheel") selection: picks an
        index into `weights` with probability proportional to its own
        weight — spin a wheel where each candidate's slice is sized by
        its weight, see where the ball lands. Used by GeneticAlgorithm's
        optional fitness-proportional selection, AntColonyOptimization's
        next-component choice (weights there are `pheromone^alpha *
        heuristic^beta`), and ArtificialBeeColony's onlooker phase.

        Every `weights[i]` must be `>= 0`. If every weight is exactly 0
        (every candidate equally undesirable, or `weights` is where a
        just-reset scout would otherwise divide by zero), falls back to
        picking uniformly at random instead of throwing — a population
        that has momentarily collapsed to "nothing looks good" is a
        normal state for these algorithms to pass through, not an error
        condition callers should have to special-case. */
    inline nat_t roulette_wheel_select(const DynArray<real_t>& weights,
                                       rng_t& rng)
    {
        real_t total = 0;

        for (nat_t i = 0; i < weights.size(); ++i)
        {
            total += weights[i];
        }

        if (total <= 0)
        {
            return random_uniform<nat_t>(rng, weights.size());
        }

        real_t pick = random(rng) * total;
        real_t running = 0;

        for (nat_t i = 0; i < weights.size(); ++i)
        {
            running += weights[i];

            if (pick < running)
            {
                return i;
            }
        }

        return weights.size() - 1; // floating-point rounding guard
    }

    /** Tournament selection: draws `k` indices uniformly at random (with
        replacement) from `fitnesses` and returns whichever of those `k`
        has the highest fitness — a cheaper, simpler-to-reason-about
        alternative to roulette-wheel selection that GeneticAlgorithm
        uses by default (larger `k` biases more strongly toward already-
        fit individuals; `k == 1` degenerates to picking a uniformly
        random individual). */
    inline nat_t tournament_select(const DynArray<real_t>& fitnesses,
                                   nat_t k, rng_t& rng)
    {
        nat_t best = random_uniform<nat_t>(rng, fitnesses.size());

        for (nat_t i = 1; i < k; ++i)
        {
            nat_t candidate = random_uniform<nat_t>(rng, fitnesses.size());

            if (fitnesses[candidate] > fitnesses[best])
            {
                best = candidate;
            }
        }

        return best;
    }

    /** Clamps `v` into `[lo, hi]` — used by ArtificialBeeColony and
        ParticleSwarmOptimization to keep a candidate solution inside its
        declared search bounds after a perturbation/velocity step might
        have pushed it outside. */
    template <typename T>
    inline T clamp_to(T v, T lo, T hi)
    {
        if (v < lo)
        {
            return lo;
        }

        if (v > hi)
        {
            return hi;
        }

        return v;
    }

} // end namespace Designar
