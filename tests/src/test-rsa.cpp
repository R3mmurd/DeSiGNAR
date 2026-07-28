/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>
#include <rsa.hpp>

using namespace std;
using namespace Designar;

int main()
{
    rng_t rng(2026);

    // Encrypt/decrypt round-trip over many random key pairs and
    // messages.
    for (int_t trial = 0; trial < 10; ++trial)
    {
        auto keys = rsa_generate_keypair<int_t>(rng);

        assert(keys.n > 1);
        assert(keys.e > 1);
        assert(keys.d > 0);

        for (int_t message = 0; message < 20 && message < keys.n; ++message)
        {
            int_t ciphertext = rsa_encrypt(message, keys.n, keys.e);
            int_t decrypted = rsa_decrypt(ciphertext, keys.n, keys.d);
            assert(decrypted == message);
        }

        // Encryption is not the identity function (for a non-trivial
        // message) — a very weak sanity check that something real is
        // happening, not that encrypt() just passes the value through.
        if (keys.n > 10)
        {
            int_t message = 7;
            int_t ciphertext = rsa_encrypt(message, keys.n, keys.e);
            assert(ciphertext != message);
        }
    }

    // A message outside [0, n) must be rejected.
    {
        auto keys = rsa_generate_keypair<int_t>(rng);
        bool threw = false;

        try
        {
            rsa_encrypt(keys.n, keys.n, keys.e);
        }
        catch (const std::domain_error&)
        {
            threw = true;
        }

        assert(threw);

        threw = false;

        try
        {
            rsa_encrypt(int_t(-1), keys.n, keys.e);
        }
        catch (const std::domain_error&)
        {
            threw = true;
        }

        assert(threw);
    }

    cout << "Everything ok!\n";

    return 0;
}
