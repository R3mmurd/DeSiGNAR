/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>
#include <backtracking.hpp>

using namespace std;
using namespace Designar;

int main()
{
    // Known n-queens solution counts (OEIS A000170).
    nat_t expected[] = {0, 1, 0, 0, 2, 10, 4, 40, 92};

    for (nat_t n = 1; n <= 8; ++n)
    {
        assert(n_queens_count_solutions(n) == expected[n]);
    }

    auto solutions = n_queens_all_solutions(8);
    assert(solutions.size() == 92);

    // Every reported solution must actually be valid: no two queens
    // share a column or a diagonal.
    for (const auto& sol : solutions)
    {
        assert(sol.size() == 8);

        for (nat_t i = 0; i < 8; ++i)
        {
            for (nat_t j = i + 1; j < 8; ++j)
            {
                assert(sol[i] != sol[j]);

                nat_t row_diff = j - i;
                nat_t col_diff = sol[i] > sol[j] ? sol[i] - sol[j]
                                                 : sol[j] - sol[i];
                assert(row_diff != col_diff);
            }
        }
    }

    // n=1 is the trivial single-solution case; n=2 and n=3 famously
    // have none at all — both are worth asserting explicitly, not just
    // implicitly via the loop above.
    assert(n_queens_all_solutions(1).size() == 1);
    assert(n_queens_all_solutions(2).is_empty());
    assert(n_queens_all_solutions(3).is_empty());

    cout << "Everything ok!\n";

    return 0;
}
