/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <trie.hpp>

using namespace Designar;

int main()
{
    Trie<char> trie;

    trie.insert(string("cat"));
    trie.insert(string("car"));
    trie.insert(string("cart"));
    trie.insert(string("dog"));

    cout << "search(\"car\"): "
         << (trie.search(string("car")) ? "found" : "not found") << endl;
    cout << "search(\"ca\"): "
         << (trie.search(string("ca")) ? "found" : "not found") << endl;

    trie.remove(string("car"));

    cout << "after remove(\"car\"), search(\"car\"): "
         << (trie.search(string("car")) ? "found" : "not found") << endl;
    cout << "search(\"cart\") (shares a prefix with the removed word): "
         << (trie.search(string("cart")) ? "found" : "not found") << endl;

    return 0;
}
