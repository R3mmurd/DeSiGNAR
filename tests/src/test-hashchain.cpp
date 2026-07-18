/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <hashchain.hpp>
#include <hash.hpp>

using namespace Designar;

namespace
{
    DynArray<unsigned char> sha256_adapter(const DynArray<unsigned char>& b)
    {
        return sha256(&b[0], b.size());
    }
} // end anonymous namespace

int main()
{
    DynArray<unsigned char> seed = sha256(std::string("hash chain seed"));
    constexpr nat_t LENGTH = 50;

    auto chain = make_hash_chain(seed, LENGTH, sha256_adapter);

    assert(chain.chain_length() == LENGTH);
    assert(chain.seed_value().equal(seed));
    assert(chain.commitment().size() == 32);

    // Rebuild every intermediate link by hand to check verify() against
    // each one — link[0] is the seed itself, link[LENGTH] is the
    // commitment.
    DynArray<DynArray<unsigned char>> links;
    DynArray<unsigned char> current = seed;
    links.append(current);

    for (nat_t i = 0; i < LENGTH; ++i)
    {
        current = sha256_adapter(current);
        links.append(current);
    }

    assert(links[LENGTH].equal(chain.commitment()));

    // S/KEY-style check: link[k] is exactly (LENGTH - k) steps before
    // the commitment.
    for (nat_t k = 0; k <= LENGTH; ++k)
    {
        assert(chain.verify(links[k], LENGTH - k));
    }

    // A candidate that isn't on the chain at all must fail, regardless
    // of remaining_steps.
    DynArray<unsigned char> not_on_chain = sha256(std::string("impostor"));

    for (nat_t steps = 0; steps <= LENGTH; ++steps)
    {
        assert(!chain.verify(not_on_chain, steps));
    }

    // A genuine link revealed with the *wrong* remaining_steps must also
    // fail — this is what stops a used-once value from being replayed
    // as if it were a different, not-yet-revealed link.
    assert(!chain.verify(links[25], 24));
    assert(!chain.verify(links[25], 26));

    return 0;
}
