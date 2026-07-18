/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <llparser.hpp>

using namespace Designar;

namespace
{
    void print_tree(ParseTreeNode* n, nat_t depth)
    {
        for (nat_t i = 0; i < depth; ++i)
        {
            cout << "  ";
        }

        cout << n->get_key().symbol;

        if (!n->get_key().lexeme.empty())
        {
            cout << " (\"" << n->get_key().lexeme << "\")";
        }

        cout << endl;

        ParseTreeNode* child = n->get_first_child();

        while (child != nullptr)
        {
            print_tree(child, depth + 1);
            child = child->get_right_sibling();
        }
    }

    /** Evaluates the arithmetic expression a parse tree built from the
        grammar below represents — a small recursive walk over the
        derivation `E -> T E'`, `E' -> + T E' | eps`, `T -> F T'`,
        `T' -> * F T' | eps`, `F -> ( E ) | NUM`. Each function takes the
        node for its own nonterminal and, for the `E'`/`T'` "tail"
        productions, the value accumulated so far from the left. This is
        exactly the kind of thing a real front end does with a parse
        tree next (build/evaluate an AST) — not something this library
        commits to a fixed class for, since every language needs its own
        shape here. */
    real_t eval_E(ParseTreeNode* e);
    real_t eval_Eprime(ParseTreeNode* e_prime, real_t inherited);
    real_t eval_T(ParseTreeNode* t);
    real_t eval_Tprime(ParseTreeNode* t_prime, real_t inherited);
    real_t eval_F(ParseTreeNode* f);

    real_t eval_E(ParseTreeNode* e)
    {
        ParseTreeNode* t = e->get_first_child();
        ParseTreeNode* e_prime = t->get_right_sibling();
        return eval_Eprime(e_prime, eval_T(t));
    }

    real_t eval_Eprime(ParseTreeNode* e_prime, real_t inherited)
    {
        if (e_prime->is_leaf())
        {
            return inherited; // E' -> eps
        }

        ParseTreeNode* t = e_prime->get_first_child()->get_right_sibling();
        ParseTreeNode* next_e_prime = t->get_right_sibling();
        return eval_Eprime(next_e_prime, inherited + eval_T(t));
    }

    real_t eval_T(ParseTreeNode* t)
    {
        ParseTreeNode* f = t->get_first_child();
        ParseTreeNode* t_prime = f->get_right_sibling();
        return eval_Tprime(t_prime, eval_F(f));
    }

    real_t eval_Tprime(ParseTreeNode* t_prime, real_t inherited)
    {
        if (t_prime->is_leaf())
        {
            return inherited; // T' -> eps
        }

        ParseTreeNode* f = t_prime->get_first_child()->get_right_sibling();
        ParseTreeNode* next_t_prime = f->get_right_sibling();
        return eval_Tprime(next_t_prime, inherited * eval_F(f));
    }

    real_t eval_F(ParseTreeNode* f)
    {
        ParseTreeNode* first = f->get_first_child();

        if (first->get_key().symbol == "NUM")
        {
            return std::stod(first->get_key().lexeme);
        }

        return eval_E(first->get_right_sibling()); // F -> ( E )
    }
} // end anonymous namespace

int main()
{
    // The full front-end pipeline: Lexer tokenizes, Grammar's FIRST/
    // FOLLOW feed LLParser's table construction, LLParser drives a
    // predictive parse into a tree.
    Grammar g;
    g.set_start_symbol("E");
    g.add_production("E", Grammar::Sequence({"T", "E'"}));
    g.add_production("E'", Grammar::Sequence({"PLUS", "T", "E'"}));
    g.add_production("E'", Grammar::Sequence());
    g.add_production("T", Grammar::Sequence({"F", "T'"}));
    g.add_production("T'", Grammar::Sequence({"TIMES", "F", "T'"}));
    g.add_production("T'", Grammar::Sequence());
    g.add_production("F", Grammar::Sequence({"LPAREN", "E", "RPAREN"}));
    g.add_production("F", Grammar::Sequence({"NUM"}));

    LLParser parser(g);

    Lexer lexer;
    lexer.add_token("NUM", "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
    lexer.add_token("PLUS", "\\+");
    lexer.add_token("TIMES", "\\*");
    lexer.add_token("LPAREN", "\\(");
    lexer.add_token("RPAREN", "\\)");
    lexer.add_token("WS", " ");

    std::string input = "(2 + 3) * 4 + 5";
    DynArray<Token> all_tokens = lexer.tokenize(input);
    DynArray<Token> tokens;

    for (nat_t i = 0; i < all_tokens.size(); ++i)
    {
        if (all_tokens[i].name != "WS")
        {
            tokens.append(all_tokens[i]);
        }
    }

    ParseTreeNode* tree = parser.parse(tokens);

    cout << "Input: " << input << "\n\n";
    cout << "Parse tree:\n";
    print_tree(tree, 0);

    cout << "\nEvaluated result: " << eval_E(tree) << endl;

    ParseTreeNode::destroy_tree(tree);

    // A syntax error, caught and reported with its position.
    try
    {
        DynArray<Token> bad_tokens = lexer.tokenize("2 + )");
        DynArray<Token> bad_filtered;

        for (nat_t i = 0; i < bad_tokens.size(); ++i)
        {
            if (bad_tokens[i].name != "WS")
            {
                bad_filtered.append(bad_tokens[i]);
            }
        }

        ParseTreeNode* bad_tree = parser.parse(bad_filtered);
        ParseTreeNode::destroy_tree(bad_tree);
    }
    catch (const std::runtime_error& e)
    {
        cout << "\nParsing \"2 + )\" failed as expected: " << e.what()
             << endl;
    }

    return 0;
}
