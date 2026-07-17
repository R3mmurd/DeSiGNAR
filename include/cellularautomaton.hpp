/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file cellularautomaton.hpp
    @brief N-dimensional dense cellular automata: a Lattice<CellT> (a
    flat, runtime-dimensional grid — the dimensionality is a
    constructor argument, not a template parameter, unlike
    MultiDimArray's compile-time N), pluggable BoundaryCondition
    (periodic/reflective/fixed) and Neighborhood (von Neumann/Moore)
    strategies that are themselves dimension-agnostic, and
    CellularAutomaton tying a Lattice + BoundaryCondition +
    Neighborhood + transition rule together with a step() that
    advances one generation. Ported and modernized from
    github.com/R3mmurd/CellularAutomata's LibCA (itself already built
    on this library) — same design, this library's own containers
    instead of hand-rolled ones, and one dimension-agnostic Moore
    neighborhood instead of a fixed 1D/2D/3D split.
    @ingroup CellularAutomata
*/

#pragma once

#include <array.hpp>

#include <stdexcept>
#include <functional>

namespace Designar
{
  /** A per-dimension index into a Lattice. Aliased separately from
      plain `DynArray<nat_t>` purely for readability at call sites —
      "this is a position", not "this is some array of counts". */
  using Coordinates = DynArray<nat_t>;

  /** A per-dimension offset — always -1, 0, or +1 in this file
      (BoundaryCondition/Neighborhood below never build any other
      offset), describing which neighbor, relative to a cell, is being
      asked for. Signed because "one step in the negative direction"
      has to be representable even though Coordinates itself is
      unsigned. */
  using Offsets = DynArray<int_t>;

  /** A flat, runtime-dimensional grid of `CellT` cells — 1D, 2D, 3D,
      or any higher dimension, all through the exact same class,
      determined by how many extents the constructor is given. Linear
      storage + row-major strides (computed once, at construction) are
      what make get()/set() O(1) regardless of dimensionality, the same
      technique this library's own (compile-time-N) MultiDimArray
      uses — this class exists as a separate, runtime-N counterpart
      specifically because a cellular automaton's dimensionality is a
      property of the *model being simulated*, not something known at
      compile time the way MultiDimArray needs it. */
  template <typename CellT>
  class Lattice
  {
  public:
    using CellType = CellT;

  private:
    nat_t num_dims;
    nat_t total_size;
    DynArray<nat_t> extents;
    DynArray<nat_t> strides;
    DynArray<CellT> cells;

    void validate_coordinates(const Coordinates& coords) const
    {
      if (coords.size() != num_dims)
      {
        throw std::domain_error(
            "Lattice: coordinate count does not match dimensionality");
      }

      for (nat_t i = 0; i < num_dims; ++i)
      {
        if (coords[i] >= extents[i])
        {
          throw std::out_of_range("Lattice: coordinate out of range");
        }
      }
    }

  public:
    explicit Lattice(const DynArray<nat_t>& _extents)
        : num_dims(_extents.size()), total_size(1), extents(_extents),
          strides(_extents.size(), nat_t(0))
    {
      if (num_dims == 0)
      {
        throw std::domain_error("Lattice: must have at least one dimension");
      }

      for (nat_t i = 0; i < num_dims; ++i)
      {
        if (extents[i] == 0)
        {
          throw std::domain_error("Lattice: every extent must be positive");
        }

        total_size *= extents[i];
      }

      strides[num_dims - 1] = 1;

      for (nat_t i = num_dims - 1; i > 0; --i)
      {
        strides[i - 1] = strides[i] * extents[i];
      }

      cells = DynArray<CellT>(total_size, CellT());
    }

    nat_t num_dimensions() const
    {
      return num_dims;
    }

    nat_t extent(nat_t dim) const
    {
      return extents[dim];
    }

    const DynArray<nat_t>& get_extents() const
    {
      return extents;
    }

    nat_t size() const
    {
      return total_size;
    }

    nat_t to_linear_index(const Coordinates& coords) const
    {
      validate_coordinates(coords);

      nat_t idx = 0;

      for (nat_t i = 0; i < num_dims; ++i)
      {
        idx += coords[i] * strides[i];
      }

      return idx;
    }

    Coordinates to_coordinates(nat_t idx) const
    {
      Coordinates result(num_dims, nat_t(0));

      for (nat_t i = 0; i < num_dims; ++i)
      {
        result[i] = (idx / strides[i]) % extents[i];
      }

      return result;
    }

    CellT& get(const Coordinates& coords)
    {
      return cells[to_linear_index(coords)];
    }

    const CellT& get(const Coordinates& coords) const
    {
      return cells[to_linear_index(coords)];
    }

