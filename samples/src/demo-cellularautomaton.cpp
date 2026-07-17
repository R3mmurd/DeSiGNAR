/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <string>

using namespace std;

#include <cellularautomaton.hpp>

using namespace Designar;

namespace
{
    void print_2d(const Lattice<int_t>& lattice)
    {
        nat_t rows = lattice.extent(0);
        nat_t cols = lattice.extent(1);

        for (nat_t r = 0; r < rows; ++r)
        {
            string line;

            for (nat_t c = 0; c < cols; ++c)
            {
                line += lattice.get(Coordinates({r, c})) == 1 ? '#' : '.';
            }

            cout << line << endl;
        }
    }

    int_t game_of_life_rule(const int_t& current,
                            const DynArray<int_t>& neighbors)
    {
        nat_t live = 0;

        for (nat_t i = 0; i < neighbors.size(); ++i)
        {
            if (neighbors[i] == 1)
            {
                ++live;
            }
        }

        if (current == 1)
        {
            return (live == 2 || live == 3) ? 1 : 0;
        }

        return (live == 3) ? 1 : 0;
    }
} // end anonymous namespace

int main()
{
    // Conway's Game of Life, seeded with a glider on a 8x8 periodic
    // (toroidal) grid.
    Lattice<int_t> lattice(DynArray<nat_t>({8, 8}));
    lattice.get(Coordinates({0, 1})) = 1;
    lattice.get(Coordinates({1, 2})) = 1;
    lattice.get(Coordinates({2, 0})) = 1;
    lattice.get(Coordinates({2, 1})) = 1;
    lattice.get(Coordinates({2, 2})) = 1;

    PeriodicBoundary<int_t> boundary;
    MooreNeighborhood<int_t> neighborhood;

    CellularAutomaton<int_t> ca(lattice, boundary, neighborhood,
                                game_of_life_rule);

    for (nat_t gen = 0; gen < 4; ++gen)
    {
        cout << "Generation " << ca.get_generation() << ":\n";
        print_2d(ca.get_lattice());
        cout << endl;
        ca.step();
    }

    return 0;
}
