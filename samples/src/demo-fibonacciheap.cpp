/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <fibonacciheap.hpp>

using namespace Designar;

int main()
{
    FibonacciHeap<int_t> heap;

    for (int_t v : {5, 3, 8, 1, 4, 7, 9})
    {
        heap.insert(v);
    }

    cout << "size: " << heap.size() << endl;
    cout << "min: " << heap.get_min() << endl;

    auto* handle = heap.insert(20);
    cout << "inserted 20, min is still: " << heap.get_min() << endl;

    heap.decrease_key(handle, 0);
    cout << "after decrease_key(20 -> 0), min: " << heap.get_min() << endl;

    // Every Dijkstra/Prim implementation stated with an O(E + V lg V)
    // bound in the literature assumes a priority queue with amortized
    // O(1) decrease_key — a Fibonacci heap specifically — which is
    // exactly what makes it the textbook choice for those algorithms
    // over a binary heap's O(lg V) decrease_key.
    cout << "extract_min() in order: ";

    while (!heap.is_empty())
    {
        cout << heap.extract_min() << " ";
    }

    cout << endl;

    return 0;
}
