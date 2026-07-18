/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file simulatedannealing.hpp
    @brief SimulatedAnnealing: Kirkpatrick et al.'s single-state local-
    search metaheuristic. Unlike every other algorithm in this module
    (which maintain a whole population/colony/swarm of candidate
    solutions), this one tracks exactly one current solution and
    randomly proposes a neighbor at each step, always accepting an
    improvement but *also* sometimes accepting a worse neighbor — with
    probability `exp(-delta / temperature)`, so early on (temperature
    high) it wanders almost freely, escaping local optima a purely
    greedy search would get stuck in, and by the end (temperature near
    0, following a geometric cooling schedule) it only accepts genuine
    improvements, settling into whatever basin it has found. Generic
    over `State` — unlike ArtificialBeeColony/ParticleSwarmOptimization,
    simulated annealing has no fixed representation in the literature
    (it is applied to everything from continuous vectors to discrete
    permutations), matching how minimax.hpp is generic over its own
    `State`.
    @ingroup ArtificialIntelligence
*/

#pragma once

#include <cmath>
#include <utility>

#include <types.hpp>
#include <random.hpp>

namespace Designar
{
    /** `NeighborFn`: `State operator()(const State&, rng_t&)` — proposes
        a random neighbor of the given state.
        `EnergyFn`: `real_t operator()(const State&)` — the value to
        *minimize* ("energy"; lower is better, matching the physical
        annealing analogy this algorithm is named after). */
    template <typename State, class NeighborFn, class EnergyFn>
    class SimulatedAnnealing
    {
        State current_state;
        real_t current_energy;
        real_t temperature;
        real_t cooling_rate;
        rng_t rng;

        NeighborFn neighbor_fn;
        EnergyFn energy_fn;

        nat_t iteration_count;
        State best_state_;
        real_t best_energy_;

    public:
        /** `cooling_rate` should be in `(0, 1)` — temperature is
            multiplied by it after every step, so smaller values cool
            faster (fewer opportunities to escape a local optimum before
            the search settles) and values close to 1 cool slowly
            (more exploration, more iterations needed to converge). */
        SimulatedAnnealing(State initial_state, real_t initial_temperature,
                          real_t cooling, rng_seed_t seed,
                          NeighborFn neighbor, EnergyFn energy)
            : current_state(initial_state),
              temperature(initial_temperature),
              cooling_rate(cooling),
              rng(seed),
              neighbor_fn(std::move(neighbor)),
              energy_fn(std::move(energy)),
              iteration_count(0),
              best_state_(std::move(initial_state))
        {
            current_energy = energy_fn(current_state);
            best_energy_ = current_energy;
        }

        void step()
        {
            State candidate = neighbor_fn(current_state, rng);
            real_t candidate_energy = energy_fn(candidate);
            real_t delta = candidate_energy - current_energy;

            if (delta <= 0 || random(rng) < std::exp(-delta / temperature))
            {
                current_state = std::move(candidate);
                current_energy = candidate_energy;

                if (current_energy < best_energy_)
                {
                    best_energy_ = current_energy;
                    best_state_ = current_state;
                }
            }

            temperature *= cooling_rate;
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

        real_t current_temperature() const
        {
            return temperature;
        }

        const State& best_state() const
        {
            return best_state_;
        }

        real_t best_energy() const
        {
            return best_energy_;
        }
    };

    /** Deduces `State`/`NeighborFn`/`EnergyFn` from the arguments passed
        in — see GeneticAlgorithm's make_genetic_algorithm() for why this
        factory exists instead of naming `SimulatedAnnealing<...>`
        directly. */
    template <typename State, class NeighborFn, class EnergyFn>
    SimulatedAnnealing<State, NeighborFn, EnergyFn> make_simulated_annealing(
        State initial_state, real_t initial_temperature, real_t cooling_rate,
        rng_seed_t seed, NeighborFn neighbor_fn, EnergyFn energy_fn)
    {
        return SimulatedAnnealing<State, NeighborFn, EnergyFn>(
            std::move(initial_state), initial_temperature, cooling_rate,
            seed, std::move(neighbor_fn), std::move(energy_fn));
    }

} // end namespace Designar
