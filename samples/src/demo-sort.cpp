/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <array.hpp>
#include <sort.hpp>

using namespace Designar;

template <class SortFn>
void demo(const char* name, SortFn sort_fn)
{
    DynArray<int_t> a = {5, 3, 8, 1, 9, 2, 7, 4, 6};

    sort_fn(a);

    cout << name << ": ";

    for (nat_t i = 0; i < a.size(); ++i)
    {
        cout << a[i] << " ";
    }

    cout << endl;
}

int main()
{
    demo("selection_sort", [](DynArray<int_t>& a) { selection_sort(a); });

    demo("bubble_sort", [](DynArray<int_t>& a) { bubble_sort(a); });

    demo("shell_sort", [](DynArray<int_t>& a) { shell_sort(a); });

    demo("merge_sort", [](DynArray<int_t>& a) { merge_sort(a); });

    demo("heap_sort", [](DynArray<int_t>& a) { heap_sort(a); });

    demo("counting_sort (keys in [0, 9])",
         [](DynArray<int_t>& a) { counting_sort(a, nat_t(9)); });

    demo("radix_sort", [](DynArray<int_t>& a) { radix_sort(a); });

    // bucket_sort needs floating-point keys in [0, 1), so it can't share
    // the int_t-only demo() helper above.
    DynArray<real_t> b = {0.42, 0.13, 0.99, 0.05, 0.71, 0.36, 0.88};
    bucket_sort(b);

    cout << "bucket_sort: ";

    for (nat_t i = 0; i < b.size(); ++i)
    {
        cout << b[i] << " ";
    }

    cout << endl;

    return 0;
}