    CellT& operator()(const Coordinates& coords)
    {
      return get(coords);
    }

    const CellT& operator()(const Coordinates& coords) const
    {
      return get(coords);
    }

    using Iterator = typename DynArray<CellT>::Iterator;

    Iterator begin()
    {
      return cells.begin();
    }

    Iterator end()
    {
      return cells.end();
    }
  };

  /** Strategy interface: given a lattice, a cell's own position, and
      an offset (-1/0/+1 per dimension) relative to it, decide which
      *actual* cell that offset refers to — the entire reason this is
      pulled out of Neighborhood is so a Neighborhood implementation
      (which only needs to know "give me the cell one step left along
      dimension i") never needs to special-case what happens at the
      lattice's own edges; that is this class's job alone. */
  template <typename CellT>
  class BoundaryCondition
  {
  public:
    virtual ~BoundaryCondition() = default;

    virtual CellT get(const Lattice<CellT>& lattice, const Coordinates& pos,
                      const Offsets& delta) const = 0;
  };

  /** Wraps around: the neighbor "one step past the last cell" of a
      dimension is that dimension's first cell, and vice versa — the
      lattice behaves as a torus (in 2D; a circle in 1D, a
      higher-dimensional torus beyond that). */
  template <typename CellT>
  class PeriodicBoundary : public BoundaryCondition<CellT>
  {
  public:
    CellT get(const Lattice<CellT>& lattice, const Coordinates& pos,
              const Offsets& delta) const override
    {
      Coordinates idxs(lattice.num_dimensions(), nat_t(0));

      for (nat_t n = 0; n < lattice.num_dimensions(); ++n)
      {
        nat_t p = pos[n];
        int_t d = delta[n];

        if (d < 0)
        {
          idxs[n] = (p == 0) ? lattice.extent(n) - 1 : p - 1;
        }
        else if (d > 0)
        {
          idxs[n] = (p == lattice.extent(n) - 1) ? 0 : p + 1;
        }
        else
        {
          idxs[n] = p;
        }
      }

      return lattice.get(idxs);
    }
  };

  /** Mirrors at the edge: the neighbor "one step past the last cell"
      is the last cell's own other neighbor (itself reflected back),
      rather than wrapping to the opposite edge (PeriodicBoundary) or
      reading a fixed constant (FixedBoundary). */
  template <typename CellT>
  class ReflectiveBoundary : public BoundaryCondition<CellT>
  {
  public:
    CellT get(const Lattice<CellT>& lattice, const Coordinates& pos,
              const Offsets& delta) const override
    {
      Coordinates idxs(lattice.num_dimensions(), nat_t(0));

      for (nat_t n = 0; n < lattice.num_dimensions(); ++n)
      {
        nat_t p = pos[n];
        int_t d = delta[n];

        if (d < 0)
        {
          idxs[n] = (p == 0) ? p + 1 : p - 1;
        }
        else if (d > 0)
        {
          idxs[n] = (p == lattice.extent(n) - 1) ? p - 1 : p + 1;
        }
        else
        {
          idxs[n] = p;
        }
      }

      return lattice.get(idxs);
    }
  };

  /** Anything past the edge reads as a fixed, caller-chosen constant
      (e.g. "outside the simulated region is always dead/empty") rather
      than wrapping (PeriodicBoundary) or mirroring (ReflectiveBoundary). */
  template <typename CellT>
  class FixedBoundary : public BoundaryCondition<CellT>
  {
    CellT fixed_value;

  public:
    explicit FixedBoundary(const CellT& _fixed_value = CellT())
        : fixed_value(_fixed_value)
    {
      // empty
    }

    const CellT& get_fixed_value() const
    {
      return fixed_value;
    }

    void set_fixed_value(const CellT& _fixed_value)
    {
      fixed_value = _fixed_value;
    }

    CellT get(const Lattice<CellT>& lattice, const Coordinates& pos,
              const Offsets& delta) const override
    {
      Coordinates idxs(lattice.num_dimensions(), nat_t(0));

      for (nat_t n = 0; n < lattice.num_dimensions(); ++n)
      {
        nat_t p = pos[n];
        int_t d = delta[n];

        if (d < 0)
        {
          if (p == 0)
          {
            return fixed_value;
          }

          idxs[n] = p - 1;
        }
        else if (d > 0)
        {
          if (p == lattice.extent(n) - 1)
          {
            return fixed_value;
          }

          idxs[n] = p + 1;
        }
        else
        {
          idxs[n] = p;
        }
      }

      return lattice.get(idxs);
    }
  };

