/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <randomizedalgorithms.hpp>
#include <graph.hpp>

using namespace Designar;

int main()
{
    rng_t rng(20260718);

    DynArray<int_t> values = {9, 3, 7, 1, 8, 2, 6, 4, 5, 0};
    DynArray<int_t> copy = values;
    nat_t k = 4;
    cout << "randomized_select: the " << k
         << "-th smallest of {9,3,7,1,8,2,6,4,5,0} is "
         << randomized_select(copy, k, rng) << endl;

    cout << "\nmiller_rabin_is_prime:\n";

    for (int_t n : {97, 100, 561, 7919})
    {
        cout << "  " << n << " is "
             << (miller_rabin_is_prime(n, 20, rng) ? "" : "NOT ")
             << "prime";

        if (n == 561)
        {
            cout << " (561 is a Carmichael number — it fools a plain "
                    "Fermat test, but not Miller-Rabin)";
        }

        cout << endl;
    }

    DynArray<int_t> stream;

    for (int_t i = 0; i < 100; ++i)
    {
        stream.append(i);
    }

    auto sample = reservoir_sample<int_t>(stream.begin(), stream.end(), 5, rng);
    cout << "\nreservoir_sample: 5 uniformly random items from [0, 100): ";

    for (int_t v : sample)
    {
        cout << v << " ";
    }

    cout << endl;

    Graph<int_t> g;
    auto* a0 = g.insert_node(0);
    auto* a1 = g.insert_node(1);
    auto* a2 = g.insert_node(2);
    auto* b0 = g.insert_node(3);
    auto* b1 = g.insert_node(4);
    auto* b2 = g.insert_node(5);

    g.insert_arc(a0, a1);
    g.insert_arc(a1, a2);
    g.insert_arc(a2, a0);
    g.insert_arc(b0, b1);
    g.insert_arc(b1, b2);
    g.insert_arc(b2, b0);
    g.insert_arc(a0, b0); // the bridge

    cout << "\nkarger_min_cut: two triangles joined by one bridge edge -> "
            "min cut = "
         << karger_min_cut(g, 200, rng) << endl;

    return 0;
}
