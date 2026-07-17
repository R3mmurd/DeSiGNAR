/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <string>
#include <vector>

using namespace std;

#include <turingmachine.hpp>

using namespace Designar;

int main()
{
    // The textbook L = {0^n 1^n} decider: mark a 0 as X, find its
    // matching 1 and mark it Y, repeat; accept once only Ys remain.
    TuringMachine<char> tm(5, 0, '_');
    tm.set_accepting(4);

    tm.add_transition(0, '0', 1, 'X', TapeDirection::RIGHT);
    tm.add_transition(0, 'Y', 3, 'Y', TapeDirection::RIGHT);
    tm.add_transition(0, '_', 4, '_', TapeDirection::STAY);

    tm.add_transition(1, '0', 1, '0', TapeDirection::RIGHT);
    tm.add_transition(1, 'Y', 1, 'Y', TapeDirection::RIGHT);
    tm.add_transition(1, '1', 2, 'Y', TapeDirection::LEFT);

    tm.add_transition(2, '0', 2, '0', TapeDirection::LEFT);
    tm.add_transition(2, 'Y', 2, 'Y', TapeDirection::LEFT);
    tm.add_transition(2, 'X', 0, 'X', TapeDirection::RIGHT);

    tm.add_transition(3, 'Y', 3, 'Y', TapeDirection::RIGHT);
    tm.add_transition(3, '_', 4, '_', TapeDirection::STAY);

    vector<string> tests = {"", "01", "0011", "000111", "0110", "10", "001"};

    for (const string& s : tests)
    {
        auto result = tm.run(s);

        cout << "\"" << s << "\": " << (result.accepted ? "accept" : "reject")
             << " (" << result.steps << " steps)" << endl;
    }

    return 0;
}
