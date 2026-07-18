/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <particleswarm.hpp>

using namespace Designar;

int main()
{
    // Minimize the sphere function sum(x_i^2) over R^5 (optimum: the
    // origin, value 0) — the same benchmark as demo-beecolony.cpp, so
    // the two swarm-intelligence algorithms can be compared side by
    // side.
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
    pso.run(200);

    cout << "Best value after " << pso.iteration()
         << " iterations: " << pso.best_value() << endl;
    cout << "Best position: ";

    for (real_t v : pso.best_position())
    {
        cout << v << " ";
    }

    cout << endl;

    return 0;
}
