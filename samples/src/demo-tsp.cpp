/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cmath>
#include <iostream>

using namespace std;

#include <tsp.hpp>

using namespace Designar;

int main()
{
    // Eight points around a square ring — the known-optimal tour is the
    // ring itself, cost exactly 8.
    real_t xs[] = {0, 1, 2, 2, 2, 1, 0, 0};
    real_t ys[] = {0, 0, 0, 1, 2, 2, 2, 1};
    nat_t n = 8;

    auto distance = [&](nat_t i, nat_t j)
    {
        real_t dx = xs[i] - xs[j];
        real_t dy = ys[i] - ys[j];
        return std::sqrt(dx * dx + dy * dy);
    };

    auto print_tour = [&](const char* name, const DynArray<nat_t>& tour)
    {
        cout << name << ": ";

        for (nat_t v : tour)
        {
            cout << v << " ";
        }

        cout << "(cost " << tsp_tour_cost(tour, distance) << ")" << endl;
    };

    print_tour("nearest_neighbor    ", tsp_nearest_neighbor(n, distance));
    print_tour("mst_approximation   ", tsp_mst_approximation(n, distance));
    cout << "known optimal (the ring itself): cost 8" << endl;

    return 0;
}
