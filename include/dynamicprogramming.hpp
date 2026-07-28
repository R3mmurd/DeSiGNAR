/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file dynamicprogramming.hpp
    @brief The classic dynamic-programming examples every algorithms
    course builds the technique on: matrix-chain multiplication order,
    the longest common subsequence, optimal polygon triangulation, and
    0/1 knapsack — each filling in a table bottom-up from smaller
    subproblems, the shape that separates DP from plain recursion (the
    subproblems overlap, so memoizing/tabulating them is what turns an
    exponential brute force into a polynomial algorithm).
    @ingroup Algorithms
*/

#pragma once

#include <algorithm>
#include <limits>
#include <string>

#include <array.hpp>
#include <point2D.hpp>

namespace Designar
{
    /** The optimal split point for every subchain, `split[i][j]`
        meaningful for `i < j` — matrix `k` (0-indexed) is the last one
        in the left half of the optimal split of the chain from matrix
        `i` to matrix `j`. */
    struct MatrixChainResult
    {
        nat_t min_multiplications;
        DynArray<DynArray<nat_t>> split;
    };

    /** `dims.size()` must be `n + 1` for a chain of `n` matrices: matrix
        `i` (0-indexed) has dimensions `dims[i] x dims[i + 1]` — the
        standard CLRS encoding, where consecutive matrices sharing an
        inner dimension is exactly what makes them chainable at all.
        Bottom-up over increasing chain length `len`, exactly like every
        other DP here: `m[i][j]` (the minimum scalar-multiplication cost
        to compute the product of matrices `i..j`) only ever needs
        shorter subchains' costs, already filled in by the time a longer
        one is considered. */
    inline MatrixChainResult matrix_chain_order(const DynArray<nat_t>& dims)
    {
        if (dims.size() < 2)
        {
            throw std::invalid_argument(
                "matrix_chain_order: need at least one matrix "
                "(dims.size() >= 2)");
        }

        nat_t n = dims.size() - 1;

        DynArray<DynArray<nat_t>> m(n, DynArray<nat_t>(n, nat_t(0)));
        DynArray<DynArray<nat_t>> s(n, DynArray<nat_t>(n, nat_t(0)));

        for (nat_t len = 2; len <= n; ++len)
        {
            for (nat_t i = 0; i + len - 1 < n; ++i)
            {
                nat_t j = i + len - 1;
                m[i][j] = std::numeric_limits<nat_t>::max();

                for (nat_t k = i; k < j; ++k)
                {
                    nat_t cost =
                        m[i][k] + m[k + 1][j] + dims[i] * dims[k + 1] * dims[j + 1];

                    if (cost < m[i][j])
                    {
                        m[i][j] = cost;
                        s[i][j] = k;
                    }
                }
            }
        }

        return MatrixChainResult{m[0][n - 1], std::move(s)};
    }

    /** Renders the optimal parenthesization as e.g. `"((A0(A1A2))A3)"`,
        walking `split` the same way it was filled — recursively, each
        half of the split parenthesized in turn — with matrices named
        `A0`, `A1`, ... in chain order. */
    inline std::string matrix_chain_parenthesization(
        const DynArray<DynArray<nat_t>>& split, nat_t i, nat_t j)
    {
        if (i == j)
        {
            return "A" + std::to_string(i);
        }

        nat_t k = split[i][j];

        return "(" + matrix_chain_parenthesization(split, i, k) +
               matrix_chain_parenthesization(split, k + 1, j) + ")";
    }

