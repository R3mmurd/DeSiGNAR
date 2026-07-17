/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <string>

using namespace std;

#include <sparsecellularautomaton.hpp>

using namespace Designar;

namespace
{
  int_t game_of_life_rule(const int_t& current, const DynArray<int_t>& neighbors)
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

  void print_active_cells(const SparseLattice<int_t>& lattice)
  {
    int_t min_r = 0;
    int_t max_r = 0;
    int_t min_c = 0;
    int_t max_c = 0;
    bool first = true;

    lattice.for_each_active(
        [&](const SparseLattice<int_t>::Entry& e)
        {
          int_t r = e.coords[0];
          int_t c = e.coords[1];

          if (first)
          {
            min_r = max_r = r;
            min_c = max_c = c;
            first = false;
          }
          else
          {
            min_r = min(min_r, r);
            max_r = max(max_r, r);
            min_c = min(min_c, c);
            max_c = max(max_c, c);
          }
        });

    for (int_t r = min_r; r <= max_r; ++r)
    {
      string line;

      for (int_t c = min_c; c <= max_c; ++c)
      {
        line += lattice.get(SparseCoordinates({r, c})) == 1 ? '#' : '.';
      }

      cout << line << endl;
    }
  }
} // end anonymous namespace

int main()
{
  // A glider on a sparse, unbounded 2D lattice: no grid size to pick
  // up front, and it is free to drift arbitrarily far in any
  // direction (including negative coordinates) without ever running
  // off an edge, since none exists.
  SparseLattice<int_t> lattice(2);
  lattice.set(SparseCoordinates({0, 1}), 1);
  lattice.set(SparseCoordinates({1, 2}), 1);
  lattice.set(SparseCoordinates({2, 0}), 1);
  lattice.set(SparseCoordinates({2, 1}), 1);
  lattice.set(SparseCoordinates({2, 2}), 1);

  SparseMooreNeighborhood<int_t> neighborhood;
  SparseCellularAutomaton<int_t> ca(lattice, neighborhood, game_of_life_rule);

  for (nat_t gen = 0; gen < 4; ++gen)
  {
    cout << "Generation " << ca.get_generation()
         << " (" << ca.get_lattice().num_active_cells() << " active cells):\n";
    print_active_cells(ca.get_lattice());
    cout << endl;
    ca.step();
  }

  return 0;
}
