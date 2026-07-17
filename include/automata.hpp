/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file automata.hpp
    @brief Finite automata: DFA, NFA (with epsilon transitions), the
    subset construction (NFA::to_dfa()), and Thompson's construction
    (regex_to_nfa()) — the classic Theory of Computation/Compilers
    triangle "regular expression -> NFA -> DFA" made concrete and
    runnable rather than left as a paper proof.
    @ingroup Automata
*/

#pragma once

#include <array.hpp>
#include <set.hpp>
#include <map.hpp>
#include <sort.hpp>

#include <stdexcept>
#include <string>

namespace Designar
{
    /** A deterministic finite automaton over `Symbol` (a single character
        by default). Every (state, symbol) pair has at most one
        transition — `accepts()` simply rejects the moment no transition
        exists for the current symbol, since a DFA missing a transition is
        exactly a DFA with an implicit dead/reject state. */
    template <typename Symbol = char>
    class DFA
    {
    public:
        using StateId = nat_t;

    private:
        nat_t num_states;
        StateId start_state;
        HashSet<StateId> accepting;
        DynArray<HashMap<Symbol, StateId>> transitions;

    public:
        DFA(nat_t _num_states, StateId _start)
            : num_states(_num_states),
              start_state(_start),
              transitions(_num_states, HashMap<Symbol, StateId>())
        {
            if (_start >= _num_states)
            {
                throw std::domain_error("DFA: start state out of range");
            }
        }

        void add_transition(StateId from, const Symbol& sym, StateId to)
        {
            transitions[from][sym] = to;
        }

        void set_accepting(StateId s)
        {
            accepting.insert(s);
        }

        bool is_accepting(StateId s) const
        {
            return accepting.search(s) != nullptr;
        }

        nat_t get_num_states() const
        {
            return num_states;
        }

        StateId get_start_state() const
        {
            return start_state;
        }

        /** The transition from `from` on `sym`, or nullptr if none exists
            (a DFA's implicit dead state). */
        const StateId* transition(StateId from, const Symbol& sym) const
        {
            return transitions[from].search(sym);
        }

        /** Whether the whole `input` sequence drives this DFA from its
            start state to an accepting one. */
        template <class Sequence>
        bool accepts(const Sequence& input) const
        {
            StateId curr = start_state;

            for (const Symbol& sym : input)
            {
                const StateId* next = transition(curr, sym);

                if (next == nullptr)
                {
                    return false;
                }

                curr = *next;
            }

            return is_accepting(curr);
        }
    };

    /** A nondeterministic finite automaton over `Symbol`: unlike DFA,
        each (state, symbol) pair can lead to *several* states at once
        (transitions is a multimap in all but name), and `EPSILON`
        transitions let the automaton move without consuming any input
        symbol at all — both of which are exactly what make Thompson's
        construction (regex_to_nfa(), building one small NFA fragment per
        regex operator and gluing them together with epsilon transitions)
        so mechanical to build correctly, compared to trying to build a
        DFA directly. */
    template <typename Symbol = char>
    class NFA
    {
    public:
        using StateId = nat_t;

        /** Not a valid `Symbol` value for any alphabet this automaton
            accepts strings over — used as the sentinel key marking an
            epsilon transition in `transitions`. Default-constructed
            `Symbol` (`\0` for `char`) is what every alphabet this library's
            automata/lexer/regex code targets already excludes by
            convention (regular text has no embedded NUL), so it doubles as
            EPSILON without needing a variant/optional-typed transition
            table. */
        static constexpr Symbol EPSILON = Symbol();

    private:
        nat_t num_states;
        StateId start_state;
        HashSet<StateId> accepting;
        DynArray<HashMap<Symbol, DynArray<StateId>>> transitions;

    public:
        NFA() : num_states(0), start_state(0)
        {
            // empty
        }

        StateId add_state()
        {
            transitions.append(HashMap<Symbol, DynArray<StateId>>());
            return num_states++;
        }

