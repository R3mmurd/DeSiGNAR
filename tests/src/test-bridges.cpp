/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <bridges.hpp>
#include <bipartitematching.hpp>

using namespace Designar;
using namespace std;

using GT = Graph<int_t, int_t>;

int main()
{
    // A - B - C, with B - D - E - B triangle, and F hanging off C.
    // Bridges: A-B, B-C, C-F. Non-bridges: B-D, D-E, E-B (all part of the
    // cycle).
    GT g;
    Node<GT>* a = g.insert_node(1);
    Node<GT>* b = g.insert_node(2);
    Node<GT>* c = g.insert_node(3);
    Node<GT>* d = g.insert_node(4);
    Node<GT>* e = g.insert_node(5);
    Node<GT>* f = g.insert_node(6);

    Arc<GT>* ab = g.insert_arc(a, b, 0);
    Arc<GT>* bc = g.insert_arc(b, c, 0);
    Arc<GT>* bd = g.insert_arc(b, d, 0);
    Arc<GT>* de = g.insert_arc(d, e, 0);
    Arc<GT>* eb = g.insert_arc(e, b, 0);
    Arc<GT>* cf = g.insert_arc(c, f, 0);

    SLList<Arc<GT>*> bridges = Bridges<GT>::find_bridges(g);
    assert(bridges.size() == 3);

    auto has = [&](Arc<GT>* target)
    { return bridges.exists([&](Arc<GT>* a2) { return a2 == target; }); };

    assert(has(ab));
    assert(has(bc));
    assert(has(cf));
    assert(!has(bd));
    assert(!has(de));
    assert(!has(eb));

    // A simple cycle has no bridges at all.
    GT cycle;
    Node<GT>* c1 = cycle.insert_node(1);
    Node<GT>* c2 = cycle.insert_node(2);
    Node<GT>* c3 = cycle.insert_node(3);
    cycle.insert_arc(c1, c2, 0);
    cycle.insert_arc(c2, c3, 0);
    cycle.insert_arc(c3, c1, 0);
    assert(Bridges<GT>::find_bridges(cycle).is_empty());

    // A tree: every edge is a bridge.
    GT tree;
    Node<GT>* t1 = tree.insert_node(1);
    Node<GT>* t2 = tree.insert_node(2);
    Node<GT>* t3 = tree.insert_node(3);
    tree.insert_arc(t1, t2, 0);
    tree.insert_arc(t2, t3, 0);
    assert(Bridges<GT>::find_bridges(tree).size() == 2);

    cout << "Bridges: Everything ok!\n";

    // Bipartite matching: classic example.
    // Left: L1,L2,L3  Right: R1,R2,R3
    // L1-R1, L1-R2, L2-R1, L3-R2, L3-R3  => max matching size 3
    GT bg;
    Node<GT>* l1 = bg.insert_node(101);
    Node<GT>* l2 = bg.insert_node(102);
    Node<GT>* l3 = bg.insert_node(103);
    Node<GT>* r1 = bg.insert_node(104);
    Node<GT>* r2 = bg.insert_node(105);
    Node<GT>* r3 = bg.insert_node(106);

    bg.insert_arc(l1, r1, 0);
    bg.insert_arc(l1, r2, 0);
    bg.insert_arc(l2, r1, 0);
    bg.insert_arc(l3, r2, 0);
    bg.insert_arc(l3, r3, 0);

    HashSet<Node<GT>*> left = {l1, l2, l3};
    BipartiteMatching<GT> bm(bg, left);
    auto matching = bm.compute();

    // Each left node's match must be its partner, symmetric, and every
    // matched node used exactly once (a valid matching).
    nat_t matched_pairs = 0;
    for (Node<GT>* l : left)
    {
        Node<GT>** r = matching.search(l);
        if (r != nullptr)
        {
            ++matched_pairs;
            Node<GT>** back = matching.search(*r);
            assert(back != nullptr && *back == l);
        }
    }
    assert(matched_pairs ==
           3); // this instance has a perfect matching on the left

    cout << "BipartiteMatching: Everything ok! (matched " << matched_pairs
         << " pairs)\n";
    return 0;
}
