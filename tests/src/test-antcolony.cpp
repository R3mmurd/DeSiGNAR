/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <antcolony.hpp>
#include <math.hpp>

using namespace Designar;

int main()
{
    // Cities on a circle: the optimal tour visits them in angular order
    // (a known-by-construction optimum, no external TSP solver needed),
    // so the test can assert the found cost is within a small factor of
    // it.
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

    real_t optimal = N * dist(0, 1);

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

    assert(aco.iteration() == 0);

    aco.run(60);

    assert(aco.iteration() == 60);
    assert(aco.best_tour().size() == N);

    // Every city visited exactly once (a valid permutation).
    DynArray<bool> seen(N, false);

    for (nat_t city : aco.best_tour())
    {
        assert(!seen[city]);
        seen[city] = true;
    }

    // Within 15% of optimal — loose enough not to be flaky, tight enough
    // to catch a broken construction/pheromone-update rule.
    assert(aco.best_cost() < optimal * 1.15);

    return 0;
}
