/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

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
    // A tiny S/KEY-style one-time-password walkthrough: a server and a
    // user agree on a secret seed and a chain length up front. The
    // server stores only the *commitment* (the chain's final value) —
    // never the seed, never any intermediate link. Each time the user
    // authenticates, they reveal the next link working *backward* from
    // the commitment toward the seed; the server verifies it by
    // re-hashing forward to the last value it has on record, then
    // remembers this new link for next time (so the same value can
    // never be replayed).
    DynArray<unsigned char> seed = sha256(std::string("correct horse battery staple"));
    constexpr nat_t NUM_LOGINS = 5;

    auto chain = make_hash_chain(seed, NUM_LOGINS, sha256_adapter);

    cout << "Server stores only the commitment: " << to_hex(chain.commitment())
         << "\n\n";

    // The user precomputes every link once, from the seed forward —
    // they will reveal these in *reverse* order, one per login.
    DynArray<DynArray<unsigned char>> links;
    DynArray<unsigned char> current = seed;
    links.append(current);

    for (nat_t i = 0; i < NUM_LOGINS; ++i)
    {
        current = sha256_adapter(current);
        links.append(current);
    }

    DynArray<unsigned char> last_accepted = chain.commitment();
    nat_t remaining = NUM_LOGINS;

    for (nat_t login = 1; login <= NUM_LOGINS; ++login)
    {
        --remaining;
        const DynArray<unsigned char>& revealed = links[remaining];

        bool ok = chain.verify(revealed, NUM_LOGINS - remaining);

        cout << "Login " << login << ": user reveals " << to_hex(revealed)
             << " -> " << (ok ? "accepted" : "REJECTED") << endl;

        if (ok)
        {
            last_accepted = revealed;
        }
    }

    (void)last_accepted;

    // Replaying an already-used link must fail: it was verified against
    // the wrong number of remaining steps the second time around.
    bool replay_ok = chain.verify(links[NUM_LOGINS - 1], 0);
    cout << "\nReplaying the first login's value with the wrong step count: "
         << (replay_ok ? "accepted (BUG)" : "rejected, as expected") << endl;

    return 0;
}
