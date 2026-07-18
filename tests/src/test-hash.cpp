/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <hash.hpp>

using namespace Designar;

int main()
{
    // super_fast_hash: not a cryptographic hash, but it must at least be
    // deterministic within a run and actually distinguish different
    // inputs (see chainedhash.hpp/openhash.hpp for it in actual use as a
    // table hash).
    assert(super_fast_hash("hello") == super_fast_hash("hello"));
    assert(super_fast_hash("hello") != super_fast_hash("world"));
    assert(super_fast_hash(std::string("hello")) ==
           super_fast_hash("hello"));

    // MD5/SHA-1/SHA-256 against the standard published test vectors —
    // FIPS 180-4's "abc"/56-character examples and RFC 1321's MD5
    // vectors. The 56-character message is deliberately chosen: it's the
    // shortest input that pushes the algorithm into a *second* 64-byte
    // block, exercising the padding/bit-length-suffix logic that a
    // single-block message (like "" or "abc") never touches.
    const std::string fips_msg =
        "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";

    assert(to_hex(md5(std::string(""))) ==
           "d41d8cd98f00b204e9800998ecf8427e");
    assert(to_hex(md5(std::string("abc"))) ==
           "900150983cd24fb0d6963f7d28e17f72");
    assert(to_hex(md5(fips_msg)) == "8215ef0796a20bcaaae116d3876c664a");
    assert(md5(std::string("")).size() == 16);

    assert(to_hex(sha1(std::string(""))) ==
           "da39a3ee5e6b4b0d3255bfef95601890afd80709");
    assert(to_hex(sha1(std::string("abc"))) ==
           "a9993e364706816aba3e25717850c26c9cd0d89d");
    assert(to_hex(sha1(fips_msg)) ==
           "84983e441c3bd26ebaae4aa1f95129e5e54670f1");
    assert(sha1(std::string("")).size() == 20);

    assert(to_hex(sha256(std::string(""))) ==
           "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855");
    assert(to_hex(sha256(std::string("abc"))) ==
           "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad");
    assert(to_hex(sha256(fips_msg)) ==
           "248d6a61d20638b8e5c026930c3e6039a33ce45964ff2167f6ecedd419db06c1");
    assert(sha256(std::string("")).size() == 32);

    // Every algorithm here is deterministic and distinguishes different
    // inputs — the bare minimum any hash function needs, cryptographic
    // or not.
    assert(to_hex(sha256(std::string("abc"))) !=
           to_hex(sha256(std::string("abd"))));

    // The const void*/nat_t overload and the std::string overload must
    // agree.
    std::string s = "designar";
    assert(to_hex(sha256(s.data(), s.size())) == to_hex(sha256(s)));

    // to_hex() round-trips through a known digest length: 2 hex
    // characters per byte, always lowercase.
    std::string hex = to_hex(sha256(std::string("designar")));
    assert(hex.size() == 64);

    for (char c : hex)
    {
        assert((c >= '0' && c <= '9') || (c >= 'a' && c <= 'f'));
    }

    return 0;
}
