/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cmath>

using namespace std;

#include <antcolony.hpp>
#include <math.hpp>

using namespace Designar;

int main()
{
    // Cities on a circle: the optimal tour visits them in angular order,
    // so a human can eyeball whether the colony actually found it.
    constexpr nat_t N = 15;
    DynArray<real_t> xs;
    DynArray<real_t> ys;

    for (nat_t i = 0; i < N; ++i)
    {
        real_t theta = 2.0 * PI * i / N;
        xs.append(std::cos(theta));
        ys.append(std::sin(theta));
    }

    auto dist = [&](nat_t i, nat_t j)
    {
        real_t dx = xs[i] - xs[j];
        real_t dy = ys[i] - ys[j];
        return std::sqrt(dx * dx + dy * dy);
    };

    auto heuristic_fn = [&](nat_t i, nat_t j) { return 1.0 / dist(i, j); };

    auto cost_fn = [&](const DynArray<nat_t>& tour)
    {
        real_t total = 0;

        for (nat_t k = 0; k + 1 < tour.size(); ++k)
        {
            total += dist(tour[k], tour[k + 1]);
        }

        total += dist(tour[tour.size() - 1], tour[0]);
        return total;
    };

    auto aco = make_ant_colony_optimization(N, 20, 1.0, 3.0, 0.4, 1.0, 0.1,
                                            20260718, heuristic_fn, cost_fn);
    aco.run(60);

    cout << "Best tour cost after " << aco.iteration()
         << " iterations: " << aco.best_cost() << endl;
    cout << "Best tour: ";

    for (nat_t city : aco.best_tour())
    {
        cout << city << " ";
    }

    cout << endl;

    return 0;
}
