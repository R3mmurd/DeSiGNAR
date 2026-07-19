/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <parser.hpp>

using namespace Designar;

namespace
{
    bool close(real_t a, real_t b, real_t eps = 1e-9)
    {
        return std::abs(a - b) < eps;
    }

    /** NodeType for most of this file: a heap-allocated `real_t`,
        counted via `live_nodes` on every allocation and every
        destruction, so a test can assert `live_nodes == 0` after a
        parse — whether it succeeded (and the caller freed the single
        result itself) or threw (and Parser::parse()'s internal
        ASTBuilder ran `destroy()` over everything still on its stack)
        — as a concrete, non-ASan-dependent check that nothing built
        along the way leaked. */
    nat_t live_nodes = 0;

    real_t* make_value(real_t v)
    {
        ++live_nodes;
        return new real_t(v);
    }

    void free_value(real_t*& n)
    {
        if (n == nullptr)
        {
            return;
        }

        --live_nodes;
        delete n;
        n = nullptr;
    }

    /** Registers a small ambiguous arithmetic grammar (same shape as
        demo-lrparser.cpp's), with every add_token()/add_rule() action
        producing a counted `real_t*`, onto `p`. `p`'s ParserKind is
        whatever the caller already constructed it with — this same
        registration sequence works for SLR, LR, and LALR alike, since
        the grammar itself is left-recursive (not LL(1)). */
    void build_arithmetic(Parser<real_t*>& p)
    {
        p.set_start_symbol("E");

        p.add_token("NUM", "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*",
                   [](const Token& t)
                   { return make_value(std::stod(t.lexeme)); });
        p.add_token("PLUS", "\\+",
                   [](const Token&) { return make_value(0); });
        p.add_token("MINUS", "-", [](const Token&) { return make_value(0); });
        p.add_token("TIMES", "\\*",
                   [](const Token&) { return make_value(0); });
        p.add_token("DIVIDE", "/",
                   [](const Token&) { return make_value(0); });
        p.add_token("LPAREN", "\\(",
                   [](const Token&) { return make_value(0); });
        p.add_token("RPAREN", "\\)",
                   [](const Token&) { return make_value(0); });
        p.skip_token("WS", " ");

        p.set_precedence("PLUS", 1, Grammar::Associativity::LEFT);
        p.set_precedence("MINUS", 1, Grammar::Associativity::LEFT);
        p.set_precedence("TIMES", 2, Grammar::Associativity::LEFT);
        p.set_precedence("DIVIDE", 2, Grammar::Associativity::LEFT);

        auto binop = [](const char* op)
        {
            return [op](DynArray<real_t*>&& c)
            {
                real_t l = *c[0];
                real_t r = *c[2];
                free_value(c[0]);
                free_value(c[1]);
                free_value(c[2]);

                if (std::string(op) == "+") return make_value(l + r);
                if (std::string(op) == "-") return make_value(l - r);
                if (std::string(op) == "*") return make_value(l * r);
                return make_value(l / r);
            };
        };

        p.add_rule("E", Grammar::Sequence({"E", "PLUS", "E"}), binop("+"));
        p.add_rule("E", Grammar::Sequence({"E", "MINUS", "E"}), binop("-"));
        p.add_rule("E", Grammar::Sequence({"E", "TIMES", "E"}), binop("*"));
        p.add_rule("E", Grammar::Sequence({"E", "DIVIDE", "E"}), binop("/"));
        p.add_rule("E", Grammar::Sequence({"LPAREN", "E", "RPAREN"}),
                  [](DynArray<real_t*>&& c)
                  {
                      real_t* inner = c[1];
                      free_value(c[0]);
                      free_value(c[2]);
                      return inner;
                  });
        p.add_rule("E", Grammar::Sequence({"NUM"}),
                  [](DynArray<real_t*>&& c) { return c[0]; });
    }

