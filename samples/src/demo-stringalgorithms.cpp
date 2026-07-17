/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <stringalgorithms.hpp>

using namespace Designar;

int main()
{
  string text = "ababcababcabc";
  string pattern = "abc";

  cout << "kmp_search matches at: ";

  for (nat_t pos : kmp_search(text, pattern))
  {
    cout << pos << " ";
  }

  cout << endl;

  cout << "rabin_karp_search matches at: ";

  for (nat_t pos : rabin_karp_search(text, pattern))
  {
    cout << pos << " ";
  }

  cout << endl;

  string a = "kitten";
  string b = "sitting";

  cout << "edit_distance(\"" << a << "\", \"" << b << "\"): "
       << edit_distance(a, b) << endl;

  return 0;
}
