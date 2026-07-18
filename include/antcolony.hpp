/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file antcolony.hpp
    @brief AntColonyOptimization: the classic Ant System metaheuristic
    (Dorigo et al.) for combinatorial problems over a fixed set of
    `num_components` components (the canonical framing is TSP — a
    "component" is a city, a "tour" is a Hamiltonian cycle over all of
    them) — repeatedly builds one tour per (virtual) ant by probabilistic
    construction biased by a shared pheromone matrix (what the colony has
    "learned" so far) and a problem-specific heuristic (what looks good
    right now, independent of history, e.g. `1 / distance`), then
    evaporates old pheromone and deposits fresh pheromone proportional to
    how good each ant's tour turned out to be.
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
    /** `HeuristicFn`: `real_t operator()(nat_t i, nat_t j)` — the
        problem-specific desirability of going from component `i` to `j`
        (larger is more desirable; the classic TSP choice is `1 /
        distance(i, j)`), independent of the colony's accumulated
        pheromone.
        `CostFn`: `real_t operator()(const DynArray<nat_t>& tour) ->
        real_t` — the total cost of a completed tour (smaller is
        better); this class assumes tours are closed cycles (its
        pheromone deposit includes the edge from the last component back
        to the first) and that the underlying cost is symmetric
        (component `i` to `j` costs the same as `j` to `i`) — both true
        for the canonical Euclidean-TSP framing this algorithm was
        designed for. */
    template <class HeuristicFn, class CostFn>
    class AntColonyOptimization
    {
        nat_t num_components;
        nat_t num_ants;
        real_t alpha;
        real_t beta;
        real_t evaporation_rate;
        real_t deposit_factor;
        rng_t rng;

        HeuristicFn heuristic_fn;
        CostFn cost_fn;

        Matrix<real_t> pheromone;
        nat_t iteration_count;
        bool has_best;
        DynArray<nat_t> best_tour_;
        real_t best_cost_;

        /** Builds one ant's tour: starts from a uniformly random
            component, then repeatedly picks the next unvisited
            component via roulette-wheel selection weighted by
            `pheromone(current, j)^alpha * heuristic(current, j)^beta` —
            the two knobs that trade off "trust what the colony has
            learned" (`alpha`) against "trust what looks good right now"
            (`beta`). */
        DynArray<nat_t> build_tour()
        {
            DynArray<bool> visited(num_components, false);
            DynArray<nat_t> tour;

            nat_t current = random_uniform<nat_t>(rng, num_components);
            tour.append(current);
            visited[current] = true;

            for (nat_t step_i = 1; step_i < num_components; ++step_i)
            {
                DynArray<nat_t> candidates;
                DynArray<real_t> weights;

                for (nat_t j = 0; j < num_components; ++j)
                {
                    if (visited[j])
                    {
                        continue;
                    }

                    real_t tau = std::pow(pheromone(current, j), alpha);
                    real_t eta = std::pow(heuristic_fn(current, j), beta);
                    candidates.append(j);
                    weights.append(tau * eta);
                }

                nat_t choice = roulette_wheel_select(weights, rng);
                current = candidates[choice];
                tour.append(current);
                visited[current] = true;
            }

            return tour;
        }

        void deposit_pheromone(const DynArray<nat_t>& tour, real_t amount,
                               Matrix<real_t>& delta) const
        {
            for (nat_t k = 0; k + 1 < tour.size(); ++k)
            {
                delta(tour[k], tour[k + 1]) += amount;
                delta(tour[k + 1], tour[k]) += amount;
            }

            nat_t last = tour[tour.size() - 1];
            nat_t first = tour[0];
            delta(last, first) += amount;
            delta(first, last) += amount;
        }

    public:
        AntColonyOptimization(nat_t components, nat_t ants, real_t a,
                              real_t b, real_t evaporation, real_t deposit,
                              real_t initial_pheromone, rng_seed_t seed,
                              HeuristicFn heuristic, CostFn cost)
            : num_components(components),
              num_ants(ants),
              alpha(a),
              beta(b),
              evaporation_rate(evaporation),
              deposit_factor(deposit),
              rng(seed),
              heuristic_fn(std::move(heuristic)),
              cost_fn(std::move(cost)),
              pheromone(components, components, initial_pheromone),
              iteration_count(0),
              has_best(false),
              best_cost_(0)
        {
            // empty
        }

        /** Every ant builds a full tour; the shared pheromone matrix
            then evaporates (`*= (1 - evaporation_rate)`, so old
            information is gradually forgotten) and every ant deposits
            `deposit_factor / cost` along its own tour's edges (better
            tours reinforce their edges more strongly) — both updates
            applied via a separate `delta` matrix so within-iteration
            deposits never influence which edges *this* iteration's own
            ants preferred. */
        void step()
        {
            Matrix<real_t> delta(num_components, num_components, 0.0);

            for (nat_t ant = 0; ant < num_ants; ++ant)
            {
                DynArray<nat_t> tour = build_tour();
                real_t cost = cost_fn(tour);

                if (!has_best || cost < best_cost_)
                {
                    has_best = true;
                    best_cost_ = cost;
                    best_tour_ = tour;
                }

                deposit_pheromone(tour, deposit_factor / cost, delta);
            }

            for (nat_t i = 0; i < num_components; ++i)
            {
                for (nat_t j = 0; j < num_components; ++j)
                {
                    pheromone(i, j) =
                        pheromone(i, j) * (1 - evaporation_rate) + delta(i, j);
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

        const DynArray<nat_t>& best_tour() const
        {
            return best_tour_;
        }

        real_t best_cost() const
        {
            return best_cost_;
        }
    };

    /** Deduces `HeuristicFn`/`CostFn` from the callbacks passed in — see
        GeneticAlgorithm's make_genetic_algorithm() for why this factory
        exists instead of naming `AntColonyOptimization<...>` directly. */
    template <class HeuristicFn, class CostFn>
    AntColonyOptimization<HeuristicFn, CostFn> make_ant_colony_optimization(
        nat_t num_components, nat_t num_ants, real_t alpha, real_t beta,
        real_t evaporation_rate, real_t deposit_factor,
        real_t initial_pheromone, rng_seed_t seed, HeuristicFn heuristic_fn,
        CostFn cost_fn)
    {
        return AntColonyOptimization<HeuristicFn, CostFn>(
            num_components, num_ants, alpha, beta, evaporation_rate,
            deposit_factor, initial_pheromone, seed,
            std::move(heuristic_fn), std::move(cost_fn));
    }

} // end namespace Designar
