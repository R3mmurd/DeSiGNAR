/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <backtracking.hpp>

using namespace Designar;

namespace
{
    void print_board(const DynArray<nat_t>& solution)
    {
        nat_t n = solution.size();

        for (nat_t row = 0; row < n; ++row)
        {
            for (nat_t col = 0; col < n; ++col)
            {
                cout << (solution[row] == col ? "Q " : ". ");
            }

            cout << endl;
        }
    }
} // end anonymous namespace

int main()
{
    cout << "N-Queens solution counts (OEIS A000170):\n";

    for (nat_t n = 1; n <= 8; ++n)
    {
        cout << "  n = " << n << ": " << n_queens_count_solutions(n)
             << " solutions\n";
    }

    cout << "\nOne solution for n = 8:\n";

    auto solutions = n_queens_all_solutions(8);
    print_board(solutions[0]);

    return 0;
}