    /** The length of the longest common subsequence of `x` and `y`
        (not necessarily contiguous in either, but in relative order in
        both) — `dp[i][j]` is that length restricted to `x`'s first `i`
        characters and `y`'s first `j`: a matching last character
        extends the LCS of both one-shorter prefixes by one, otherwise
        it's whichever of "drop x's last character" or "drop y's last
        character" leaves the longer LCS. */
    inline nat_t longest_common_subsequence_length(const std::string& x,
                                                    const std::string& y)
    {
        nat_t m = x.size();
        nat_t n = y.size();

        DynArray<DynArray<nat_t>> dp(m + 1, DynArray<nat_t>(n + 1, nat_t(0)));

        for (nat_t i = 1; i <= m; ++i)
        {
            for (nat_t j = 1; j <= n; ++j)
            {
                if (x[i - 1] == y[j - 1])
                {
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                }
                else
                {
                    dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
                }
            }
        }

        return dp[m][n];
    }

    /** The actual longest common subsequence itself (one of possibly
        several of the same maximal length), reconstructed by walking
        the filled DP table from `(m, n)` back to `(0, 0)`, following
        whichever move (diagonal on a match, otherwise toward the larger
        neighbor) is what produced each cell's value in the first
        place. */
    inline std::string longest_common_subsequence(const std::string& x,
                                                   const std::string& y)
    {
        nat_t m = x.size();
        nat_t n = y.size();

        DynArray<DynArray<nat_t>> dp(m + 1, DynArray<nat_t>(n + 1, nat_t(0)));

        for (nat_t i = 1; i <= m; ++i)
        {
            for (nat_t j = 1; j <= n; ++j)
            {
                if (x[i - 1] == y[j - 1])
                {
                    dp[i][j] = dp[i - 1][j - 1] + 1;
                }
                else
                {
                    dp[i][j] = std::max(dp[i - 1][j], dp[i][j - 1]);
                }
            }
        }

        std::string result;
        nat_t i = m;
        nat_t j = n;

        while (i > 0 && j > 0)
        {
            if (x[i - 1] == y[j - 1])
            {
                result += x[i - 1];
                --i;
                --j;
            }
            else if (dp[i - 1][j] >= dp[i][j - 1])
            {
                --i;
            }
            else
            {
                --j;
            }
        }

        std::reverse(result.begin(), result.end());
        return result;
    }

    /** Minimum-weight triangulation of a convex polygon given by its
        vertices `v[0..n-1]` in order — `t[i][j]` (`i < j`) is the
        minimum total triangle-perimeter weight to triangulate the
        sub-polygon `v[i], v[i+1], ..., v[j]`, built by trying every
        vertex `k` strictly between `i` and `j` as the apex of the
        triangle `(v[i], v[k], v[j])` sitting on top of the sub-polygon's
        base edge `v[i]v[j]`, and combining with however `v[i..k]` and
        `v[k..j]` are best triangulated on their own — the same "split
        the range, solve both halves, combine" shape as matrix-chain
        order, just over a polygon's vertices instead of a matrix
        chain. */
    inline real_t optimal_polygon_triangulation(const DynArray<Point2D>& v)
    {
        nat_t n = v.size();

        if (n < 3)
        {
            throw std::invalid_argument(
                "optimal_polygon_triangulation: need at least 3 vertices");
        }

        auto weight = [&](nat_t i, nat_t k, nat_t j)
        {
            return v[i].distance_with(v[k]) + v[k].distance_with(v[j]) +
                   v[j].distance_with(v[i]);
        };

        DynArray<DynArray<real_t>> t(n, DynArray<real_t>(n, real_t(0)));

        for (nat_t len = 2; len < n; ++len)
        {
            for (nat_t i = 0; i + len < n; ++i)
            {
                nat_t j = i + len;
                t[i][j] = std::numeric_limits<real_t>::max();

                for (nat_t k = i + 1; k < j; ++k)
                {
                    real_t cost = t[i][k] + t[k][j] + weight(i, k, j);

                    if (cost < t[i][j])
                    {
                        t[i][j] = cost;
                    }
                }
            }
        }

        return t[0][n - 1];
    }

    struct KnapsackResult
    {
        nat_t max_value;

        /** `chosen[i]` is whether item `i` is in the optimal selection. */
        DynArray<bool> chosen;
    };

