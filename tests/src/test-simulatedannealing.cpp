/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <simulatedannealing.hpp>
#include <linearalgebra.hpp>

using namespace Designar;

int main()
{
    // Sphere function sum(x_i^2) over R^5, minimum 0 at the origin.
    auto neighbor_fn = [](const Vector<real_t>& v, rng_t& rng)
    {
        Vector<real_t> next = v;
        nat_t i = random_uniform<nat_t>(rng, v.size());
        next[i] += random_uniform<real_t>(rng, -0.5, 0.5);
        return next;
    };

    auto energy_fn = [](const Vector<real_t>& v)
    {
        real_t s = 0;

        for (nat_t i = 0; i < v.size(); ++i)
        {
            s += v[i] * v[i];
        }

        return s;
    };

    Vector<real_t> start(5, 5.0);
    auto sa = make_simulated_annealing(start, 100.0, 0.995, 20260718,
                                       neighbor_fn, energy_fn);

    assert(sa.iteration() == 0);
    assert(std::abs(sa.current_temperature() - 100.0) < 1e-9);

    real_t initial_best = sa.best_energy();

    sa.run(3000);

    assert(sa.iteration() == 3000);
    assert(sa.current_temperature() < 100.0); // cooled
    assert(sa.best_state().size() == 5);
    // best-ever energy is only ever replaced by something strictly
    // lower, so it never regresses.
    assert(sa.best_energy() <= initial_best);
    // Loose enough not to be flaky, tight enough to catch a broken
    // acceptance rule or cooling schedule.
    assert(sa.best_energy() < 1e-1);

    return 0;
}
