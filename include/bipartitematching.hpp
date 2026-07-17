/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file bipartitematching.hpp
    @brief BipartiteMatching: maximum bipartite matching via
    augmenting paths (Kuhn's algorithm).
    @ingroup Graphs
*/

#pragma once

#include <graphalgorithms.hpp>

namespace Designar
{
    /** Finds a maximum matching in a bipartite graph via the augmenting-path
        method (Kuhn's algorithm / the bipartite specialization of
        Ford-Fulkerson): repeatedly find, via DFS from an unmatched node on
        the left side, an alternating path that ends at an unmatched node
        on the right side, and flip every edge along it between matched
        and unmatched — each such augmentation grows the matching by
        exactly one edge, and no augmenting path existing is exactly the
        condition (by König's theorem) for the current matching to already
        be maximum.

        `g` is taken as a plain (not necessarily bipartite) graph together
        with an explicit `left` side; edges from a `left` node to another
        `left` node, or between two non-`left` nodes, are simply never
        followed, so passing a non-bipartite graph or a `left` set that
        does not actually bipartition it just silently ignores the
        offending edges rather than throwing — the caller is trusted to
        pass an actual bipartition. */
    template <class GT>
    class BipartiteMatching
    {
        const GT& g;
        HashSet<Node<GT>*> left;
        HashMap<Node<GT>*, Node<GT>*> match;

        bool try_augment(Node<GT>* u, HashSet<Node<GT>*>& visited)
        {
            for (AdArcIt<GT> it(g, u); it.has_current(); it.next())
            {
                Node<GT>* v = it.get_tgt_node();

                if (left.search(v) != nullptr) // only cross to the right side
                    continue;

                if (visited.search(v) != nullptr)
                    continue;

                visited.insert(v);

                Node<GT>** matched_to_v = match.search(v);

                if (matched_to_v == nullptr ||
                    try_augment(*matched_to_v, visited))
                {
                    // operator[] (insert-or-update), not insert(): v (and u, if
                    // it was previously matched to someone else before this
                    // augmenting path stole it away) may already be a key in
                    // `match` from an earlier augmentation — insert() silently
                    // does nothing when the key already exists, which left
                    // stale pairings in place instead of rewriting them.
                    match[v] = u;
                    match[u] = v;
                    return true;
                }
            }

            return false;
        }

    public:
        BipartiteMatching(const GT& _g, const HashSet<Node<GT>*>& _left)
            : g(_g), left(_left)
        {
            // empty
        }

        /** Computes and returns a maximum matching, as a HashMap where both
            `m[u] == v` and `m[v] == u` hold for every matched pair `(u, v)`
            (one of `u`, `v` on the left, the other on the right) — looking a
            node up in either direction costs one hash lookup either way,
            at the price of storing every pair twice. */
        HashMap<Node<GT>*, Node<GT>*> compute()
        {
            match.clear();

            for (Node<GT>* u : left)
            {
                if (match.search(u) != nullptr)
                    continue;

                HashSet<Node<GT>*> visited;
                try_augment(u, visited);
            }

            return match;
        }
    };

} // end namespace Designar
