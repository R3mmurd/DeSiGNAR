/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <graphalgorithms.hpp>

using namespace Designar;

using DGT = Digraph<string>;

int main()
{
    DGT dg;

    DGT::Node* a = dg.insert_node("a");
    DGT::Node* b = dg.insert_node("b");
    DGT::Node* c = dg.insert_node("c");
    DGT::Node* d = dg.insert_node("d");
    DGT::Node* e = dg.insert_node("e");
    DGT::Node* f = dg.insert_node("f");

    dg.insert_arc(a, b);
    dg.insert_arc(b, c);
    dg.insert_arc(c, a);

    dg.insert_arc(c, d);
    dg.insert_arc(d, e);
    dg.insert_arc(e, d);
    dg.insert_arc(f, e);

    cout << "strongly_connected_components (Tarjan, single DFS pass):\n";

    auto sccs = strongly_connected_components(dg);

    nat_t i = 0;

    for (const auto& comp : sccs)
    {
        cout << "  Component " << ++i << ": ";

        for (auto* p : comp)
        {
            cout << p->get_info() << " ";
        }

        cout << endl;
    }

    cout << "\nKosaraju_compute_strong_connected_components (same graph, "
            "for comparison):\n";

    auto kl = Kosaraju_compute_strong_connected_components(dg);

    i = 0;

    for (const DGT& sg : kl)
    {
        cout << "  Component " << ++i << ": ";

        sg.for_each_node([](DGT::Node* p) { cout << p->get_info() << " "; });

        cout << endl;
    }

    return 0;
}
