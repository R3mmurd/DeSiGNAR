/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <dynamicprogramming.hpp>

using namespace Designar;

int main()
{
    // matrix_chain_order: CLRS's own worked example.
    DynArray<nat_t> dims = {30, 35, 15, 5, 10, 20, 25};
    auto mc = matrix_chain_order(dims);

    cout << "matrix_chain_order: minimum multiplications = "
         << mc.min_multiplications << endl;
    cout << "  optimal parenthesization: "
         << matrix_chain_parenthesization(mc.split, 0, dims.size() - 2)
         << endl;

    // longest_common_subsequence
    std::string x = "ABCBDAB";
    std::string y = "BDCABA";

    cout << "\nlongest_common_subsequence(\"" << x << "\", \"" << y
         << "\") = \"" << longest_common_subsequence(x, y) << "\" (length "
         << longest_common_subsequence_length(x, y) << ")" << endl;

    // optimal_polygon_triangulation
    DynArray<Point2D> pentagon = {Point2D(0, 0), Point2D(2, -1),
                                 Point2D(4, 0), Point2D(3, 2),
                                 Point2D(1, 2)};

    cout << "\noptimal_polygon_triangulation cost for a convex pentagon: "
         << optimal_polygon_triangulation(pentagon) << endl;

    // knapsack_01
    DynArray<nat_t> weights = {2, 3, 4, 5};
    DynArray<nat_t> values = {3, 4, 5, 6};
    auto ks = knapsack_01(weights, values, 5);

    cout << "\nknapsack_01 (capacity 5): max_value = " << ks.max_value
         << ", chosen items: ";

    for (nat_t i = 0; i < weights.size(); ++i)
    {
        if (ks.chosen[i])
        {
            cout << "(w=" << weights[i] << ", v=" << values[i] << ") ";
        }
    }

    cout << endl;

    // knapsack_fractional: the same-looking problem, but with items
    // divisible — a greedy by-ratio choice is optimal here, unlike 0/1.
    DynArray<nat_t> fw = {10, 20, 30};
    DynArray<nat_t> fv = {60, 100, 120};
    auto fks = knapsack_fractional(fw, fv, 50);

    cout << "\nknapsack_fractional (capacity 50): max_value = "
         << fks.max_value << ", fractions taken: ";

    for (nat_t i = 0; i < fw.size(); ++i)
    {
        cout << "(w=" << fw[i] << ", v=" << fv[i] << ", " << fks.fraction[i]
             << ") ";
    }

    cout << endl;

    return 0;
}
