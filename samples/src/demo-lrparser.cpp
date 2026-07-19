/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <lrparser.hpp>

using namespace Designar;

namespace
{
    /** A small, real AST — unlike llparser.hpp's ParseTreeNode (a
        generic parse tree every grammar symbol gets a node for), this
        one only has the two shapes an arithmetic expression actually
        needs: a number, or a binary operation over two already-built
        subtrees. Built live by ExprASTBuilder's semantic actions during
        the LALR parse below, then reused twice — once to interpret it
        (evaluate a numeric result) and once to translate it (print it
        as postfix/RPN) — the two downstream uses this whole file exists
        to enable. */
    struct ExprNode
    {
        enum Kind
        {
            NUMBER,
            BINARY_OP,
            OP_TAG // transient: only ever appears as a make_leaf() result
                   // for an operator/paren token, consumed and freed
                   // inside reduce() itself, never part of the final tree
        } kind;

        real_t value;
        std::string op;
        ExprNode* left;
        ExprNode* right;
    };

    void destroy_expr(ExprNode* n)
    {
        if (n == nullptr)
        {
            return;
        }

        destroy_expr(n->left);
        destroy_expr(n->right);
        delete n;
    }

    real_t interpret(ExprNode* n)
    {
        if (n->kind == ExprNode::NUMBER)
        {
            return n->value;
        }

        real_t l = interpret(n->left);
        real_t r = interpret(n->right);

        if (n->op == "+") return l + r;
        if (n->op == "-") return l - r;
        if (n->op == "*") return l * r;
        return l / r;
    }

    void translate_to_postfix(ExprNode* n, std::ostream& out)
    {
        if (n->kind == ExprNode::NUMBER)
        {
            out << n->value << " ";
            return;
        }

        translate_to_postfix(n->left, out);
        translate_to_postfix(n->right, out);
        out << n->op << " ";
    }

    /** The semantic-action builder driving the AST construction itself:
        make_leaf() turns a NUM token into a real ExprNode::NUMBER leaf,
        and every other token (an operator or a parenthesis) into a
        transient OP_TAG carrying just its lexeme; reduce() then either
        collapses a unit production through unchanged (`E -> NUM`),
        unwraps a parenthesized one (freeing both OP_TAG bookends), or
        builds a real ExprNode::BINARY_OP node from a completed `E -> E
        op E` (freeing the now-consumed OP_TAG in the middle). */
    struct ExprASTBuilder
    {
        using NodeType = ExprNode*;

        NodeType make_leaf(const Token& t) const
        {
            if (t.name == "NUM")
            {
                return new ExprNode{ExprNode::NUMBER, std::stod(t.lexeme),
                                    "", nullptr, nullptr};
            }

            return new ExprNode{ExprNode::OP_TAG, 0, t.lexeme, nullptr,
                                nullptr};
        }

        NodeType reduce(const Grammar::Symbol&, DynArray<NodeType>&& c) const
        {
            if (c.size() == 1)
            {
                return c[0];
            }

            // E -> ( E )
            if (c[0]->kind == ExprNode::OP_TAG && c[0]->op == "(")
            {
                delete c[0];
                delete c[2];
                return c[1];
            }

            // E -> E op E
            ExprNode* node = new ExprNode{ExprNode::BINARY_OP, 0, c[1]->op,
                                          c[0], c[2]};
            delete c[1];
            return node;
        }

        void destroy(NodeType& node) const
        {
            destroy_expr(node);
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
} // end anonymous namespace

int main()
{
    // The full front-end pipeline: Lexer tokenizes, Grammar's
    // FIRST/FOLLOW plus declared operator precedence feed LALRParser's
    // table construction, and semantic actions build a real AST live
    // during the parse — no separate simplification pass over a parse
    // tree, unlike demo-llparser.cpp.
    Grammar g = make_arithmetic_grammar();
    LALRParser parser(g);

    cout << "LALR conflicts resolved by declared precedence: "
         << parser.get_conflicts().size() << endl;

    Lexer lexer;
    lexer.add_token("NUM", "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
    lexer.add_token("PLUS", "\\+");
    lexer.add_token("MINUS", "-");
    lexer.add_token("TIMES", "\\*");
    lexer.add_token("DIVIDE", "/");
    lexer.add_token("LPAREN", "\\(");
    lexer.add_token("RPAREN", "\\)");
    lexer.add_token("WS", " ");

    std::string input = "3 + 4 * 2 - 6 / 3";
    DynArray<Token> all_tokens = lexer.tokenize(input);
    DynArray<Token> tokens;

    for (nat_t i = 0; i < all_tokens.size(); ++i)
    {
        if (all_tokens[i].name != "WS")
        {
            tokens.append(all_tokens[i]);
        }
    }

    ExprNode* ast = parser.parse(
        tokens, [](const Token& t) { return t.name; }, ExprASTBuilder());

    cout << "\nInput: " << input << endl;

    cout << "\nInterpreted (evaluated) result: " << interpret(ast) << endl;

    cout << "Translated (postfix/RPN): ";
    translate_to_postfix(ast, cout);
    cout << endl;

    destroy_expr(ast);

    // A syntax error, caught and reported with its position — the AST
    // nodes already built before the error (if any) are cleaned up via
    // ExprASTBuilder::destroy(), not leaked.
    try
    {
        DynArray<Token> bad_all = lexer.tokenize("3 + )");
        DynArray<Token> bad_tokens;

        for (nat_t i = 0; i < bad_all.size(); ++i)
        {
            if (bad_all[i].name != "WS")
            {
                bad_tokens.append(bad_all[i]);
            }
        }

        ExprNode* bad_ast = parser.parse(
            bad_tokens, [](const Token& t) { return t.name; },
            ExprASTBuilder());
        destroy_expr(bad_ast);
    }
    catch (const std::runtime_error& e)
    {
        cout << "\nParsing \"3 + )\" failed as expected: " << e.what()
             << endl;
    }

    return 0;
}
