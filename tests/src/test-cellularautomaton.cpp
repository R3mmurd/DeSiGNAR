/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <cellularautomaton.hpp>

using namespace std;
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
} // end anonymous namespace

int main()
{
  // Conway's Game of Life: the "block" still life never changes.
  {
    Lattice<int_t> lattice(DynArray<nat_t>({6, 6}));
    lattice.get(Coordinates({1, 1})) = 1;
    lattice.get(Coordinates({1, 2})) = 1;
    lattice.get(Coordinates({2, 1})) = 1;
    lattice.get(Coordinates({2, 2})) = 1;

    PeriodicBoundary<int_t> boundary;
    MooreNeighborhood<int_t> neighborhood;

    CellularAutomaton<int_t> ca(lattice, boundary, neighborhood, game_of_life_rule);
    ca.step();

    assert(ca.get_lattice().get(Coordinates({1, 1})) == 1);
    assert(ca.get_lattice().get(Coordinates({1, 2})) == 1);
    assert(ca.get_lattice().get(Coordinates({2, 1})) == 1);
    assert(ca.get_lattice().get(Coordinates({2, 2})) == 1);
    assert(ca.get_lattice().get(Coordinates({0, 0})) == 0);

    cout << "CellularAutomaton: Game of Life block (still life) Everything ok!\n";
  }

  // The "blinker" oscillator: a horizontal 3-cell line becomes a
  // vertical one after one generation, and back after another (period 2).
  {
    Lattice<int_t> lattice(DynArray<nat_t>({5, 5}));
    lattice.get(Coordinates({2, 1})) = 1;
    lattice.get(Coordinates({2, 2})) = 1;
    lattice.get(Coordinates({2, 3})) = 1;

    PeriodicBoundary<int_t> boundary;
    MooreNeighborhood<int_t> neighborhood;

    CellularAutomaton<int_t> ca(lattice, boundary, neighborhood, game_of_life_rule);
    ca.step();

    assert(ca.get_lattice().get(Coordinates({1, 2})) == 1);
    assert(ca.get_lattice().get(Coordinates({2, 2})) == 1);
    assert(ca.get_lattice().get(Coordinates({3, 2})) == 1);
    assert(ca.get_lattice().get(Coordinates({2, 1})) == 0);
    assert(ca.get_lattice().get(Coordinates({2, 3})) == 0);

    ca.step();

    assert(ca.get_lattice().get(Coordinates({2, 1})) == 1);
    assert(ca.get_lattice().get(Coordinates({2, 2})) == 1);
    assert(ca.get_lattice().get(Coordinates({2, 3})) == 1);
    assert(ca.get_lattice().get(Coordinates({1, 2})) == 0);
    assert(ca.get_lattice().get(Coordinates({3, 2})) == 0);

    assert(ca.get_generation() == 2);

    cout << "CellularAutomaton: Game of Life blinker (period-2 oscillator) Everything ok!\n";
  }

  // 1D lattice: a pure right-shift rule (next cell = left neighbor's
  // current value) on a periodic boundary must, after exactly
  // `size` steps, return to its starting configuration — this
  // exercises the N-dimensional machinery generically at N=1, and is
  // exactly verifiable without needing any particular well-known CA
  // rule's output memorized.
  {
    nat_t size = 8;
    Lattice<int_t> lattice(DynArray<nat_t>({size}));
    lattice.get(Coordinates({0})) = 1;

    PeriodicBoundary<int_t> boundary;
    VonNeumannNeighborhood<int_t> neighborhood; // [left, right] for 1D

    auto shift_rule = [](const int_t&, const DynArray<int_t>& neighbors)
    { return neighbors[0]; }; // left neighbor

    CellularAutomaton<int_t> ca(lattice, boundary, neighborhood, shift_rule);

    ca.step();
    assert(ca.get_lattice().get(Coordinates({1})) == 1);
    assert(ca.get_lattice().get(Coordinates({0})) == 0);

    ca.step(size - 1);

    for (nat_t i = 0; i < size; ++i)
    {
      assert(ca.get_lattice().get(Coordinates({i})) == (i == 0 ? 1 : 0));
    }

    assert(ca.get_generation() == size);

    cout << "CellularAutomaton: 1D shift rule, full period Everything ok!\n";
  }

  // Boundary conditions: Reflective and Fixed, checked directly
  // (rather than through a whole CellularAutomaton) against a small
  // 1D lattice [10, 20, 30].
  {
    Lattice<int_t> lattice(DynArray<nat_t>({3}));
    lattice.get(Coordinates({0})) = 10;
    lattice.get(Coordinates({1})) = 20;
    lattice.get(Coordinates({2})) = 30;

    ReflectiveBoundary<int_t> reflective;
    // One step left of position 0 reflects back to position 1 (10's
    // *other* neighbor), not to position 2 the way Periodic would.
    assert(reflective.get(lattice, Coordinates({0}), Offsets({-1})) == 20);
    assert(reflective.get(lattice, Coordinates({2}), Offsets({1})) == 20);

    FixedBoundary<int_t> fixed(int_t(-1));
    assert(fixed.get(lattice, Coordinates({0}), Offsets({-1})) == -1);
    assert(fixed.get(lattice, Coordinates({2}), Offsets({1})) == -1);
    assert(fixed.get(lattice, Coordinates({1}), Offsets({0})) == 20);

    PeriodicBoundary<int_t> periodic;
    assert(periodic.get(lattice, Coordinates({0}), Offsets({-1})) == 30);
    assert(periodic.get(lattice, Coordinates({2}), Offsets({1})) == 10);

    cout << "CellularAutomaton: boundary conditions Everything ok!\n";
  }

  // Lattice itself: dimensionality/extent validation and the
  // linear-index <-> coordinates round trip.
  {
    Lattice<int_t> lattice(DynArray<nat_t>({4, 3, 2}));
    assert(lattice.num_dimensions() == 3);
    assert(lattice.size() == 24);

    for (nat_t idx = 0; idx < lattice.size(); ++idx)
    {
      Coordinates coords = lattice.to_coordinates(idx);
      assert(lattice.to_linear_index(coords) == idx);
    }

    bool threw = false;

    try
    {
      lattice.get(Coordinates({0, 0})); // wrong number of coordinates
    }
    catch (const domain_error&)
    {
      threw = true;
    }

    assert(threw);

    threw = false;

    try
    {
      lattice.get(Coordinates({10, 0, 0})); // out of range
    }
    catch (const out_of_range&)
    {
      threw = true;
    }

    assert(threw);

    cout << "Lattice: dimensionality/bounds checking Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
