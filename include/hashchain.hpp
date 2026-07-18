/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file hashchain.hpp
    @brief HashChain: the classic iterated hash chain — apply a hash
    function `H` repeatedly over a seed, `x_0 = seed, x_i = H(x_{i-1})`,
    and publish only the final value `x_length` (the "commitment").
    Anyone who is later given some `x_k` can verify it really is a link
    of the chain leading to that commitment simply by re-hashing it
    forward the remaining `length - k` steps and checking the result
    matches — but, since that requires only forward evaluation of `H`,
    never inverting it, nobody who has only ever seen `x_k` can compute
    `x_{k-1}` (an *earlier*, not-yet-revealed link) themselves. This is
    the basis of S/KEY-style one-time-password schemes (each
    authentication reveals the next unused link, working backward from
    the commitment toward the seed) and of simple blockchain-style
    structures (each block's stored hash commits to everything before
    it).
    @ingroup Hashing
*/

#pragma once

#include <utility>

#include <types.hpp>
#include <array.hpp>

namespace Designar
{
    /** `HashFn`: `DynArray<unsigned char> operator()(const
        DynArray<unsigned char>&)` — hash.hpp's md5()/sha1()/sha256()
        take a `const void*, nat_t` (or `const std::string&`) instead,
        so wrap one in a small adapter lambda; see make_hash_chain()'s
        doc comment for exactly what that looks like. */
    template <class HashFn>
    class HashChain
    {
        HashFn hash_fn;
        nat_t length;
        DynArray<unsigned char> seed;
        DynArray<unsigned char> commitment_;

    public:
        /** Computes the whole chain right away: `length` applications of
            `hash_fn` starting from `seed_value`, keeping only the final
            result (`commitment()`) — every intermediate link is
            recomputed on demand by verify() instead of being stored, so
            this class's own footprint stays O(digest size) regardless of
            how long the chain is. */
        HashChain(DynArray<unsigned char> seed_value, nat_t chain_length,
                 HashFn fn)
            : hash_fn(std::move(fn)),
              length(chain_length),
              seed(std::move(seed_value))
        {
            DynArray<unsigned char> current = seed;

            for (nat_t i = 0; i < length; ++i)
            {
                current = hash_fn(current);
            }

            commitment_ = std::move(current);
        }

        nat_t chain_length() const
        {
            return length;
        }

        const DynArray<unsigned char>& seed_value() const
        {
            return seed;
        }

        /** `x_length` — the one value safe to publish/store ahead of
            time; verify() checks a later-revealed link against this. */
        const DynArray<unsigned char>& commitment() const
        {
            return commitment_;
        }

        /** True exactly when re-hashing `candidate` forward
            `remaining_steps` times reproduces commitment() — i.e.
            `candidate` really is `x_{length - remaining_steps}`, the
            link `remaining_steps` steps before the commitment. The
            S/KEY authentication check: the verifier only ever needs the
            *current* commitment (or the previously-accepted link) and
            the newly revealed link, never the original seed. */
        bool verify(const DynArray<unsigned char>& candidate,
                   nat_t remaining_steps) const
        {
            DynArray<unsigned char> current = candidate;

            for (nat_t i = 0; i < remaining_steps; ++i)
            {
                current = hash_fn(current);
            }

            return current.equal(commitment_);
        }
    };

    /** Deduces `HashFn` from the callback passed in — see
        GeneticAlgorithm's make_genetic_algorithm() (genetic.hpp) for why
        this factory exists instead of naming `HashChain<...>` directly.

        `hash_fn` must take and return a `DynArray<unsigned char>`; wrap
        one of hash.hpp's algorithms in a small adapter lambda to get
        that shape — `DynArray<unsigned char>` stores its elements
        contiguously, so `[](const DynArray<unsigned char>& b) { return
        sha256(&b[0], b.size()); }` (for a non-empty `b`, always true
        here since a digest is never empty) does it. */
    template <class HashFn>
    HashChain<HashFn> make_hash_chain(DynArray<unsigned char> seed,
                                      nat_t chain_length, HashFn hash_fn)
    {
        return HashChain<HashFn>(std::move(seed), chain_length,
                                 std::move(hash_fn));
    }

} // end namespace Designar
