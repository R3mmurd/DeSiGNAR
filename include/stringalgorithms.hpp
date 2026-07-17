/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file stringalgorithms.hpp
    @brief String-matching and string-distance algorithms: Knuth-Morris-Pratt
    and Rabin-Karp substring search, and Levenshtein edit distance.
    Generic over any random-access sequence of equality-comparable
    symbols (a `std::string` of `char` being the common case, but not
    hardcoded to it), matching this library's general preference for
    not assuming a specific container/symbol type where the algorithm
    itself does not need to.
    @ingroup Algorithms
*/

#pragma once

#include <array.hpp>
#include <types.hpp>

namespace Designar
{
    /** Builds the KMP "failure function" (a.k.a. prefix function) for
        `pattern`: `fail[i]` is the length of the longest proper prefix of
        `pattern[0..i]` that is also a suffix of it — the key piece of
        precomputation that lets kmp_search() below skip re-examining text
        characters it has already matched after a mismatch, instead of
        restarting the pattern from its beginning (which is what makes
        naive substring search O(n*m) in the worst case). */
    template <typename Sequence>
    DynArray<nat_t> kmp_failure_function(const Sequence& pattern)
    {
        nat_t m = pattern.size();
        DynArray<nat_t> fail(m, 0);

        if (m == 0)
        {
            return fail;
        }

        nat_t k = 0;

        for (nat_t i = 1; i < m; ++i)
        {
            while (k > 0 && pattern[i] != pattern[k])
            {
                k = fail[k - 1];
            }

            if (pattern[i] == pattern[k])
            {
                ++k;
            }

            fail[i] = k;
        }

        return fail;
    }

    /** Knuth-Morris-Pratt substring search: finds every occurrence
        (possibly overlapping) of `pattern` in `text` in O(n + m) time —
        versus naive search's O(n*m) worst case — by precomputing
        kmp_failure_function() once and using it to avoid ever re-comparing
        a text character that has already been matched. Returns the
        starting index of every match, in order. */
    template <typename Sequence>
    DynArray<nat_t> kmp_search(const Sequence& text, const Sequence& pattern)
    {
        DynArray<nat_t> matches;
        nat_t m = pattern.size();

        if (m == 0 || m > text.size())
        {
            return matches;
        }

        DynArray<nat_t> fail = kmp_failure_function(pattern);
        nat_t n = text.size();
        nat_t k = 0;

        for (nat_t i = 0; i < n; ++i)
        {
            while (k > 0 && text[i] != pattern[k])
            {
                k = fail[k - 1];
            }

            if (text[i] == pattern[k])
            {
                ++k;
            }

            if (k == m)
            {
                matches.append(i - m + 1);
                k = fail[k - 1];
            }
        }

        return matches;
    }

    /** Rabin-Karp substring search: finds every occurrence of `pattern` in
        `text` by rolling a polynomial hash of a length-m window across
        `text` in O(1) per step (add the incoming symbol, subtract the
        outgoing one, rather than rehashing the whole window), comparing
        that hash against the pattern's own hash and only falling back to
        an actual character-by-character comparison when the hashes match
        (a spurious hash collision is possible, so this verification step
        is required for correctness — this is not "trust the hash", it is
        "use the hash to skip almost all of the O(n*m) naive comparisons"). */
    template <typename Sequence>
    DynArray<nat_t> rabin_karp_search(const Sequence& text,
                                      const Sequence& pattern)
    {
        DynArray<nat_t> matches;
        nat_t m = pattern.size();
        nat_t n = text.size();

        if (m == 0 || m > n)
        {
            return matches;
        }

        constexpr nat_t base = 256;
        constexpr nat_t modulus = 1000000007ULL;

        nat_t high_order = 1;

        for (nat_t i = 0; i + 1 < m; ++i)
        {
            high_order = (high_order * base) % modulus;
        }

        nat_t pattern_hash = 0, window_hash = 0;

        for (nat_t i = 0; i < m; ++i)
        {
            pattern_hash = (pattern_hash * base + nat_t(pattern[i])) % modulus;
            window_hash = (window_hash * base + nat_t(text[i])) % modulus;
        }

        auto matches_here = [&](nat_t start)
        {
            for (nat_t j = 0; j < m; ++j)
            {
                if (text[start + j] != pattern[j])
                {
                    return false;
                }
            }

            return true;
        };

        for (nat_t i = 0;; ++i)
        {
            if (pattern_hash == window_hash && matches_here(i))
            {
                matches.append(i);
            }

            if (i + m >= n)
            {
                break;
            }

            window_hash = (window_hash + modulus -
                           (nat_t(text[i]) * high_order) % modulus) %
                          modulus;
            window_hash = (window_hash * base + nat_t(text[i + m])) % modulus;
        }

        return matches;
    }

    /** The Levenshtein edit distance between `a` and `b`: the minimum
        number of single-symbol insertions, deletions, and substitutions
        needed to turn `a` into `b`, via the standard O(n*m)-time,
        O(min(n,m))-space dynamic program (only the previous row of the
        full edit-distance matrix is ever needed to compute the next one,
        so the whole matrix is never actually allocated). */
    template <typename Sequence>
    nat_t edit_distance(const Sequence& a, const Sequence& b)
    {
        nat_t n = a.size();
        nat_t m = b.size();

        DynArray<nat_t> prev(m + 1, 0);
        DynArray<nat_t> curr(m + 1, 0);

        for (nat_t j = 0; j <= m; ++j)
        {
            prev[j] = j;
        }

        for (nat_t i = 1; i <= n; ++i)
        {
            curr[0] = i;

            for (nat_t j = 1; j <= m; ++j)
            {
                if (a[i - 1] == b[j - 1])
                {
                    curr[j] = prev[j - 1];
                }
                else
                {
                    curr[j] = 1 + std::min({prev[j - 1], prev[j], curr[j - 1]});
                }
            }

            std::swap(prev, curr);
        }

        return prev[m];
    }

} // end namespace Designar
