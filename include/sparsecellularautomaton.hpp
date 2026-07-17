/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file sparsecellularautomaton.hpp
    @brief N-dimensional sparse cellular automata: unlike Lattice
    (cellularautomaton.hpp), a bounded, fixed-extent dense grid,
    SparseLattice represents an unbounded grid backed by a HashMap
    keyed by serialized coordinates, storing only cells that have
    ever been explicitly set — suited to simulations (e.g. a Game of
    Life pattern that grows without ever being told a bounding box up
    front) where allocating a dense array over the whole reachable
    space would be wasteful or simply not knowable in advance.
    Coordinates here are signed (`int_t`), not the unsigned `nat_t`
    Coordinates the dense Lattice uses, since "unbounded in every
    direction" specifically requires negative coordinates to be
    representable. SparseCellularAutomaton::step() only re-evaluates
    active cells and their neighbors (the "frontier"), not an
    impossible pass over all of an infinite space — this relies on
    the standard cellular-automaton convention that a cell with no
    active neighbors and which is itself inactive stays inactive
    forever (e.g. Conway's Life: a dead cell with zero live neighbors
    stays dead), so anything outside the frontier is guaranteed
    unchanged without needing to visit it.
    @ingroup CellularAutomata
*/

#pragma once

#include <array.hpp>
#include <map.hpp>

#include <stdexcept>
#include <functional>
#include <string>

namespace Designar
{
    /** A per-dimension signed coordinate into a SparseLattice — signed
        because the space it indexes is unbounded in both directions
        along every dimension. */
    using SparseCoordinates = DynArray<int_t>;

    /** A per-dimension offset, exactly like Offsets in
        cellularautomaton.hpp, but not restricted to -1/0/+1: since
        there is no fixed extent to reflect or wrap against here, a
        sparse neighborhood is free to reach further than one step if a
        caller ever wants that. */
    using SparseOffsets = DynArray<int_t>;

    /** A flat, unbounded, runtime-dimensional grid of `CellT` cells,
        backed by a HashMap from a serialized coordinate string to the
        cell's value — only cells that have been explicitly `set()`
        occupy any memory; every other coordinate implicitly holds
        `background`. */
    template <typename CellT>
    class SparseLattice
    {
    public:
        using CellType = CellT;

        struct Entry
        {
            SparseCoordinates coords;
            CellT value;
        };

    private:
        nat_t num_dims;
        CellT background;
        HashMap<std::string, Entry> cells;

        void validate_coordinates(const SparseCoordinates& coords) const
        {
            if (coords.size() != num_dims)
            {
                throw std::domain_error("SparseLattice: coordinate count does "
                                        "not match dimensionality");
            }
        }

        static std::string make_key(const SparseCoordinates& coords)
        {
            std::string key;

            for (nat_t i = 0; i < coords.size(); ++i)
            {
                if (i > 0)
                {
                    key += ',';
                }

                key += std::to_string(coords[i]);
            }

            return key;
        }

    public:
        explicit SparseLattice(nat_t _num_dims,
                               const CellT& _background = CellT())
            : num_dims(_num_dims), background(_background)
        {
            if (num_dims == 0)
            {
                throw std::domain_error(
                    "SparseLattice: must have at least one dimension");
            }
        }

        nat_t num_dimensions() const
        {
            return num_dims;
        }

        const CellT& get_background() const
        {
            return background;
        }

        nat_t num_active_cells() const
        {
            return cells.size();
        }

        CellT get(const SparseCoordinates& coords) const
        {
            validate_coordinates(coords);
            const Entry* e = cells.search(make_key(coords));
            return e == nullptr ? background : e->value;
        }

        void set(const SparseCoordinates& coords, const CellT& value)
        {
            validate_coordinates(coords);
            cells[make_key(coords)] = Entry{coords, value};
        }

        /** Applies `op(const Entry&)` to every explicitly-tracked
            (active) cell — the only way to enumerate cells in a lattice
            that has no bounds to loop over. */
        template <class Operation>
        void for_each_active(Operation&& op) const
        {
            for (const auto& item : cells)
            {
                op(item.second);
            }
        }
    };

    /** Strategy interface: which cells (as absolute coordinates, one
        per neighbor) count as `pos`'s neighbors — unlike
        cellularautomaton.hpp's Neighborhood, there is no
        BoundaryCondition collaborator here, since an unbounded lattice
        has no edge to special-case. */
    template <typename CellT>
    class SparseNeighborhood
    {
    public:
        virtual ~SparseNeighborhood() = default;

        virtual DynArray<SparseCoordinates>
        get_neighbor_coordinates(nat_t num_dims,
                                 const SparseCoordinates& pos) const = 0;
    };

