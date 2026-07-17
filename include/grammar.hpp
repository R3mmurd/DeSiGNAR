/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file grammar.hpp
    @brief Context-free grammars, FIRST, and FOLLOW — the standard
    prerequisite to building a predictive (LL(1)) parser, computed
    exactly the way the Dragon Book presents them: two fixed-point
    iterations over the grammar's productions, the second (FOLLOW)
    depending on the first (FIRST) being fully computed already.
    @ingroup Compilers
*/

#pragma once

#include <array.hpp>
#include <set.hpp>
#include <map.hpp>

#include <stdexcept>
#include <string>

namespace Designar
{
    /** A context-free grammar over `std::string` symbols: every symbol
        used on the left-hand side of some production is a nonterminal,
        everything else (anything appearing only on right-hand sides)
        is implicitly a terminal — there is no separate terminal
        registry to keep in sync. A production's right-hand side is a
        `DynArray<std::string>`; an *empty* one specifically means an
        epsilon production (`A -> ε`), so no separate epsilon symbol is
        needed in the grammar representation itself (`EPSILON` below
        exists purely as a marker *within computed FIRST sets*, where
        "this symbol/sequence can vanish entirely" has to be
        representable). */
    class Grammar
    {
    public:
        using Symbol = std::string;
        using Sequence = DynArray<Symbol>;

        /** Marks "can derive the empty string" inside a computed FIRST
            set — never a member of `nonterminals`, never appears in any
            actual production's right-hand side (those use an empty
            `Sequence` for that instead). */
        static const Symbol EPSILON;

        /** The conventional end-of-input marker FOLLOW(start_symbol)
            always contains. */
        static const Symbol END_OF_INPUT;

    private:
        Symbol start_symbol;
        HashSet<Symbol> nonterminals;
        HashMap<Symbol, DynArray<Sequence>> productions;

        static HashSet<Symbol>
        first_of_symbol(const Symbol& sym,
                        const HashMap<Symbol, HashSet<Symbol>>& first_sets,
                        bool is_nt)
        {
            HashSet<Symbol> result;

            if (!is_nt)
            {
                result.insert(sym);
                return result;
            }

            const HashSet<Symbol>* found = first_sets.search(sym);

            if (found != nullptr)
            {
                result = *found;
            }

            return result;
        }

    public:
        Grammar() = default;

        void set_start_symbol(const Symbol& s)
        {
            start_symbol = s;
            nonterminals.insert(s);

            if (productions.search(s) == nullptr)
            {
                productions.insert(s, DynArray<Sequence>());
            }
        }

        const Symbol& get_start_symbol() const
        {
            return start_symbol;
        }

        bool is_nonterminal(const Symbol& sym) const
        {
            return nonterminals.search(sym) != nullptr;
        }

        bool is_terminal(const Symbol& sym) const
        {
            return !is_nonterminal(sym);
        }

        const HashSet<Symbol>& get_nonterminals() const
        {
            return nonterminals;
        }

        /** Adds `lhs -> rhs` (`rhs` empty means an epsilon production).
            `lhs` becomes a nonterminal if it was not one already —
            there is no separate "declare your nonterminals first"
            step. */
        void add_production(const Symbol& lhs, const Sequence& rhs)
        {
            nonterminals.insert(lhs);

            DynArray<Sequence>* rhs_list = productions.search(lhs);

            if (rhs_list == nullptr)
            {
                rhs_list = productions.insert(lhs, DynArray<Sequence>());
            }

            rhs_list->append(rhs);
        }

        const DynArray<Sequence>* get_productions(const Symbol& lhs) const
        {
            return productions.search(lhs);
        }

        /** FIRST of `seq[from..)` — the union of FIRST(seq[from]),
            FIRST(seq[from+1]), etc. for as long as every symbol seen so
            far is nullable (its FIRST set contains EPSILON), stopping at
            (and including) the first non-nullable symbol's FIRST set; if
            every symbol from `from` to the end is nullable (including the
            case where there are no symbols at all, `from >= seq.size()`),
            EPSILON itself is included in the result. `first_sets` must
            already hold every nonterminal's fully-computed FIRST set
            (i.e. this is meant to be called with compute_first()'s
            result, not while that fixed point is still converging). */
        HashSet<Symbol> first_of_sequence(
            const Sequence& seq, nat_t from,
            const HashMap<Symbol, HashSet<Symbol>>& first_sets) const
        {
            HashSet<Symbol> result;

            if (from >= seq.size())
            {
                result.insert(EPSILON);
                return result;
            }

            for (nat_t i = from; i < seq.size(); ++i)
            {
                HashSet<Symbol> sym_first =
                    first_of_symbol(seq[i], first_sets, is_nonterminal(seq[i]));

                bool nullable = false;

                for (const Symbol& s : sym_first)
                {
                    if (s == EPSILON)
                    {
                        nullable = true;
                        continue;
                    }

                    result.insert(s);
                }

                if (!nullable)
                {
                    return result;
                }
            }

            result.insert(EPSILON);
            return result;
        }