    /** The non-left-recursive counterpart of build_arithmetic() above,
        LL(1)-parseable, over the same four operators (left-to-right,
        no explicit precedence needed since `*`/`/` sit one grammar
        level below `+`/`-` the classical way). Used to exercise
        `ParserKind::LL` through the facade. */
    void build_ll_arithmetic(Parser<real_t*>& p)
    {
        p.set_start_symbol("E");

        p.add_token("NUM", "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*",
                   [](const Token& t)
                   { return make_value(std::stod(t.lexeme)); });
        p.add_token("PLUS", "\\+",
                   [](const Token&) { return make_value(0); });
        p.add_token("TIMES", "\\*",
                   [](const Token&) { return make_value(0); });
        p.add_token("LPAREN", "\\(",
                   [](const Token&) { return make_value(0); });
        p.add_token("RPAREN", "\\)",
                   [](const Token&) { return make_value(0); });
        p.skip_token("WS", " ");

        // Eprime/Tprime fold a chain of pending operations against the
        // T/F that precedes them — see demo-parser.cpp's fold_tail()
        // for the general technique; inlined here directly since this
        // test only ever adds (never multiplies-after-adding) at the
        // top level, so a plain running total suffices.
        p.add_rule("E", Grammar::Sequence({"T", "Eprime"}),
                  [](DynArray<real_t*>&& c)
                  {
                      real_t total = *c[0];
                      free_value(c[0]);
                      total += *c[1];
                      free_value(c[1]);
                      return make_value(total);
                  });
        p.add_rule("Eprime", Grammar::Sequence({"PLUS", "T", "Eprime"}),
                  [](DynArray<real_t*>&& c)
                  {
                      real_t total = *c[1] + *c[2];
                      free_value(c[0]);
                      free_value(c[1]);
                      free_value(c[2]);
                      return make_value(total);
                  });
        p.add_rule("Eprime", Grammar::Sequence(),
                  [](DynArray<real_t*>&&) { return make_value(0); });
        p.add_rule("T", Grammar::Sequence({"F", "Tprime"}),
                  [](DynArray<real_t*>&& c)
                  {
                      real_t total = *c[0] * *c[1];
                      free_value(c[0]);
                      free_value(c[1]);
                      return make_value(total);
                  });
        p.add_rule("Tprime", Grammar::Sequence({"TIMES", "F", "Tprime"}),
                  [](DynArray<real_t*>&& c)
                  {
                      real_t total = *c[1] * *c[2];
                      free_value(c[0]);
                      free_value(c[1]);
                      free_value(c[2]);
                      return make_value(total);
                  });
        p.add_rule("Tprime", Grammar::Sequence(),
                  [](DynArray<real_t*>&&) { return make_value(1); });
        p.add_rule("F", Grammar::Sequence({"LPAREN", "E", "RPAREN"}),
                  [](DynArray<real_t*>&& c)
                  {
                      real_t* inner = c[1];
                      free_value(c[0]);
                      free_value(c[2]);
                      return inner;
                  });
        p.add_rule("F", Grammar::Sequence({"NUM"}),
                  [](DynArray<real_t*>&& c) { return c[0]; });
    }
} // end anonymous namespace

