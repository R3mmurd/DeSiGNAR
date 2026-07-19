/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file llparser.hpp
    @brief LLParser: builds an LL(1) predictive parsing table from a
    Grammar's FIRST/FOLLOW sets (grammar.hpp) and drives a table-driven
    parse over a token stream (lexer.hpp's Token), producing a parse
    tree — the piece FIRST/FOLLOW were computed for in the first place,
    completing the lex -> parse front-end pipeline. Like lrparser.hpp's
    LR family, LLParser::parse() can also drive an `ASTBuilder` functor
    live during the parse instead of only ever building the generic
    ParseTreeNode below, so a caller who wants a real, typed AST straight
    out of a top-down parse (rather than a parse tree to simplify
    afterwards) doesn't have to switch to a bottom-up parser just to get
    that — see LLParser's own class comment for why driving an
    ASTBuilder top-down needs a slightly different technique than
    lrparser.hpp's naturally bottom-up stack does.
    @ingroup Compilers
*/

#pragma once

#include <stdexcept>
#include <string>
#include <utility>

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

    /** The default ASTBuilder: reduces to a generic parse tree, reusing
        ParseTreeNode/ParseNodeInfo above rather than inventing a second
        tree type — a caller who just wants what LLParser::parse()
        already gives them gets exactly that, for free, and
        lrparser.hpp's LR family reuses this same builder (via its own
        `#include <llparser.hpp>`) so a caller switching from one parser
        flavor to another for the same grammar can get the same generic
        tree shape back from either. Also models the `destroy(NodeType&)`
        half of the ASTBuilder concept (see the comment above LLParser
        and LRParserBase::parse() in lrparser.hpp): every custom builder
        must supply one too, even if a no-op, so a driver can clean up
        already-built nodes still sitting on its internal stack(s) if a
        syntax error throws partway through a parse — a plain-value
        NodeType (e.g. a `real_t` produced by an evaluating builder) needs
        nothing done, but a heap-allocated one like this builder's own
        `ParseTreeNode*` would otherwise leak. */
    struct DefaultLRTreeBuilder
    {
        using NodeType = ParseTreeNode*;

        NodeType make_leaf(const Token& t) const
        {
            return new ParseTreeNode(ParseNodeInfo{t.name, t.lexeme});
        }

        NodeType reduce(const Grammar::Symbol& lhs, const Grammar::Sequence&,
                        DynArray<NodeType>&& children) const
        {
            ParseTreeNode* node = new ParseTreeNode(ParseNodeInfo{lhs, ""});

            for (NodeType c : children)
            {
                node->append_child(c);
            }

            return node;
        }

        void destroy(NodeType& node) const
        {
            ParseTreeNode::destroy_tree(node);
        }
    };

    /** Builds its LL(1) table once at construction time (via `grammar`'s
        own compute_first()/compute_follow()) and can then parse()
        any number of token streams against it.

        Like LRParserBase in lrparser.hpp, `parse()` can drive an
        `ASTBuilder` functor live during the parse (see the templated
        `parse()` overload's own comment for the exact concept:
        `NodeType`, `make_leaf()`, `reduce()`, `destroy()`) instead of
        only ever producing a generic ParseTreeNode. The two parsers
        need genuinely different plumbing to do it, though, because of
        the direction each one builds in: an LR parser's control stack
        already *is* a bottom-up construction — by the time something
        is popped off it to be reduced, every symbol of that
        production's right-hand side has already been shifted or
        reduced onto the node stack in order, so LRParserBase::parse()
        just pops `rhs.size()` already-built NodeTypes straight off it.
        LLParser's control stack runs the other way: predicting
        `A -> X Y Z` for a nonterminal `A` pushes `X`, `Y`, `Z` so they
        get *matched against input* in that order, but none of them has
        been "built" yet — the plain ParseTreeNode version below
        sidesteps this entirely by pre-allocating each child node before
        descending into it and filling it in along the way, which works
        because ParseTreeNode is mutable and cheap to allocate as an
        empty shell. An arbitrary ASTBuilder::NodeType is neither (it
        might be a plain `real_t`, with no "empty shell" state at all,
        and building it is exactly the point of calling `reduce()`), so
        there is nothing to pre-allocate. Instead, a second "semantic
        stack" of NodeTypes accumulates values as terminals are matched
        (via `make_leaf`) and nonterminals fully expand (via nested
        `reduce` calls), and the control stack carries an extra kind of
        entry beyond "a symbol still waiting to be matched/expanded": a
        reduce marker carrying `(lhs, rhs)`, pushed *underneath* `X Y Z`
        when `A` is predicted (i.e. popped only after all three have
        been fully processed) so that by the time it is popped, exactly
        `rhs.size()` values — one per symbol of `X Y Z`, each already
        built bottom-up by its own nested expansion — are sitting on top
        of the semantic stack, ready to be popped off and handed to
        `builder.reduce(lhs, rhs, ...)`; the result is pushed back onto
        the semantic stack in `A`'s place, exactly mirroring what the LR
        driver gets for free from its stack's natural shape. When the
        control stack finally empties (the start symbol's own reduce
        marker having fired), the one value left on the semantic stack
        is the whole parse's result. */
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
            `demo-grammar.cpp`'s bare `"+"`/`"*"`. Delegates to the
            three-argument `parse()` overload below with
            DefaultLRTreeBuilder, so this keeps returning exactly the
            same generic ParseTreeNode it always has. */
        ParseTreeNode* parse(const DynArray<Token>& tokens) const
        {
            return parse(tokens, [](const Token& t) { return t.name; },
                        DefaultLRTreeBuilder());
        }

        template <class TokenSymbolFn>
        ParseTreeNode* parse(const DynArray<Token>& tokens,
                            TokenSymbolFn&& token_symbol) const
        {
            return parse(tokens, std::forward<TokenSymbolFn>(token_symbol),
                        DefaultLRTreeBuilder());
        }

        /** `ASTBuilder`: a type exposing `using NodeType = ...;` plus
            `NodeType make_leaf(const Token&) const` (called for every
            terminal matched against the input), `NodeType reduce(const
            Grammar::Symbol& lhs, const Grammar::Sequence& rhs,
            DynArray<NodeType>&& children) const` (called once per
            nonterminal fully expanded, `children` already in
            left-to-right order and `rhs` the exact production the LL(1)
            table selected — letting the builder tell apart two
            productions of the same `lhs` that happen to produce the
            same number of children, the same reason lrparser.hpp's
            `ASTBuilder::reduce()` takes `rhs` too), and `void
            destroy(NodeType&) const` for the error path (see the class
            comment above for the "semantic stack" mechanism this
            drives, and LRParserBase::parse()'s own comment in
            lrparser.hpp for why `destroy()` exists at all). */
        template <class TokenSymbolFn, class ASTBuilder>
        typename std::decay_t<ASTBuilder>::NodeType
        parse(const DynArray<Token>& tokens, TokenSymbolFn&& token_symbol,
              ASTBuilder&& builder) const
        {
            using NodeType = typename std::decay_t<ASTBuilder>::NodeType;

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

            /** One control-stack entry: either a single grammar symbol
                still waiting to be matched (a terminal) or expanded (a
                nonterminal), or a reduce marker recording the `(lhs,
                rhs)` of a nonterminal whose entire right-hand side has
                already been pushed above it — see the class comment
                above for why LL needs this second kind of entry that
                LR's naturally bottom-up stack never has to. */
            struct ControlEntry
            {
                bool is_reduce_marker;
                Grammar::Symbol symbol; // meaningful iff !is_reduce_marker
                Grammar::Symbol lhs;    // meaningful iff is_reduce_marker
                Grammar::Sequence rhs;  // meaningful iff is_reduce_marker
            };

            DynStack<ControlEntry> control_stack;
            DynStack<NodeType> value_stack;

            control_stack.push(ControlEntry{
                false, grammar.get_start_symbol(), Grammar::Symbol(),
                Grammar::Sequence()});

            nat_t pos = 0;

            try
            {
                while (!control_stack.is_empty())
                {
                    ControlEntry top = control_stack.pop();

                    if (top.is_reduce_marker)
                    {
                        DynArray<NodeType> reversed_children;

                        for (nat_t i = 0; i < top.rhs.size(); ++i)
                        {
                            reversed_children.append(value_stack.pop());
                        }

                        DynArray<NodeType> children;

                        for (nat_t i = reversed_children.size(); i-- > 0;)
                        {
                            children.append(std::move(reversed_children[i]));
                        }

                        value_stack.push(builder.reduce(
                            top.lhs, top.rhs, std::move(children)));
                        continue;
                    }

                    Grammar::Symbol lookahead = current_symbol(pos);

                    if (grammar.is_terminal(top.symbol))
                    {
                        if (top.symbol != lookahead)
                        {
                            throw std::runtime_error(
                                "LLParser: unexpected token '" + lookahead +
                                "' at position " +
                                std::to_string(current_position(pos)) +
                                " (expected '" + top.symbol + "')");
                        }

                        value_stack.push(builder.make_leaf(tokens[pos]));
                        ++pos;
                        continue;
                    }

                    const HashMap<Grammar::Symbol, Grammar::Sequence>* row =
                        table.search(top.symbol);
                    const Grammar::Sequence* rhs =
                        row != nullptr ? row->search(lookahead) : nullptr;

                    if (rhs == nullptr)
                    {
                        throw std::runtime_error(
                            "LLParser: unexpected token '" + lookahead +
                            "' at position " +
                            std::to_string(current_position(pos)) +
                            " while parsing '" + top.symbol + "'");
                    }

                    control_stack.push(
                        ControlEntry{true, Grammar::Symbol(), top.symbol,
                                    *rhs});

                    for (nat_t i = rhs->size(); i-- > 0;)
                    {
                        control_stack.push(ControlEntry{
                            false, (*rhs)[i], Grammar::Symbol(),
                            Grammar::Sequence()});
                    }
                }

                if (current_symbol(pos) != Grammar::END_OF_INPUT)
                {
                    throw std::runtime_error(
                        "LLParser: expected end of input at position " +
                        std::to_string(current_position(pos)) + ", found '" +
                        current_symbol(pos) + "'");
                }

                return value_stack.pop();
            }
            catch (...)
            {
                while (!value_stack.is_empty())
                {
                    NodeType node = value_stack.pop();
                    builder.destroy(node);
                }

                throw;
            }
        }
    };

} // end namespace Designar
