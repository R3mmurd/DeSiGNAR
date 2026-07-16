/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file bloomfilter.hpp
    @brief BloomFilter: a probabilistic set membership structure — false
    positives are possible ("might contain"), false negatives are not
    ("definitely does not contain") — trading a small, tunable
    false-positive rate for O(k) membership tests and O(1) bits of
    storage per expected element regardless of the element's own size.
    @ingroup DataStructures
*/

#pragma once

#include <bitset.hpp>
#include <hash.hpp>

namespace Designar
{
  /** Backed by a single bit array and `num_hashes` independent-looking
      hash functions; insert() sets, for each of the `num_hashes` hashes
      of the key, the corresponding bit; might_contain() reports true
      only if *every one* of those bits is already set. A single shared
      bit array (rather than `num_hashes` separate arrays) is what makes
      this space-efficient, at the cost of being unable to ever
      "un-insert" a key: clearing a bit could make some *other* key that
      happens to also hash to it start reporting false negatives, which
      would break the "no false negatives" guarantee — so there is no
      remove().

      Rather than compute `num_hashes` genuinely independent hash
      functions, this uses the standard Kirsch-Mitzenmacher trick: derive
      only two independent hashes `h1`, `h2` and synthesize the rest as
      `h1 + i*h2` — provably as good as `num_hashes` fully independent
      hashes for this data structure's purposes, and far cheaper. */
  template <typename Key>
  class BloomFilter
  {
    DynBitSet bits;
    nat_t num_hashes;
    nat_t num_items;

    /** A strong 64-bit bit-mixer (MurmurHash3's `fmix64` finalizer):
        thoroughly scrambles its input so that the output looks
        statistically independent of it, even though it is a pure
        function of it. Used to derive `h2` from `h1` below — a naive
        combination (e.g. XOR-ing in a constant, or feeding `k` through
        hash_combine() seeded with a small constant) leaves `h2` too
        correlated with `h1` in practice (both still being simple
        functions of the same underlying super_fast_hash(k)), which
        showed up as a measurably higher false-positive rate than the
        formulas below predict; this finalizer's avalanche property is
        specifically designed to break that correlation. */
    static nat_t mix64(nat_t x)
    {
      x ^= x >> 33;
      x *= 0xff51afd7ed558ccdULL;
      x ^= x >> 33;
      x *= 0xc4ceb9fe1a85ec53ULL;
      x ^= x >> 33;
      return x;
    }

    std::pair<nat_t, nat_t> base_hashes(const Key& k) const
    {
      nat_t h1 = super_fast_hash(k);
      nat_t h2 = mix64(h1 + 0x9e3779b97f4a7c15ULL);

      if (h2 == 0) // avoid a degenerate all-same-index probe sequence
      {
        h2 = 1;
      }

      return {h1, h2};
    }

    nat_t bit_index(nat_t h1, nat_t h2, nat_t i) const
    {
      return (h1 + i * h2) % bits.size();
    }

  public:
    /** `num_bits` is the raw bit-array size and `k` the number of hash
        functions to synthesize; see optimal_num_bits()/
        optimal_num_hashes() to derive both from an expected element
        count and a target false-positive rate instead of picking them
        by hand. */
    BloomFilter(nat_t num_bits, nat_t k)
        : bits(num_bits, false), num_hashes(k), num_items(0)
    {
      if (num_bits == 0)
      {
        throw std::domain_error("num_bits must be positive");
      }

      if (k == 0)
      {
        throw std::domain_error("num_hashes must be positive");
      }
    }

    void insert(const Key& k)
    {
      auto [h1, h2] = base_hashes(k);

      for (nat_t i = 0; i < num_hashes; ++i)
      {
        bits.set_bit(bit_index(h1, h2, i), true);
      }

      ++num_items;
    }

    /** True means "possibly in the set" (a false positive is possible,
        with probability that grows with how full the filter is); false
        means "definitely not in the set" (this is always exact). */
    bool might_contain(const Key& k) const
    {
      auto [h1, h2] = base_hashes(k);

      for (nat_t i = 0; i < num_hashes; ++i)
      {
        if (!bits.get_bit(bit_index(h1, h2, i)))
        {
          return false;
        }
      }

      return true;
    }

    nat_t size() const
    {
      return num_items;
    }

    nat_t bit_count() const
    {
      return bits.size();
    }

    nat_t hash_count() const
    {
      return num_hashes;
    }

    void clear()
    {
      bits = DynBitSet(bits.size(), false);
      num_items = 0;
    }

    /** The number of bits (m) that minimizes the false-positive rate for
        `expected_items` (n) elements at a target false-positive rate `p`
        (0 < p < 1): the standard `m = -n*ln(p) / (ln 2)^2` formula. */
    static nat_t optimal_num_bits(nat_t expected_items, real_t false_positive_rate)
    {
      if (expected_items == 0)
      {
        throw std::domain_error("expected_items must be positive");
      }

      if (false_positive_rate <= 0. || false_positive_rate >= 1.)
      {
        throw std::domain_error("false_positive_rate must be in (0, 1)");
      }

      real_t m = -(real_t(expected_items) * std::log(false_positive_rate)) /
                 (std::log(2.) * std::log(2.));

      return std::max<nat_t>(1, nat_t(std::ceil(m)));
    }

    /** The number of hash functions (k) that minimizes the
        false-positive rate given `num_bits` (m) and `expected_items`
        (n): the standard `k = (m/n)*ln 2` formula. */
    static nat_t optimal_num_hashes(nat_t num_bits, nat_t expected_items)
    {
      if (expected_items == 0)
      {
        throw std::domain_error("expected_items must be positive");
      }

      real_t k = (real_t(num_bits) / real_t(expected_items)) * std::log(2.);

      return std::max<nat_t>(1, nat_t(std::round(k)));
    }
  };

} // end namespace Designar
