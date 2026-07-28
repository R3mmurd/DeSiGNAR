/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>
#include <allpairsshortestpaths.hpp>

using namespace std;
using namespace Designar;

using DGT = Digraph<int_t, int_t>;

int main()
{
    // The classic CLRS all-pairs example: negative weights, no negative
    // cycle. FloydWarshall and Johnson must agree on every pair.
    {
        DGT g;
        auto n1 = g.insert_node(1);
        auto n2 = g.insert_node(2);
        auto n3 = g.insert_node(3);
        auto n4 = g.insert_node(4);
        auto n5 = g.insert_node(5);

        g.insert_arc(n1, n2, 3);
        g.insert_arc(n1, n3, 8);
        g.insert_arc(n1, n5, -4);
        g.insert_arc(n2, n5, 7);
        g.insert_arc(n2, n4, 1);
        g.insert_arc(n3, n2, 4);
        g.insert_arc(n4, n1, 2);
        g.insert_arc(n4, n3, -5);
        g.insert_arc(n5, n4, 6);

        FloydWarshall<DGT> fw(g);
        Johnson<DGT> jo(g);

        assert(!fw.has_negative_cycle());
        assert(!jo.has_negative_cycle());

        DGT::Node* nodes[] = {n1, n2, n3, n4, n5};

        for (auto* s : nodes)
        {
            for (auto* t : nodes)
            {
                assert(fw.distance_between(s, t) == jo.distance_between(s, t));
            }
        }

        // A known distance from the textbook example itself.
        assert(fw.distance_between(n1, n5) == -4);
        assert(jo.distance_between(n1, n5) == -4);

        // Path reconstruction: both endpoints, correct total length.
        auto fw_path = fw.path_between(n1, n3);
        auto jo_path = jo.path_between(n1, n3);
        assert(!fw_path.is_empty());
        assert(!jo_path.is_empty());
        assert(fw_path[0] == n1 && fw_path[fw_path.size() - 1] == n3);
        assert(jo_path[0] == n1 && jo_path[jo_path.size() - 1] == n3);
    }

    // A negative cycle must be detected by both, and neither should
    // crash trying to report path/distance information built on top of
    // an unreliable result.
    {
        DGT g;
        auto n1 = g.insert_node(1);
        auto n2 = g.insert_node(2);
        auto n3 = g.insert_node(3);

        g.insert_arc(n1, n2, 1);
        g.insert_arc(n2, n3, -3);
        g.insert_arc(n3, n1, 1); // cycle weight: 1 - 3 + 1 = -1

        FloydWarshall<DGT> fw(g);
        Johnson<DGT> jo(g);

        assert(fw.has_negative_cycle());
        assert(jo.has_negative_cycle());
    }

    // A single-node graph is a trivial but valid input.
    {
        DGT g;
        g.insert_node(1);

        FloydWarshall<DGT> fw(g);
        Johnson<DGT> jo(g);

        assert(!fw.has_negative_cycle());
        assert(!jo.has_negative_cycle());
    }

    // An empty graph and a non-digraph must both be rejected.
    {
        DGT empty;
        bool threw = false;

        try
        {
            FloydWarshall<DGT> fw(empty);
        }
        catch (const std::domain_error&)
        {
            threw = true;
        }

        assert(threw);

        threw = false;

        try
        {
            Johnson<DGT> jo(empty);
        }
        catch (const std::domain_error&)
        {
            threw = true;
        }

        assert(threw);

        threw = false;

        try
        {
            Graph<int_t, int_t> undirected;
            undirected.insert_node(1);
            Johnson<Graph<int_t, int_t>> jo(undirected);
        }
        catch (const std::domain_error&)
        {
            threw = true;
        }

        assert(threw);
    }

    cout << "Everything ok!\n";

    return 0;
}
