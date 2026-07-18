/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file particleswarm.hpp
    @brief ParticleSwarmOptimization: Kennedy & Eberhart's PSO
    metaheuristic for continuous function minimization over
    `[lower, upper]^dimension` — a swarm of particles, each with a
    position and a velocity, converging on good solutions by blending
    three pulls: its own current momentum (`inertia`), the pull back
    toward the best position it has personally ever visited
    (`cognitive`), and the pull toward the best position *any* particle
    in the swarm has ever visited (`social`). Structurally the closest
    of this library's metaheuristics to ArtificialBeeColony (same
    problem shape, same `Vector<real_t>` representation) but with no
    abandonment/scout mechanism — a particle's own inertia is what keeps
    it exploring instead.
    @ingroup ArtificialIntelligence
*/

#pragma once

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
    class ParticleSwarmOptimization
    {
        nat_t dimension;
        real_t lower_bound;
        real_t upper_bound;
        nat_t num_particles;
        real_t inertia;
        real_t cognitive;
        real_t social;
        rng_t rng;

        ObjectiveFn objective_fn;

        DynArray<Vector<real_t>> positions;
        DynArray<Vector<real_t>> velocities;
        DynArray<Vector<real_t>> personal_best_positions;
        DynArray<real_t> personal_best_values;
        nat_t iteration_count;
        Vector<real_t> global_best_position_;
        real_t global_best_value_;

        Vector<real_t> random_position()
        {
            Vector<real_t> position(dimension, real_t());

            for (nat_t d = 0; d < dimension; ++d)
            {
                position[d] =
                    random_uniform<real_t>(rng, lower_bound, upper_bound);
            }

            return position;
        }

    public:
        ParticleSwarmOptimization(nat_t dim, real_t lo, real_t hi,
                                  nat_t particles, real_t w, real_t c1,
                                  real_t c2, rng_seed_t seed,
                                  ObjectiveFn objective)
            : dimension(dim),
              lower_bound(lo),
              upper_bound(hi),
              num_particles(particles),
              inertia(w),
              cognitive(c1),
              social(c2),
              rng(seed),
              objective_fn(std::move(objective)),
              iteration_count(0),
              global_best_value_(0)
        {
            for (nat_t i = 0; i < num_particles; ++i)
            {
                Vector<real_t> position = random_position();
                real_t value = objective_fn(position);

                velocities.append(Vector<real_t>(dimension, real_t()));
                personal_best_positions.append(position);
                personal_best_values.append(value);

                if (i == 0 || value < global_best_value_)
                {
                    global_best_value_ = value;
                    global_best_position_ = position;
                }

                positions.append(std::move(position));
            }
        }

        /** Updates every particle's velocity (inertia + cognitive pull
            toward its own best + social pull toward the swarm's best,
            each randomly weighted per dimension so the swarm doesn't
            collapse onto a single deterministic trajectory), moves it
            by that velocity (clamped back into bounds), and refreshes
            personal/global bests if the new position improves on
            them. */
        void step()
        {
            for (nat_t i = 0; i < num_particles; ++i)
            {
                for (nat_t d = 0; d < dimension; ++d)
                {
                    real_t r1 = random(rng);
                    real_t r2 = random(rng);

                    velocities[i][d] =
                        inertia * velocities[i][d] +
                        cognitive * r1 *
                            (personal_best_positions[i][d] -
                             positions[i][d]) +
                        social * r2 *
                            (global_best_position_[d] - positions[i][d]);

                    positions[i][d] =
                        clamp_to(positions[i][d] + velocities[i][d],
                                lower_bound, upper_bound);
                }

                real_t value = objective_fn(positions[i]);

                if (value < personal_best_values[i])
                {
                    personal_best_values[i] = value;
                    personal_best_positions[i] = positions[i];

                    if (value < global_best_value_)
                    {
                        global_best_value_ = value;
                        global_best_position_ = positions[i];
                    }
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

        const Vector<real_t>& best_position() const
        {
            return global_best_position_;
        }

        real_t best_value() const
        {
            return global_best_value_;
        }
    };

    /** Deduces `ObjectiveFn` from the callback passed in — see
        GeneticAlgorithm's make_genetic_algorithm() for why this factory
        exists instead of naming `ParticleSwarmOptimization<...>`
        directly. */
    template <class ObjectiveFn>
    ParticleSwarmOptimization<ObjectiveFn> make_particle_swarm_optimization(
        nat_t dimension, real_t lower_bound, real_t upper_bound,
        nat_t num_particles, real_t inertia, real_t cognitive, real_t social,
        rng_seed_t seed, ObjectiveFn objective_fn)
    {
        return ParticleSwarmOptimization<ObjectiveFn>(
            dimension, lower_bound, upper_bound, num_particles, inertia,
            cognitive, social, seed, std::move(objective_fn));
    }

} // end namespace Designar