        void add_transition(StateId from, const Symbol& sym, StateId to)
        {
            DynArray<StateId>* targets = transitions[from].search(sym);

            if (targets == nullptr)
            {
                DynArray<StateId> new_targets;
                new_targets.append(to);
                transitions[from].insert(sym, std::move(new_targets));
            }
            else
            {
                targets->append(to);
            }
        }

        void add_epsilon(StateId from, StateId to)
        {
            add_transition(from, EPSILON, to);
        }

        void set_start(StateId s)
        {
            start_state = s;
        }

        void set_accepting(StateId s)
        {
            accepting.insert(s);
        }

        StateId get_start_state() const
        {
            return start_state;
        }

        bool is_accepting(StateId s) const
        {
            return accepting.search(s) != nullptr;
        }

        nat_t get_num_states() const
        {
            return num_states;
        }

        /** Every state reachable from any state in `states` via zero or
            more epsilon transitions, `states` itself included. */
        HashSet<StateId> epsilon_closure(const HashSet<StateId>& states) const
        {
            HashSet<StateId> closure = states;
            DynArray<StateId> pending;

            for (StateId s : states)
            {
                pending.append(s);
            }

            while (!pending.is_empty())
            {
                StateId s = pending.remove_last();

                const DynArray<StateId>* targets =
                    transitions[s].search(EPSILON);

                if (targets == nullptr)
                {
                    continue;
                }

                for (nat_t i = 0; i < targets->size(); ++i)
                {
                    StateId t = (*targets)[i];

                    if (closure.search(t) == nullptr)
                    {
                        closure.insert(t);
                        pending.append(t);
                    }
                }
            }

            return closure;
        }

        /** Every state directly reachable from some state in `states` by
            consuming exactly one `sym` (no epsilon-closure applied — the
            caller almost always wants epsilon_closure(move(states, sym))
            as a pair, not move() alone). */
        DynArray<StateId> move(const HashSet<StateId>& states,
                               const Symbol& sym) const
        {
            DynArray<StateId> result;

            for (StateId s : states)
            {
                const DynArray<StateId>* targets = transitions[s].search(sym);

                if (targets == nullptr)
                {
                    continue;
                }

                for (nat_t i = 0; i < targets->size(); ++i)
                {
                    result.append((*targets)[i]);
                }
            }

            return result;
        }

        /** Every symbol this NFA has an actual (non-epsilon) transition
            on — what to_dfa() below treats as the resulting DFA's
            alphabet. */
        HashSet<Symbol> alphabet() const
        {
            HashSet<Symbol> symbols;

            for (nat_t s = 0; s < num_states; ++s)
            {
                for (auto& entry : transitions[s])
                {
                    if (!(entry.first == EPSILON))
                    {
                        symbols.insert(entry.first);
                    }
                }
            }

            return symbols;
        }

        template <class Sequence>
        bool accepts(const Sequence& input) const
        {
            HashSet<StateId> singleton;
            singleton.insert(start_state);

            HashSet<StateId> current = epsilon_closure(singleton);

            for (const Symbol& sym : input)
            {
                DynArray<StateId> moved = move(current, sym);

                HashSet<StateId> moved_set;

                for (nat_t i = 0; i < moved.size(); ++i)
                {
                    moved_set.insert(moved[i]);
                }

                current = epsilon_closure(moved_set);

                if (current.is_empty())
                {
                    return false;
                }
            }

            for (StateId s : current)
            {
                if (is_accepting(s))
                {
                    return true;
                }
            }

            return false;
        }

