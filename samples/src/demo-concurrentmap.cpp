/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <thread>
#include <string>

using namespace std;

#include <concurrentmap.hpp>

using namespace Designar;

void writer(ConcurrentMap<int_t, string>& m, int_t start, int_t count)
{
    for (int_t i = 0; i < count; ++i)
    {
        m.put(start + i, "value " + to_string(start + i));
    }
}

int main()
{
    ConcurrentMap<int_t, string> map;

    // Two threads writing disjoint key ranges concurrently — safe because
    // every ConcurrentMap operation holds its own internal lock.
    thread t1{writer, ref(map), 0, 50};
    thread t2{writer, ref(map), 50, 50};

    t1.join();
    t2.join();

    cout << "size after concurrent writes: " << map.size() << endl;
    cout << "get(25): " << *map.get(25) << endl;

    return 0;
}
