/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <grammar.hpp>

using namespace std;
using namespace Designar;

namespace
{
    bool set_equals(const HashSet<string>& s, const DynArray<string>& expected)
    {
        if (s.size() != expected.size())
        {
            return false;
        }

        for (nat_t i = 0; i < expected.size(); ++i)
        {
            if (s.search(expected[i]) == nullptr)
            {
                return false;
            }
        }

        return true;
    }

    /** The canonical Aho/Sethi/Ullman ("Dragon Book") expression
        grammar:
          E  -> T E'
          E' -> + T E' | eps
          T  -> F T'
          T' -> * F T' | eps
          F  -> ( E ) | id
        whose FIRST/FOLLOW sets are textbook-known exactly:
          FIRST(F) = FIRST(T) = FIRST(E) = { (, id }
          FIRST(E') = { +, eps }
          FIRST(T') = { *, eps }
          FOLLOW(E) = FOLLOW(E') = { ), $ }
          FOLLOW(T) = FOLLOW(T') = { +, ), $ }
          FOLLOW(F) = { +, *, ), $ } */
    Grammar make_expression_grammar()
    {
        Grammar g;
        g.set_start_symbol("E");

        g.add_production("E", Grammar::Sequence({"T", "E'"}));

        g.add_production("E'", Grammar::Sequence({"+", "T", "E'"}));
        g.add_production("E'", Grammar::Sequence()); // eps

        g.add_production("T", Grammar::Sequence({"F", "T'"}));

        g.add_production("T'", Grammar::Sequence({"*", "F", "T'"}));
        g.add_production("T'", Grammar::Sequence()); // eps

        g.add_production("F", Grammar::Sequence({"(", "E", ")"}));
        g.add_production("F", Grammar::Sequence({"id"}));

        return g;
    }
} // end anonymous namespace

int main()
{
    Grammar g = make_expression_grammar();

    assert(g.is_nonterminal("E"));
    assert(g.is_nonterminal("F"));
    assert(g.is_terminal("id"));
    assert(g.is_terminal("+"));
    assert(g.is_terminal("("));

    auto first = g.compute_first();

    assert(set_equals(*first.search("F"), DynArray<string>({"(", "id"})));
    assert(set_equals(*first.search("T"), DynArray<string>({"(", "id"})));
    assert(set_equals(*first.search("E"), DynArray<string>({"(", "id"})));
    assert(set_equals(*first.search("E'"),
                      DynArray<string>({"+", Grammar::EPSILON})));
    assert(set_equals(*first.search("T'"),
                      DynArray<string>({"*", Grammar::EPSILON})));

    cout << "Grammar: FIRST sets (Dragon Book expression grammar) Everything "
            "ok!\n";

    auto follow = g.compute_follow(first);

    assert(set_equals(*follow.search("E"), DynArray<string>({")", "$"})));
    assert(set_equals(*follow.search("E'"), DynArray<string>({")", "$"})));
    assert(set_equals(*follow.search("T"), DynArray<string>({"+", ")", "$"})));
    assert(set_equals(*follow.search("T'"), DynArray<string>({"+", ")", "$"})));
    assert(set_equals(*follow.search("F"),
                      DynArray<string>({"+", "*", ")", "$"})));

    cout << "Grammar: FOLLOW sets (Dragon Book expression grammar) Everything "
            "ok!\n";

    // first_of_sequence, checked directly: a sequence whose every
    // symbol is nullable is itself nullable (EPSILON is in its FIRST),
    // and a sequence starting with a non-nullable symbol stops there.
    {
        Grammar::Sequence eps_only({"E'", "T'"}); // both nullable alone
        HashSet<string> f = g.first_of_sequence(eps_only, 0, first);
        assert(set_equals(f, DynArray<string>({"+", "*", Grammar::EPSILON})));

        Grammar::Sequence starts_with_terminal({"id", "E'"});
        HashSet<string> f2 =
            g.first_of_sequence(starts_with_terminal, 0, first);
        assert(set_equals(f2, DynArray<string>({"id"})));

        Grammar::Sequence empty_seq;
        HashSet<string> f3 = g.first_of_sequence(empty_seq, 0, first);
        assert(set_equals(f3, DynArray<string>({Grammar::EPSILON})));

        cout << "Grammar: first_of_sequence direct checks Everything ok!\n";
    }

    // A second, simpler grammar (no epsilon productions at all) as a
    // sanity check that FIRST/FOLLOW work correctly even without any
    // nullable nonterminal:
    //   S -> a S b
    //   S -> c
    {
        Grammar g2;
        g2.set_start_symbol("S");
        g2.add_production("S", Grammar::Sequence({"a", "S", "b"}));
        g2.add_production("S", Grammar::Sequence({"c"}));

        auto first2 = g2.compute_first();
        assert(set_equals(*first2.search("S"), DynArray<string>({"a", "c"})));

        auto follow2 = g2.compute_follow(first2);
        assert(set_equals(*follow2.search("S"), DynArray<string>({"b", "$"})));

        cout << "Grammar: FIRST/FOLLOW without epsilon productions Everything "
                "ok!\n";
    }

    cout << "Everything ok!\n";
    return 0;
}
