/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <lrparser.hpp>

using namespace Designar;

namespace
{
    bool close(real_t a, real_t b, real_t eps = 1e-9)
    {
        return std::abs(a - b) < eps;
    }

    DynArray<Token> filter_ws(const DynArray<Token>& all)
    {
        DynArray<Token> tokens;

        for (nat_t i = 0; i < all.size(); ++i)
        {
            if (all[i].name != "WS")
            {
                tokens.append(all[i]);
            }
        }

        return tokens;
    }

    /** A tiny semantic-action AST builder used to prove the parser
        "reaches the AST", not just a parse tree: leaves are either a
        numeric value (NUM tokens) or an operator tag (everything else);
        reduce() collapses a single-child production through unchanged
        and evaluates a completed `E -> E op E` / `E -> ( E )` production
        directly, live during the parse. */
    struct EvalNode
    {
        bool is_value;
        real_t value;
        std::string op;
    };

    struct EvalBuilder
    {
        using NodeType = EvalNode;

        NodeType make_leaf(const Token& t) const
        {
            if (t.name == "NUM")
            {
                return EvalNode{true, std::stod(t.lexeme), ""};
            }

            return EvalNode{false, 0, t.lexeme};
        }

        NodeType reduce(const Grammar::Symbol&, const Grammar::Sequence& rhs,
                        DynArray<NodeType>&& c) const
        {
            if (c.size() == 1)
            {
                return c[0];
            }

            if (c.size() == 3)
            {
                if (rhs[0] == "LPAREN")
                {
                    return c[1]; // E -> ( E )
                }

                real_t l = c[0].value;
                real_t r = c[2].value;
                const std::string& op = c[1].op;
                real_t result = 0;

                if (op == "+")
                {
                    result = l + r;
                }
                else if (op == "-")
                {
                    result = l - r;
                }
                else if (op == "*")
                {
                    result = l * r;
                }
                else if (op == "/")
                {
                    result = l / r;
                }

                return EvalNode{true, result, ""};
            }

            return EvalNode{true, 0, ""};
        }

        void destroy(NodeType&) const
        {
            // A plain value type — nothing to clean up.
        }
    };

    Grammar make_arithmetic_grammar()
    {
        Grammar g;
        g.set_start_symbol("E");
        g.add_production("E", Grammar::Sequence({"E", "PLUS", "E"}));
        g.add_production("E", Grammar::Sequence({"E", "MINUS", "E"}));
        g.add_production("E", Grammar::Sequence({"E", "TIMES", "E"}));
        g.add_production("E", Grammar::Sequence({"E", "DIVIDE", "E"}));
        g.add_production("E", Grammar::Sequence({"LPAREN", "E", "RPAREN"}));
        g.add_production("E", Grammar::Sequence({"NUM"}));
        g.set_precedence("PLUS", 1, Grammar::Associativity::LEFT);
        g.set_precedence("MINUS", 1, Grammar::Associativity::LEFT);
        g.set_precedence("TIMES", 2, Grammar::Associativity::LEFT);
        g.set_precedence("DIVIDE", 2, Grammar::Associativity::LEFT);
        return g;
    }