        /** The subset construction: each DFA state is a *set* of NFA
            states (every NFA state the automaton could simultaneously be
            in), discovered breadth-first starting from the epsilon-closure
            of the NFA's own start state. Two subsets are recognized as
            "the same DFA state" by comparing their elements as a sorted
            DynArray (this library's HashSet has no built-in set-equality
            or hashing-a-set support, and a linear scan through the
            handful of subsets a typical NFA produces is simpler than
            adding either). */
        DFA<Symbol> to_dfa() const
        {
            DynArray<DynArray<StateId>> discovered_keys;
            DynArray<HashSet<StateId>> discovered_sets;

            auto canonical = [](const HashSet<StateId>& s)
            {
                DynArray<StateId> v;

                for (StateId x : s)
                {
                    v.append(x);
                }

                inline_sort(v);
                return v;
            };

            auto find_or_add = [&](const HashSet<StateId>& s) -> nat_t
            {
                DynArray<StateId> key = canonical(s);

                for (nat_t i = 0; i < discovered_keys.size(); ++i)
                {
                    if (discovered_keys[i].equal(key))
                    {
                        return i;
                    }
                }

                discovered_keys.append(key);
                discovered_sets.append(s);
                return discovered_keys.size() - 1;
            };

            HashSet<StateId> start_singleton;
            start_singleton.insert(start_state);

            HashSet<StateId> start_set = epsilon_closure(start_singleton);
            nat_t start_dfa_state = find_or_add(start_set);

            HashSet<Symbol> alpha = alphabet();

            DynArray<HashMap<Symbol, nat_t>> dfa_transitions;
            DynArray<bool> dfa_accepting;

            DynArray<nat_t> worklist;
            worklist.append(start_dfa_state);

            HashSet<nat_t> processed;

            while (!worklist.is_empty())
            {
                nat_t curr = worklist.remove_last();

                if (processed.search(curr) != nullptr)
                {
                    continue;
                }

                processed.insert(curr);

                while (dfa_transitions.size() <= curr)
                {
                    dfa_transitions.append(HashMap<Symbol, nat_t>());
                }

                while (dfa_accepting.size() <= curr)
                {
                    dfa_accepting.append(false);
                }

                // A copy, not a reference: find_or_add() below may append to
                // discovered_sets, reallocating its backing storage and
                // invalidating any reference taken into it beforehand.
                HashSet<StateId> curr_set = discovered_sets[curr];

                bool acc = false;

                for (StateId s : curr_set)
                {
                    if (is_accepting(s))
                    {
                        acc = true;
                        break;
                    }
                }

                dfa_accepting[curr] = acc;

                for (const Symbol& sym : alpha)
                {
                    DynArray<StateId> moved = move(curr_set, sym);

                    if (moved.is_empty())
                    {
                        continue;
                    }

                    HashSet<StateId> moved_set;

                    for (nat_t i = 0; i < moved.size(); ++i)
                    {
                        moved_set.insert(moved[i]);
                    }

                    HashSet<StateId> closure = epsilon_closure(moved_set);

                    nat_t next_state = find_or_add(closure);
                    dfa_transitions[curr][sym] = next_state;

                    if (processed.search(next_state) == nullptr)
                    {
                        worklist.append(next_state);
                    }
                }
            }

            DFA<Symbol> dfa(discovered_keys.size(), start_dfa_state);

            for (nat_t i = 0; i < discovered_keys.size(); ++i)
            {
                if (dfa_accepting[i])
                {
                    dfa.set_accepting(i);
                }

                for (auto& entry : dfa_transitions[i])
                {
                    dfa.add_transition(i, entry.first, entry.second);
                }
            }

            return dfa;
        }
    };

    /** Thompson's construction: builds an NFA fragment (one start state,
        one accept state) per regex operator and glues fragments together
        with epsilon transitions, following exactly the textbook
        construction rules for concatenation, alternation (|), Kleene
        star (*), one-or-more (+), and zero-or-one (?). Supported syntax:
        literal characters, `(...)`  grouping, `|`, `*`, `+`, `?`, and
        `\\` to escape any of those as a literal. No character classes
        (`[abc]`), anchors, or repetition counts (`{m,n}`) — this is
        Thompson's construction made concrete for teaching, not a
        production regex engine. */
    class RegexToNFA
    {
        const std::string& pattern;
        nat_t pos;
        NFA<char> nfa;

        struct Fragment
        {
            NFA<char>::StateId start;
            NFA<char>::StateId accept;
        };

