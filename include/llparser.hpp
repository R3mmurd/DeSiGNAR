/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file llparser.hpp
    @brief LLParser: builds an LL(1) predictive parsing table from a
    Grammar's FIRST/FOLLOW sets (grammar.hpp) and drives a table-driven
    parse over a token stream (lexer.hpp's Token), producing a parse
    tree — the piece FIRST/FOLLOW were computed for in the first place,
    completing the lex -> parse front-end pipeline.
    @ingroup Compilers
*/

#pragma once

#include <stdexcept>
#include <string>

#include <array.hpp>
#include <stack.hpp>
#include <nodesdef.hpp>
#include <grammar.hpp>
#include <lexer.hpp>

namespace Designar
{
    /** One parse-tree node's payload: `symbol` is always set (which
        grammar symbol — terminal or nonterminal — this node stands
        for); `lexeme` is only meaningful on a terminal leaf (the actual
        text matched, e.g. `"42"` for a `NUM` terminal) and stays empty
        on every nonterminal node (those represent a whole subderivation,
        not a single piece of matched text). */
    struct ParseNodeInfo
    {
        Grammar::Symbol symbol;
        std::string lexeme;
    };

    /** The parse tree itself is a plain MTreeNode (nodesdef.hpp) —
        already this library's general M-ary tree node, no reason for a
        second tree type. Callers own the tree LLParser::parse() returns
        and must eventually `ParseTreeNode::destroy_tree(root)` it, the
        same ownership convention every other MTreeNode user in this
        library (e.g. test-mtreenode.cpp) already follows. */
    using ParseTreeNode = MTreeNode<ParseNodeInfo>;

    /** Builds its LL(1) table once at construction time (via `grammar`'s
        own compute_first()/compute_follow()) and can then parse()
        any number of token streams against it. */
    class LLParser
    {
        const Grammar& grammar;

        /** `table[A][a]` is the right-hand side to expand `A` into when
            the next input symbol is `a` — absent if that combination
            can never be reached by any production of `A` (a genuine
            syntax error, not a missing table entry to fill in). */
        HashMap<Grammar::Symbol, HashMap<Grammar::Symbol, Grammar::Sequence>>
            table;

        /** Records `table[nt][terminal] = rhs`, throwing if that cell is
            already occupied. Since every production is processed
            exactly once and contributes to a given cell at most once
            per production (each terminal appears once in the `HashSet`
            FIRST/FOLLOW result being iterated), the *only* way this can
            ever fire is two genuinely different productions of `nt`
            both wanting cell `[nt][terminal]` — the textbook definition
            of "this grammar is not LL(1)" — so there is no need to
            compare `rhs` against whatever's already there first. */
        void add_table_entry(const Grammar::Symbol& nt,
                             const Grammar::Symbol& terminal,
                             const Grammar::Sequence& rhs)
        {
            HashMap<Grammar::Symbol, Grammar::Sequence>* row =
                table.search(nt);

            if (row == nullptr)
            {
                row = table.insert(
                    nt, HashMap<Grammar::Symbol, Grammar::Sequence>());
            }

            if (row->search(terminal) != nullptr)
            {
                throw std::domain_error(
                    "LLParser: grammar is not LL(1) — conflict on "
                    "nonterminal '" +
                    nt + "', terminal '" + terminal + "'");
            }

            row->insert(terminal, rhs);
        }

        void build_table()
        {
            HashMap<Grammar::Symbol, HashSet<Grammar::Symbol>> first_sets =
                grammar.compute_first();
            HashMap<Grammar::Symbol, HashSet<Grammar::Symbol>> follow_sets =
                grammar.compute_follow(first_sets);

            for (const Grammar::Symbol& nt : grammar.get_nonterminals())
            {
                const DynArray<Grammar::Sequence>* rhs_list =
                    grammar.get_productions(nt);

                if (rhs_list == nullptr)
                {
                    continue;
                }

                for (nat_t r = 0; r < rhs_list->size(); ++r)
                {
                    const Grammar::Sequence& rhs = (*rhs_list)[r];
                    HashSet<Grammar::Symbol> seq_first =
                        grammar.first_of_sequence(rhs, 0, first_sets);
                    bool nullable = false;

                    for (const Grammar::Symbol& s : seq_first)
                    {
                        if (s == Grammar::EPSILON)
                        {
                            nullable = true;
                            continue;
                        }

                        add_table_entry(nt, s, rhs);
                    }

                    if (!nullable)
                    {
                        continue;
                    }

                    const HashSet<Grammar::Symbol>* follow =
                        follow_sets.search(nt);

                    if (follow == nullptr)
                    {
                        continue;
                    }

                    for (const Grammar::Symbol& s : *follow)
                    {
                        add_table_entry(nt, s, rhs);
                    }
                }
            }
        }

    public:
        /** Computes FIRST/FOLLOW and builds the LL(1) table right away
            — throws immediately if `grammar` turns out not to be LL(1),
            so a caller building a grammar interactively finds out at
            the point they hand it to this class, not later while
            parsing some unrelated input. `grammar` is kept by
            reference (not copied) and must outlive this parser. */
        explicit LLParser(const Grammar& g) : grammar(g)
        {
            build_table();
        }

        /** `TokenSymbolFn`: `Grammar::Symbol operator()(const Token&)` —
            maps a lexer token to the grammar terminal it represents.
            Defaults to the token's own `.name` (the convention this
            library's `demo-llparser.cpp` uses: lexer rule names spelled
            to match the grammar's terminal names); pass a different
            callback (e.g. one returning `token.lexeme`) for a grammar
            whose terminals are spelled as literal text instead, like
            `demo-grammar.cpp`'s bare `"+"`/`"*"`. */
        ParseTreeNode* parse(const DynArray<Token>& tokens) const
        {
            return parse(tokens, [](const Token& t) { return t.name; });
        }

        template <class TokenSymbolFn>
        ParseTreeNode* parse(const DynArray<Token>& tokens,
                            TokenSymbolFn&& token_symbol) const
        {
            auto current_symbol = [&](nat_t pos) -> Grammar::Symbol
            {
                return pos < tokens.size() ? token_symbol(tokens[pos])
                                           : Grammar::END_OF_INPUT;
            };

            auto current_position = [&](nat_t pos) -> nat_t
            {
                if (pos < tokens.size())
                {
                    return tokens[pos].position;
                }

                return tokens.is_empty() ? 0
                                         : tokens[tokens.size() - 1].position;
            };

            ParseTreeNode* root =
                new ParseTreeNode(ParseNodeInfo{grammar.get_start_symbol(),
                                                ""});
            ParseTreeNode* eof_marker =
                new ParseTreeNode(ParseNodeInfo{Grammar::END_OF_INPUT, ""});

            DynStack<ParseTreeNode*> stack;
            stack.push(eof_marker);
            stack.push(root);

            nat_t pos = 0;

            try
            {
                while (!stack.is_empty())
                {
                    ParseTreeNode* top = stack.pop();
                    const Grammar::Symbol& top_symbol =
                        top->get_key().symbol;
                    Grammar::Symbol lookahead = current_symbol(pos);

                    if (top_symbol == Grammar::END_OF_INPUT)
                    {
                        if (lookahead != Grammar::END_OF_INPUT)
                        {
                            throw std::runtime_error(
                                "LLParser: expected end of input at "
                                "position " +
                                std::to_string(current_position(pos)) +
                                ", found '" + lookahead + "'");
                        }

                        delete top;
                        break;
                    }

                    if (grammar.is_terminal(top_symbol))
                    {
                        if (top_symbol != lookahead)
                        {
                            throw std::runtime_error(
                                "LLParser: unexpected token '" + lookahead +
                                "' at position " +
                                std::to_string(current_position(pos)) +
                                " (expected '" + top_symbol + "')");
                        }

                        top->get_key().lexeme = tokens[pos].lexeme;
                        ++pos;
                        continue;
                    }

                    const HashMap<Grammar::Symbol, Grammar::Sequence>* row =
                        table.search(top_symbol);
                    const Grammar::Sequence* rhs =
                        row != nullptr ? row->search(lookahead) : nullptr;

                    if (rhs == nullptr)
                    {
                        throw std::runtime_error(
                            "LLParser: unexpected token '" + lookahead +
                            "' at position " +
                            std::to_string(current_position(pos)) +
                            " while parsing '" + top_symbol + "'");
                    }

                    DynArray<ParseTreeNode*> children;

                    for (nat_t i = 0; i < rhs->size(); ++i)
                    {
                        ParseTreeNode* child = new ParseTreeNode(
                            ParseNodeInfo{(*rhs)[i], ""});
                        top->append_child(child);
                        children.append(child);
                    }

                    for (nat_t i = children.size(); i-- > 0;)
                    {
                        stack.push(children[i]);
                    }
                }
            }
            catch (...)
            {
                ParseTreeNode::destroy_tree(root);
                delete eof_marker;
                throw;
            }

            return root;
        }
    };

} // end namespace Designar
