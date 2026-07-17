/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file turingmachine.hpp
    @brief TuringMachine: a single-tape Turing machine simulator — the
    Theory of Computation model automata.hpp's DFA/NFA deliberately
    stop short of, since a Turing machine's whole significance is
    exactly the extra power an unbounded, read-write tape adds over a
    finite-state read-only input.
    @ingroup Automata
*/

#pragma once

#include <array.hpp>
#include <set.hpp>
#include <map.hpp>

#include <stdexcept>

namespace Designar
{
    enum class TapeDirection
    {
        LEFT,
        RIGHT,
        STAY
    };

    /** `Symbol` is both the tape alphabet and (implicitly, via `blank`)
        how the tape represents "nothing written here yet" — the tape
        itself is unbounded in both directions, implemented as two
        DynArrays (`right`, for non-negative head positions, and `left`,
        for negative ones) that each grow — filling in with `blank` —
        only as far as the head has actually moved, rather than
        allocating a fixed-size tape up front the way a naive array-based
        implementation might. */
    template <typename Symbol = char>
    class TuringMachine
    {
    public:
        using StateId = nat_t;

        struct Transition
        {
            StateId next_state;
            Symbol write;
            TapeDirection move;
        };

        struct RunResult
        {
            bool accepted;
            nat_t steps;
            DynArray<Symbol> tape;
        };

    private:
        nat_t num_states;
        StateId start_state;
        Symbol blank;
        HashSet<StateId> accepting_states;
        HashSet<StateId> rejecting_states;
        DynArray<HashMap<Symbol, Transition>> transitions;

        Symbol read_tape(const DynArray<Symbol>& left,
                         const DynArray<Symbol>& right, int_t pos) const
        {
            if (pos >= 0)
            {
                return nat_t(pos) < right.size() ? right[nat_t(pos)] : blank;
            }

            nat_t idx = nat_t(-pos - 1);
            return idx < left.size() ? left[idx] : blank;
        }

        void write_tape(DynArray<Symbol>& left, DynArray<Symbol>& right,
                        int_t pos, const Symbol& sym) const
        {
            if (pos >= 0)
            {
                while (nat_t(pos) >= right.size())
                {
                    right.append(blank);
                }

                right[nat_t(pos)] = sym;
            }
            else
            {
                nat_t idx = nat_t(-pos - 1);

                while (idx >= left.size())
                {
                    left.append(blank);
                }

                left[idx] = sym;
            }
        }

        /** Assembles the final left-to-right tape contents from the two
            halves, trimming leading/trailing blank cells so the result
            reflects only what the machine actually wrote (or read as
            input), not however far the head happened to wander into
            still-blank territory. */
        DynArray<Symbol> assemble_tape(const DynArray<Symbol>& left,
                                       const DynArray<Symbol>& right) const
        {
            DynArray<Symbol> tape;

            for (nat_t i = left.size(); i > 0; --i)
            {
                tape.append(left[i - 1]);
            }

            for (nat_t i = 0; i < right.size(); ++i)
            {
                tape.append(right[i]);
            }

            nat_t first = 0;

            while (first < tape.size() && tape[first] == blank)
            {
                ++first;
            }

            nat_t last = tape.size();

            while (last > first && tape[last - 1] == blank)
            {
                --last;
            }

            DynArray<Symbol> trimmed;

            for (nat_t i = first; i < last; ++i)
            {
                trimmed.append(tape[i]);
            }

            return trimmed;
        }

    public:
        TuringMachine(nat_t _num_states, StateId _start, const Symbol& _blank)
            : num_states(_num_states),
              start_state(_start),
              blank(_blank),
              transitions(_num_states, HashMap<Symbol, Transition>())
        {
            if (_start >= _num_states)
            {
                throw std::domain_error(
                    "TuringMachine: start state out of range");
            }
        }

        void add_transition(StateId from, const Symbol& read, StateId to,
                            const Symbol& write, TapeDirection move)
        {
            transitions[from][read] = Transition{to, write, move};
        }

        void set_accepting(StateId s)
        {
            accepting_states.insert(s);
        }

        void set_rejecting(StateId s)
        {
            rejecting_states.insert(s);
        }

        nat_t get_num_states() const
        {
            return num_states;
        }

        /** Runs the machine on `input` until it reaches an accepting or
            rejecting state, or has no transition for the symbol currently
            under the head (an implicit reject — a "stuck" machine is not
            an accepting one). Throws if the machine still hasn't halted
            after `max_steps` — a Turing machine can genuinely loop forever
            (that undecidability is precisely the point of the model), so
            unlike DFA::accepts()/NFA::accepts(), this cannot promise to
            always return an answer; `max_steps` is this simulator's only
            way to give up gracefully instead of hanging. */
        template <class Sequence>
        RunResult run(const Sequence& input, nat_t max_steps = 1000000) const
        {
            DynArray<Symbol> left;
            DynArray<Symbol> right;

            for (const Symbol& s : input)
            {
                right.append(s);
            }

            StateId state = start_state;
            int_t head = 0;
            nat_t steps = 0;

            while (steps < max_steps)
            {
                if (accepting_states.search(state) != nullptr)
                {
                    return RunResult{true, steps, assemble_tape(left, right)};
                }

                if (rejecting_states.search(state) != nullptr)
                {
                    return RunResult{false, steps, assemble_tape(left, right)};
                }

                Symbol curr = read_tape(left, right, head);
                const Transition* t = transitions[state].search(curr);

                if (t == nullptr)
                {
                    return RunResult{false, steps, assemble_tape(left, right)};
                }

                write_tape(left, right, head, t->write);
                state = t->next_state;

                if (t->move == TapeDirection::LEFT)
                {
                    --head;
                }
                else if (t->move == TapeDirection::RIGHT)
                {
                    ++head;
                }

                ++steps;
            }

            throw std::runtime_error(
                "TuringMachine::run: exceeded max_steps without halting "
                "(the machine may not halt on this input at all)");
        }
    };

} // end namespace Designar