int main()
{
    // Every ParserKind this facade supports must agree on the result of
    // the same kind of input (accounting for each grammar's own shape),
    // and every parse must leave `live_nodes` back at zero once the
    // caller frees the single value it got back.
    for (ParserKind kind : {ParserKind::SLR, ParserKind::LR, ParserKind::LALR})
    {
        Parser<real_t*> parser(kind, free_value);
        build_arithmetic(parser);

        assert(parser.get_conflicts().is_empty());

        real_t* result = parser.parse("2 + 3 * 4");
        assert(close(*result, 14));
        assert(live_nodes == 1);
        free_value(result);
        assert(live_nodes == 0);

        real_t* result2 = parser.parse("2 * (3 + 4)");
        assert(close(*result2, 14));
        free_value(result2);
        assert(live_nodes == 0);
    }

    // ParserKind::LL, over the non-left-recursive counterpart grammar.
    {
        Parser<real_t*> parser(ParserKind::LL, free_value);
        build_ll_arithmetic(parser);

        // LL has no conflict concept at all; the facade documents
        // returning a permanently-empty array for it rather than
        // throwing.
        assert(parser.get_conflicts().is_empty());

        real_t* result = parser.parse("2 + 3 * 4");
        assert(close(*result, 14));
        free_value(result);
        assert(live_nodes == 0);
    }

    // skip_token() filtering: WS tokens registered via skip_token()
    // must never reach make_leaf() (registering a WS action at all
    // would be a mistake, since skip_token() takes none) — parsing an
    // input with embedded spaces must succeed exactly as if they were
    // never there.
    {
        Parser<real_t*> parser(ParserKind::LALR, free_value);
        build_arithmetic(parser);

        real_t* spaced = parser.parse("1   +   2");
        assert(close(*spaced, 3));
        free_value(spaced);

        real_t* tight = parser.parse("1+2");
        assert(close(*tight, 3));
        free_value(tight);

        assert(live_nodes == 0);
    }

    // parse(const DynArray<Token>&) overload: a caller-supplied token
    // stream, not filtered for skip_token() names by this overload (by
    // design — see parser.hpp) — passing an already-WS-free stream
    // still works the same way.
    {
        Parser<real_t*> parser(ParserKind::LALR, free_value);
        build_arithmetic(parser);

        DynArray<Token> tokens{Token{"NUM", "5", 0}, Token{"PLUS", "+", 1},
                              Token{"NUM", "6", 2}};
        real_t* result = parser.parse(tokens);
        assert(close(*result, 11));
        free_value(result);
        assert(live_nodes == 0);
    }

    // Rules/tokens/grammar declarations freeze once build() has run:
    // every mutating registration method must throw std::logic_error
    // afterwards, the same way a Bison/Flex-generated parser cannot be
    // handed more grammar once it has already been generated.
    {
        Parser<real_t*> parser(ParserKind::LALR, free_value);
        build_arithmetic(parser);
        parser.build();

        bool threw_token = false;

        try
        {
            parser.add_token("EXTRA", "z", [](const Token&)
                            { return make_value(0); });
        }
        catch (const std::logic_error&)
        {
            threw_token = true;
        }

        assert(threw_token);

        bool threw_skip = false;

        try
        {
            parser.skip_token("EXTRA_WS", "w");
        }
        catch (const std::logic_error&)
        {
            threw_skip = true;
        }

        assert(threw_skip);

        bool threw_rule = false;

        try
        {
            parser.add_rule("E", Grammar::Sequence({"NUM", "NUM"}),
                           [](DynArray<real_t*>&& c)
                           {
                               free_value(c[1]);
                               return c[0];
                           });
        }
        catch (const std::logic_error&)
        {
            threw_rule = true;
        }

        assert(threw_rule);

        bool threw_start = false;

        try
        {
            parser.set_start_symbol("E");
        }
        catch (const std::logic_error&)
        {
            threw_start = true;
        }

        assert(threw_start);

        bool threw_prec = false;

        try
        {
            parser.set_precedence("PLUS", 3, Grammar::Associativity::RIGHT);
        }
        catch (const std::logic_error&)
        {
            threw_prec = true;
        }

        assert(threw_prec);

        // The parser built before the freeze still works fine — the
        // freeze only blocks further *registration*, not parsing.
        real_t* result = parser.parse("1 + 1");
        assert(close(*result, 2));
        free_value(result);
        assert(live_nodes == 0);
    }

    // A Parser cannot be constructed without a node destroyer at all —
    // an empty std::function is rejected outright, since it is the one
    // thing standing between a syntax error and a leak.
    {
        bool threw = false;

        try
        {
            Parser<real_t*> bad_parser(ParserKind::LALR,
                                       std::function<void(real_t*&)>());
            (void)bad_parser;
        }
        catch (const std::invalid_argument&)
        {
            threw = true;
        }

        assert(threw);
    }

    // The syntax-error exception-safety path: a malformed input throws,
    // and every real_t* already built while parsing (however many that
    // turns out to be) is freed by the ASTBuilder's destroy() — driven
    // here through Parser's own internal dispatch — before the
    // exception reaches this catch, leaving live_nodes at zero with no
    // manual cleanup required from the caller at all.
    {
        Parser<real_t*> parser(ParserKind::LALR, free_value);
        build_arithmetic(parser);

        bool threw = false;

        try
        {
            real_t* bad = parser.parse("2 + )");
            free_value(bad);
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        assert(threw);
        assert(live_nodes == 0);
    }

    // Same error-path guarantee via the LL-kind parser.
    {
        Parser<real_t*> parser(ParserKind::LL, free_value);
        build_ll_arithmetic(parser);

        bool threw = false;

        try
        {
            real_t* bad = parser.parse("2 + ");
            free_value(bad);
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        assert(threw);
        assert(live_nodes == 0);
    }

    return 0;
}
