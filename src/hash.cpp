/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <random>

#include <hash.hpp>

namespace Designar
{

    namespace
    {
        /** A per-process random salt mixed into every super_fast_hash()
            computation.

            super_fast_hash() is a fixed, publicly-known, unkeyed hash
            function (Hsieh's SuperFastHash). If a program ever stores
            attacker-controlled keys in a SeparateChainingHashTable (e.g.
            HTTP header names, form field names), an attacker who knows
            the exact hash
            function can precompute a large set of colliding keys and
            degrade every table operation from expected O(1) to O(n) — a
            classic algorithmic-complexity / hash-flooding denial-of-service
            (the same class of attack that led PHP, Python, and Ruby to add
            randomized hash seeding around 2011-2012).

            Mixing in a salt that is randomized once per process and never
            exposed makes such precomputation useless: the attacker cannot
            know the salt in advance, so colliding keys for *this* process
            cannot be prepared ahead of time. This does not change hash
            table correctness — a hash table only ever needs hash values
            to be consistent *within* a single run, never across runs or
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

    namespace
    {
        inline uint32_t rotl32(uint32_t x, uint32_t c)
        {
            return (x << c) | (x >> (32 - c));
        }

        inline uint32_t rotr32(uint32_t x, uint32_t c)
        {
            return (x >> c) | (x << (32 - c));
        }

        /** Common Merkle-Damgard padding for MD5/SHA-1/SHA-256 alike:
            append a single `1` bit (byte 0x80, since all three operate on
            whole bytes), then zero bytes until the total length is 8
            bytes short of a multiple of 64, then the original message
            length in bits as an 8-byte field — little-endian for MD5,
            big-endian for SHA-1/SHA-256 (`big_endian_length` picks
            which). */
        DynArray<unsigned char> pad_message(const unsigned char* data,
                                            nat_t len, bool big_endian_length)
        {
            uint64_t bit_len = uint64_t(len) * 8;
            nat_t new_len = ((len + 8) / 64 + 1) * 64;
            DynArray<unsigned char> msg(new_len, (unsigned char)0);

            for (nat_t i = 0; i < len; ++i)
            {
                msg[i] = data[i];
            }

            msg[len] = 0x80;

            for (nat_t i = 0; i < 8; ++i)
            {
                nat_t shift = big_endian_length ? (7 - i) : i;
                msg[new_len - 8 + i] =
                    (unsigned char)((bit_len >> (8 * shift)) & 0xff);
            }

            return msg;
        }

        constexpr uint32_t md5_K[64] = {
            0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee, 0xf57c0faf,
            0x4787c62a, 0xa8304613, 0xfd469501, 0x698098d8, 0x8b44f7af,
            0xffff5bb1, 0x895cd7be, 0x6b901122, 0xfd987193, 0xa679438e,
            0x49b40821, 0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
            0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8, 0x21e1cde6,
            0xc33707d6, 0xf4d50d87, 0x455a14ed, 0xa9e3e905, 0xfcefa3f8,
            0x676f02d9, 0x8d2a4c8a, 0xfffa3942, 0x8771f681, 0x6d9d6122,
            0xfde5380c, 0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
            0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05, 0xd9d4d039,
            0xe6db99e5, 0x1fa27cf8, 0xc4ac5665, 0xf4292244, 0x432aff97,
            0xab9423a7, 0xfc93a039, 0x655b59c3, 0x8f0ccc92, 0xffeff47d,
            0x85845dd1, 0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
            0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391};

        constexpr uint32_t md5_S[64] = {
            7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
            5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20, 5, 9,  14, 20,
            4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
            6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21};

        constexpr uint32_t sha256_K[64] = {
            0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b,
            0x59f111f1, 0x923f82a4, 0xab1c5ed5, 0xd807aa98, 0x12835b01,
            0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7,
            0xc19bf174, 0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc,
            0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da, 0x983e5152,
            0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147,
            0x06ca6351, 0x14292967, 0x27b70a85, 0x2e1b2138, 0x4d2c6dfc,
            0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
            0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819,
            0xd6990624, 0xf40e3585, 0x106aa070, 0x19a4c116, 0x1e376c08,
            0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f,
            0x682e6ff3, 0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208,
            0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};
    } // end anonymous namespace

