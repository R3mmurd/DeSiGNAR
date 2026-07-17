/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <unionfind.hpp>

using namespace Designar;

int main()
{
    DisjointSet<string> friends;

    for (const string& name : {string("Ana"), string("Beto"), string("Caro"),
                               string("Dario"), string("Eva")})
    {
        friends.make_set(name);
    }

    friends.union_sets("Ana", "Beto");
    friends.union_sets("Caro", "Dario");

    cout << "Ana and Beto in same set: "
         << (friends.same_set("Ana", "Beto") ? "yes" : "no") << endl;

    cout << "Ana and Caro in same set: "
         << (friends.same_set("Ana", "Caro") ? "yes" : "no") << endl;

    friends.union_sets("Beto", "Caro");

    cout << "after union_sets(Beto, Caro), Ana and Dario in same set: "
         << (friends.same_set("Ana", "Dario") ? "yes" : "no") << endl;

    return 0;
}
