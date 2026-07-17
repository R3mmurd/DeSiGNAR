/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <maxflow.hpp>

using namespace Designar;

using GT = Digraph<string, int_t>;

int main()
{
    // Diamond: s->a->t and s->b->t, capacity 5 on every arc, so the max
    // flow is 10 (5 through each path).
    GT g;
    auto s = g.insert_node("s");
    auto a = g.insert_node("a");
    auto b = g.insert_node("b");
    auto t = g.insert_node("t");

    g.insert_arc(s, a, 5);
    g.insert_arc(a, t, 5);
    g.insert_arc(s, b, 5);
    g.insert_arc(b, t, 5);

    EdmondsKarp<GT> ek(g);
    cout << "EdmondsKarp   max flow s -> t: " << ek.max_flow(s, t) << endl;

    FordFulkerson<GT> ff(g);
    cout << "FordFulkerson max flow s -> t: " << ff.max_flow(s, t) << endl;

    return 0;
}
