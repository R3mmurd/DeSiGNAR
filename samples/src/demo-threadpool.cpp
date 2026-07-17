/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <future>
#include <vector>

using namespace std;

#include <threadpool.hpp>

using namespace Designar;

int main()
{
    ThreadPool pool(4);

    cout << "worker count: " << pool.size() << endl;

    vector<future<int_t>> results;

    for (int_t i = 1; i <= 10; ++i)
    {
        results.push_back(pool.submit([i] { return i * i; }));
    }

    cout << "squares: ";

    for (auto& f : results)
    {
        cout << f.get() << " ";
    }

    cout << endl;

    return 0;
}