  /** Strategy interface: which cells (via `bcond`, so it never has to
      handle edges itself) count as `pos`'s neighbors. */
  template <typename CellT>
  class Neighborhood
  {
  public:
    virtual ~Neighborhood() = default;

    virtual DynArray<CellT> get_neighbors(const Lattice<CellT>& lattice,
                                          const BoundaryCondition<CellT>& bcond,
                                          const Coordinates& pos) const = 0;
  };

  /** The 2*order axis-aligned neighbors (one step along each dimension,
      in each direction) — the "+" shape in 2D, generalized to any
      dimensionality. */
  template <typename CellT>
  class VonNeumannNeighborhood : public Neighborhood<CellT>
  {
  public:
    DynArray<CellT> get_neighbors(const Lattice<CellT>& lattice,
                                  const BoundaryCondition<CellT>& bcond,
                                  const Coordinates& pos) const override
    {
      DynArray<CellT> result;
      nat_t order = lattice.num_dimensions();

      for (nat_t i = 0; i < order; ++i)
      {
        Offsets delta(order, int_t(0));

        delta[i] = -1;
        result.append(bcond.get(lattice, pos, delta));

        delta[i] = 1;
        result.append(bcond.get(lattice, pos, delta));
      }

      return result;
    }
  };

  /** Every one of the 3^order - 1 cells within one step along every
      dimension simultaneously (the center, all-zero offset, excluded)
      — the 8-neighbor square in 2D, generalized to any dimensionality
      by treating each of the `order` offsets as a base-3 digit
      (-1/0/+1) and enumerating every combination via a simple odometer
      counter, rather than needing a separate hand-written loop nest
      per dimensionality the way a fixed 1D/2D/3D implementation would. */
  template <typename CellT>
  class MooreNeighborhood : public Neighborhood<CellT>
  {
  public:
    DynArray<CellT> get_neighbors(const Lattice<CellT>& lattice,
                                  const BoundaryCondition<CellT>& bcond,
                                  const Coordinates& pos) const override
    {
      DynArray<CellT> result;
      nat_t order = lattice.num_dimensions();

      nat_t total_combos = 1;

      for (nat_t i = 0; i < order; ++i)
      {
        total_combos *= 3;
      }

      Offsets delta(order, int_t(0));

      for (nat_t combo = 0; combo < total_combos; ++combo)
      {
        nat_t rem = combo;
        bool all_zero = true;

        for (nat_t i = 0; i < order; ++i)
        {
          int_t d = int_t(rem % 3) - 1;
          delta[i] = d;
          rem /= 3;

          if (d != 0)
          {
            all_zero = false;
          }
        }

        if (!all_zero)
        {
          result.append(bcond.get(lattice, pos, delta));
        }
      }

      return result;
    }
  };

  /** Ties a Lattice, a BoundaryCondition, a Neighborhood, and a
      transition rule together: `rule(current_cell, neighbor_values)`
      returns what that cell becomes next generation. step() computes
      every cell's next value from a read-only snapshot of the current
      generation before overwriting anything — updating in place
      instead would let a cell see some neighbors already updated to
      next-generation values and others not, which is not what "one
      synchronous generation" means for a cellular automaton. */
  template <typename CellT>
  class CellularAutomaton
  {
  public:
    using RuleFn = std::function<CellT(const CellT&, const DynArray<CellT>&)>;

  private:
    Lattice<CellT> lattice;
    const BoundaryCondition<CellT>& boundary;
    const Neighborhood<CellT>& neighborhood;
    RuleFn rule;
    nat_t generation;

  public:
    CellularAutomaton(const Lattice<CellT>& _lattice,
                      const BoundaryCondition<CellT>& _boundary,
                      const Neighborhood<CellT>& _neighborhood, RuleFn _rule)
        : lattice(_lattice), boundary(_boundary), neighborhood(_neighborhood),
          rule(std::move(_rule)), generation(0)
    {
      // empty
    }

    const Lattice<CellT>& get_lattice() const
    {
      return lattice;
    }

    Lattice<CellT>& get_lattice()
    {
      return lattice;
    }

    nat_t get_generation() const
    {
      return generation;
    }

    void step()
    {
      Lattice<CellT> next = lattice;

      for (nat_t idx = 0; idx < lattice.size(); ++idx)
      {
        Coordinates pos = lattice.to_coordinates(idx);
        DynArray<CellT> neighbors = neighborhood.get_neighbors(lattice, boundary, pos);
        next.get(pos) = rule(lattice.get(pos), neighbors);
      }

      lattice = std::move(next);
      ++generation;
    }

    void step(nat_t num_steps)
    {
      for (nat_t i = 0; i < num_steps; ++i)
      {
        step();
      }
    }
  };

} // end namespace Designar
