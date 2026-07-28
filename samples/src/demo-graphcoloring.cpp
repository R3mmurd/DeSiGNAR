/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <graph.hpp>
#include <graphcoloring.hpp>

using namespace Designar;

using GT = Graph<string>;

int main()
{
    // The Petersen-graph-flavored example: a 5-cycle with a "pentagram"
    // of extra chords added, which pushes the chromatic number to 3.
    GT g;
    GT::Node* a = g.insert_node("A");
    GT::Node* b = g.insert_node("B");
    GT::Node* c = g.insert_node("C");
    GT::Node* d = g.insert_node("D");
    GT::Node* e = g.insert_node("E");

    g.insert_arc(a, b);
    g.insert_arc(b, c);
    g.insert_arc(c, d);
    g.insert_arc(d, e);
    g.insert_arc(e, a);
    g.insert_arc(a, c);
    g.insert_arc(b, d);

    auto coloring = greedy_graph_coloring(g);

    cout << "Greedy (Welsh-Powell) coloring:\n";

    g.for_each_node(
        [&](GT::Node* node)
        { cout << "  " << node->get_info() << " -> color "
               << *coloring.search(node) << endl; });

    cout << "\nColors used: " << count_colors_used(coloring) << endl;

    return 0;
}
