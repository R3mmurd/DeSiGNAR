/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <beecolony.hpp>

using namespace Designar;

int main()
{
    // Minimize the sphere function sum(x_i^2) over R^5 (optimum: the
    // origin, value 0).
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
    abc.run(200);

    cout << "Best value after " << abc.iteration()
         << " iterations: " << abc.best_value() << endl;
    cout << "Best solution: ";

    for (real_t v : abc.best_solution())
    {
        cout << v << " ";
    }

    cout << endl;

    return 0;
}
