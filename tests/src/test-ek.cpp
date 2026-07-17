/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <maxflow.hpp>

using namespace Designar;
using namespace std;

using GT = Digraph<string, int_t>;

int main()
{
    // Classic textbook max-flow example (CLRS Fig 26.1-ish), max flow = 23
    GT g;
    auto s = g.insert_node("s");
    auto v1 = g.insert_node("v1");
    auto v2 = g.insert_node("v2");
    auto v3 = g.insert_node("v3");
    auto v4 = g.insert_node("v4");
    auto t = g.insert_node("t");

    g.insert_arc(s, v1, 16);
    g.insert_arc(s, v2, 13);
    g.insert_arc(v1, v2, 10);
    g.insert_arc(v2, v1, 4);
    g.insert_arc(v1, v3, 12);
    g.insert_arc(v3, v2, 9);
    g.insert_arc(v2, v4, 14);
    g.insert_arc(v4, v3, 7);
    g.insert_arc(v3, t, 20);
    g.insert_arc(v4, t, 4);

    EdmondsKarp<GT> ek(g);
    int_t flow = ek.max_flow(s, t);
    assert(flow == 23);

    // Simple diamond: s->a->t and s->b->t, capacities 5 each path -> max flow
    // 10
    GT g2;
    auto ss = g2.insert_node("s");
    auto a = g2.insert_node("a");
    auto b = g2.insert_node("b");
    auto tt = g2.insert_node("t");
    g2.insert_arc(ss, a, 5);
    g2.insert_arc(a, tt, 5);
    g2.insert_arc(ss, b, 5);
    g2.insert_arc(b, tt, 5);

    EdmondsKarp<GT> ek2(g2);
    assert(ek2.max_flow(ss, tt) == 10);

    // Bottleneck: s->a(10)->t but a->t only 3
    GT g3;
    auto s3 = g3.insert_node("s");
    auto a3 = g3.insert_node("a");
    auto t3 = g3.insert_node("t");
    g3.insert_arc(s3, a3, 10);
    g3.insert_arc(a3, t3, 3);
    EdmondsKarp<GT> ek3(g3);
    assert(ek3.max_flow(s3, t3) == 3);

    // No path -> 0 flow
    GT g4;
    auto s4 = g4.insert_node("s");
    auto t4 = g4.insert_node("t");
    g4.insert_node("isolated");
    EdmondsKarp<GT> ek4(g4);
    assert(ek4.max_flow(s4, t4) == 0);

    cout << "EdmondsKarp: Everything ok!\n";
    return 0;
}
