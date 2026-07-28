/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file backtracking.hpp
    @brief A small generic backtracking search over a `DynArray<T>`
    partial solution, driven by three callbacks (candidate generation,
    partial-solution validity, and what to do with a completed
    solution), plus the canonical N-Queens application built on it —
    the textbook example of the technique.
    @ingroup Algorithms
*/

#pragma once

#include <array.hpp>

namespace Designar
{
    /** Depth-first-searches every way to extend `partial` up to
        `target_size` elements: at each step, tries every value
        `candidates(partial)` returns for the next position, keeping it
        only if `is_valid(partial)` accepts the partial solution with
        that value tentatively appended, and undoing it (backtracking)
        otherwise. Every completed solution (`partial.size() ==
        target_size`) is reported via `on_solution(partial)`; returning
        `true` from this call stops the search early (useful for "find
        the first solution"), `false` keeps searching for more (useful
        for "count/collect all solutions"). Returns whether the search
        was stopped early by `on_solution`. */
    template <typename T, class CandidatesFn, class IsValidFn,
              class OnSolutionFn>
    bool solve_backtracking(DynArray<T>& partial, nat_t target_size,
                            CandidatesFn&& candidates, IsValidFn&& is_valid,
                            OnSolutionFn&& on_solution)
    {
        if (partial.size() == target_size)
        {
            return on_solution(partial);
        }

        for (const T& candidate : candidates(partial))
        {
            partial.append(candidate);

            if (is_valid(partial) &&
                solve_backtracking(partial, target_size, candidates, is_valid,
                                   on_solution))
            {
                return true;
            }

            partial.remove_last();
        }

        return false;
    }

    /** The N-Queens problem: place `n` mutually non-attacking queens on
        an n×n board, one per row — `solution[r]` is the column of the
        queen in row `r`. `is_valid` only ever needs to check the queen
        just placed (row `partial.size() - 1`) against every earlier
        row, since every previously-placed queen was already validated
        against *its* predecessors when it was placed. */
    inline bool n_queens_is_valid(const DynArray<nat_t>& partial)
    {
        nat_t row = partial.size() - 1;
        nat_t col = partial[row];

        for (nat_t r = 0; r < row; ++r)
        {
            nat_t c = partial[r];

            if (c == col)
            {
                return false;
            }

            nat_t row_diff = row > r ? row - r : r - row;
            nat_t col_diff = col > c ? col - c : c - col;

            if (row_diff == col_diff)
            {
                return false;
            }
        }

        return true;
    }

    inline DynArray<nat_t> n_queens_candidates(nat_t n)
    {
        DynArray<nat_t> cols;

        for (nat_t c = 0; c < n; ++c)
        {
            cols.append(c);
        }

        return cols;
    }

    /** Every solution to the n-queens problem, each as
        `solution[row] = column`. */
    inline DynArray<DynArray<nat_t>> n_queens_all_solutions(nat_t n)
    {
        DynArray<DynArray<nat_t>> solutions;
        DynArray<nat_t> partial;
        DynArray<nat_t> cols = n_queens_candidates(n);

        solve_backtracking<nat_t>(
            partial, n, [&](const DynArray<nat_t>&) -> const DynArray<nat_t>&
            { return cols; },
            n_queens_is_valid,
            [&](const DynArray<nat_t>& sol)
            {
                solutions.append(sol);
                return false; // keep searching for every solution
            });

        return solutions;
    }

    /** Counts n-queens solutions without materializing each one — the
        classic OEIS A000170 sequence (n_queens_count_solutions(8) ==
        92). */
    inline nat_t n_queens_count_solutions(nat_t n)
    {
        nat_t count = 0;
        DynArray<nat_t> partial;
        DynArray<nat_t> cols = n_queens_candidates(n);

        solve_backtracking<nat_t>(
            partial, n, [&](const DynArray<nat_t>&) -> const DynArray<nat_t>&
            { return cols; },
            n_queens_is_valid,
            [&](const DynArray<nat_t>&)
            {
                ++count;
                return false;
            });

        return count;
    }

} // end namespace Designar