    /** The 2*order axis-aligned neighbors, exactly like
        cellularautomaton.hpp's VonNeumannNeighborhood but returning raw
        coordinates (offset directly, no wraparound/reflection/fixed
        value to resolve) since the sparse lattice is unbounded. */
    template <typename CellT>
    class SparseVonNeumannNeighborhood : public SparseNeighborhood<CellT>
    {
    public:
        DynArray<SparseCoordinates>
        get_neighbor_coordinates(nat_t num_dims,
                                 const SparseCoordinates& pos) const override
        {
            DynArray<SparseCoordinates> result;

            for (nat_t i = 0; i < num_dims; ++i)
            {
                SparseCoordinates minus = pos;
                minus[i] -= 1;
                result.append(minus);

                SparseCoordinates plus = pos;
                plus[i] += 1;
                result.append(plus);
            }

            return result;
        }
    };

    /** Every one of the 3^order - 1 cells within one step along every
        dimension simultaneously, exactly like
        cellularautomaton.hpp's MooreNeighborhood but returning raw
        coordinates via the same base-3-odometer enumeration. */
    template <typename CellT>
    class SparseMooreNeighborhood : public SparseNeighborhood<CellT>
    {
    public:
        DynArray<SparseCoordinates>
        get_neighbor_coordinates(nat_t num_dims,
                                 const SparseCoordinates& pos) const override
        {
            DynArray<SparseCoordinates> result;

            nat_t total_combos = 1;

            for (nat_t i = 0; i < num_dims; ++i)
            {
                total_combos *= 3;
            }

            for (nat_t combo = 0; combo < total_combos; ++combo)
            {
                nat_t rem = combo;
                bool all_zero = true;
                SparseCoordinates neighbor = pos;

                for (nat_t i = 0; i < num_dims; ++i)
                {
                    int_t d = int_t(rem % 3) - 1;
                    rem /= 3;

                    if (d != 0)
                    {
                        all_zero = false;
                    }

                    neighbor[i] = pos[i] + d;
                }

                if (!all_zero)
                {
                    result.append(neighbor);
                }
            }

            return result;
        }
    };

    /** Ties a SparseLattice, a SparseNeighborhood, and a transition
        rule together. step() only recomputes the "frontier" — every
        currently-active cell plus all of their neighbors — rather than
        every cell in an unbounded space, relying on the standard
        cellular-automaton convention that a cell with an entirely
        background neighborhood and which is itself at the background
        value stays there forever (see the file-level comment). Cells
        whose recomputed value equals `background` are pruned back out
        of the lattice's HashMap so it does not grow without bound as a
        pattern moves or dies out. */
    template <typename CellT>
    class SparseCellularAutomaton
    {
    public:
        using RuleFn =
            std::function<CellT(const CellT&, const DynArray<CellT>&)>;

    private:
        SparseLattice<CellT> lattice;
        const SparseNeighborhood<CellT>& neighborhood;
        RuleFn rule;
        nat_t generation;

        static std::string make_key(const SparseCoordinates& coords)
        {
            std::string key;

            for (nat_t i = 0; i < coords.size(); ++i)
            {
                if (i > 0)
                {
                    key += ',';
                }

                key += std::to_string(coords[i]);
            }

            return key;
        }

    public:
        SparseCellularAutomaton(const SparseLattice<CellT>& _lattice,
                                const SparseNeighborhood<CellT>& _neighborhood,
                                RuleFn _rule)
            : lattice(_lattice),
              neighborhood(_neighborhood),
              rule(std::move(_rule)),
              generation(0)
        {
            // empty
        }

        const SparseLattice<CellT>& get_lattice() const
        {
            return lattice;
        }

        SparseLattice<CellT>& get_lattice()
        {
            return lattice;
        }

        nat_t get_generation() const
        {
            return generation;
        }

        void step()
        {
            nat_t num_dims = lattice.num_dimensions();

            HashMap<std::string, SparseCoordinates> frontier;

            lattice.for_each_active(
                [&](const typename SparseLattice<CellT>::Entry& e)
                {
                    frontier[make_key(e.coords)] = e.coords;

                    for (const SparseCoordinates& n :
                         neighborhood.get_neighbor_coordinates(num_dims,
                                                               e.coords))
                    {
                        frontier[make_key(n)] = n;
                    }
                });

            SparseLattice<CellT> next(num_dims, lattice.get_background());

            for (const auto& item : frontier)
            {
                const SparseCoordinates& pos = item.second;

                DynArray<CellT> neighbor_values;

                for (const SparseCoordinates& n :
                     neighborhood.get_neighbor_coordinates(num_dims, pos))
                {
                    neighbor_values.append(lattice.get(n));
                }

                CellT new_value = rule(lattice.get(pos), neighbor_values);

                if (!(new_value == lattice.get_background()))
                {
                    next.set(pos, new_value);
                }
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
