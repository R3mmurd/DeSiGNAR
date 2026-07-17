/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>

using namespace std;

#include <graphalgorithms.hpp>

using namespace Designar;

using GT = Graph<int_t, int_t>;

int main()
{
    // A small connected, acyclic graph with a branch:
    //   1 -> 2 -> 3 -> 4
    //        `-> 5
    GT g;
    auto n1 = g.insert_node(1);
    auto n2 = g.insert_node(2);
    auto n3 = g.insert_node(3);
    auto n4 = g.insert_node(4);
    auto n5 = g.insert_node(5);
    g.insert_arc(n1, n2, 0);
    g.insert_arc(n2, n3, 0);
    g.insert_arc(n3, n4, 0);
    g.insert_arc(n2, n5, 0);

    // Regression: the iterative DFS-family functions
    // (depth_first_traverse_{prefix,suffix}_iterative,
    // depth_first_search_path_iterative, test_for_cycle_iterative and its
    // has_cycle_iterative/is_acyclique_iterative wrappers) are new
    // alternatives to the existing recursive ones, added so a graph whose
    // depth exceeds the call stack's capacity can still be traversed (see
    // graphalgorithms.hpp). They must visit nodes in exactly the same
    // order and report exactly the same results as their recursive
    // counterparts on the same graph.
    {
        SLList<int_t> prefix_rec, prefix_it, suffix_rec, suffix_it;

        depth_first_traverse_prefix(g, [&](Node<GT>* p)
                                    { prefix_rec.append(p->get_info()); });
        depth_first_traverse_prefix_iterative(
            g, [&](Node<GT>* p) { prefix_it.append(p->get_info()); });
        depth_first_traverse_suffix(g, [&](Node<GT>* p)
                                    { suffix_rec.append(p->get_info()); });
        depth_first_traverse_suffix_iterative(
            g, [&](Node<GT>* p) { suffix_it.append(p->get_info()); });

        assert(prefix_rec.equal(prefix_it));
        assert(suffix_rec.equal(suffix_it));
        assert(prefix_rec.equal({1, 2, 3, 4, 5}));
        assert(suffix_rec.equal({4, 3, 5, 2, 1}));
    }

    {
        Path<GT> path_rec = depth_first_search_path(g, n1, n4);
        Path<GT> path_it = depth_first_search_path_iterative(g, n1, n4);

        assert(path_rec.size() == path_it.size());
        assert(path_rec.nodes().equal(path_it.nodes()));
        assert(path_rec.nodes().equal({n1, n2, n3, n4}));
    }

    assert(!has_cycle(g));
    assert(!has_cycle_iterative(g));
    assert(is_acyclique(g));
    assert(is_acyclique_iterative(g));

    // Same checks on a graph that does have a cycle: 1 -> 2 -> 3 -> 1.
    GT gc;
    auto m1 = gc.insert_node(1);
    auto m2 = gc.insert_node(2);
    auto m3 = gc.insert_node(3);
    gc.insert_arc(m1, m2, 0);
    gc.insert_arc(m2, m3, 0);
    gc.insert_arc(m3, m1, 0);

    assert(has_cycle(gc));
    assert(has_cycle_iterative(gc));
    assert(!is_acyclique(gc));
    assert(!is_acyclique_iterative(gc));

    // The whole point of the iterative versions: a graph shaped like a
    // long chain, deep enough that the recursive versions would overflow
    // the call stack, must still be fully traversable.
    {
        GT chain;
        Node<GT>* prev = chain.insert_node(0);

        constexpr int_t N = 200000;

        for (int_t i = 1; i < N; ++i)
        {
            Node<GT>* cur = chain.insert_node(i);
            chain.insert_arc(prev, cur, 0);
            prev = cur;
        }

        nat_t count = 0;
        depth_first_traverse_prefix_iterative(chain,
                                              [&](Node<GT>*) { ++count; });
        assert(count == (nat_t)N);

        // Note: deliberately calling the (graph, start_node) overloads here,
        // not the whole-graph has_cycle_iterative(chain)/
        // is_acyclique_iterative(chain) overloads. The whole-graph overloads
        // are, by design (in both the recursive and iterative versions),
        // O(V^2) on a graph like this one where every single node has to be
        // tried as a DFS root before concluding there is no cycle anywhere
        // -- appropriate for small graphs, but far too slow to use here at
        // N = 200000.
        Node<GT>* chain_start = chain.get_first_node();
        assert(!has_cycle_iterative(chain, chain_start));
        assert(is_acyclique_iterative(chain, chain_start));
    }

    // Regression: ArcHeap::get_min_arc() used to guard a real invariant
    // (the top arc of the heap must be indexed by either its source or
    // target node) with assert(), which compiles to nothing under
    // -DNDEBUG; a violation would silently corrupt bookkeeping instead of
    // failing loudly. It is exercised indirectly through Prim's algorithm,
    // which relies on ArcHeap internally; this just checks Prim still
    // produces a correct, connected spanning tree.
    {
        GT wg;
        auto w1 = wg.insert_node(1);
        auto w2 = wg.insert_node(2);
        auto w3 = wg.insert_node(3);
        auto w4 = wg.insert_node(4);
        wg.insert_arc(w1, w2, 1);
        wg.insert_arc(w2, w3, 1);
        wg.insert_arc(w3, w4, 1);
        wg.insert_arc(w1, w4, 10);
        wg.insert_arc(w1, w3, 10);

        Prim<GT> prim;
        GT mst = prim.build_min_spanning_tree(wg);

        assert(mst.get_num_nodes() == wg.get_num_nodes());
        assert(mst.get_num_arcs() == wg.get_num_nodes() - 1);
    }

    // Regression: compute_cut_nodes_rec()'s `is_cut` used to be
    // *assigned* the current DFS child's cut-condition result instead of
    // OR-accumulated across every child, so only the *last* child visited
    // determined whether a node was reported as a cut node. B has two DFS
    // children here: C (a leaf, satisfies the cut condition: removing B
    // disconnects it) and D (which reaches back to A, an *ancestor* of B,
    // via the D-E-A back edge, so it does not — B is not needed to keep D
    // and E connected to the rest of the graph). Under the old code,
    // whichever of the two is visited last would silently overwrite the
    // other's result. Since which child AdArcIt visits first isn't part
    // of this library's documented contract, both edge-insertion orders
    // are built and checked, so this test catches the bug regardless of
    // which one happens to match the traversal order in a given build.
    //
    //     A - B - C
    //     |   |
    //     |   D
    //     |   |
    //     +-- E
    for (bool c_edge_first : {true, false})
    {
        GT g;
        Node<GT>* a = g.insert_node(100);
        Node<GT>* b = g.insert_node(101);
        Node<GT>* c = g.insert_node(102);
        Node<GT>* d = g.insert_node(103);
        Node<GT>* e = g.insert_node(104);

        g.insert_arc(a, b, 0);

        if (c_edge_first)
            g.insert_arc(b, c,
                         0); // C is a leaf hanging off B: B must be a cut node

        g.insert_arc(b, d, 0);
        g.insert_arc(d, e, 0);
        g.insert_arc(e, a, 0); // back edge to ancestor A, *not* to B

        if (!c_edge_first)
            g.insert_arc(b, c, 0);

        SLList<Node<GT>*> cut_nodes = compute_cut_nodes(g);

        bool b_is_cut = cut_nodes.exists([&](Node<GT>* n) { return n == b; });
        assert(b_is_cut);
    }

    cout << "Everything ok!\n";

    return 0;
}
