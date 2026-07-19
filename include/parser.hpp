/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file parser.hpp
    @brief Parser<NodeType>: a Bison/Flex-inspired facade over
    Grammar (grammar.hpp), Lexer (lexer.hpp), LLParser (llparser.hpp),
    and the LR family (lrparser.hpp) — one object that owns its own
    grammar and lexer, lets a caller register tokens and rules each with
    a semantic action attached (the way a .l/.y pair does), and turns a
    plain source string directly into a finished, user-defined AST via a
    single `parse()` call.

    This is emphatically a convenience layered *on top of* the four
    classes above, not a replacement for them: Grammar, Lexer, LLParser,
    and the LR family are still exactly what you want when teaching or
    experimenting with any one of those pieces in isolation (which is
    why demo-grammar.cpp, demo-lexer.cpp, demo-llparser.cpp, and
    demo-lrparser.cpp all still build a Grammar and a Lexer by hand and
    wire them together themselves). What this facade buys a caller who
    already knows they want the whole pipeline is: no separately-kept
    Grammar/Lexer for the caller to keep alive and in sync (Parser owns
    both internally — see the class comment on member declaration order
    below for exactly how that removes the dangling-reference hazard
    that owning them externally would otherwise create), one action
    attached directly to each token/rule at the point it is declared
    (instead of a callback keyed by name/shape after the fact, the way
    demo-lrparser.cpp's ExprASTBuilder has to be), and a runtime-
    selectable choice of which underlying algorithm (LL(1), SLR(1),
    canonical LR(1), or LALR(1)) drives the parse, so that choice can be
    a configuration decision instead of a recompile.
    @ingroup Compilers
*/

#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>

#include <array.hpp>
#include <set.hpp>
#include <map.hpp>
#include <grammar.hpp>
#include <lexer.hpp>
#include <llparser.hpp>
#include <lrparser.hpp>

namespace Designar
{
    /** Which underlying table-driven algorithm a Parser drives its
        parse with. Deliberately a runtime value, not a template
        parameter: Grammar/Lexer/rule/token registration all happen
        through the very same `Parser<NodeType>` object regardless of
        `kind`, so a caller can decide (or even try more than one, over
        the very same registered grammar and lexer — see
        demo-parser.cpp) which flavor to build tables with as late as
        `build()`/the first `parse()` call, without that choice
        infecting the type of the `Parser` object itself the way a
        template parameter would. The tradeoff already documented at
        length in lrparser.hpp still applies here (SLR is weakest/
        cheapest, canonical LR(1) is strongest/most expensive, LALR(1)
        is the usual practical middle ground) — this enum just lets a
        caller pick a point on that spectrum without hand-writing four
        near-identical code paths. */
    enum class ParserKind
    {
        LL,
        SLR,
        LR,
        LALR
    };

    /** One registered production's semantic action, keyed by its exact
        right-hand side — a `Parser<NodeType>` may register several
        productions for the same left-hand side (e.g. `E -> E PLUS E`
        and `E -> E TIMES E` both have lhs `E`), each needing its own
        action, so `rhs` is what disambiguates which action fires for a
        given reduce (see lrparser.hpp's ASTBuilder doc comment for why
        `reduce()` is handed `rhs` at all). A small `DynArray` of these
        per left-hand side, scanned linearly in `add_rule()`/`reduce()`,
        is deliberately not a hash-keyed structure: a teaching-sized
        grammar has at most a handful of productions per nonterminal, the
        same "small N, no premature optimization" call this library
        already makes elsewhere (e.g. LR0State's comment in
        lrparser.hpp). */
    template <class NodeType>
    struct ParserRuleAction
    {
        Grammar::Sequence rhs;
        std::function<NodeType(DynArray<NodeType>&&)> action;
    };

    /** The `ASTBuilder` a `Parser<NodeType>` drives its chosen concrete
        parser with: not a nested class of `Parser` itself, specifically
        because [temp.local] forbids a class nested (at any depth)
        inside `template <class NodeType> class X` from declaring a
        member also named `NodeType` — and the `ASTBuilder` concept
        (see lrparser.hpp/llparser.hpp) requires a member type spelled
        exactly `NodeType`. Templating this helper on its own parameter
        (named `Node` here, aliased to the public name `NodeType` below)
        sidesteps that restriction entirely, since this class's own
        template-parameter list is never lexically nested inside
        `Parser`'s. It holds *pointers* to `Parser`'s action tables
        (populated by `add_token()`/`add_rule()`) rather than a `Parser*`
        needing friendship, so it stays a small, dependency-free
        implementation detail of `Parser::parse()` below — not part of
        this library's public API. */
    template <class Node>
    class ParserActionBuilder
    {
    public:
        using NodeType = Node;

        const HashMap<Grammar::Symbol, std::function<NodeType(const Token&)>>*
            token_actions;
        const HashMap<Grammar::Symbol, DynArray<ParserRuleAction<NodeType>>>*
            rule_actions;
        const std::function<void(NodeType&)>* node_destroyer;

        NodeType make_leaf(const Token& t) const
        {
            const std::function<NodeType(const Token&)>* action =
                token_actions->search(t.name);

            if (action == nullptr)
            {
                throw std::logic_error(
                    "Parser: no action registered for token '" + t.name +
                    "' — every add_token() call must supply one, and "
                    "skip_token() tokens must never reach make_leaf() "
                    "(they should have been filtered out of the token "
                    "stream before parsing)");
            }

            return (*action)(t);
        }

        NodeType reduce(const Grammar::Symbol& lhs,
                        const Grammar::Sequence& rhs,
                        DynArray<NodeType>&& children) const
        {
            const DynArray<ParserRuleAction<NodeType>>* candidates =
                rule_actions->search(lhs);

            if (candidates != nullptr)
            {
                for (const ParserRuleAction<NodeType>& candidate :
                    *candidates)
                {
                    if (candidate.rhs.equal(rhs))
                    {
                        return candidate.action(std::move(children));
                    }
                }
            }

            throw std::logic_error(
                "Parser: no action registered for a production with "
                "left-hand side '" +
                lhs +
                "' matching the right-hand side that fired — every "
                "add_rule() call must supply one");
        }

        void destroy(NodeType& node) const
        {
            (*node_destroyer)(node);
        }
    };

    /** A Bison/Flex-style facade: register tokens (flex-style, each with
        a pattern and an action producing a leaf `NodeType`) and grammar
        rules (bison-style, each with a right-hand side and an action
        combining already-built child `NodeType`s), then call `parse()`
        to go straight from a source string to a finished AST.

        Member declaration order is load-bearing, not cosmetic: `grammar`
        and `lexer` are declared first, before the `std::unique_ptr` that
        will eventually hold whichever concrete parser (`LLParser`,
        `SLRParser`, `LRParser`, or `LALRParser`) `kind` selects. Every
        one of those concrete parsers keeps only a `const Grammar&` —
        they do not, and by design should not, own the grammar they
        parse against (see grammar.hpp/lrparser.hpp/llparser.hpp: keeping
        Grammar reusable and independently testable is the whole point
        of factoring it out on its own). Something has to own it, then,
        for a facade that hands out a ready-to-use parser with nothing
        else for the caller to keep alive — that something is this
        class. Because C++ destroys non-static data members in the
        reverse of their declaration order, `grammar`/`lexer` (declared
        first) are destroyed *last*, i.e. strictly after the
        `unique_ptr`-held concrete parser (declared later) has already
        been destroyed — so the concrete parser's internal `const
        Grammar&` never dangles for any part of its own lifetime,
        including its own destructor running. Get this order backwards
        and every parse (or even just the concrete parser's destructor,
        depending on what it touches) is a use-after-free waiting to
        happen. */
    template <class NodeType>
    class Parser
    {
        Grammar grammar;
        Lexer lexer;

        ParserKind kind;

        /** Names registered via `skip_token()` — filtered out of the
            token stream in `parse(const std::string&)` before it ever
            reaches the underlying parser, exactly the role whitespace/
            comment filtering plays by hand in demo-lrparser.cpp and
            demo-llparser.cpp today. Kept as a set of names rather than,
            say, a flag alongside each Lexer rule, since Lexer itself
            has no notion of "skip" at all (by design: Lexer only knows
            how to tokenize, filtering is a parser-level concern) and
            this class has no reason to add one there when a simple
            membership check here does the job. */
        HashSet<Grammar::Symbol> skipped_tokens;

        HashMap<Grammar::Symbol, std::function<NodeType(const Token&)>>
            token_actions;
        HashMap<Grammar::Symbol, DynArray<ParserRuleAction<NodeType>>>
            rule_actions;

        /** The one callback every usable `Parser` must supply (via the
            constructor — there is no default and no setter, precisely
            so a `Parser<NodeType>` that could actually leak on its
            error path can never be constructed): how to free an
            already-built `NodeType` still sitting on the internal
            builder's stack(s) when a syntax error throws partway
            through a parse. Mirrors `ASTBuilder::destroy()` in
            lrparser.hpp/llparser.hpp exactly, just supplied once here
            (at facade-construction time) instead of once per parser
            object there, since a `Parser<NodeType>` only ever has one
            `NodeType` in play for its whole lifetime. */
        std::function<void(NodeType&)> node_destroyer;

        bool built = false;

        std::unique_ptr<LLParser> ll_parser;
        std::unique_ptr<SLRParser> slr_parser;
        std::unique_ptr<LRParser> lr_parser;
        std::unique_ptr<LALRParser> lalr_parser;

        void check_not_built(const char* what) const
        {
            if (built)
            {
                throw std::logic_error(
                    std::string("Parser: cannot call ") + what +
                    "() after build() — rules are frozen once tables "
                    "are generated, the same way a Bison/Flex-generated "
                    "parser is generated once from a finished .y/.l "
                    "file and does not accept further grammar/lexer "
                    "changes afterwards");
            }
        }

        ParserActionBuilder<NodeType> make_builder() const
        {
            return ParserActionBuilder<NodeType>{
                &token_actions, &rule_actions, &node_destroyer};
        }

        static Grammar::Symbol token_symbol(const Token& t)
        {
            return t.name;
        }

        DynArray<Token> tokenize_and_filter(const std::string& input) const
        {
            DynArray<Token> all_tokens = lexer.tokenize(input);
            DynArray<Token> tokens;

            for (nat_t i = 0; i < all_tokens.size(); ++i)
            {
                if (skipped_tokens.search(all_tokens[i].name) == nullptr)
                {
                    tokens.append(all_tokens[i]);
                }
            }

            return tokens;
        }

    public:
        /** `node_destroyer` is mandatory (not defaulted, not settable
            later) because it is the one thing standing between a syntax
            error and a leak: every concrete parser's `parse()` already
            guarantees it calls `destroy()` on every `NodeType` still
            live on its internal stack(s) before an exception escapes
            (see LRParserBase::parse()/LLParser::parse()'s own try/catch
            blocks) — but only if the `ASTBuilder` it was handed actually
            knows how to destroy one. A `Parser<NodeType>` that let this
            be optional could silently leak on exactly the code path
            memory safety matters most on. */
        Parser(ParserKind k, std::function<void(NodeType&)> destroyer)
            : kind(k), node_destroyer(std::move(destroyer))
        {
            if (!node_destroyer)
            {
                throw std::invalid_argument(
                    "Parser: node_destroyer must not be empty — it is "
                    "the only thing that frees an already-built NodeType "
                    "still on the stack if a syntax error throws "
                    "partway through a parse");
            }
        }

        /** Registers a lexer rule named `name` (flex-style: `pattern` is
            the same regex syntax Lexer::add_token() accepts) together
            with the action that turns a matched `Token` into a leaf
            `NodeType` — the make_leaf() half of the ASTBuilder concept,
            supplied once per token up front instead of dispatched by
            hand inside one big callback the way demo-lrparser.cpp's
            ExprASTBuilder::make_leaf() has to. `name` doubles as the
            grammar terminal `add_rule()` right-hand sides refer to
            (Parser always maps a Token to a grammar symbol via its own
            `.name`, the same default convention LLParser/LRParserBase
            already use) — so a token registered here and a terminal
            spelled the same way in some `add_rule()` call are, by
            construction, the same symbol. */
        void add_token(const Grammar::Symbol& name, const std::string& pattern,
                      std::function<NodeType(const Token&)> action)
        {
            check_not_built("add_token");
            lexer.add_token(name, pattern);
            token_actions.insert(name, std::move(action));
        }

        /** Registers a lexer rule exactly like `add_token()`, but marks
            `name` to be dropped from the token stream before parsing —
            for tokens that exist only so the lexer has *something* to
            match (whitespace, comments) and that a grammar has no
            business ever seeing, the same role the hand-rolled
            "skip every WS token" loop plays in demo-lrparser.cpp and
            demo-llparser.cpp today. Takes no action (and must not: a
            skipped token can never reach `make_leaf()`, since it never
            reaches the underlying parser's token stream at all) and
            must not also be given a grammar terminal of the same name
            via `add_rule()`'s right-hand sides — nothing enforces that
            here, exactly as nothing stops a caller from writing an
            unreachable grammar production elsewhere in this library
            either. */
        void skip_token(const Grammar::Symbol& name, const std::string& pattern)
        {
            check_not_built("skip_token");
            lexer.add_token(name, pattern);
            skipped_tokens.insert(name);
        }

        /** Forwards to `grammar.set_start_symbol()` — see grammar.hpp. */
        void set_start_symbol(const Grammar::Symbol& s)
        {
            check_not_built("set_start_symbol");
            grammar.set_start_symbol(s);
        }

        /** Forwards to `grammar.set_precedence()` — see grammar.hpp.
            Only ever consulted by the LR family's table construction
            (`ParserKind::SLR`/`LR`/`LALR`); harmless, and simply never
            looked at, if `kind` is `ParserKind::LL`. */
        void set_precedence(const Grammar::Symbol& terminal, nat_t level,
                            Grammar::Associativity assoc)
        {
            check_not_built("set_precedence");
            grammar.set_precedence(terminal, level, assoc);
        }

        /** Registers `lhs -> rhs` with `grammar` and records `action` as
            the reduce() to call whenever this exact production fires —
            the bison-style half of this facade, letting a caller attach
            one semantic action per rule at the point the rule itself is
            declared, instead of writing a single `ASTBuilder::reduce()`
            that inspects `rhs` (or worse, an already-built child, the
            way an older version of demo-lrparser.cpp's ExprASTBuilder
            had to) to figure out which production just fired. */
        void add_rule(const Grammar::Symbol& lhs, const Grammar::Sequence& rhs,
                     std::function<NodeType(DynArray<NodeType>&&)> action)
        {
            check_not_built("add_rule");
            grammar.add_production(lhs, rhs);

            DynArray<ParserRuleAction<NodeType>>* bucket =
                rule_actions.search(lhs);

            if (bucket == nullptr)
            {
                bucket = rule_actions.insert(
                    lhs, DynArray<ParserRuleAction<NodeType>>());
            }

            bucket->append(ParserRuleAction<NodeType>{rhs, std::move(action)});
        }

        /** Builds the underlying concrete parser (constructing its
            ACTION/GOTO or LL(1) table right away) if it has not been
            built yet; a no-op on a second call. Calling this explicitly
            is entirely optional — `parse()` calls it lazily on first
            use — but it lets a caller fail fast (e.g. discover "this
            grammar is not LL(1)", or inspect `get_conflicts()`) right
            after finishing grammar/token registration, instead of only
            at the first `parse()` call, mirroring how a real yacc/bison
            run reports grammar problems at generation time, before any
            input is ever fed to the generated parser. Freezes further
            `add_token()`/`skip_token()`/`add_rule()`/
            `set_start_symbol()`/`set_precedence()` calls (see
            `check_not_built()`), since every one of those would leave an
            already-built table silently stale. */
        void build()
        {
            if (built)
            {
                return;
            }

            switch (kind)
            {
            case ParserKind::LL:
                ll_parser = std::make_unique<LLParser>(grammar);
                break;
            case ParserKind::SLR:
                slr_parser = std::make_unique<SLRParser>(grammar);
                break;
            case ParserKind::LR:
                lr_parser = std::make_unique<LRParser>(grammar);
                break;
            case ParserKind::LALR:
                lalr_parser = std::make_unique<LALRParser>(grammar);
                break;
            }

            built = true;
        }

        /** Tokenizes `input` with the internal lexer, drops every token
            registered via `skip_token()`, and parses what remains with
            the selected concrete parser, dispatching every `make_leaf`/
            `reduce` through the actions registered via `add_token()`/
            `add_rule()`. Builds the underlying parser first if `build()`
            has not already been called. */
        NodeType parse(const std::string& input)
        {
            build();
            return parse(tokenize_and_filter(input));
        }

        /** Parses an already-tokenized stream directly — for a caller
            composing with a separately-built Lexer, feeding in tokens
            from some other source, or testing this facade's parsing/
            action-dispatch behavior without re-tokenizing a string each
            time. Unlike `parse(const std::string&)`, this does *not*
            filter `skip_token()` names out for you: a caller supplying
            tokens directly is assumed to already know which of them
            should or shouldn't reach the grammar. */
        NodeType parse(const DynArray<Token>& tokens)
        {
            build();

            ParserActionBuilder<NodeType> builder = make_builder();

            switch (kind)
            {
            case ParserKind::LL:
                return ll_parser->parse(tokens, token_symbol, builder);
            case ParserKind::SLR:
                return slr_parser->parse(tokens, token_symbol, builder);
            case ParserKind::LR:
                return lr_parser->parse(tokens, token_symbol, builder);
            case ParserKind::LALR:
                return lalr_parser->parse(tokens, token_symbol, builder);
            }

            throw std::logic_error("Parser::parse: unknown ParserKind");
        }

        /** Forwards to the selected concrete LR-family parser's own
            `get_conflicts()`. `ParserKind::LL` has no such concept at
            all (LLParser rejects a non-LL(1) grammar outright at
            `build()` time instead of resolving a conflict and
            remembering it, see llparser.hpp), so this returns a static,
            permanently-empty array for it rather than throwing — "no
            conflicts" is simply the true answer for an LL(1) table by
            construction, and a caller who writes `if
            (!p.get_conflicts().is_empty())` after `build()` regardless
            of which `kind` it built with gets the answer it would
            expect without special-casing `ParserKind::LL` itself.
            Builds the underlying parser first if it has not already
            been built (conflicts, like everything else about the
            table, do not exist until then). */
        const DynArray<LRConflict>& get_conflicts()
        {
            build();

            switch (kind)
            {
            case ParserKind::SLR:
                return slr_parser->get_conflicts();
            case ParserKind::LR:
                return lr_parser->get_conflicts();
            case ParserKind::LALR:
                return lalr_parser->get_conflicts();
            case ParserKind::LL:
                break;
            }

            static const DynArray<LRConflict> no_conflicts;
            return no_conflicts;
        }
    };

} // end namespace Designar
