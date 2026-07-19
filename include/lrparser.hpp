/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file lrparser.hpp
    @brief The LR family of bottom-up parsers, built on top of Grammar
    (grammar.hpp): SLRParser, LRParser (canonical LR(1)), and LALRParser
    (LR(1) states merged by core) — sharing one augmented-grammar/item
    representation and one shift-reduce-goto driver (LRParserBase), so
    only *how each flavor's ACTION/GOTO table gets built* differs between
    them, the same "shared mechanism, per-algorithm differences only
    where needed" shape as this library's tree/graph algorithms. Every
    flavor here drives an `ASTBuilder` functor live during the parse —
    one callback per shift, one per reduce — so the parser can hand back
    a real, typed AST directly, suitable for driving an interpreter or a
    translator; a default builder producing a generic parse tree
    (llparser.hpp's ParseTreeNode) is provided for callers who just want
    that instead. LLParser (llparser.hpp) offers the very same
    `ASTBuilder` mechanism over its top-down table-driven parse, via a
    "semantic stack" technique fitted to LL's different stack shape —
    see llparser.hpp's own file-level and class comments for why that
    needs a different technique than the one below.
    @ingroup Compilers
*/

#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <algorithm>

#include <array.hpp>
#include <set.hpp>
#include <map.hpp>
#include <stack.hpp>
#include <grammar.hpp>
#include <lexer.hpp>
#include <llparser.hpp>

namespace Designar
{
    /** One production of the augmented grammar, indexed by position in
        LRParserBase::prods — production 0 is always the fresh
        `augmented_start -> start_symbol` rule every flavor below adds
        internally, so "the item for production 0 is complete" is exactly
        the accept condition, uniformly across all three parsers. */
    struct LRProduction
    {
        Grammar::Symbol lhs;
        Grammar::Sequence rhs;
    };

    /** An LR(0) item: production index + dot position. Two items with
        the same (prod, dot) are the same item regardless of any lookahead
        — the "core" every LALR state is merged by. */
    struct LR0Item
    {
        nat_t prod;
        nat_t dot;

        bool operator==(const LR0Item& o) const
        {
            return prod == o.prod && dot == o.dot;
        }

        bool operator<(const LR0Item& o) const
        {
            return prod != o.prod ? prod < o.prod : dot < o.dot;
        }
    };

    /** A state is a canonical (sorted, duplicate-free) set of items —
        kept as a plain DynArray rather than a hashed set, since state
        counts for a teaching-sized grammar are small enough that a
        linear "have we seen this item-set before" scan during collection
        construction is not a real cost, and it sidesteps needing a hash
        function for item types entirely (the same non-premature-
        optimization call as randomizedalgorithms.hpp's mod_pow). */
    using LR0State = DynArray<LR0Item>;

    /** An LR(1) item additionally carries a single lookahead terminal —
        the textbook representation, where a state having multiple
        lookaheads for the same (prod, dot) shows up as multiple distinct
        items sharing that core, not as one item with a lookahead set
        (that set-per-core shape is exactly LALRItem below, built by
        merging same-core LR1State items together). */
    struct LR1Item
    {
        nat_t prod;
        nat_t dot;
        Grammar::Symbol lookahead;

        bool operator==(const LR1Item& o) const
        {
            return prod == o.prod && dot == o.dot && lookahead == o.lookahead;
        }

        bool operator<(const LR1Item& o) const
        {
            if (prod != o.prod)
            {
                return prod < o.prod;
            }

            if (dot != o.dot)
            {
                return dot < o.dot;
            }

            return lookahead < o.lookahead;
        }
    };

    using LR1State = DynArray<LR1Item>;

    /** A LALR item: one entry per distinct (prod, dot) core within a
        merged state, carrying the *union* of every lookahead any
        canonical LR(1) state sharing that core had for it — the whole
        point of the LALR construction (see LALRParser). */
    struct LALRItem
    {
        nat_t prod;
        nat_t dot;
        HashSet<Grammar::Symbol> lookaheads;
    };

    using LALRState = DynArray<LALRItem>;

    enum class LRConflictKind
    {
        SHIFT_REDUCE,
        REDUCE_REDUCE
    };

    /** A recorded, *resolved* conflict — table construction never throws
        over one (unlike LLParser's non-LL(1) rejection), it resolves by
        the yacc/bison convention (shift wins over reduce; the
        lower-indexed, i.e. earlier-declared, production wins a
        reduce/reduce tie) and just remembers that it had to. */
    struct LRConflict
    {
        nat_t state;
        Grammar::Symbol terminal;
        LRConflictKind kind;

        /** The production index that did *not* win — always shift's
            implicit target for SHIFT_REDUCE (there is no "losing shift"
            to name, since shift has no production index at all), so
            this field is only meaningful for REDUCE_REDUCE. */
        nat_t losing_production;
    };

    /** Sorts `state` and removes adjacent duplicates in place — the
        canonicalization every collection-construction routine below
        needs so two independently-built item sets can be compared for
        equality by simple element-wise comparison. */
    template <typename Item>
    void canonicalize_state(DynArray<Item>& state)
    {
        std::sort(state.begin(), state.end());

        DynArray<Item> deduped;

        for (nat_t i = 0; i < state.size(); ++i)
        {
            if (deduped.is_empty() ||
                !(deduped[deduped.size() - 1] == state[i]))
            {
                deduped.append(state[i]);
            }
        }

        state = std::move(deduped);
    }

    template <typename Item>
    bool same_canonical_state(const DynArray<Item>& a, const DynArray<Item>& b)
    {
        if (a.size() != b.size())
        {
            return false;
        }

        for (nat_t i = 0; i < a.size(); ++i)
        {
            if (!(a[i] == b[i]))
            {
                return false;
            }
        }

        return true;
    }

    /** Shared groundwork + shared driver for the whole LR family:
        augmenting the grammar with a fresh start production, flattening
        every production into one indexed list, computing FIRST/FOLLOW
        once, LR(0)/LR(1) closure and canonical-collection construction
        (used respectively by SLRParser and by both LRParser and
        LALRParser), the reduce-table conflict-resolution routine every
        flavor's constructor calls, and the actual shift-reduce-goto
        parse() loop — the one piece that is *completely* independent of
        which flavor built the tables, since by the time parse() runs, a
        SLR/LALR/canonical-LR1 table looks exactly the same shape:
        `transitions[state][symbol]` for shift/goto, `reduce_table[state]
        [terminal]` for reduce, `accepting_states` for accept. */
    class LRParserBase
    {
    protected:
        const Grammar& grammar;
        Grammar::Symbol augmented_start;
        DynArray<LRProduction> prods;
        HashMap<Grammar::Symbol, DynArray<nat_t>> productions_by_lhs;
        HashMap<Grammar::Symbol, HashSet<Grammar::Symbol>> first_sets;
        HashMap<Grammar::Symbol, HashSet<Grammar::Symbol>> follow_sets;

        DynArray<HashMap<Grammar::Symbol, nat_t>> transitions;
        DynArray<HashMap<Grammar::Symbol, nat_t>> reduce_table;
        HashSet<nat_t> accepting_states;
        DynArray<LRConflict> conflicts;

        /** Per-production precedence, computed once from `grammar`'s
            optional Grammar::set_precedence() declarations: the standard
            yacc rule of "a production's precedence is that of its
            rightmost terminal" — `-1` (no entry) if that terminal (or
            the production itself, if it has no terminal at all) has no
            declared precedence, in which case build_reduce_table() falls
            back to blanket shift-preference for any conflict involving
            it. */
        DynArray<int_t> prod_prec_level;
        DynArray<Grammar::Associativity> prod_prec_assoc;

        explicit LRParserBase(const Grammar& g) : grammar(g)
        {
            augmented_start = grammar.get_start_symbol() + "'";

            while (grammar.is_nonterminal(augmented_start))
            {
                augmented_start += "'";
            }

            prods.append(LRProduction{
                augmented_start,
                Grammar::Sequence({grammar.get_start_symbol()})});
            productions_by_lhs.insert(augmented_start, DynArray<nat_t>({0}));

            for (const Grammar::Symbol& nt : grammar.get_nonterminals())
            {
                const DynArray<Grammar::Sequence>* rhs_list =
                    grammar.get_productions(nt);

                if (rhs_list == nullptr)
                {
                    continue;
                }

                DynArray<nat_t> indices;

                for (nat_t r = 0; r < rhs_list->size(); ++r)
                {
                    indices.append(prods.size());
                    prods.append(LRProduction{nt, (*rhs_list)[r]});
                }

                productions_by_lhs.insert(nt, std::move(indices));
            }

            first_sets = grammar.compute_first();
            follow_sets = grammar.compute_follow(first_sets);

            prod_prec_level = DynArray<int_t>(prods.size(), int_t(-1));
            prod_prec_assoc = DynArray<Grammar::Associativity>(
                prods.size(), Grammar::Associativity::LEFT);

            for (nat_t i = 0; i < prods.size(); ++i)
            {
                const Grammar::Sequence& rhs = prods[i].rhs;

                for (nat_t k = rhs.size(); k-- > 0;)
                {
                    if (!grammar.is_terminal(rhs[k]))
                    {
                        continue;
                    }

                    const std::pair<nat_t, Grammar::Associativity>* prec =
                        grammar.get_precedence(rhs[k]);

                    if (prec != nullptr)
                    {
                        prod_prec_level[i] = (int_t)prec->first;
                        prod_prec_assoc[i] = prec->second;
                    }

                    break;
                }
            }
        }

        /** The closure of an LR(0) item set: for every item
            `[A -> alpha . B beta]` with `B` a nonterminal, add
            `[B -> . gamma]` for each of `B`'s productions, repeating
            until a full pass adds nothing new. `state` is taken by value
            (not reference) and each item read into a local copy before
            any `append()` below, specifically so a reallocation
            triggered by growing `state` mid-loop can never leave a
            dangling reference into it — the same hazard class fixed
            elsewhere in this library (e.g. DistanceCmp's documented
            danger in graphalgorithms.hpp). */
        LR0State lr0_closure(LR0State state) const
        {
            bool changed = true;

            while (changed)
            {
                changed = false;

                for (nat_t i = 0; i < state.size(); ++i)
                {
                    const LR0Item item = state[i];
                    const LRProduction& p = prods[item.prod];

                    if (item.dot >= p.rhs.size())
                    {
                        continue;
                    }

                    const Grammar::Symbol& b = p.rhs[item.dot];

                    if (!grammar.is_nonterminal(b))
                    {
                        continue;
                    }

                    const DynArray<nat_t>* b_prods =
                        productions_by_lhs.search(b);

                    if (b_prods == nullptr)
                    {
                        continue;
                    }

                    for (nat_t k = 0; k < b_prods->size(); ++k)
                    {
                        LR0Item new_item{(*b_prods)[k], 0};
                        bool found = false;

                        for (nat_t j = 0; j < state.size(); ++j)
                        {
                            if (state[j] == new_item)
                            {
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            state.append(new_item);
                            changed = true;
                        }
                    }
                }
            }

            canonicalize_state(state);
            return state;
        }

        LR0State lr0_goto(const LR0State& state, const Grammar::Symbol& x) const
        {
            LR0State moved;

            for (const LR0Item& item : state)
            {
                const LRProduction& p = prods[item.prod];

                if (item.dot < p.rhs.size() && p.rhs[item.dot] == x)
                {
                    moved.append(LR0Item{item.prod, item.dot + 1});
                }
            }

            return lr0_closure(std::move(moved));
        }

        /** Builds the canonical LR(0) collection: starting from the
            closure of `[augmented_start -> . start_symbol]`, repeatedly
            computes GOTO on every symbol that follows a dot in some
            existing state, adding newly-discovered states until a full
            pass adds nothing new — the standard worklist-free
            construction (re-scanning every state each pass), which is
            plenty fast for a teaching-sized grammar's state count. */
        void build_lr0_collection(
            DynArray<LR0State>& states,
            DynArray<HashMap<Grammar::Symbol, nat_t>>& trans) const
        {
            states.clear();
            trans.clear();

            states.append(lr0_closure(LR0State({LR0Item{0, 0}})));
            trans.append(HashMap<Grammar::Symbol, nat_t>());

            bool changed = true;

            while (changed)
            {
                changed = false;

                for (nat_t s = 0; s < states.size(); ++s)
                {
                    HashSet<Grammar::Symbol> symbols_after_dot;

                    for (const LR0Item& item : states[s])
                    {
                        const LRProduction& p = prods[item.prod];

                        if (item.dot < p.rhs.size())
                        {
                            symbols_after_dot.insert(p.rhs[item.dot]);
                        }
                    }

                    for (const Grammar::Symbol& x : symbols_after_dot)
                    {
                        if (trans[s].search(x) != nullptr)
                        {
                            continue;
                        }

                        LR0State target = lr0_goto(states[s], x);

                        if (target.is_empty())
                        {
                            continue;
                        }

                        nat_t target_index = states.size();
                        bool found = false;

                        for (nat_t t = 0; t < states.size(); ++t)
                        {
                            if (same_canonical_state(states[t], target))
                            {
                                target_index = t;
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            states.append(std::move(target));
                            trans.append(HashMap<Grammar::Symbol, nat_t>());
                            changed = true;
                        }

                        trans[s].insert(x, target_index);
                    }
                }
            }
        }

        /** The closure of an LR(1) item set: for every item
            `[A -> alpha . B beta, a]` with `B` a nonterminal, add
            `[B -> . gamma, b]` for every `b` in FIRST(beta a) and every
            production `B -> gamma` — reusing Grammar::first_of_sequence
            directly on `beta` with `a` appended, rather than
            reimplementing FIRST-of-a-string here. */
        LR1State lr1_closure(LR1State state) const
        {
            bool changed = true;

            while (changed)
            {
                changed = false;

                for (nat_t i = 0; i < state.size(); ++i)
                {
                    const LR1Item item = state[i];
                    const LRProduction& p = prods[item.prod];

                    if (item.dot >= p.rhs.size())
                    {
                        continue;
                    }

                    const Grammar::Symbol& b = p.rhs[item.dot];

                    if (!grammar.is_nonterminal(b))
                    {
                        continue;
                    }

                    Grammar::Sequence beta_a;

                    for (nat_t k = item.dot + 1; k < p.rhs.size(); ++k)
                    {
                        beta_a.append(p.rhs[k]);
                    }

                    beta_a.append(item.lookahead);

                    HashSet<Grammar::Symbol> lookaheads =
                        grammar.first_of_sequence(beta_a, 0, first_sets);

                    const DynArray<nat_t>* b_prods =
                        productions_by_lhs.search(b);

                    if (b_prods == nullptr)
                    {
                        continue;
                    }

                    for (nat_t k = 0; k < b_prods->size(); ++k)
                    {
                        for (const Grammar::Symbol& la : lookaheads)
                        {
                            if (la == Grammar::EPSILON)
                            {
                                continue;
                            }

                            LR1Item new_item{(*b_prods)[k], 0, la};
                            bool found = false;

                            for (nat_t j = 0; j < state.size(); ++j)
                            {
                                if (state[j] == new_item)
                                {
                                    found = true;
                                    break;
                                }
                            }

                            if (!found)
                            {
                                state.append(new_item);
                                changed = true;
                            }
                        }
                    }
                }
            }

            canonicalize_state(state);
            return state;
        }

        LR1State lr1_goto(const LR1State& state, const Grammar::Symbol& x) const
        {
            LR1State moved;

            for (const LR1Item& item : state)
            {
                const LRProduction& p = prods[item.prod];

                if (item.dot < p.rhs.size() && p.rhs[item.dot] == x)
                {
                    moved.append(LR1Item{item.prod, item.dot + 1, item.lookahead});
                }
            }

            return lr1_closure(std::move(moved));
        }

        /** Builds the canonical LR(1) collection — same worklist shape as
            build_lr0_collection(), just over LR1Item/LR1State; used
            directly by LRParser and, before merging, by LALRParser. */
        void build_lr1_collection(
            DynArray<LR1State>& states,
            DynArray<HashMap<Grammar::Symbol, nat_t>>& trans) const
        {
            states.clear();
            trans.clear();

            states.append(lr1_closure(
                LR1State({LR1Item{0, 0, Grammar::END_OF_INPUT}})));
            trans.append(HashMap<Grammar::Symbol, nat_t>());

            bool changed = true;

            while (changed)
            {
                changed = false;

                for (nat_t s = 0; s < states.size(); ++s)
                {
                    HashSet<Grammar::Symbol> symbols_after_dot;

                    for (const LR1Item& item : states[s])
                    {
                        const LRProduction& p = prods[item.prod];

                        if (item.dot < p.rhs.size())
                        {
                            symbols_after_dot.insert(p.rhs[item.dot]);
                        }
                    }

                    for (const Grammar::Symbol& x : symbols_after_dot)
                    {
                        if (trans[s].search(x) != nullptr)
                        {
                            continue;
                        }

                        LR1State target = lr1_goto(states[s], x);

                        if (target.is_empty())
                        {
                            continue;
                        }

                        nat_t target_index = states.size();
                        bool found = false;

                        for (nat_t t = 0; t < states.size(); ++t)
                        {
                            if (same_canonical_state(states[t], target))
                            {
                                target_index = t;
                                found = true;
                                break;
                            }
                        }

                        if (!found)
                        {
                            states.append(std::move(target));
                            trans.append(HashMap<Grammar::Symbol, nat_t>());
                            changed = true;
                        }

                        trans[s].insert(x, target_index);
                    }
                }
            }
        }

        struct ReduceCandidate
        {
            nat_t state;
            Grammar::Symbol terminal;
            nat_t prod;
        };

        /** Resolves every candidate reduce entry against `trans`
            (already built by the caller — the *same* object as
            `this->transitions`, taken by mutable reference here
            specifically so a reduce that wins a shift/reduce conflict on
            precedence can remove the losing shift transition, see below)
            and against each other, filling `this->reduce_table` and
            `this->conflicts` — shared by all three flavors, which differ
            only in *how* they compute `candidates` (FOLLOW-based for
            SLR, item-lookahead-based for LR(1)/LALR).

            Blanket shift-preference (no precedence declared for either
            side) needs no special handling: a candidate simply never
            gets added to `reduce_table` when `trans[state]` already has
            an entry for that terminal, and the driver checks `trans`
            before `reduce_table`, so shift wins automatically. But once
            Grammar::set_precedence() lets a reduce legitimately outrank
            a *grammatically real* shift (e.g. reducing `E -> E * E`
            before shifting `+`), that shift transition still exists in
            `trans` on its own merits (nothing about the automaton
            construction knows about precedence) — so if reduce is to
            actually win at parse time, the losing shift entry has to be
            removed from `trans[state]` here, not just left for the
            reduce_table entry to coexist uselessly alongside a shift the
            driver would still try first. Safe to remove: `trans[state]`
            entries keyed by a *terminal* are shift-only (GOTO after a
            reduce always looks up a *nonterminal* key, a disjoint
            namespace from terminals), so this can never disturb GOTO. */
        void build_reduce_table(
            nat_t num_states, DynArray<HashMap<Grammar::Symbol, nat_t>>& trans,
            const DynArray<ReduceCandidate>& candidates)
        {
            reduce_table.clear();

            for (nat_t s = 0; s < num_states; ++s)
            {
                reduce_table.append(HashMap<Grammar::Symbol, nat_t>());
            }

            for (const ReduceCandidate& c : candidates)
            {
                if (trans[c.state].search(c.terminal) != nullptr)
                {
                    int_t prod_prec = prod_prec_level[c.prod];
                    const std::pair<nat_t, Grammar::Associativity>* term_prec =
                        grammar.get_precedence(c.terminal);

                    bool reduce_wins = false;
                    bool resolved_by_precedence = false;

                    if (prod_prec >= 0 && term_prec != nullptr)
                    {
                        resolved_by_precedence = true;

                        if ((nat_t)prod_prec > term_prec->first)
                        {
                            reduce_wins = true;
                        }
                        else if ((nat_t)prod_prec == term_prec->first &&
                                term_prec->second ==
                                    Grammar::Associativity::LEFT)
                        {
                            reduce_wins = true;
                        }
                    }

                    if (!reduce_wins)
                    {
                        if (!resolved_by_precedence)
                        {
                            conflicts.append(LRConflict{
                                c.state, c.terminal,
                                LRConflictKind::SHIFT_REDUCE, c.prod});
                        }

                        continue;
                    }

                    trans[c.state].remove(c.terminal);
                }

                nat_t* existing = reduce_table[c.state].search(c.terminal);

                if (existing != nullptr)
                {
                    if (c.prod < *existing)
                    {
                        conflicts.append(LRConflict{
                            c.state, c.terminal,
                            LRConflictKind::REDUCE_REDUCE, *existing});
                        *existing = c.prod;
                    }
                    else if (c.prod > *existing)
                    {
                        conflicts.append(LRConflict{
                            c.state, c.terminal,
                            LRConflictKind::REDUCE_REDUCE, c.prod});
                    }

                    continue;
                }

                reduce_table[c.state].insert(c.terminal, c.prod);
            }
        }

    public:
        const DynArray<LRConflict>& get_conflicts() const
        {
            return conflicts;
        }

        bool has_conflicts() const
        {
            return !conflicts.is_empty();
        }

        /** `TokenSymbolFn`: `Grammar::Symbol operator()(const Token&)` —
            same convention as LLParser::parse(). */
        ParseTreeNode* parse(const DynArray<Token>& tokens) const
        {
            return parse(
                tokens, [](const Token& t) { return t.name; },
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
            `NodeType make_leaf(const Token&) const` (called on every
            shift), `NodeType reduce(const Grammar::Symbol& lhs, const
            Grammar::Sequence& rhs, DynArray<NodeType>&& children) const`
            (called on every reduce, `children` already in left-to-right
            order), and `void destroy(NodeType&) const` — the yacc-style
            mechanism this whole file exists to offer: the builder is
            invoked live during the parse, so whatever `NodeType` it
            produces (a real, typed AST node — not necessarily a tree at
            all) comes back directly from this call, ready to hand to an
            interpreter or a translator. `reduce()` is given `rhs` (the
            exact right-hand side that fired, not just `lhs`) because
            `lhs` plus a child count alone often cannot tell two
            productions of the same nonterminal apart — e.g. `E -> E +
            E` and `E -> E * E` both have lhs `E` and three children;
            without `rhs` a builder is forced to inspect the *content* of
            an already-built child to recover which operator fired,
            which stops working the moment children aren't tagged with
            something to inspect (exactly the situation a facade handing
            out one callback per production, like parser.hpp's `Parser`,
            is in) — with `rhs` in hand a builder can instead dispatch on
            the `(lhs, rhs)` pair directly, unambiguously, before ever
            looking at a child. `destroy()` exists purely for the error
            path: if a syntax error throws partway through a parse,
            every `NodeType` still sitting on the internal node stack is
            passed to it before the exception propagates, so a
            heap-allocated `NodeType` (like DefaultLRTreeBuilder's
            `ParseTreeNode*`) doesn't leak — a plain-value `NodeType`
            (like a `real_t` an evaluating builder produces) can just
            make it a no-op. */
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

            DynStack<nat_t> state_stack;
            DynStack<NodeType> node_stack;
            state_stack.push(0);

            nat_t pos = 0;

            try
            {
                while (true)
                {
                    nat_t s = state_stack.top();
                    Grammar::Symbol lookahead = current_symbol(pos);

                    if (lookahead == Grammar::END_OF_INPUT &&
                        accepting_states.search(s) != nullptr)
                    {
                        return node_stack.top();
                    }

                    const nat_t* shift_target =
                        transitions[s].search(lookahead);

                    if (shift_target != nullptr)
                    {
                        node_stack.push(builder.make_leaf(tokens[pos]));
                        state_stack.push(*shift_target);
                        ++pos;
                        continue;
                    }

                    const nat_t* reduce_prod =
                        reduce_table[s].search(lookahead);

                    if (reduce_prod != nullptr)
                    {
                        const LRProduction& p = prods[*reduce_prod];

                        DynArray<NodeType> reversed_children;

                        for (nat_t i = 0; i < p.rhs.size(); ++i)
                        {
                            reversed_children.append(node_stack.pop());
                            state_stack.pop();
                        }

                        DynArray<NodeType> children;

                        for (nat_t i = reversed_children.size(); i-- > 0;)
                        {
                            children.append(std::move(reversed_children[i]));
                        }

                        NodeType reduced =
                            builder.reduce(p.lhs, p.rhs, std::move(children));

                        nat_t prev_state = state_stack.top();
                        const nat_t* goto_target =
                            transitions[prev_state].search(p.lhs);

                        if (goto_target == nullptr)
                        {
                            throw std::logic_error(
                                "LR parser: missing GOTO entry after "
                                "reducing '" +
                                p.lhs +
                                "' — the parsing table was built "
                                "inconsistently");
                        }

                        node_stack.push(std::move(reduced));
                        state_stack.push(*goto_target);
                        continue;
                    }

                    throw std::runtime_error(
                        "LR parser: unexpected token '" + lookahead +
                        "' at position " +
                        std::to_string(current_position(pos)));
                }
            }
            catch (...)
            {
                while (!node_stack.is_empty())
                {
                    NodeType node = node_stack.pop();
                    builder.destroy(node);
                }

                throw;
            }
        }
    };

    /** SLR(1): the LR(0) automaton, with every completed item
        `[A -> alpha ., ]` contributing a reduce action for every
        terminal in FOLLOW(A) — the simplest, weakest member of the
        family (rejects some unambiguous grammars LALR(1)/LR(1) accept,
        e.g. the classic `S -> L = R | R; L -> * R | id; R -> L`, since
        FOLLOW(R) conflates contexts a smarter lookahead would keep
        separate). */
    class SLRParser : public LRParserBase
    {
    public:
        explicit SLRParser(const Grammar& g) : LRParserBase(g)
        {
            DynArray<LR0State> states;
            build_lr0_collection(states, transitions);

            DynArray<ReduceCandidate> candidates;

            for (nat_t s = 0; s < states.size(); ++s)
            {
                for (const LR0Item& item : states[s])
                {
                    const LRProduction& p = prods[item.prod];

                    if (item.dot < p.rhs.size())
                    {
                        continue;
                    }

                    if (item.prod == 0)
                    {
                        accepting_states.insert(s);
                        continue;
                    }

                    const HashSet<Grammar::Symbol>* follow =
                        follow_sets.search(p.lhs);

                    if (follow == nullptr)
                    {
                        continue;
                    }

                    for (const Grammar::Symbol& t : *follow)
                    {
                        candidates.append(ReduceCandidate{s, t, item.prod});
                    }
                }
            }

            build_reduce_table(states.size(), transitions, candidates);
        }
    };

    /** Canonical LR(1): every completed item `[A -> alpha ., a]`
        contributes a reduce action for exactly its own lookahead `a` —
        the most powerful (and most expensive, state-count-wise) member
        of the family; every deterministic context-free grammar that has
        an LR parser at all has an LR(1) one. */
    class LRParser : public LRParserBase
    {
    public:
        explicit LRParser(const Grammar& g) : LRParserBase(g)
        {
            DynArray<LR1State> states;
            build_lr1_collection(states, transitions);

            DynArray<ReduceCandidate> candidates;

            for (nat_t s = 0; s < states.size(); ++s)
            {
                for (const LR1Item& item : states[s])
                {
                    const LRProduction& p = prods[item.prod];

                    if (item.dot < p.rhs.size())
                    {
                        continue;
                    }

                    if (item.prod == 0)
                    {
                        accepting_states.insert(s);
                        continue;
                    }

                    candidates.append(
                        ReduceCandidate{s, item.lookahead, item.prod});
                }
            }

            build_reduce_table(states.size(), transitions, candidates);
        }
    };

    /** LALR(1): builds the canonical LR(1) collection above, then merges
        every pair of states sharing an identical LR(0) core (the same
        set of (production, dot) pairs, ignoring lookahead) into one
        state whose items carry the *union* of the merged states'
        lookaheads — the standard "build LR(1), then merge" construction
        (Dragon Book Algorithm 4.59), traded for the more involved
        (but state-count-optimal) direct LALR construction since
        correctness, not performance, is what matters here. Same-core
        canonical LR(1) states are guaranteed to have isomorphic
        transition structure, so the merged automaton's transitions are
        well-defined by mapping every original transition's target
        through the same core-grouping. Merging can never introduce a
        *new* shift/reduce conflict (shift actions come from `transitions`
        alone, unaffected by lookahead merging) but it can introduce a
        reduce/reduce conflict that did not exist in the canonical LR(1)
        collection — the classic example being `S -> a A d | b B d |
        a B e | b A e; A -> c; B -> c`, which LR(1) accepts and LALR(1)
        does not; `get_conflicts()` reports it, resolved by the same
        earliest-production convention as everywhere else in this file. */
    class LALRParser : public LRParserBase
    {
    public:
        explicit LALRParser(const Grammar& g) : LRParserBase(g)
        {
            DynArray<LR1State> lr1_states;
            DynArray<HashMap<Grammar::Symbol, nat_t>> lr1_transitions;
            build_lr1_collection(lr1_states, lr1_transitions);

            DynArray<LR0State> unique_cores;
            DynArray<nat_t> old_to_merged(lr1_states.size(), nat_t(0));

            for (nat_t i = 0; i < lr1_states.size(); ++i)
            {
                LR0State core;

                for (const LR1Item& item : lr1_states[i])
                {
                    core.append(LR0Item{item.prod, item.dot});
                }

                canonicalize_state(core);

                nat_t found_index = unique_cores.size();
                bool found = false;

                for (nat_t c = 0; c < unique_cores.size(); ++c)
                {
                    if (same_canonical_state(unique_cores[c], core))
                    {
                        found_index = c;
                        found = true;
                        break;
                    }
                }

                if (!found)
                {
                    unique_cores.append(std::move(core));
                }

                old_to_merged[i] = found_index;
            }

            DynArray<LALRState> merged_states;

            for (nat_t i = 0; i < unique_cores.size(); ++i)
            {
                merged_states.append(LALRState());
            }

            for (nat_t i = 0; i < lr1_states.size(); ++i)
            {
                nat_t m = old_to_merged[i];

                for (const LR1Item& item : lr1_states[i])
                {
                    LALRItem* found_item = nullptr;

                    for (nat_t k = 0; k < merged_states[m].size(); ++k)
                    {
                        if (merged_states[m][k].prod == item.prod &&
                            merged_states[m][k].dot == item.dot)
                        {
                            found_item = &merged_states[m][k];
                            break;
                        }
                    }

                    if (found_item == nullptr)
                    {
                        merged_states[m].append(
                            LALRItem{item.prod, item.dot, HashSet<Grammar::Symbol>()});
                        found_item = &merged_states[m][merged_states[m].size() - 1];
                    }

                    found_item->lookaheads.insert(item.lookahead);
                }
            }

            for (nat_t i = 0; i < unique_cores.size(); ++i)
            {
                transitions.append(HashMap<Grammar::Symbol, nat_t>());
            }

            for (nat_t i = 0; i < lr1_states.size(); ++i)
            {
                nat_t m = old_to_merged[i];

                for (const auto& kv : lr1_transitions[i])
                {
                    transitions[m].insert(kv.first, old_to_merged[kv.second]);
                }
            }

            DynArray<ReduceCandidate> candidates;

            for (nat_t m = 0; m < merged_states.size(); ++m)
            {
                for (const LALRItem& item : merged_states[m])
                {
                    const LRProduction& p = prods[item.prod];

                    if (item.dot < p.rhs.size())
                    {
                        continue;
                    }

                    if (item.prod == 0)
                    {
                        accepting_states.insert(m);
                        continue;
                    }

                    for (const Grammar::Symbol& la : item.lookaheads)
                    {
                        candidates.append(ReduceCandidate{m, la, item.prod});
                    }
                }
            }

            build_reduce_table(merged_states.size(), transitions, candidates);
        }
    };

} // end namespace Designar
