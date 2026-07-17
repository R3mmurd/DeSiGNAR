/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file lexer.hpp
    @brief A DFA-based lexer: register named token patterns as
    regular expressions (reusing automata.hpp's regex_to_nfa() and
    NFA::to_dfa()), then tokenize an input string via maximal munch —
    at each position, every still-registered token's DFA is simulated
    in lockstep and the longest match wins, ties broken by whichever
    token was registered first (the same "first rule, longest match"
    convention lex/flex use), exactly what turns "a pile of regular
    expressions" into the tokenizing phase of a compiler front end.
    @ingroup Compilers
*/

#pragma once

#include <automata.hpp>

#include <stdexcept>
#include <string>
#include <vector>

namespace Designar
{
    /** One token recognized in the input: which rule matched, the
        exact substring consumed, and where it started. */
    struct Token
    {
        std::string name;
        std::string lexeme;
        nat_t position;
    };

    /** Token rules must consume at least one character — a rule whose
        pattern can match the empty string (e.g. `a*`) is accepted, but
        it is only ever considered a match once at least one character
        has been read, so tokenize() always makes forward progress and
        can never loop on a zero-length token. */
    class Lexer
    {
        struct Rule
        {
            std::string name;
            DFA<char> dfa;
        };

        // DFA<char> has no default constructor (a DFA always needs at
        // least one state), so DynArray (which resizes via `new T[]`,
        // requiring default-constructibility) cannot hold it directly —
        // std::vector, which move-constructs on growth instead, is used
        // here for the same reason graphagent.hpp uses it for
        // std::future.
        std::vector<Rule> rules;

    public:
        Lexer() = default;

        /** Registers a token named `name` matched by the regular
            expression `pattern` (the same syntax RegexToNFA/regex_to_nfa
            support: literals, `(...)`, `|`, `*`, `+`, `?`, `\` to escape).
            Rules are tried in registration order when breaking ties
            between equally-long matches, so register more specific
            patterns (e.g. a keyword like "if") before more general ones
            (e.g. a generic identifier pattern) if both could match the
            same input. */
        void add_token(const std::string& name, const std::string& pattern)
        {
            NFA<char> nfa = regex_to_nfa(pattern);
            rules.push_back(Rule{name, nfa.to_dfa()});
        }

        /** Scans `input` left to right, greedily consuming the longest
            prefix any registered rule accepts at each position (ties
            broken by registration order), and returns the resulting
            token sequence. Throws std::runtime_error at the first
            position no registered rule can consume even one character
            from — a lexical error, the same way a real lexer would
            reject an unrecognized character. */
        DynArray<Token> tokenize(const std::string& input) const
        {
            DynArray<Token> result;
            nat_t pos = 0;

            while (pos < input.size())
            {
                nat_t best_len = 0;
                nat_t best_rule = rules.size();

                for (nat_t r = 0; r < rules.size(); ++r)
                {
                    const DFA<char>& dfa = rules[r].dfa;
                    DFA<char>::StateId state = dfa.get_start_state();
                    nat_t len = 0;
                    nat_t matched_len = 0;
                    bool matched = false;

                    for (nat_t i = pos; i < input.size(); ++i)
                    {
                        const DFA<char>::StateId* next =
                            dfa.transition(state, input[i]);

                        if (next == nullptr)
                        {
                            break;
                        }

                        state = *next;
                        ++len;

                        if (dfa.is_accepting(state))
                        {
                            matched = true;
                            matched_len = len;
                        }
                    }

                    if (matched && matched_len > best_len)
                    {
                        best_len = matched_len;
                        best_rule = r;
                    }
                }

                if (best_rule == rules.size())
                {
                    throw std::runtime_error(
                        "Lexer::tokenize: no rule matches at position " +
                        std::to_string(pos) + " (character '" + input[pos] +
                        "')");
                }

                result.append(Token{rules[best_rule].name,
                                    input.substr(pos, best_len), pos});
                pos += best_len;
            }

            return result;
        }
    };

} // end namespace Designar
