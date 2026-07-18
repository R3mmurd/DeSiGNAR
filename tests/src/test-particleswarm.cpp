/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <particleswarm.hpp>

using namespace Designar;

int main()
{
    // Sphere function sum(x_i^2) over R^5, minimum 0 at the origin.
    auto objective = [](const Vector<real_t>& v)
    {
        real_t s = 0;

        for (nat_t i = 0; i < v.size(); ++i)
        {
            s += v[i] * v[i];
        }

        return s;
    };

    auto pso = make_particle_swarm_optimization(5, -10.0, 10.0, 30, 0.7, 1.5,
                                                1.5, 20260718, objective);

    assert(pso.iteration() == 0);

    real_t initial_best = pso.best_value();

    pso.run(200);

    assert(pso.iteration() == 200);
    assert(pso.best_position().size() == 5);
    // The global best is only ever replaced by something strictly
    // better, so it never regresses.
    assert(pso.best_value() <= initial_best);
    // Loose enough not to be flaky, tight enough to catch a broken
    // velocity/position update rule.
    assert(pso.best_value() < 1e-2);

    return 0;
}