    std::string to_hex(const DynArray<unsigned char>& digest)
    {
        static const char* hex_chars = "0123456789abcdef";
        std::string result;

        for (nat_t i = 0; i < digest.size(); ++i)
        {
            result += hex_chars[(digest[i] >> 4) & 0xf];
            result += hex_chars[digest[i] & 0xf];
        }

        return result;
    }

    DynArray<unsigned char> md5(const void* data_ptr, nat_t len)
    {
        const unsigned char* data =
            reinterpret_cast<const unsigned char*>(data_ptr);
        DynArray<unsigned char> msg = pad_message(data, len, false);

        uint32_t a0 = 0x67452301, b0 = 0xefcdab89, c0 = 0x98badcfe,
                 d0 = 0x10325476;

        for (nat_t offset = 0; offset < msg.size(); offset += 64)
        {
            uint32_t M[16];

            for (nat_t i = 0; i < 16; ++i)
            {
                M[i] = (uint32_t)msg[offset + i * 4] |
                       ((uint32_t)msg[offset + i * 4 + 1] << 8) |
                       ((uint32_t)msg[offset + i * 4 + 2] << 16) |
                       ((uint32_t)msg[offset + i * 4 + 3] << 24);
            }

            uint32_t A = a0, B = b0, C = c0, D = d0;

            for (nat_t i = 0; i < 64; ++i)
            {
                uint32_t F;
                nat_t g;

                if (i < 16)
                {
                    F = (B & C) | (~B & D);
                    g = i;
                }
                else if (i < 32)
                {
                    F = (D & B) | (~D & C);
                    g = (5 * i + 1) % 16;
                }
                else if (i < 48)
                {
                    F = B ^ C ^ D;
                    g = (3 * i + 5) % 16;
                }
                else
                {
                    F = C ^ (B | ~D);
                    g = (7 * i) % 16;
                }

                F = F + A + md5_K[i] + M[g];
                A = D;
                D = C;
                C = B;
                B = B + rotl32(F, md5_S[i]);
            }

            a0 += A;
            b0 += B;
            c0 += C;
            d0 += D;
        }

        DynArray<unsigned char> digest(16, (unsigned char)0);
        uint32_t vals[4] = {a0, b0, c0, d0};

        for (nat_t i = 0; i < 4; ++i)
        {
            for (nat_t j = 0; j < 4; ++j)
            {
                digest[i * 4 + j] = (unsigned char)((vals[i] >> (8 * j)) & 0xff);
            }
        }

        return digest;
    }