        /** FIRST(A) for every nonterminal A, via fixed-point iteration
            over every production until a full pass adds nothing new —
            the standard textbook algorithm; a single top-down recursive
            computation does not work directly here because grammars
            (in general) are mutually and even self-recursive. */
        HashMap<Symbol, HashSet<Symbol>> compute_first() const
        {
            HashMap<Symbol, HashSet<Symbol>> first_sets;

            for (const Symbol& nt : nonterminals)
            {
                first_sets.insert(nt, HashSet<Symbol>());
            }

            bool changed = true;

            while (changed)
            {
                changed = false;

                for (const Symbol& nt : nonterminals)
                {
                    const DynArray<Sequence>* rhs_list = productions.search(nt);

                    if (rhs_list == nullptr)
                    {
                        continue;
                    }

                    HashSet<Symbol>& nt_first = *first_sets.search(nt);

                    for (nat_t r = 0; r < rhs_list->size(); ++r)
                    {
                        const Sequence& rhs = (*rhs_list)[r];
                        HashSet<Symbol> seq_first =
                            first_of_sequence(rhs, 0, first_sets);

                        for (const Symbol& s : seq_first)
                        {
                            if (nt_first.search(s) == nullptr)
                            {
                                nt_first.insert(s);
                                changed = true;
                            }
                        }
                    }
                }
            }

            return first_sets;
        }

        /** FOLLOW(A) for every nonterminal A, via fixed-point iteration —
            must be called with the *fully converged* result of
            compute_first(), since every step here needs FIRST(beta) for
            various right-hand-side suffixes beta. */
        HashMap<Symbol, HashSet<Symbol>>
        compute_follow(const HashMap<Symbol, HashSet<Symbol>>& first_sets) const
        {
            HashMap<Symbol, HashSet<Symbol>> follow_sets;

            for (const Symbol& nt : nonterminals)
            {
                follow_sets.insert(nt, HashSet<Symbol>());
            }

            HashSet<Symbol>* start_follow = follow_sets.search(start_symbol);

            if (start_follow != nullptr)
            {
                start_follow->insert(END_OF_INPUT);
            }

            bool changed = true;

            while (changed)
            {
                changed = false;

                for (const Symbol& lhs : nonterminals)
                {
                    const DynArray<Sequence>* rhs_list =
                        productions.search(lhs);

                    if (rhs_list == nullptr)
                    {
                        continue;
                    }

                    for (nat_t r = 0; r < rhs_list->size(); ++r)
                    {
                        const Sequence& rhs = (*rhs_list)[r];

                        for (nat_t i = 0; i < rhs.size(); ++i)
                        {
                            if (!is_nonterminal(rhs[i]))
                            {
                                continue;
                            }

                            HashSet<Symbol> beta_first =
                                first_of_sequence(rhs, i + 1, first_sets);
                            HashSet<Symbol>& xi_follow =
                                *follow_sets.search(rhs[i]);

                            bool beta_nullable = false;

                            for (const Symbol& s : beta_first)
                            {
                                if (s == EPSILON)
                                {
                                    beta_nullable = true;
                                    continue;
                                }

                                if (xi_follow.search(s) == nullptr)
                                {
                                    xi_follow.insert(s);
                                    changed = true;
                                }
                            }

                            if (beta_nullable)
                            {
                                const HashSet<Symbol>& lhs_follow =
                                    *follow_sets.search(lhs);

                                for (const Symbol& s : lhs_follow)
                                {
                                    if (xi_follow.search(s) == nullptr)
                                    {
                                        xi_follow.insert(s);
                                        changed = true;
                                    }
                                }
                            }
                        }
                    }
                }
            }

            return follow_sets;
        }
    };

    inline const Grammar::Symbol Grammar::EPSILON = "";
    inline const Grammar::Symbol Grammar::END_OF_INPUT = "$";

} // end namespace Designar
