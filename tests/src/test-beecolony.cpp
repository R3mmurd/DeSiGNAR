/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <beecolony.hpp>

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

    auto abc = make_artificial_bee_colony(5, -10.0, 10.0, 30, 20, 20260718,
                                          objective);

    assert(abc.iteration() == 0);

    real_t initial_best = abc.best_value();

    abc.run(200);

    assert(abc.iteration() == 200);
    assert(abc.best_solution().size() == 5);
    // Greedy replacement means best-ever value never regresses.
    assert(abc.best_value() <= initial_best);
    // Loose enough not to be flaky, tight enough to catch a broken
    // employed/onlooker/scout phase.
    assert(abc.best_value() < 1e-2);

    return 0;
}
