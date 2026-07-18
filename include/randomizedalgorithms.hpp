/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file randomizedalgorithms.hpp
    @brief The classic randomized algorithms (CLRS's own randomized-
    algorithms material and Motwani-Raghavan's "Randomized Algorithms"):
    randomized_select() (expected-linear-time order statistics),
    miller_rabin_is_prime() (Monte Carlo primality testing),
    reservoir_sample() (uniform sampling from a one-pass stream), and
    karger_min_cut() (the randomized contraction algorithm for global
    min-cut).
    @ingroup Algorithms
*/

#pragma once

#include <stdexcept>
#include <utility>

#include <types.hpp>
#include <array.hpp>
#include <random.hpp>
#include <numbertheory.hpp>
#include <relation.hpp>
#include <map.hpp>
#include <graphutilities.hpp>

namespace Designar
{
    /** Lomuto partition of `a[l..r]` around a *uniformly random* element
        of that range (swapped into position `r` first) — the source of
        randomized_select()'s expected-linear-time guarantee: an
        adversary who knows this algorithm cannot construct an input
        that reliably triggers its worst case, since the pivot choice
        does not depend on the input's arrangement at all. */
    template <typename T, class Cmp>
    nat_t randomized_partition(DynArray<T>& a, nat_t l, nat_t r, rng_t& rng,
                              Cmp& cmp)
    {
        nat_t pivot_idx = l + random_uniform<nat_t>(rng, r - l + 1);
        std::swap(a[pivot_idx], a[r]);

        nat_t i = l;

        for (nat_t j = l; j < r; ++j)
        {
            if (cmp(a[j], a[r]))
            {
                std::swap(a[i], a[j]);
                ++i;
            }
        }

        std::swap(a[i], a[r]);
        return i;
    }

    /** CLRS 9.2's RANDOMIZED-SELECT: the `k`-th smallest element (0-based)
        of `a`, in expected `O(a.size())` time — partition around a
        random pivot, then recurse into *only* the side known to contain
        rank `k` (written iteratively here, the usual tail-call-to-loop
        transformation, so it doesn't grow the call stack). Reorders `a`
        in place (like `std::nth_element`) and returns a reference into
        it; `k` must be `< a.size()`. */
    template <typename T, class Cmp = std::less<T>>
    T& randomized_select(DynArray<T>& a, nat_t k, rng_t& rng, Cmp cmp = Cmp())
    {
        if (k >= a.size())
        {
            throw std::out_of_range("randomized_select: k out of range");
        }

        nat_t l = 0;
        nat_t r = a.size() - 1;

        while (true)
        {
            if (l == r)
            {
                return a[l];
            }

            nat_t pivot_idx = randomized_partition(a, l, r, rng, cmp);

            if (k == pivot_idx)
            {
                return a[pivot_idx];
            }
            else if (k < pivot_idx)
            {
                r = pivot_idx - 1;
            }
            else
            {
                l = pivot_idx + 1;
            }
        }
    }

    /** The Miller-Rabin primality test: a Monte Carlo algorithm that
        never wrongly reports a prime as composite, but can (with
        probability at most `4^-rounds`) wrongly report a composite as
        prime — writes `n - 1 = 2^r * d` with `d` odd, then for each of
        `rounds` independent random witnesses `a` checks whether `a^d`,
        `a^(2d)`, ..., `a^(2^(r-1)*d)` ever hits `n - 1` (mod `n`); a
        witness that reaches neither `1` nor `n - 1` at the right point
        proves `n` composite outright. Each additional round
        exponentially shrinks the false-positive probability, which is
        the entire point of spending more than one round. */
    inline bool miller_rabin_is_prime(int_t n, nat_t rounds, rng_t& rng)
    {
        if (n < 2)
        {
            return false;
        }

        if (n == 2 || n == 3)
        {
            return true;
        }

        if (n % 2 == 0)
        {
            return false;
        }

        int_t d = n - 1;
        int_t r = 0;

        while (d % 2 == 0)
        {
            d /= 2;
            ++r;
        }

        for (nat_t round = 0; round < rounds; ++round)
        {
            int_t a = 2 + random_uniform<int_t>(rng, n - 3); // a in [2, n-2]
            int_t x = mod_pow(a, d, n);

            if (x == 1 || x == n - 1)
            {
                continue;
            }

            bool possibly_composite = true;

            for (int_t i = 0; i < r - 1; ++i)
            {
                x = mod_pow(x, int_t(2), n);

                if (x == n - 1)
                {
                    possibly_composite = false;
                    break;
                }
            }

            if (possibly_composite)
            {
                return false;
            }
        }

        return true;
    }

    /** Reservoir sampling (Vitter's Algorithm R): visits `[begin, end)`
        exactly once, producing a uniformly random sample of `k` items
        (or every item, if fewer than `k` were seen) without knowing the
        stream's length in advance — the first `k` items always seed the
        reservoir; every item after that replaces a uniformly random
        reservoir slot with probability `k / (items seen so far)`, which
        is exactly what keeps every item's final inclusion probability
        equal regardless of when it was seen. */
    template <typename T, class It>
    DynArray<T> reservoir_sample(It begin, It end, nat_t k, rng_t& rng)
    {
        DynArray<T> reservoir;
        nat_t count = 0;

        for (It it = begin; it != end; ++it)
        {
            ++count;

            if (reservoir.size() < k)
            {
                reservoir.append(*it);
            }
            else
            {
                nat_t j = random_uniform<nat_t>(rng, count);

                if (j < k)
                {
                    reservoir[j] = *it;
                }
            }
        }

        return reservoir;
    }

    /** Karger's randomized contraction algorithm for global min-cut on
        an undirected `Graph` (graph.hpp/graphalgorithms.hpp's `GT`
        convention): repeatedly picks a uniformly random remaining edge
        and contracts it (merges its two endpoints into one, tracked via
        a union-find rather than by mutating `g` itself — see
        EquivalenceRelation, relation.hpp) until only 2 "super-nodes"
        remain; the edges still crossing between them are one candidate
        min cut. A single trial only finds the *true* min cut with
        probability `>= 2 / (n * (n - 1))` (`n` = node count), so this
        runs `trials` independent trials and keeps the smallest cut seen
        — the standard way that per-trial probability gets amplified
        toward certainty (running `~n^2 * ln(n)` trials pushes the
        failure probability down to `~1/n`, per the textbook analysis;
        this function leaves the trial count to the caller rather than
        picking one itself). */
    template <class GT>
    nat_t karger_min_cut(const GT& g, nat_t trials, rng_t& rng)
    {
        nat_t n = g.get_num_nodes();

        if (n < 2)
        {
            throw std::domain_error(
                "karger_min_cut: graph needs at least 2 nodes");
        }

        HashMap<Node<GT>*, nat_t> index_of;
        g.enum_for_each_node(
            [&](nat_t i, Node<GT>* node) { index_of.insert(node, i); });

        DynArray<std::pair<nat_t, nat_t>> edges;
        g.for_each_arc(
            [&](Arc<GT>* arc)
            {
                nat_t u = *index_of.search(arc->get_src_node());
                nat_t v = *index_of.search(arc->get_tgt_node());
                edges.append(std::make_pair(u, v));
            });

        nat_t best = edges.size();

        for (nat_t trial = 0; trial < trials; ++trial)
        {
            EquivalenceRelation dsu(n);
            nat_t components = n;

            DynArray<std::pair<nat_t, nat_t>> shuffled = edges;

            for (nat_t i = shuffled.size(); i-- > 1;)
            {
                nat_t j = random_uniform<nat_t>(rng, i + 1);
                std::swap(shuffled[i], shuffled[j]);
            }

            nat_t idx = 0;

            while (components > 2 && idx < shuffled.size())
            {
                const auto& e = shuffled[idx++];

                if (!dsu.are_connected(e.first, e.second))
                {
                    dsu.join(e.first, e.second);
                    --components;
                }
            }

            nat_t cut = 0;

            for (const auto& e : edges)
            {
                if (!dsu.are_connected(e.first, e.second))
                {
                    ++cut;
                }
            }

            if (cut < best)
            {
                best = cut;
            }
        }

        return best;
    }

} // end namespace Designar
