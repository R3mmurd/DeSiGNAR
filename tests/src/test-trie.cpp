/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <string>

#include <trie.hpp>

using namespace Designar;
using namespace std;

int main()
{
    Trie<char> t;
    assert(t.is_empty());

    assert(t.insert(string("cat")));
    assert(t.insert(string("car")));
    assert(t.insert(string("cart")));
    assert(t.insert(string("dog")));
    assert(!t.insert(string("cat"))); // duplicate

    assert(t.size() == 4);

    assert(t.search(string("cat")));
    assert(t.search(string("car")));
    assert(t.search(string("cart")));
    assert(t.search(string("dog")));
    assert(!t.search(string("ca")));
    assert(!t.search(string("do")));
    assert(!t.search(string("cats")));

    assert(t.has_prefix(string("ca")));
    assert(t.has_prefix(string("car")));
    assert(t.has_prefix(string("do")));
    assert(!t.has_prefix(string("xyz")));
    assert(!t.has_prefix(string("cats")));

    assert(t.remove(string("car")));
    assert(!t.search(string("car")));
    assert(t.search(string("cart")));    // shared-prefix sibling still there
    assert(t.has_prefix(string("car"))); // "cart" still passes through "car"
    assert(t.size() == 3);
    assert(!t.remove(string("car"))); // already removed

    Trie<char> t2 = t; // copy
    t2.insert(string("zzz"));
    assert(t2.search(string("zzz")));
    assert(!t.search(string("zzz"))); // independent copy

    Trie<char> t3 = std::move(t2);
    assert(t3.search(string("zzz")));

    cout << "Trie: Everything ok!\n";
    return 0;
}
