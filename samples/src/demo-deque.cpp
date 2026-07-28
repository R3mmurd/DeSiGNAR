/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <deque.hpp>

using namespace Designar;

int main()
{
    DynDeque<int_t> deque;

    deque.push_back(1);
    deque.push_back(2);
    deque.push_back(3);
    deque.push_front(0);
    deque.push_front(-1);

    cout << "Deque after pushing 1,2,3 to the back and 0,-1 to the front:\n";
    cout << "front: " << deque.front() << ", back: " << deque.back() << endl;
    cout << "size: " << deque.size() << endl;

    cout << "\npop_front()/pop_back() alternating: ";

    bool from_front = true;

    while (!deque.is_empty())
    {
        if (from_front)
        {
            cout << deque.pop_front() << " ";
        }
        else
        {
            cout << deque.pop_back() << " ";
        }

        from_front = !from_front;
    }

    cout << endl;

    return 0;
}