    DynArray<unsigned char> sha1(const void* data_ptr, nat_t len)
    {
        const unsigned char* data =
            reinterpret_cast<const unsigned char*>(data_ptr);
        DynArray<unsigned char> msg = pad_message(data, len, true);

        uint32_t h0 = 0x67452301, h1 = 0xEFCDAB89, h2 = 0x98BADCFE,
                 h3 = 0x10325476, h4 = 0xC3D2E1F0;

        for (nat_t offset = 0; offset < msg.size(); offset += 64)
        {
            uint32_t w[80];

            for (nat_t i = 0; i < 16; ++i)
            {
                w[i] = ((uint32_t)msg[offset + i * 4] << 24) |
                       ((uint32_t)msg[offset + i * 4 + 1] << 16) |
                       ((uint32_t)msg[offset + i * 4 + 2] << 8) |
                       ((uint32_t)msg[offset + i * 4 + 3]);
            }

            for (nat_t i = 16; i < 80; ++i)
            {
                w[i] = rotl32(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
            }

            uint32_t a = h0, b = h1, c = h2, d = h3, e = h4;

            for (nat_t i = 0; i < 80; ++i)
            {
                uint32_t f, k;

                if (i < 20)
                {
                    f = (b & c) | ((~b) & d);
                    k = 0x5A827999;
                }
                else if (i < 40)
                {
                    f = b ^ c ^ d;
                    k = 0x6ED9EBA1;
                }
                else if (i < 60)
                {
                    f = (b & c) | (b & d) | (c & d);
                    k = 0x8F1BBCDC;
                }
                else
                {
                    f = b ^ c ^ d;
                    k = 0xCA62C1D6;
                }

                uint32_t temp = rotl32(a, 5) + f + e + k + w[i];
                e = d;
                d = c;
                c = rotl32(b, 30);
                b = a;
                a = temp;
            }

            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
        }

        DynArray<unsigned char> digest(20, (unsigned char)0);
        uint32_t vals[5] = {h0, h1, h2, h3, h4};

        for (nat_t i = 0; i < 5; ++i)
        {
            for (nat_t j = 0; j < 4; ++j)
            {
                digest[i * 4 + j] =
                    (unsigned char)((vals[i] >> (8 * (3 - j))) & 0xff);
            }
        }

        return digest;
    }

    DynArray<unsigned char> sha256(const void* data_ptr, nat_t len)
    {
        const unsigned char* data =
            reinterpret_cast<const unsigned char*>(data_ptr);
        DynArray<unsigned char> msg = pad_message(data, len, true);

        uint32_t h0 = 0x6a09e667, h1 = 0xbb67ae85, h2 = 0x3c6ef372,
                 h3 = 0xa54ff53a, h4 = 0x510e527f, h5 = 0x9b05688c,
                 h6 = 0x1f83d9ab, h7 = 0x5be0cd19;

        for (nat_t offset = 0; offset < msg.size(); offset += 64)
        {
            uint32_t w[64];

            for (nat_t i = 0; i < 16; ++i)
            {
                w[i] = ((uint32_t)msg[offset + i * 4] << 24) |
                       ((uint32_t)msg[offset + i * 4 + 1] << 16) |
                       ((uint32_t)msg[offset + i * 4 + 2] << 8) |
                       ((uint32_t)msg[offset + i * 4 + 3]);
            }

            for (nat_t i = 16; i < 64; ++i)
            {
                uint32_t s0 = rotr32(w[i - 15], 7) ^ rotr32(w[i - 15], 18) ^
                             (w[i - 15] >> 3);
                uint32_t s1 = rotr32(w[i - 2], 17) ^ rotr32(w[i - 2], 19) ^
                             (w[i - 2] >> 10);
                w[i] = w[i - 16] + s0 + w[i - 7] + s1;
            }

            uint32_t a = h0, b = h1, c = h2, d = h3, e = h4, f = h5, g = h6,
                     h = h7;

            for (nat_t i = 0; i < 64; ++i)
            {
                uint32_t S1 = rotr32(e, 6) ^ rotr32(e, 11) ^ rotr32(e, 25);
                uint32_t ch = (e & f) ^ ((~e) & g);
                uint32_t temp1 = h + S1 + ch + sha256_K[i] + w[i];
                uint32_t S0 = rotr32(a, 2) ^ rotr32(a, 13) ^ rotr32(a, 22);
                uint32_t maj = (a & b) ^ (a & c) ^ (b & c);
                uint32_t temp2 = S0 + maj;

                h = g;
                g = f;
                f = e;
                e = d + temp1;
                d = c;
                c = b;
                b = a;
                a = temp1 + temp2;
            }

            h0 += a;
            h1 += b;
            h2 += c;
            h3 += d;
            h4 += e;
            h5 += f;
            h6 += g;
            h7 += h;
        }

        DynArray<unsigned char> digest(32, (unsigned char)0);
        uint32_t vals[8] = {h0, h1, h2, h3, h4, h5, h6, h7};

        for (nat_t i = 0; i < 8; ++i)
        {
            for (nat_t j = 0; j < 4; ++j)
            {
                digest[i * 4 + j] =
                    (unsigned char)((vals[i] >> (8 * (3 - j))) & 0xff);
            }
        }

        return digest;
    }

} // end namespace Designar
