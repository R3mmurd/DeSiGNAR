/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <leftistheap.hpp>

using namespace Designar;

int main()
{
    LeftistHeap<int_t> heap;

    for (int_t v : {5, 3, 8, 1, 4, 7, 9})
    {
        heap.insert(v);
    }

    cout << "size: " << heap.size() << endl;

    cout << "extract_min() in order: ";

    while (!heap.is_empty())
    {
        cout << heap.extract_min() << " ";
    }

    cout << endl;

    return 0;
}