        char peek() const
        {
            return pos < pattern.size() ? pattern[pos] : '\0';
        }

        char advance()
        {
            return pattern[pos++];
        }

        Fragment make_literal(char c)
        {
            auto s0 = nfa.add_state();
            auto s1 = nfa.add_state();
            nfa.add_transition(s0, c, s1);
            return Fragment{s0, s1};
        }

        Fragment make_concat(const Fragment& a, const Fragment& b)
        {
            nfa.add_epsilon(a.accept, b.start);
            return Fragment{a.start, b.accept};
        }

        Fragment make_union(const Fragment& a, const Fragment& b)
        {
            auto s0 = nfa.add_state();
            auto s1 = nfa.add_state();

            nfa.add_epsilon(s0, a.start);
            nfa.add_epsilon(s0, b.start);
            nfa.add_epsilon(a.accept, s1);
            nfa.add_epsilon(b.accept, s1);

            return Fragment{s0, s1};
        }

        Fragment make_star(const Fragment& a)
        {
            auto s0 = nfa.add_state();
            auto s1 = nfa.add_state();

            nfa.add_epsilon(s0, a.start);
            nfa.add_epsilon(s0, s1);
            nfa.add_epsilon(a.accept, a.start);
            nfa.add_epsilon(a.accept, s1);

            return Fragment{s0, s1};
        }

        Fragment make_plus(const Fragment& a)
        {
            auto s1 = nfa.add_state();

            nfa.add_epsilon(a.accept, a.start);
            nfa.add_epsilon(a.accept, s1);

            return Fragment{a.start, s1};
        }

        Fragment make_optional(const Fragment& a)
        {
            nfa.add_epsilon(a.start, a.accept);
            return a;
        }

        Fragment parse_expr()
        {
            Fragment left = parse_term();

            while (peek() == '|')
            {
                advance();
                Fragment right = parse_term();
                left = make_union(left, right);
            }

            return left;
        }

        Fragment parse_term()
        {
            Fragment left = parse_factor();

            while (peek() != '\0' && peek() != '|' && peek() != ')')
            {
                Fragment right = parse_factor();
                left = make_concat(left, right);
            }

            return left;
        }

        Fragment parse_factor()
        {
            Fragment base = parse_base();

            while (peek() == '*' || peek() == '+' || peek() == '?')
            {
                char op = advance();

                if (op == '*')
                {
                    base = make_star(base);
                }
                else if (op == '+')
                {
                    base = make_plus(base);
                }
                else
                {
                    base = make_optional(base);
                }
            }

            return base;
        }

        Fragment parse_base()
        {
            if (peek() == '(')
            {
                advance();
                Fragment inner = parse_expr();

                if (peek() != ')')
                {
                    throw std::domain_error(
                        "regex_to_nfa: unbalanced parentheses");
                }

                advance();
                return inner;
            }

            if (peek() == '\0')
            {
                throw std::domain_error(
                    "regex_to_nfa: unexpected end of pattern");
            }

            char c = advance();

            if (c == '\\')
            {
                if (peek() == '\0')
                {
                    throw std::domain_error(
                        "regex_to_nfa: dangling escape at end of pattern");
                }

                c = advance();
            }

            return make_literal(c);
        }

    public:
        explicit RegexToNFA(const std::string& _pattern)
            : pattern(_pattern), pos(0)
        {
            // empty
        }

        NFA<char> build()
        {
            if (pattern.empty())
            {
                throw std::domain_error("regex_to_nfa: empty pattern");
            }

            Fragment f = parse_expr();

            if (pos != pattern.size())
            {
                throw std::domain_error(
                    "regex_to_nfa: unexpected character in pattern");
            }

            nfa.set_start(f.start);
            nfa.set_accepting(f.accept);

            return std::move(nfa);
        }
    };

    inline NFA<char> regex_to_nfa(const std::string& pattern)
    {
        RegexToNFA builder(pattern);
        return builder.build();
    }

} // end namespace Designar
