/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <simulatedannealing.hpp>
#include <linearalgebra.hpp>

using namespace Designar;

int main()
{
    // Same sphere-function benchmark as demo-beecolony.cpp/
    // demo-particleswarm.cpp, this time solved by perturbing a single
    // state instead of maintaining a whole population/swarm.
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
    sa.run(3000);

    cout << "Best energy after " << sa.iteration()
         << " iterations: " << sa.best_energy() << endl;
    cout << "Final temperature: " << sa.current_temperature() << endl;
    cout << "Best state: ";

    for (real_t v : sa.best_state())
    {
        cout << v << " ";
    }

    cout << endl;

    return 0;
}