    /** 0/1 knapsack: choose a subset of items (each either fully taken
        or left out entirely — the "0/1", as opposed to the fractional
        version knapsack_fractional() below solves optimally instead, a
        greedy by-ratio choice) maximizing total value subject to a
        weight capacity. `dp[i][w]` is the best
        value achievable using only the first `i` items with capacity
        `w`: item `i` either isn't used, or (only when it fits) is —
        whichever of those two choices leaves the larger value. */
    inline KnapsackResult knapsack_01(const DynArray<nat_t>& weights,
                                      const DynArray<nat_t>& values,
                                      nat_t capacity)
    {
        if (weights.size() != values.size())
        {
            throw std::invalid_argument(
                "knapsack_01: weights and values must have the same size");
        }

        nat_t n = weights.size();

        DynArray<DynArray<nat_t>> dp(n + 1,
                                     DynArray<nat_t>(capacity + 1, nat_t(0)));

        for (nat_t i = 1; i <= n; ++i)
        {
            for (nat_t w = 0; w <= capacity; ++w)
            {
                dp[i][w] = dp[i - 1][w];

                if (weights[i - 1] <= w)
                {
                    nat_t with_item = dp[i - 1][w - weights[i - 1]] + values[i - 1];

                    if (with_item > dp[i][w])
                    {
                        dp[i][w] = with_item;
                    }
                }
            }
        }

        DynArray<bool> chosen(n, false);
        nat_t w = capacity;

        for (nat_t i = n; i > 0; --i)
        {
            if (dp[i][w] != dp[i - 1][w])
            {
                chosen[i - 1] = true;
                w -= weights[i - 1];
            }
        }

        return KnapsackResult{dp[n][capacity], std::move(chosen)};
    }

    struct FractionalKnapsackResult
    {
        real_t max_value;

        /** `fraction[i]` is how much of item `i` (0 = none, 1 = all of
            it) the optimal selection takes. */
        DynArray<real_t> fraction;
    };

    /** The fractional relaxation of 0/1 knapsack: items can be broken
        into any fraction, which — unlike the 0/1 version — a plain
        greedy choice solves *optimally*, not just approximately: sort
        by value/weight ratio descending, then take as much of each item
        as the remaining capacity allows, highest ratio first. This is
        the textbook contrast the two knapsack variants are usually
        taught side by side for: the same-looking problem is solvable
        by two entirely different techniques (DP vs. greedy) depending
        on one modeling choice (divisible or not), and only one of them
        stays optimal once that choice is "not divisible". */
    inline FractionalKnapsackResult
    knapsack_fractional(const DynArray<nat_t>& weights,
                        const DynArray<nat_t>& values, nat_t capacity)
    {
        if (weights.size() != values.size())
        {
            throw std::invalid_argument(
                "knapsack_fractional: weights and values must have the "
                "same size");
        }

        nat_t n = weights.size();

        DynArray<nat_t> order(n, nat_t(0));

        for (nat_t i = 0; i < n; ++i)
        {
            order[i] = i;
        }

        std::sort(order.begin(), order.end(),
                 [&](nat_t a, nat_t b)
                 {
                     return real_t(values[a]) * real_t(weights[b]) >
                            real_t(values[b]) * real_t(weights[a]);
                 });

        DynArray<real_t> fraction(n, real_t(0));
        real_t remaining = real_t(capacity);
        real_t total_value = real_t(0);

        for (nat_t idx = 0; idx < n && remaining > real_t(0); ++idx)
        {
            nat_t i = order[idx];

            if (real_t(weights[i]) <= remaining)
            {
                fraction[i] = real_t(1);
                remaining -= real_t(weights[i]);
                total_value += real_t(values[i]);
            }
            else if (weights[i] > 0)
            {
                real_t taken = remaining / real_t(weights[i]);
                fraction[i] = taken;
                total_value += taken * real_t(values[i]);
                remaining = real_t(0);
            }
        }

        return FractionalKnapsackResult{total_value, std::move(fraction)};
    }

} // end namespace Designar
