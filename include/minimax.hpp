/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file minimax.hpp
    @brief Generic minimax game-tree search with alpha-beta pruning:
    `alpha_beta_search()` returns the value of a position under
    optimal play by both sides, `find_best_move()` wraps it to also
    report *which* child of the root achieves that value (what an
    actual game-playing agent needs). Both are free functions
    templated on `State` plus three caller-supplied callables
    (`get_children`, `is_terminal`, `evaluate`) rather than a fixed
    "Game" interface/base class, so any two-player, zero-sum,
    perfect-information game (tic-tac-toe, checkers, a toy game tree)
    can reuse the exact same search code by supplying its own move
    generation and evaluation, the standard textbook decomposition for
    this algorithm.
    @ingroup ArtificialIntelligence
*/

#pragma once

#include <array.hpp>

#include <algorithm>
#include <limits>

namespace Designar
{
    /** The minimax value of `state` (from the maximizing player's point
        of view) under optimal play, searched to `depth` plies or until
        `is_terminal(state)`, with alpha-beta pruning: `alpha` is the
        best value the maximizing player can already guarantee
        somewhere above this call, `beta` the best the minimizing player
        can already guarantee — once a node's own best-so-far value
        makes the outcome worse for whichever side is *above* this
        node than an alternative it already has, every remaining
        sibling is provably irrelevant and is skipped rather than
        searched. */
    template <typename State, class GetChildren, class IsTerminal,
              class Evaluate>
    real_t alpha_beta_search(const State& state, int_t depth, real_t alpha,
                             real_t beta, bool maximizing_player,
                             GetChildren&& get_children,
                             IsTerminal&& is_terminal, Evaluate&& evaluate)
    {
        if (depth <= 0 || is_terminal(state))
        {
            return evaluate(state);
        }

        DynArray<State> children = get_children(state);

        if (children.is_empty())
        {
            return evaluate(state);
        }

        if (maximizing_player)
        {
            real_t value = -std::numeric_limits<real_t>::infinity();

            for (nat_t i = 0; i < children.size(); ++i)
            {
                real_t child_value = alpha_beta_search(
                    children[i], depth - 1, alpha, beta, false, get_children,
                    is_terminal, evaluate);
                value = std::max(value, child_value);
                alpha = std::max(alpha, value);

                if (alpha >= beta)
                {
                    break; // beta cutoff: the minimizer above already has a
                           // better (smaller) alternative than anything this
                           // branch can still produce.
                }
            }

            return value;
        }

        real_t value = std::numeric_limits<real_t>::infinity();

        for (nat_t i = 0; i < children.size(); ++i)
        {
            real_t child_value =
                alpha_beta_search(children[i], depth - 1, alpha, beta, true,
                                  get_children, is_terminal, evaluate);
            value = std::min(value, child_value);
            beta = std::min(beta, value);

            if (beta <= alpha)
            {
                break; // alpha cutoff: symmetric to the beta cutoff above.
            }
        }

        return value;
    }

    /** The outcome of find_best_move(): the minimax score of `state`
        and which of its children achieves it (`has_move` is false only
        when `state` has no children at all, i.e. it is itself
        terminal — `best_state` is then meaningless and left equal to
        `state`). */
    template <typename State>
    struct MoveSearchResult
    {
        real_t score;
        State best_state;
        bool has_move;
    };

    /** Exactly alpha_beta_search()'s search, but for the root of
        the search rather than an internal node: reports which child
        achieves the minimax value, since that (not just the value
        itself) is what a caller actually driving a game needs. */
    template <typename State, class GetChildren, class IsTerminal,
              class Evaluate>
    MoveSearchResult<State>
    find_best_move(const State& state, int_t depth, bool maximizing_player,
                   GetChildren&& get_children, IsTerminal&& is_terminal,
                   Evaluate&& evaluate)
    {
        DynArray<State> children = get_children(state);

        if (children.is_empty())
        {
            return MoveSearchResult<State>{evaluate(state), state, false};
        }

        real_t alpha = -std::numeric_limits<real_t>::infinity();
        real_t beta = std::numeric_limits<real_t>::infinity();

        real_t best_score = maximizing_player
                                ? -std::numeric_limits<real_t>::infinity()
                                : std::numeric_limits<real_t>::infinity();
        State best_state = children[0];

        for (nat_t i = 0; i < children.size(); ++i)
        {
            real_t score = alpha_beta_search(
                children[i], depth - 1, alpha, beta, !maximizing_player,
                get_children, is_terminal, evaluate);

            bool better =
                maximizing_player ? (score > best_score) : (score < best_score);

            if (better)
            {
                best_score = score;
                best_state = children[i];
            }

            if (maximizing_player)
            {
                alpha = std::max(alpha, best_score);
            }
            else
            {
                beta = std::min(beta, best_score);
            }

            if (alpha >= beta)
            {
                break;
            }
        }

        return MoveSearchResult<State>{best_score, best_state, true};
    }

} // end namespace Designar
