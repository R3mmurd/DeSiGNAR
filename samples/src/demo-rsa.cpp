/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <rsa.hpp>

using namespace Designar;

int main()
{
    rng_t rng(2026);

    auto keys = rsa_generate_keypair<int_t>(rng);

    cout << "Generated key pair:\n";
    cout << "  n = " << keys.n << endl;
    cout << "  e = " << keys.e << " (public)" << endl;
    cout << "  d = " << keys.d << " (private)" << endl;

    int_t message = 42;
    cout << "\nmessage = " << message << endl;

    int_t ciphertext = rsa_encrypt(message, keys.n, keys.e);
    cout << "ciphertext = rsa_encrypt(message, n, e) = " << ciphertext << endl;

    int_t decrypted = rsa_decrypt(ciphertext, keys.n, keys.d);
    cout << "rsa_decrypt(ciphertext, n, d) = " << decrypted
         << (decrypted == message ? " (matches original message)" : " (MISMATCH!)")
         << endl;

    return 0;
}
