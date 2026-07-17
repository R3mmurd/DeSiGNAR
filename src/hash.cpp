/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <hash.hpp>

#include <random>

namespace Designar
{

    namespace
    {
        /** A per-process random salt mixed into every super_fast_hash()
            computation.

            super_fast_hash() is a fixed, publicly-known, unkeyed hash
            function (Hsieh's SuperFastHash). If a program ever stores
            attacker-controlled keys in an LHashTable (e.g. HTTP header
            names, form field names), an attacker who knows the exact hash
            function can precompute a large set of colliding keys and
            degrade every table operation from expected O(1) to O(n) — a
            classic algorithmic-complexity / hash-flooding denial-of-service
            (the same class of attack that led PHP, Python, and Ruby to add
            randomized hash seeding around 2011-2012).

            Mixing in a salt that is randomized once per process and never
            exposed makes such precomputation useless: the attacker cannot
            know the salt in advance, so colliding keys for *this* process
            cannot be prepared ahead of time. This does not change hash
            table correctness — LHashTable only ever needs hash values to be
            consistent *within* a single run, never across runs or
            processes — so lookups, inserts, and iteration all behave
            exactly as before; only the concrete numeric hash values differ
            from run to run. */
        nat_t process_hash_salt()
        {
            static const nat_t salt = []() -> nat_t
            {
                std::random_device rd;
                return (nat_t(rd()) << 32) ^ nat_t(rd());
            }();

            return salt;
        }
    } // end anonymous namespace

    nat_t super_fast_hash(void* key, nat_t len)
    {
        const unsigned char* data = reinterpret_cast<unsigned char*>(key);

        nat_t hash = len ^ process_hash_salt(), tmp;

        int rem;

        if (len <= 0 || data == nullptr)
        {
            return 0;
        }

        rem = len & 3;
        len >>= 2;

        for (; len > 0; --len)
        {
            hash += get16bits(data);
            tmp = (get16bits(data + 2) << 11) ^ hash;
            hash = (hash << 16) ^ tmp;
            data += 2 * sizeof(uint16_t);
            hash += hash >> 11;
        }

        switch (rem)
        {
            case 3:
                hash += get16bits(data);
                hash ^= hash << 16;
                hash ^= ((signed char)data[sizeof(uint16_t)]) << 18;
                hash += hash >> 11;
                break;
            case 2:
                hash += get16bits(data);
                hash ^= hash << 11;
                hash += hash >> 17;
                break;
            case 1:
                hash += (signed char)*data;
                hash ^= hash << 10;
                hash += hash >> 1;
        }

        /* Force "avalanching" of final 127 bits */
        hash ^= hash << 3;
        hash += hash >> 5;
        hash ^= hash << 4;
        hash += hash >> 17;
        hash ^= hash << 25;
        hash += hash >> 6;

        return hash;
    }

} // end namespace Designar
