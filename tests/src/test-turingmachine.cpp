/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <turingmachine.hpp>

using namespace std;
using namespace Designar;

namespace
{
    /** The textbook Turing machine deciding L = {0^n 1^n : n >= 0}: mark
        the leftmost unmarked 0 as X, scan right past 0s/Ys to the
        matching 1 and mark it Y, scan back left to the last X, and
        repeat; once no 0s remain (only Ys), scan right confirming
        nothing but Ys until the blank. Any symbol seen where no
        transition is defined is an implicit reject — e.g. finding a 1
        before any 0 (wrong order), running off the 0-block without
        finding a matching 1 (more 0s than 1s), or finding a stray 0/1
        during the final verification scan (more 1s than 0s). */
    TuringMachine<char> make_0n1n_machine()
    {
        // States: 0 = find next 0 to mark, 1 = seek matching 1 rightward,
        // 2 = seek back left to the last X, 3 = verify only Ys remain,
        // 4 = accept.
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

        return tm;
    }
} // end anonymous namespace

int main()
{
    TuringMachine<char> tm = make_0n1n_machine();

    for (const string& s : {string(""), string("01"), string("0011"),
                            string("000111"), string("00001111")})
    {
        auto result = tm.run(s);
        assert(result.accepted);
    }

    for (const string& s :
         {string("0"), string("1"), string("10"), string("0110"), string("001"),
          string("0111"), string("00011"), string("000111000")})
    {
        auto result = tm.run(s);
        assert(!result.accepted);
    }

    cout << "TuringMachine: 0^n 1^n decider Everything ok!\n";

    // A trivial machine that just writes a fixed symbol and halts,
    // checking the tape ends up as expected and the reported step count
    // is exact.
    {
        TuringMachine<char> writer(2, 0, '_');
        writer.set_accepting(1);
        writer.add_transition(0, '_', 1, 'X', TapeDirection::STAY);

        auto result = writer.run(string(""));
        assert(result.accepted);
        assert(result.steps == 1);
        assert(result.tape.size() == 1 && result.tape[0] == 'X');

        cout << "TuringMachine: trivial writer, exact tape/step-count "
                "Everything ok!\n";
    }

    // A machine with no transition for its very first input symbol must
    // reject immediately (0 steps), not loop or crash.
    {
        TuringMachine<char> tm2(1, 0, '_');
        tm2.set_accepting(
            0); // never reached, since it rejects immediately below instead

        // Deliberately no transitions at all: any non-blank input rejects
        // on the first step.
        TuringMachine<char> empty_transitions(1, 0, '_');

        auto result = empty_transitions.run(string("a"));
        assert(!result.accepted);
        assert(result.steps == 0);

        cout << "TuringMachine: immediate reject (no matching transition) "
                "Everything ok!\n";
    }

    // A machine that never halts must be caught by max_steps rather than
    // hanging the test suite forever.
    {
        TuringMachine<char> looper(1, 0, '_');
        looper.add_transition(0, '_', 0, '_', TapeDirection::RIGHT);

        bool threw = false;

        try
        {
            looper.run(string(""), 1000);
        }
        catch (const runtime_error&)
        {
            threw = true;
        }

        assert(threw);

        cout << "TuringMachine: non-halting machine caught by max_steps "
                "Everything ok!\n";
    }

    cout << "Everything ok!\n";
    return 0;
}
