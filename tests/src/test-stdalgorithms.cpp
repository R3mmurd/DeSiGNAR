/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <algorithm>
#include <numeric>
#include <array.hpp>
#include <list.hpp>

using namespace Designar;
using namespace std;

int main()
{
  // DynArray: RandomAccessIterator, so every <algorithm> that needs
  // random access (std::sort, std::distance in O(1)) must work too.
  {
    DynArray<int_t> arr = {5, 3, 1, 4, 2};

    assert(distance(arr.begin(), arr.end()) == int_t(arr.size()));

    sort(arr.begin(), arr.end());
    assert(is_sorted(arr.begin(), arr.end()));
    assert(arr[0] == 1 && arr[4] == 5);

    auto found = find(arr.begin(), arr.end(), 3);
    assert(found != arr.end() && *found == 3);

    int_t total = accumulate(arr.begin(), arr.end(), int_t(0));
    assert(total == 1 + 2 + 3 + 4 + 5);

    auto min_it = min_element(arr.begin(), arr.end());
    auto max_it = max_element(arr.begin(), arr.end());
    assert(*min_it == 1 && *max_it == 5);

    reverse(arr.begin(), arr.end());
    assert(arr[0] == 5 && arr[4] == 1);

    nat_t evens = count_if(arr.begin(), arr.end(),
                           [](int_t x) { return x % 2 == 0; });
    assert(evens == 2);

    nat_t visited = 0;
    for_each(arr.begin(), arr.end(), [&visited](int_t)
             { ++visited; });
    assert(visited == arr.size());

    // Regression: RandomAccessIterator::operator[](i) used to move_to(i)
    // on *this before reading, i.e. it[i] silently relocated `it` itself
    // instead of behaving like *(it + i) — exactly the kind of thing
    // std::sort's internal use of iterator indexing would have exercised
    // (and corrupted) had it compiled to begin with. it[i] must leave it
    // pointing at the same element as before.
    {
      auto it = arr.begin();
      int_t at_1 = it[1];
      assert(at_1 == arr[1]);
      assert(*it == arr[0]);
    }

    // Regression: operator+()/operator-()/operator*() used to be
    // non-const member functions, which fails to compile against
    // standard <algorithm> implementations (libc++, MSVC STL) that hold
    // iterators in const-qualified locals/parameters internally.
    {
      const auto first = arr.begin();
      const auto shifted = first + 2;
      assert(*shifted == arr[2]);
      assert(shifted - first == 2);
    }

    cout << "std:: algorithms on DynArray: Everything ok!\n";
  }

  // DLList: BidirectionalIterator — no std::sort (that needs random
  // access), but every forward/bidirectional algorithm should work,
  // including ones that walk the sequence backwards.
  {
    DLList<int_t> list = {1, 2, 3, 4, 5};

    assert(distance(list.begin(), list.end()) == int_t(list.size()));

    int_t total = accumulate(list.begin(), list.end(), int_t(0));
    assert(total == 1 + 2 + 3 + 4 + 5);

    auto found = find(list.begin(), list.end(), 4);
    assert(found != list.end() && *found == 4);

    // Manual backward walk exercising operator--, which is exactly what
    // std::reverse_iterator would use internally (DLList does not expose
    // rbegin()/rend() itself, but its iterator's bidirectionality is
    // still what a caller wrapping it in std::reverse_iterator relies on).
    DynArray<int_t> reversed;
    auto it = list.end();
    do
    {
      --it;
      reversed.append(*it);
    } while (it != list.begin());
    assert(reversed[0] == 5 && reversed[4] == 1);

    cout << "std:: algorithms on DLList: Everything ok!\n";
  }

  // SLList: ForwardIterator only — the algorithms that only need a
  // single forward pass (std::find, std::accumulate, std::for_each,
  // std::count) must still work even without ++/-- symmetry or
  // random access.
  {
    SLList<int_t> list = {10, 20, 30, 40};

    assert(distance(list.begin(), list.end()) == int_t(list.size()));

    int_t total = accumulate(list.begin(), list.end(), int_t(0));
    assert(total == 10 + 20 + 30 + 40);

    auto found = find(list.begin(), list.end(), 30);
    assert(found != list.end() && *found == 30);

    nat_t occurrences = count(list.begin(), list.end(), 20);
    assert(occurrences == 1);

    cout << "std:: algorithms on SLList: Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
