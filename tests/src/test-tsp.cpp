/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <iostream>
#include <tsp.hpp>

using namespace std;
using namespace Designar;

namespace
{
    void assert_valid_tour(const DynArray<nat_t>& tour, nat_t n)
    {
        assert(tour.size() == n);

        DynArray<bool> seen(n, false);

        for (nat_t v : tour)
        {
            assert(v < n);
            assert(!seen[v]);
            seen[v] = true;
        }
    }
} // end anonymous namespace

int main()
{
    // Eight points evenly spaced around a square ring, each consecutive
    // pair (in ring order) exactly distance 1 apart — the known-optimal
    // tour is the ring itself, cost exactly 8.
    real_t xs[] = {0, 1, 2, 2, 2, 1, 0, 0};
    real_t ys[] = {0, 0, 0, 1, 2, 2, 2, 1};
    nat_t n = 8;

    auto distance = [&](nat_t i, nat_t j)
    {
        real_t dx = xs[i] - xs[j];
        real_t dy = ys[i] - ys[j];
        return std::sqrt(dx * dx + dy * dy);
    };

    auto nn_tour = tsp_nearest_neighbor(n, distance);
    assert_valid_tour(nn_tour, n);

    auto mst_tour = tsp_mst_approximation(n, distance);
    assert_valid_tour(mst_tour, n);

    real_t known_optimal = 8.0;
    real_t nn_cost = tsp_tour_cost(nn_tour, distance);
    real_t mst_cost = tsp_tour_cost(mst_tour, distance);

    // Both heuristics must find *a* valid tour at least as good as the
    // proven 2-approximation bound (nearest-neighbor has no such
    // guarantee in general, but does on this especially symmetric
    // instance); the MST-based approximation's guarantee is the one
    // that must always hold.
    assert(mst_cost <= 2.0 * known_optimal + 1e-6);
    assert(nn_cost <= 2.0 * known_optimal + 1e-6);

    // On this perfectly symmetric ring, both heuristics actually find
    // the true optimal tour.
    assert(std::abs(mst_cost - known_optimal) < 1e-6);
    assert(std::abs(nn_cost - known_optimal) < 1e-6);

    // Edge cases.
    auto single = tsp_mst_approximation(1, distance);
    assert(single.size() == 1);

    auto single_nn = tsp_nearest_neighbor(1, distance);
    assert(single_nn.size() == 1);

    bool threw = false;

    try
    {
        tsp_mst_approximation(0, distance);
    }
    catch (const std::domain_error&)
    {
        threw = true;
    }

    assert(threw);

    threw = false;

    try
    {
        tsp_nearest_neighbor(0, distance);
    }
    catch (const std::domain_error&)
    {
        threw = true;
    }

    assert(threw);

    cout << "Everything ok!\n";

    return 0;
}
