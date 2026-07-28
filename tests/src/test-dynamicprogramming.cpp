/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <iostream>
#include <dynamicprogramming.hpp>

using namespace std;
using namespace Designar;

namespace
{
    bool is_subsequence_of(const std::string& s, const std::string& t)
    {
        nat_t i = 0;

        for (char c : t)
        {
            if (i < s.size() && s[i] == c)
            {
                ++i;
            }
        }

        return i == s.size();
    }
} // end anonymous namespace

int main()
{
    // matrix_chain_order: CLRS's own worked example — six matrices
    // A1(30x35) A2(35x15) A3(15x5) A4(5x10) A5(10x20) A6(20x25), whose
    // known-optimal cost and parenthesization CLRS states explicitly.
    {
        DynArray<nat_t> dims = {30, 35, 15, 5, 10, 20, 25};
        auto result = matrix_chain_order(dims);
        assert(result.min_multiplications == 15125);

        std::string paren =
            matrix_chain_parenthesization(result.split, 0, dims.size() - 2);
        assert(paren == "((A0(A1A2))((A3A4)A5))");

        // A single matrix needs no multiplications at all.
        DynArray<nat_t> single_dims = {10, 20};
        auto single_result = matrix_chain_order(single_dims);
        assert(single_result.min_multiplications == 0);

        bool threw = false;

        try
        {
            DynArray<nat_t> empty_dims;
            matrix_chain_order(empty_dims);
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }

        assert(threw);
    }

    // longest_common_subsequence: the classic ABCBDAB/BDCABA example
    // (length 4, e.g. "BCBA" or "BDAB" — several length-4 subsequences
    // exist; only the length and the subsequence property are checked).
    {
        std::string x = "ABCBDAB";
        std::string y = "BDCABA";

        assert(longest_common_subsequence_length(x, y) == 4);

        std::string lcs = longest_common_subsequence(x, y);
        assert(lcs.size() == 4);
        assert(is_subsequence_of(lcs, x));
        assert(is_subsequence_of(lcs, y));

        // Disjoint strings have an empty LCS.
        assert(longest_common_subsequence_length("abc", "xyz") == 0);
        assert(longest_common_subsequence("abc", "xyz").empty());

        // Identical strings are their own LCS.
        assert(longest_common_subsequence_length("hello", "hello") == 5);
    }

    // optimal_polygon_triangulation: a unit square has exactly two
    // triangulations (one diagonal or the other); each splits it into
    // two triangles whose *perimeters* (this problem's cost measure
    // deliberately counts each triangle's full perimeter, so a shared
    // diagonal is counted once per triangle that uses it, not once
    // overall) are each `2 + sqrt(2)` (two unit sides plus one
    // diagonal), for a total of `4 + 2*sqrt(2)` — the same by the
    // square's symmetry regardless of which diagonal is chosen.
    {
        DynArray<Point2D> square = {Point2D(0, 0), Point2D(1, 0),
                                    Point2D(1, 1), Point2D(0, 1)};

        real_t cost = optimal_polygon_triangulation(square);
        real_t expected = 4.0 + 2.0 * std::sqrt(2.0);
        assert(std::abs(cost - expected) < 1e-9);

        bool threw = false;

        try
        {
            DynArray<Point2D> too_few = {Point2D(0, 0), Point2D(1, 0)};
            optimal_polygon_triangulation(too_few);
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }

        assert(threw);
    }

    // knapsack_01: small hand-checkable example — items (weight, value)
    // = (2,3), (3,4), (4,5), (5,6), capacity 5; the optimal choice is
    // items 0 and 1 (total weight 5, total value 7), strictly better
    // than any single item alone (the best single item is worth 6).
    {
        DynArray<nat_t> weights = {2, 3, 4, 5};
        DynArray<nat_t> values = {3, 4, 5, 6};

        auto result = knapsack_01(weights, values, 5);
        assert(result.max_value == 7);

        nat_t total_weight = 0;
        nat_t total_value = 0;

        for (nat_t i = 0; i < weights.size(); ++i)
        {
            if (result.chosen[i])
            {
                total_weight += weights[i];
                total_value += values[i];
            }
        }

        assert(total_weight <= 5);
        assert(total_value == result.max_value);
        assert(result.chosen[0] && result.chosen[1]);
        assert(!result.chosen[2] && !result.chosen[3]);

        // Zero capacity: nothing fits.
        auto zero_cap = knapsack_01(weights, values, 0);
        assert(zero_cap.max_value == 0);

        bool threw = false;

        try
        {
            DynArray<nat_t> mismatched_values = {1, 2};
            knapsack_01(weights, mismatched_values, 5);
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }

        assert(threw);
    }

    cout << "Everything ok!\n";

    return 0;
}