    Lexer make_arithmetic_lexer()
    {
        Lexer lexer;
        lexer.add_token("NUM",
                        "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
        lexer.add_token("PLUS", "\\+");
        lexer.add_token("MINUS", "-");
        lexer.add_token("TIMES", "\\*");
        lexer.add_token("DIVIDE", "/");
        lexer.add_token("LPAREN", "\\(");
        lexer.add_token("RPAREN", "\\)");
        lexer.add_token("WS", " ");
        return lexer;
    }
} // end anonymous namespace

int main()
{
    // A simple unambiguous, left-recursive grammar — structurally not
    // LL(1)-parseable (left recursion), but all three LR flavors must
    // accept it with zero conflicts and agree on the resulting parse
    // tree's shape.
    {
        Grammar g;
        g.set_start_symbol("E");
        g.add_production("E", Grammar::Sequence({"E", "PLUS", "T"}));
        g.add_production("E", Grammar::Sequence({"T"}));
        g.add_production("T", Grammar::Sequence({"ID"}));

        SLRParser slr(g);
        LRParser lr1(g);
        LALRParser lalr(g);

        assert(!slr.has_conflicts());
        assert(!lr1.has_conflicts());
        assert(!lalr.has_conflicts());

        Lexer lexer;
        lexer.add_token("ID", "(a|b|c|d)");
        lexer.add_token("PLUS", "\\+");
        lexer.add_token("WS", " ");

        DynArray<Token> tokens = filter_ws(lexer.tokenize("a + b + c"));

        ParseTreeNode* slr_tree = slr.parse(tokens);
        ParseTreeNode* lr1_tree = lr1.parse(tokens);
        ParseTreeNode* lalr_tree = lalr.parse(tokens);

        // Left-associative shape: ((a + b) + c) — the outermost node's
        // first child must itself be a non-leaf E (not a as a bare leaf).
        assert(slr_tree->get_key().symbol == "E");
        assert(!slr_tree->get_first_child()->is_leaf());
        assert(lr1_tree->get_key().symbol == "E");
        assert(!lr1_tree->get_first_child()->is_leaf());
        assert(lalr_tree->get_key().symbol == "E");
        assert(!lalr_tree->get_first_child()->is_leaf());

        ParseTreeNode::destroy_tree(slr_tree);
        ParseTreeNode::destroy_tree(lr1_tree);
        ParseTreeNode::destroy_tree(lalr_tree);
    }

    // The classic SLR-fails/LALR-succeeds grammar (Dragon Book):
    // S -> L = R | R ; L -> * R | id ; R -> L — FOLLOW(R) conflates
    // contexts SLR's table can't tell apart, but both LALR(1) and
    // canonical LR(1) parse it cleanly.
    {
        Grammar g;
        g.set_start_symbol("S");
        g.add_production("S", Grammar::Sequence({"L", "EQ", "R"}));
        g.add_production("S", Grammar::Sequence({"R"}));
        g.add_production("L", Grammar::Sequence({"STAR", "R"}));
        g.add_production("L", Grammar::Sequence({"ID"}));
        g.add_production("R", Grammar::Sequence({"L"}));

        SLRParser slr(g);
        LALRParser lalr(g);
        LRParser lr1(g);

        assert(slr.has_conflicts());
        assert(!lalr.has_conflicts());
        assert(!lr1.has_conflicts());

        Lexer lexer;
        lexer.add_token("ID", "(a|b|c)");
        lexer.add_token("EQ", "=");
        lexer.add_token("STAR", "\\*");
        lexer.add_token("WS", " ");

        DynArray<Token> tokens = filter_ws(lexer.tokenize("* a = b"));
        ParseTreeNode* tree = lalr.parse(tokens);
        assert(tree->get_key().symbol == "S");
        ParseTreeNode::destroy_tree(tree);
    }

    // The classic LALR-fails/LR(1)-succeeds grammar (Dragon Book):
    // S -> a A d | b B d | a B e | b A e ; A -> c ; B -> c — merging
    // same-core canonical LR(1) states introduces a reduce/reduce
    // conflict here that the unmerged canonical collection never had.
    {
        Grammar g;
        g.set_start_symbol("S");
        g.add_production("S", Grammar::Sequence({"A_", "A", "D"}));
        g.add_production("S", Grammar::Sequence({"B_", "B", "D"}));
        g.add_production("S", Grammar::Sequence({"A_", "B", "E"}));
        g.add_production("S", Grammar::Sequence({"B_", "A", "E"}));
        g.add_production("A", Grammar::Sequence({"C"}));
        g.add_production("B", Grammar::Sequence({"C"}));

        LALRParser lalr(g);
        LRParser lr1(g);

        assert(lalr.has_conflicts());
        assert(!lr1.has_conflicts());

        bool has_reduce_reduce = false;

        for (const LRConflict& conflict : lalr.get_conflicts())
        {
            if (conflict.kind == LRConflictKind::REDUCE_REDUCE)
            {
                has_reduce_reduce = true;
            }
        }

        assert(has_reduce_reduce);

        // Canonical LR(1) still parses every valid sentence correctly.
        Lexer lexer;
        lexer.add_token("A_", "a");
        lexer.add_token("B_", "b");
        lexer.add_token("C", "c");
        lexer.add_token("D", "d");
        lexer.add_token("E", "e");

        DynArray<Token> t1 = lexer.tokenize("acd");
        ParseTreeNode* tree1 = lr1.parse(t1);
        assert(tree1->get_key().symbol == "S");
        ParseTreeNode::destroy_tree(tree1);

        DynArray<Token> t2 = lexer.tokenize("bce");
        ParseTreeNode* tree2 = lr1.parse(t2);
        assert(tree2->get_key().symbol == "S");
        ParseTreeNode::destroy_tree(tree2);
    }

    // Ambiguous arithmetic grammar (E -> E+E|E*E|E-E|E/E|(E)|NUM),
    // resolved via declared precedence/associativity (not just blanket
    // shift-preference — this specifically exercises Grammar::
    // set_precedence(), verifying operator precedence *between*
    // different operators, not just one operator's own associativity):
    // AST built live via semantic actions and evaluated to the correct
    // numeric result.
    {
        Grammar g = make_arithmetic_grammar();
        LALRParser lalr(g);
        assert(!lalr.has_conflicts());

        Lexer lexer = make_arithmetic_lexer();

        auto eval = [&](const std::string& input) -> real_t
        {
            DynArray<Token> tokens = filter_ws(lexer.tokenize(input));
            EvalNode result = lalr.parse(
                tokens, [](const Token& t) { return t.name; },
                EvalBuilder());
            return result.value;
        };

        assert(close(eval("2 + 3 * 4"), 14));   // precedence: * before +
        assert(close(eval("2 * 3 + 4"), 10));   // precedence, other order
        assert(close(eval("8 - 3 - 2"), 3));    // left-associativity of -
        assert(close(eval("10 / 2 / 5"), 1));   // left-associativity of /
        assert(close(eval("2 * (3 + 4)"), 14)); // parens override precedence
    }

    // AST construction via semantic actions must agree regardless of
    // which of the three flavors drove the parse, when all three accept
    // the grammar (the simple grammar from the first case above).
    {
        Grammar g;
        g.set_start_symbol("E");
        g.add_production("E", Grammar::Sequence({"E", "PLUS", "T"}));
        g.add_production("E", Grammar::Sequence({"T"}));
        g.add_production("T", Grammar::Sequence({"NUM"}));

        struct SumBuilder
        {
            using NodeType = real_t;

            real_t make_leaf(const Token& t) const
            {
                return t.name == "NUM" ? std::stod(t.lexeme) : 0;
            }

            real_t reduce(const Grammar::Symbol&, const Grammar::Sequence&,
                         DynArray<real_t>&& c) const
            {
                if (c.size() == 1)
                {
                    return c[0];
                }

                return c[0] + c[2]; // E -> E PLUS T
            }

            void destroy(NodeType&) const
            {
                // A plain value type — nothing to clean up.
            }
        };

        Lexer lexer;
        lexer.add_token(
            "NUM", "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
        lexer.add_token("PLUS", "\\+");
        lexer.add_token("WS", " ");

        DynArray<Token> tokens = filter_ws(lexer.tokenize("1 + 2 + 3 + 4"));

        SLRParser slr(g);
        LRParser lr1(g);
        LALRParser lalr(g);

        auto run = [&](const auto& parser)
        {
            return parser.parse(
                tokens, [](const Token& t) { return t.name; }, SumBuilder());
        };

        real_t slr_result = run(slr);
        real_t lr1_result = run(lr1);
        real_t lalr_result = run(lalr);

        assert(close(slr_result, 10));
        assert(close(lr1_result, 10));
        assert(close(lalr_result, 10));
    }

    // A syntax error, caught and reported with its position — same
    // style as LLParser's own error handling.
    {
        Grammar g = make_arithmetic_grammar();
        LALRParser lalr(g);
        Lexer lexer = make_arithmetic_lexer();

        bool threw = false;

        try
        {
            DynArray<Token> tokens = filter_ws(lexer.tokenize("2 + )"));
            ParseTreeNode* tree = lalr.parse(tokens);
            ParseTreeNode::destroy_tree(tree);
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        assert(threw);
    }

    return 0;
}
