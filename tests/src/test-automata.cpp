/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <automata.hpp>

using namespace std;
using namespace Designar;

namespace
{
    // Cross-checks an NFA and its to_dfa() conversion agree on every test
    // string — the fundamental correctness property of the subset
    // construction (it must accept exactly the same language).
    void assert_nfa_dfa_agree(const NFA<char>& nfa,
                              const DynArray<string>& tests)
    {
        DFA<char> dfa = nfa.to_dfa();

        for (nat_t i = 0; i < tests.size(); ++i)
        {
            assert(nfa.accepts(tests[i]) == dfa.accepts(tests[i]));
        }
    }
} // end anonymous namespace

int main()
{
    // A hand-built DFA: accepts binary strings with an even number of 1s
    // (state 0 = even count seen so far / accepting, state 1 = odd).
    {
        DFA<char> dfa(2, 0);
        dfa.set_accepting(0);
        dfa.add_transition(0, '0', 0);
        dfa.add_transition(0, '1', 1);
        dfa.add_transition(1, '0', 1);
        dfa.add_transition(1, '1', 0);

        assert(dfa.accepts(string("")));
        assert(dfa.accepts(string("00")));
        assert(dfa.accepts(string("11")));
        assert(dfa.accepts(string("1010")));
        assert(!dfa.accepts(string("1")));
        assert(!dfa.accepts(string("111")));

        cout << "DFA: hand-built even-number-of-1s Everything ok!\n";
    }

    // A hand-built NFA with an epsilon transition, and its to_dfa().
    {
        NFA<char> nfa;
        auto s0 = nfa.add_state();
        auto s1 = nfa.add_state();
        auto s2 = nfa.add_state();

        nfa.set_start(s0);
        nfa.set_accepting(s2);

        nfa.add_transition(s0, 'a', s1);
        nfa.add_epsilon(s1, s2);         // "a" alone should be accepted
        nfa.add_transition(s2, 'b', s2); // and "ab*" too

        assert(nfa.accepts(string("a")));
        assert(nfa.accepts(string("ab")));
        assert(nfa.accepts(string("abbb")));
        assert(!nfa.accepts(string("")));
        assert(!nfa.accepts(string("b")));

        assert_nfa_dfa_agree(nfa, {string(""), string("a"), string("ab"),
                                   string("abbb"), string("b"), string("ba")});

        cout << "NFA: hand-built with epsilon + to_dfa() Everything ok!\n";
    }

    // regex_to_nfa(): literal concatenation.
    {
        NFA<char> nfa = regex_to_nfa("abc");

        assert(nfa.accepts(string("abc")));
        assert(!nfa.accepts(string("ab")));
        assert(!nfa.accepts(string("abcd")));
        assert(!nfa.accepts(string("")));

        assert_nfa_dfa_agree(
            nfa, {string("abc"), string("ab"), string("abcd"), string("")});

        cout << "regex_to_nfa: concatenation Everything ok!\n";
    }

    // regex_to_nfa(): alternation.
    {
        NFA<char> nfa = regex_to_nfa("cat|dog");

        assert(nfa.accepts(string("cat")));
        assert(nfa.accepts(string("dog")));
        assert(!nfa.accepts(string("cow")));
        assert(!nfa.accepts(string("catdog")));

        assert_nfa_dfa_agree(nfa, {string("cat"), string("dog"), string("cow"),
                                   string("catdog")});

        cout << "regex_to_nfa: alternation Everything ok!\n";
    }

    // regex_to_nfa(): Kleene star, plus, optional.
    {
        NFA<char> star = regex_to_nfa("a*");
        assert(star.accepts(string("")));
        assert(star.accepts(string("a")));
        assert(star.accepts(string("aaaa")));
        assert(!star.accepts(string("b")));
        assert_nfa_dfa_agree(
            star, {string(""), string("a"), string("aaaa"), string("b")});

        NFA<char> plus = regex_to_nfa("a+");
        assert(!plus.accepts(string("")));
        assert(plus.accepts(string("a")));
        assert(plus.accepts(string("aaaa")));
        assert_nfa_dfa_agree(plus, {string(""), string("a"), string("aaaa")});

        NFA<char> opt = regex_to_nfa("colou?r");
        assert(opt.accepts(string("color")));
        assert(opt.accepts(string("colour")));
        assert(!opt.accepts(string("colouur")));
        assert_nfa_dfa_agree(
            opt, {string("color"), string("colour"), string("colouur")});

        cout << "regex_to_nfa: *, +, ? Everything ok!\n";
    }

    // regex_to_nfa(): grouping and a more realistic combined pattern —
    // (a|b)*abb, the textbook example for Thompson's construction /
    // subset construction (Aho-Sethi-Ullman "Dragon Book").
    {
        NFA<char> nfa = regex_to_nfa("(a|b)*abb");

        assert(nfa.accepts(string("abb")));
        assert(nfa.accepts(string("aabb")));
        assert(nfa.accepts(string("babb")));
        assert(nfa.accepts(string("aaababb")));
        assert(!nfa.accepts(string("ab")));
        assert(!nfa.accepts(string("abbb")));
        assert(!nfa.accepts(string("")));

        assert_nfa_dfa_agree(nfa, {string("abb"), string("aabb"),
                                   string("babb"), string("aaababb"),
                                   string("ab"), string("abbb"), string("")});

        cout << "regex_to_nfa: grouping, (a|b)*abb Everything ok!\n";
    }

    // regex_to_nfa(): backslash-escaped literal operator characters.
    {
        NFA<char> nfa = regex_to_nfa("a\\*b");

        assert(nfa.accepts(string("a*b")));
        assert(!nfa.accepts(string("aab")));
        assert(!nfa.accepts(string("ab")));

        cout << "regex_to_nfa: escaped literal Everything ok!\n";
    }

    // regex_to_nfa(): malformed patterns must throw, not crash.
    {
        bool threw = false;

        try
        {
            regex_to_nfa("(ab");
        }
        catch (const domain_error&)
        {
            threw = true;
        }

        assert(threw);

        threw = false;

        try
        {
            regex_to_nfa("");
        }
        catch (const domain_error&)
        {
            threw = true;
        }

        assert(threw);

        cout << "regex_to_nfa: malformed patterns rejected Everything ok!\n";
    }

    cout << "Everything ok!\n";
    return 0;
}
