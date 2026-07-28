/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <iostream>
#include <graph.hpp>
#include <graphcoloring.hpp>

using namespace std;
using namespace Designar;

using GT = Graph<int_t>;

namespace
{
    void assert_valid_coloring(const GT& g,
                               const HashMap<Node<GT>*, nat_t>& coloring)
    {
        g.for_each_arc(
            [&](Arc<GT>* a)
            {
                Node<GT>* u = a->get_src_node();
                Node<GT>* v = a->get_tgt_node();
                assert(*coloring.search(u) != *coloring.search(v));
            });
    }
} // end anonymous namespace

int main()
{
    // A 4-cycle is bipartite (chromatic number 2); Welsh-Powell's own
    // guarantee is only <= max_degree + 1 = 3 here, so anywhere in
    // {2, 3} is an acceptable (valid) result.
    {
        GT g;
        auto n1 = g.insert_node(1);
        auto n2 = g.insert_node(2);
        auto n3 = g.insert_node(3);
        auto n4 = g.insert_node(4);
        g.insert_arc(n1, n2);
        g.insert_arc(n2, n3);
        g.insert_arc(n3, n4);
        g.insert_arc(n4, n1);

        auto coloring = greedy_graph_coloring(g);
        nat_t num_colors = count_colors_used(coloring);

        assert(num_colors >= 2 && num_colors <= 3);
        assert_valid_coloring(g, coloring);
    }

    // K4 (complete graph on 4 nodes) truly needs 4 colors — every pair
    // is adjacent, so Welsh-Powell has no choice but to find exactly 4.
    {
        GT g;
        Node<GT>* nodes[4];

        for (int_t i = 0; i < 4; ++i)
        {
            nodes[i] = g.insert_node(i);
        }

        for (int_t i = 0; i < 4; ++i)
        {
            for (int_t j = i + 1; j < 4; ++j)
            {
                g.insert_arc(nodes[i], nodes[j]);
            }
        }

        auto coloring = greedy_graph_coloring(g);
        assert(count_colors_used(coloring) == 4);
        assert_valid_coloring(g, coloring);
    }

    // A single isolated node needs exactly one color.
    {
        GT g;
        g.insert_node(1);

        auto coloring = greedy_graph_coloring(g);
        assert(count_colors_used(coloring) == 1);
    }

    cout << "Everything ok!\n";

    return 0;
}
