/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <randomizedalgorithms.hpp>
#include <graph.hpp>

using namespace Designar;

int main()
{
    rng_t rng(20260718);

    // randomized_select: for a permutation of 0..N-1, the k-th smallest
    // must be exactly k, for every k.
    {
        constexpr nat_t N = 200;
        DynArray<int_t> base;

        for (int_t i = 0; i < (int_t)N; ++i)
        {
            base.append(i);
        }

        for (nat_t i = base.size() - 1; i > 0; --i)
        {
            nat_t j = random_uniform<nat_t>(rng, i + 1);
            std::swap(base[i], base[j]);
        }

        for (nat_t k = 0; k < N; ++k)
        {
            DynArray<int_t> copy = base;
            int_t v = randomized_select(copy, k, rng);
            assert(v == (int_t)k);
        }

        DynArray<int_t> single = {42};
        assert(randomized_select(single, 0, rng) == 42);

        bool threw = false;

        try
        {
            randomized_select(single, 1, rng);
        }
        catch (const std::out_of_range&)
        {
            threw = true;
        }

        assert(threw);
    }

    // miller_rabin_is_prime: known primes must always pass, known
    // composites (including a Carmichael number, which is exactly what
    // Fermat's-little-theorem-only tests get fooled by) must always
    // fail.
    {
        int_t primes[] = {2,   3,   5,   7,     11,     13,
                          17,  19,  23,  97,    997,    7919,
                          999999937};

        for (int_t p : primes)
        {
            assert(miller_rabin_is_prime(p, 30, rng));
        }

        int_t composites[] = {0, 1, 4, 6, 8, 9, 10, 100, 1000000, 999999999};

        for (int_t c : composites)
        {
            assert(!miller_rabin_is_prime(c, 30, rng));
        }

        // Carmichael numbers: pass every Fermat test but must still be
        // correctly identified as composite here.
        assert(!miller_rabin_is_prime(561, 30, rng));
        assert(!miller_rabin_is_prime(1105, 30, rng));
        assert(!miller_rabin_is_prime(1729, 30, rng));
    }

    // reservoir_sample
    {
        constexpr nat_t STREAM_SIZE = 1000;
        constexpr nat_t K = 20;
        DynArray<int_t> stream;

        for (int_t i = 0; i < (int_t)STREAM_SIZE; ++i)
        {
            stream.append(i);
        }

        auto sample = reservoir_sample<int_t>(stream.begin(), stream.end(),
                                              K, rng);
        assert(sample.size() == K);

        for (int_t v : sample)
        {
            assert(v >= 0 && v < (int_t)STREAM_SIZE);
        }

        // Fewer items than k: the whole stream comes back.
        DynArray<int_t> small = {1, 2, 3};
        auto sample2 = reservoir_sample<int_t>(small.begin(), small.end(),
                                               10, rng);
        assert(sample2.size() == 3);

        // Rough uniformity check over many independent samplings: every
        // position in the stream should get selected roughly K/N of the
        // time — loose bounds, just enough to catch a badly broken
        // replacement rule (e.g. always keeping the first K forever).
        DynArray<nat_t> counts(STREAM_SIZE, nat_t(0));
        constexpr nat_t TRIALS = 3000;

        for (nat_t t = 0; t < TRIALS; ++t)
        {
            auto s = reservoir_sample<int_t>(stream.begin(), stream.end(), K,
                                             rng);

            for (int_t v : s)
            {
                ++counts[v];
            }
        }

        real_t expected = real_t(TRIALS * K) / real_t(STREAM_SIZE);

        for (nat_t i = 0; i < STREAM_SIZE; i += 137) // sample a few indices
        {
            real_t ratio = real_t(counts[i]) / expected;
            assert(ratio > 0.5 && ratio < 1.5);
        }
    }

    // karger_min_cut: two triangles joined by a single bridge edge has a
    // known min cut of exactly 1 (the bridge itself).
    {
        Graph<int_t> g;
        auto* a0 = g.insert_node(0);
        auto* a1 = g.insert_node(1);
        auto* a2 = g.insert_node(2);
        auto* b0 = g.insert_node(3);
        auto* b1 = g.insert_node(4);
        auto* b2 = g.insert_node(5);

        g.insert_arc(a0, a1);
        g.insert_arc(a1, a2);
        g.insert_arc(a2, a0);
        g.insert_arc(b0, b1);
        g.insert_arc(b1, b2);
        g.insert_arc(b2, b0);
        g.insert_arc(a0, b0); // the bridge: the only min cut

        assert(karger_min_cut(g, 200, rng) == 1);
    }

    // A 4-cycle (every node degree 2) has min cut 2 (remove any two
    // opposite edges to disconnect it).
    {
        Graph<int_t> g;
        auto* n0 = g.insert_node(0);
        auto* n1 = g.insert_node(1);
        auto* n2 = g.insert_node(2);
        auto* n3 = g.insert_node(3);

        g.insert_arc(n0, n1);
        g.insert_arc(n1, n2);
        g.insert_arc(n2, n3);
        g.insert_arc(n3, n0);

        assert(karger_min_cut(g, 200, rng) == 2);
    }

    return 0;
}
