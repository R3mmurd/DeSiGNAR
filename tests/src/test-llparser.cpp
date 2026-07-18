/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <llparser.hpp>

using namespace Designar;

namespace
{
    /** The classic expression grammar (same shape as demo-grammar.cpp's
        E/E'/T/T'/F), with terminals spelled to match the lexer rule
        names used below, so LLParser's default TokenSymbolFn
        (token.name) applies directly. */
    Grammar expression_grammar()
    {
        Grammar g;
        g.set_start_symbol("E");
        g.add_production("E", Grammar::Sequence({"T", "E'"}));
        g.add_production("E'", Grammar::Sequence({"PLUS", "T", "E'"}));
        g.add_production("E'", Grammar::Sequence()); // eps
        g.add_production("T", Grammar::Sequence({"F", "T'"}));
        g.add_production("T'", Grammar::Sequence({"TIMES", "F", "T'"}));
        g.add_production("T'", Grammar::Sequence()); // eps
        g.add_production("F", Grammar::Sequence({"LPAREN", "E", "RPAREN"}));
        g.add_production("F", Grammar::Sequence({"NUM"}));
        return g;
    }

    Lexer expression_lexer()
    {
        Lexer lexer;
        lexer.add_token("NUM",
                        "(0|1|2|3|4|5|6|7|8|9)(0|1|2|3|4|5|6|7|8|9)*");
        lexer.add_token("PLUS", "\\+");
        lexer.add_token("TIMES", "\\*");
        lexer.add_token("LPAREN", "\\(");
        lexer.add_token("RPAREN", "\\)");
        lexer.add_token("WS", " ");
        return lexer;
    }

    DynArray<Token> tokenize(Lexer& lexer, const std::string& input)
    {
        DynArray<Token> all = lexer.tokenize(input);
        DynArray<Token> out;

        for (nat_t i = 0; i < all.size(); ++i)
        {
            if (all[i].name != "WS")
            {
                out.append(all[i]);
            }
        }

        return out;
    }

    /** Walks the parse tree back into the flat sequence of terminal
        lexemes it was built from — a cheap way to check the tree
        actually corresponds to the input it was parsed from, without
        hand-writing the full expected tree shape for every case. */
    void collect_lexemes(ParseTreeNode* n, DynArray<std::string>& out)
    {
        if (n->is_leaf())
        {
            if (!n->get_key().lexeme.empty())
            {
                out.append(n->get_key().lexeme);
            }

            return;
        }

        ParseTreeNode* child = n->get_first_child();

        while (child != nullptr)
        {
            collect_lexemes(child, out);
            child = child->get_right_sibling();
        }
    }
} // end anonymous namespace

int main()
{
    Grammar g = expression_grammar();
    LLParser parser(g);
    Lexer lexer = expression_lexer();

    // Valid parses: the tree's root is the start symbol, and the
    // terminal lexemes collected from the tree reconstruct the
    // original (non-whitespace) input tokens in order.
    {
        DynArray<Token> tokens = tokenize(lexer, "2 + 3 * 4");
        ParseTreeNode* tree = parser.parse(tokens);

        assert(tree->get_key().symbol == "E");
        assert(tree->has_children());

        DynArray<std::string> lexemes;
        collect_lexemes(tree, lexemes);
        assert(lexemes.equal({"2", "+", "3", "*", "4"}));

        ParseTreeNode::destroy_tree(tree);
        assert(tree == nullptr);
    }

    {
        DynArray<Token> tokens = tokenize(lexer, "(2 + 3) * 4");
        ParseTreeNode* tree = parser.parse(tokens);

        DynArray<std::string> lexemes;
        collect_lexemes(tree, lexemes);
        assert(lexemes.equal({"(", "2", "+", "3", ")", "*", "4"}));

        ParseTreeNode::destroy_tree(tree);
    }

    {
        DynArray<Token> tokens = tokenize(lexer, "42");
        ParseTreeNode* tree = parser.parse(tokens);

        DynArray<std::string> lexemes;
        collect_lexemes(tree, lexemes);
        assert(lexemes.equal({"42"}));

        ParseTreeNode::destroy_tree(tree);
    }

    // Syntax errors: each of these must throw std::runtime_error,
    // reporting the position of the offending (or missing) token.
    {
        DynArray<Token> tokens = tokenize(lexer, "2 +"); // missing operand
        bool threw = false;

        try
        {
            parser.parse(tokens);
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        assert(threw);
    }

    {
        DynArray<Token> tokens = tokenize(lexer, "(2 + 3"); // unbalanced
        bool threw = false;

        try
        {
            parser.parse(tokens);
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        assert(threw);
    }

    {
        DynArray<Token> tokens = tokenize(lexer, "2 + 3 4"); // trailing
        bool threw = false;

        try
        {
            parser.parse(tokens);
        }
        catch (const std::runtime_error&)
        {
            threw = true;
        }

        assert(threw);
    }

    // A non-LL(1) grammar (two productions of A both starting with the
    // same terminal 'b', so FIRST(A) has a genuine conflict) must throw
    // at LLParser *construction* time, not later while parsing.
    {
        Grammar bad;
        bad.set_start_symbol("S");
        bad.add_production("S", Grammar::Sequence({"a", "A"}));
        bad.add_production("A", Grammar::Sequence({"b"}));
        bad.add_production("A", Grammar::Sequence({"b", "c"}));

        bool threw = false;

        try
        {
            LLParser bad_parser(bad);
            (void)bad_parser;
        }
        catch (const std::domain_error&)
        {
            threw = true;
        }

        assert(threw);
    }

    return 0;
}
