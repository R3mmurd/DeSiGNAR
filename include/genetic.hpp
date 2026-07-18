/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file genetic.hpp
    @brief GeneticAlgorithm: a generic evolutionary-search metaheuristic.
    Maintains a population of `Individual` values scored by a
    caller-supplied fitness function (higher is better), advancing one
    generation at a time via elitism + tournament selection + crossover
    + mutation — the same tunable knobs any textbook GA exposes, with
    every problem-specific piece (what an individual *is*, how fit one
    is, how two combine, how one mutates) supplied as a callback rather
    than baked in, so this one class serves everything from bit-string
    optimization to real-valued function minimization to permutation
    problems.
    @ingroup ArtificialIntelligence
*/

#pragma once

#include <utility>

#include <types.hpp>
#include <array.hpp>
#include <random.hpp>
#include <metaheuristic.hpp>

namespace Designar
{
    /** `InitFn`: `Individual operator()(rng_t&)` — builds one random
        individual (used to seed the initial population).
        `FitnessFn`: `real_t operator()(const Individual&)` — higher is
        better; this class always maximizes, so a minimization problem
        should negate its objective before returning it.
        `CrossoverFn`: `Individual operator()(const Individual&,
        const Individual&, rng_t&)` — combines two parents into one
        offspring.
        `MutateFn`: `void operator()(Individual&, rng_t&)` — perturbs an
        individual in place.

        Construct via make_genetic_algorithm() rather than naming this
        class template directly — the four callback types are painful to
        spell out by hand and are deducible from the callbacks actually
        passed in. */
    template <typename Individual, class InitFn, class FitnessFn,
              class CrossoverFn, class MutateFn>
    class GeneticAlgorithm
    {
        nat_t population_size;
        real_t crossover_rate;
        real_t mutation_rate;
        nat_t elitism_count;
        rng_t rng;

        InitFn init_fn;
        FitnessFn fitness_fn;
        CrossoverFn crossover_fn;
        MutateFn mutate_fn;

        DynArray<Individual> population;
        DynArray<real_t> fitnesses;
        nat_t generation_count;
        bool has_best;
        Individual best_individual_;
        real_t best_fitness_;

        /** Scores every individual in the current population, replacing
            `fitnesses` wholesale, and updates the best-ever-seen
            individual (which may come from an earlier generation — a GA
            does not guarantee the *current* population still contains
            its best find, only that this class remembers it). */
        void evaluate_population()
        {
            fitnesses.clear();

            for (nat_t i = 0; i < population.size(); ++i)
            {
                real_t f = fitness_fn(population[i]);
                fitnesses.append(f);

                if (!has_best || f > best_fitness_)
                {
                    has_best = true;
                    best_fitness_ = f;
                    best_individual_ = population[i];
                }
            }
        }

    public:
        GeneticAlgorithm(nat_t pop_size, real_t cx_rate, real_t mut_rate,
                         nat_t elitism, rng_seed_t seed, InitFn init,
                         FitnessFn fitness, CrossoverFn crossover,
                         MutateFn mutate)
            : population_size(pop_size),
              crossover_rate(cx_rate),
              mutation_rate(mut_rate),
              elitism_count(elitism),
              rng(seed),
              init_fn(std::move(init)),
              fitness_fn(std::move(fitness)),
              crossover_fn(std::move(crossover)),
              mutate_fn(std::move(mutate)),
              generation_count(0),
              has_best(false),
              best_fitness_(0)
        {
            for (nat_t i = 0; i < population_size; ++i)
            {
                population.append(init_fn(rng));
            }

            evaluate_population();
        }

        /** Advances one generation: the fittest `elitism_count`
            individuals survive unchanged (so the best-ever fitness
            found is never lost to an unlucky round of selection); every
            other slot is filled by tournament-selecting two parents,
            recombining them with probability `crossover_rate` (a plain
            clone of one parent otherwise), then mutating the result
            with probability `mutation_rate`. */
        void step()
        {
            DynArray<bool> is_elite(population_size, false);
            DynArray<Individual> next_population;

            nat_t elite_target = elitism_count < population_size
                                     ? elitism_count
                                     : population_size;

            for (nat_t e = 0; e < elite_target; ++e)
            {
                nat_t best_idx = population_size;

                for (nat_t i = 0; i < population_size; ++i)
                {
                    if (is_elite[i])
                    {
                        continue;
                    }

                    if (best_idx == population_size ||
                        fitnesses[i] > fitnesses[best_idx])
                    {
                        best_idx = i;
                    }
                }

                is_elite[best_idx] = true;
                next_population.append(population[best_idx]);
            }

            while (next_population.size() < population_size)
            {
                nat_t p1 = tournament_select(fitnesses, 3, rng);
                nat_t p2 = tournament_select(fitnesses, 3, rng);

                Individual child = random(rng) < crossover_rate
                                       ? crossover_fn(population[p1],
                                                       population[p2], rng)
                                       : population[p1];

                if (random(rng) < mutation_rate)
                {
                    mutate_fn(child, rng);
                }

                next_population.append(std::move(child));
            }

            population = std::move(next_population);
            evaluate_population();
            ++generation_count;
        }

        void run(nat_t generations)
        {
            for (nat_t i = 0; i < generations; ++i)
            {
                step();
            }
        }

        nat_t generation() const
        {
            return generation_count;
        }

        const Individual& best_individual() const
        {
            return best_individual_;
        }

        real_t best_fitness() const
        {
            return best_fitness_;
        }

        const DynArray<Individual>& population_view() const
        {
            return population;
        }
    };

    /** Deduces `Individual` from the explicit template argument and the
        four callback types from the arguments actually passed —
        `GeneticAlgorithm<Individual, InitFn, FitnessFn, CrossoverFn,
        MutateFn>` is otherwise painful to spell out by hand (the same
        reason `std::make_tuple`/`std::make_pair` exist instead of naming
        `std::tuple<...>`/`std::pair<...>` directly). */
    template <typename Individual, class InitFn, class FitnessFn,
              class CrossoverFn, class MutateFn>
    GeneticAlgorithm<Individual, InitFn, FitnessFn, CrossoverFn, MutateFn>
    make_genetic_algorithm(nat_t population_size, real_t crossover_rate,
                           real_t mutation_rate, nat_t elitism_count,
                           rng_seed_t seed, InitFn init_fn,
                           FitnessFn fitness_fn, CrossoverFn crossover_fn,
                           MutateFn mutate_fn)
    {
        return GeneticAlgorithm<Individual, InitFn, FitnessFn, CrossoverFn,
                                MutateFn>(
            population_size, crossover_rate, mutation_rate, elitism_count,
            seed, std::move(init_fn), std::move(fitness_fn),
            std::move(crossover_fn), std::move(mutate_fn));
    }

} // end namespace Designar
