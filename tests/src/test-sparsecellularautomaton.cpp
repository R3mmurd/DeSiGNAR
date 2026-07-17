/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <sparsecellularautomaton.hpp>

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

  bool has_cell(const SparseLattice<int_t>& lattice, int_t r, int_t c)
  {
    return lattice.get(SparseCoordinates({r, c})) == 1;
  }
} // end anonymous namespace

int main()
{
  // The "block" still life: entirely unaffected by an unbounded
  // background, exactly as it should be with no fixed grid at all.
  {
    SparseLattice<int_t> lattice(2);
    lattice.set(SparseCoordinates({1, 1}), 1);
    lattice.set(SparseCoordinates({1, 2}), 1);
    lattice.set(SparseCoordinates({2, 1}), 1);
    lattice.set(SparseCoordinates({2, 2}), 1);

    SparseMooreNeighborhood<int_t> neighborhood;
    SparseCellularAutomaton<int_t> ca(lattice, neighborhood, game_of_life_rule);
    ca.step();

    assert(has_cell(ca.get_lattice(), 1, 1));
    assert(has_cell(ca.get_lattice(), 1, 2));
    assert(has_cell(ca.get_lattice(), 2, 1));
    assert(has_cell(ca.get_lattice(), 2, 2));
    assert(ca.get_lattice().num_active_cells() == 4);

    cout << "SparseCellularAutomaton: Game of Life block (still life) Everything ok!\n";
  }

  // The blinker oscillator, using coordinates that would be entirely
  // out of range for any small fixed-size dense Lattice, to make the
  // "unbounded" part of this actually matter — including negative
  // coordinates, which a dense Lattice cannot represent at all.
  {
    SparseLattice<int_t> lattice(2);
    lattice.set(SparseCoordinates({-1000, -1000}), 1);
    lattice.set(SparseCoordinates({-1000, -999}), 1);
    lattice.set(SparseCoordinates({-1000, -998}), 1);

    SparseMooreNeighborhood<int_t> neighborhood;
    SparseCellularAutomaton<int_t> ca(lattice, neighborhood, game_of_life_rule);
    ca.step();

    assert(has_cell(ca.get_lattice(), -1001, -999));
    assert(has_cell(ca.get_lattice(), -1000, -999));
    assert(has_cell(ca.get_lattice(), -999, -999));
    assert(!has_cell(ca.get_lattice(), -1000, -1000));
    assert(!has_cell(ca.get_lattice(), -1000, -998));
    assert(ca.get_lattice().num_active_cells() == 3);

    ca.step();

    assert(has_cell(ca.get_lattice(), -1000, -1000));
    assert(has_cell(ca.get_lattice(), -1000, -999));
    assert(has_cell(ca.get_lattice(), -1000, -998));
    assert(ca.get_generation() == 2);

    cout << "SparseCellularAutomaton: Game of Life blinker (period-2, negative coords) Everything ok!\n";
  }

  // A single live cell (no neighbors) must die out and prune to a
  // completely empty (0 active cells) lattice within one generation
  // — verifying the "prune back to background" behavior that keeps
  // the frontier from growing without bound.
  {
    SparseLattice<int_t> lattice(2);
    lattice.set(SparseCoordinates({0, 0}), 1);

    SparseMooreNeighborhood<int_t> neighborhood;
    SparseCellularAutomaton<int_t> ca(lattice, neighborhood, game_of_life_rule);
    ca.step();

    assert(ca.get_lattice().num_active_cells() == 0);

    cout << "SparseCellularAutomaton: lone cell dies and prunes to empty Everything ok!\n";
  }

  // SparseVonNeumannNeighborhood and a 1D shift rule, mirroring the
  // dense test's approach for exact verifiability but on an unbounded
  // 1D line rather than a fixed-size periodic ring, so there is no
  // wraparound at all: a single live cell simply advances by one
  // position (with no left neighbor to inherit from) every step,
  // instead of ever cycling back.
  {
    SparseLattice<int_t> lattice(1);
    lattice.set(SparseCoordinates({0}), 1);

    SparseVonNeumannNeighborhood<int_t> neighborhood; // [left, right] for 1D

    auto shift_rule = [](const int_t&, const DynArray<int_t>& neighbors)
    { return neighbors[0]; }; // left neighbor

    SparseCellularAutomaton<int_t> ca(lattice, neighborhood, shift_rule);

    for (int_t step = 1; step <= 5; ++step)
    {
      ca.step();
      assert(ca.get_lattice().get(SparseCoordinates({step})) == 1);
      assert(ca.get_lattice().num_active_cells() == 1);
    }

    cout << "SparseCellularAutomaton: 1D unbounded shift rule Everything ok!\n";
  }

  // SparseLattice basics: dimensionality validation, background
  // value, and that get() on a never-set coordinate returns the
  // background without creating an entry.
  {
    SparseLattice<int_t> lattice(3, int_t(-1));
    assert(lattice.num_dimensions() == 3);
    assert(lattice.get_background() == -1);
    assert(lattice.get(SparseCoordinates({5, -5, 100})) == -1);
    assert(lattice.num_active_cells() == 0);

    bool threw = false;

    try
    {
      lattice.get(SparseCoordinates({0, 0})); // wrong number of coordinates
    }
    catch (const domain_error&)
    {
      threw = true;
    }

    assert(threw);

    cout << "SparseLattice: dimensionality/background checking Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
