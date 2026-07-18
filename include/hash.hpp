/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file hash.hpp
    @brief Hash functions: super_fast_hash() (a fast, non-cryptographic
    hash used throughout this library's own hash tables — see
    chainedhash.hpp/openhash.hpp) and MD5/SHA-1/SHA-256, the classic
    cryptographic message digests, for callers that need a hash with
    actual security properties (integrity verification, hashchain.hpp's
    hash chains, password/commitment schemes) rather than just a fast
    bucket index. MD5 and SHA-1 are included for their teaching value
    and for interoperating with formats/protocols still built on them,
    but both have known, practical collision attacks — see their own
    doc comments below; SHA-256 is the one actually fit for a real
    security use today.
    @ingroup Hashing
*/

#pragma once

#include <cstdint>
#include <cstring>
#include <string>

#include <types.hpp>
#include <array.hpp>

#undef get16bits
#if (defined(__GNUC__) && defined(__i386__)) || defined(__WATCOMC__) ||        \
    defined(_MSC_VER) || defined(__BORLANDC__) || defined(__TURBOC__)
#define get16bits(d) (*((const uint16_t*)(d)))
#endif

#if !defined(get16bits)
#define get16bits(d)                                                           \
    ((((uint32_t)(((const uint8_t*)(d))[1])) << 8) +                           \
     (uint32_t)(((const uint8_t*)(d))[0]))
#endif

namespace Designar
{
    nat_t super_fast_hash(void*, nat_t);

    inline nat_t super_fast_hash(const char* key)
    {
        return super_fast_hash((void*)key, strlen(key));
    }

    inline nat_t super_fast_hash(const std::string& key)
    {
        return super_fast_hash((void*)key.c_str(), key.size());
    }

    template <typename Key>
    inline nat_t super_fast_hash(const Key& key)
    {
        return super_fast_hash((void*)&key, sizeof(key));
    }

    template <typename First, typename Second>
    inline nat_t super_fast_hash(const std::pair<First, Second>& p)
    {
        return super_fast_hash<First>(p.first) ^
               super_fast_hash<Second>(p.second);
    }

    template <typename T>
    void hash_combine(size_t& seed, const T& val)
    {
        seed ^=
            super_fast_hash<T>(val) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    }

    template <typename T, typename... Ts>
    void hash_combine(size_t& seed, const T& val, const Ts&... args)
    {
        hash_combine(seed, val);
        hash_combine(seed, args...);
    }

    void hash_combine(size_t& seed);

    template <typename... Ts>
    size_t hash_val(const Ts&... args)
    {
        size_t seed{0};
        hash_combine(seed, args...);
        return seed;
    }

    /** Renders a digest (as returned by md5()/sha1()/sha256() below) as
        a lowercase hex string — the conventional way to print/compare/
        store a message digest, and the format every published test
        vector for these algorithms is written in. */
    std::string to_hex(const DynArray<unsigned char>& digest);

    /** MD5 (Rivest, RFC 1321): produces a 16-byte digest.
        @warning Cryptographically broken — practical collision attacks
        are known (two *different* inputs producing the same digest can
        be constructed deliberately). Included for its teaching value
        and for interoperating with legacy formats/protocols still built
        on it; do not use it anywhere a collision would matter (digital
        signatures, certificate fingerprints, deduplication of untrusted
        input). Use sha256() for real security use. */
    DynArray<unsigned char> md5(const void* data, nat_t len);

    inline DynArray<unsigned char> md5(const std::string& s)
    {
        return md5(s.data(), s.size());
    }

    /** SHA-1 (NIST FIPS 180-4): produces a 20-byte digest.
        @warning Cryptographically broken the same way MD5 is (a
        practical, demonstrated collision attack exists — the 2017
        "SHAttered" attack). Same caveats as md5() above; use sha256()
        for real security use. */
    DynArray<unsigned char> sha1(const void* data, nat_t len);

    inline DynArray<unsigned char> sha1(const std::string& s)
    {
        return sha1(s.data(), s.size());
    }

    /** SHA-256 (NIST FIPS 180-4, the SHA-2 family): produces a 32-byte
        digest. No practical collision or preimage attack is known — this
        is the one to actually reach for when a hash needs real security
        properties (integrity verification, commitments, hashchain.hpp's
        hash chains). */
    DynArray<unsigned char> sha256(const void* data, nat_t len);

    inline DynArray<unsigned char> sha256(const std::string& s)
    {
        return sha256(s.data(), s.size());
    }

} // end namespace Designar
