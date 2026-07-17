/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <bloomfilter.hpp>

using namespace Designar;

int main()
{
    nat_t expected_items = 1000;
    real_t target_fp_rate = 0.01;

    nat_t num_bits =
        BloomFilter<string>::optimal_num_bits(expected_items, target_fp_rate);
    nat_t num_hashes =
        BloomFilter<string>::optimal_num_hashes(num_bits, expected_items);

    BloomFilter<string> filter(num_bits, num_hashes);

    filter.insert(string("apple"));
    filter.insert(string("banana"));

    cout << "might_contain(\"apple\"): "
         << (filter.might_contain(string("apple")) ? "yes" : "no") << endl;

    // A false positive is possible here (never a false negative) — the
    // whole trade-off a Bloom filter makes for O(1) space per element
    // regardless of the element's own size.
    cout << "might_contain(\"cherry\") (never inserted): "
         << (filter.might_contain(string("cherry")) ? "yes" : "no") << endl;

    return 0;
}
