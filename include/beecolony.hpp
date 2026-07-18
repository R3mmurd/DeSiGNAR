/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file beecolony.hpp
    @brief ArtificialBeeColony: Karaboga's ABC metaheuristic for
    continuous function minimization over `[lower, upper]^dimension`.
    A colony of `colony_size` "food sources" (candidate solutions, each a
    `Vector<real_t>`) is refined in three phases per iteration: employed
    bees perturb their own source toward another random source and
    greedily keep the result only if it improves; onlooker bees revisit
    sources chosen with probability proportional to how good they are
    (better sources get explored more); scout bees replace any source
    that has gone `limit` iterations without improving with a fresh
    random point, the mechanism that keeps the colony from getting stuck
    exploiting a mediocre local optimum forever.
    @ingroup ArtificialIntelligence
*/

#pragma once

#include <cmath>

#include <types.hpp>
#include <array.hpp>
#include <random.hpp>
#include <metaheuristic.hpp>
#include <linearalgebra.hpp>

namespace Designar
{
    /** `ObjectiveFn`: `real_t operator()(const Vector<real_t>&)` — the
        function to *minimize*. */
    template <class ObjectiveFn>
    class ArtificialBeeColony
    {
        nat_t dimension;
        real_t lower_bound;
        real_t upper_bound;
        nat_t colony_size;
        nat_t limit;
        rng_t rng;

        ObjectiveFn objective_fn;

        DynArray<Vector<real_t>> sources;
        DynArray<real_t> values;
        DynArray<nat_t> trial_counts;
        nat_t iteration_count;
        bool has_best;
        Vector<real_t> best_solution_;
        real_t best_value_;

        /** Karaboga's standard transform from a value-to-minimize into a
            positive "the bigger the better" weight roulette_wheel_select
            can use: values `>= 0` shrink toward 0 as they grow (never
            reaching it), values `< 0` grow without bound as they become
            more negative — either way, smaller `v` always yields a
            strictly larger fitness. */
        static real_t fitness_from_value(real_t v)
        {
            return v >= 0 ? 1 / (1 + v) : 1 + std::abs(v);
        }

        Vector<real_t> random_source()
        {
            Vector<real_t> source(dimension, real_t());

            for (nat_t d = 0; d < dimension; ++d)
            {
                source[d] = random_uniform<real_t>(rng, lower_bound,
                                                    upper_bound);
            }

            return source;
        }

        /** Perturbs source `i` along one randomly chosen dimension,
            moving toward (or away from, depending on `phi`'s sign) a
            second randomly chosen source `k != i` — the one piece of
            "shared information" ABC's employed/onlooker phases use, as
            opposed to a pure random restart. */
        Vector<real_t> generate_candidate(nat_t i)
        {
            nat_t k = i;

            while (k == i)
            {
                k = random_uniform<nat_t>(rng, colony_size);
            }

            nat_t d = random_uniform<nat_t>(rng, dimension);
            real_t phi = random_uniform<real_t>(rng, -1.0, 1.0);

            Vector<real_t> candidate = sources[i];
            candidate[d] = clamp_to(
                candidate[d] + phi * (candidate[d] - sources[k][d]),
                lower_bound, upper_bound);

            return candidate;
        }

        void update_best(nat_t i)
        {
            if (!has_best || values[i] < best_value_)
            {
                has_best = true;
                best_value_ = values[i];
                best_solution_ = sources[i];
            }
        }

        /** Greedy selection: source `i` is replaced by `candidate` only
            if strictly better, in which case its trial counter resets to
            0 (it just proved it can still be improved); otherwise the
            counter increments (another sign this source may be near a
            local optimum the colony should eventually abandon). */
        void greedy_replace(nat_t i, const Vector<real_t>& candidate)
        {
            real_t candidate_value = objective_fn(candidate);

            if (candidate_value < values[i])
            {
                sources[i] = candidate;
                values[i] = candidate_value;
                trial_counts[i] = 0;
                update_best(i);
            }
            else
            {
                ++trial_counts[i];
            }
        }

    public:
        ArtificialBeeColony(nat_t dim, real_t lo, real_t hi, nat_t size,
                           nat_t abandonment_limit, rng_seed_t seed,
                           ObjectiveFn objective)
            : dimension(dim),
              lower_bound(lo),
              upper_bound(hi),
              colony_size(size),
              limit(abandonment_limit),
              rng(seed),
              objective_fn(std::move(objective)),
              trial_counts(size, nat_t(0)),
              iteration_count(0),
              has_best(false),
              best_value_(0)
        {
            for (nat_t i = 0; i < colony_size; ++i)
            {
                sources.append(random_source());
                values.append(objective_fn(sources[i]));
                update_best(i);
            }
        }

        void step()
        {
            for (nat_t i = 0; i < colony_size; ++i)
            {
                greedy_replace(i, generate_candidate(i));
            }

            DynArray<real_t> fits;

            for (nat_t i = 0; i < colony_size; ++i)
            {
                fits.append(fitness_from_value(values[i]));
            }

            for (nat_t o = 0; o < colony_size; ++o)
            {
                nat_t i = roulette_wheel_select(fits, rng);
                greedy_replace(i, generate_candidate(i));
            }

            for (nat_t i = 0; i < colony_size; ++i)
            {
                if (trial_counts[i] > limit)
                {
                    sources[i] = random_source();
                    values[i] = objective_fn(sources[i]);
                    trial_counts[i] = 0;
                    update_best(i);
                }
            }

            ++iteration_count;
        }

        void run(nat_t iterations)
        {
            for (nat_t i = 0; i < iterations; ++i)
            {
                step();
            }
        }

        nat_t iteration() const
        {
            return iteration_count;
        }

        const Vector<real_t>& best_solution() const
        {
            return best_solution_;
        }

        real_t best_value() const
        {
            return best_value_;
        }
    };

    /** Deduces `ObjectiveFn` from the callback passed in — see
        GeneticAlgorithm's make_genetic_algorithm() for why this factory
        exists instead of naming `ArtificialBeeColony<...>` directly. */
    template <class ObjectiveFn>
    ArtificialBeeColony<ObjectiveFn> make_artificial_bee_colony(
        nat_t dimension, real_t lower_bound, real_t upper_bound,
        nat_t colony_size, nat_t abandonment_limit, rng_seed_t seed,
        ObjectiveFn objective_fn)
    {
        return ArtificialBeeColony<ObjectiveFn>(
            dimension, lower_bound, upper_bound, colony_size,
            abandonment_limit, seed, std::move(objective_fn));
    }

} // end namespace Designar
