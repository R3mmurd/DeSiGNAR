/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <iostream>

using namespace std;

# include <intutilities.H>

using namespace Designar;

int main()
{
  assert(factorial(0) == 1);
  assert(factorial(1) == 1);
  assert(factorial(2) == 2);
  assert(factorial(3) == 6);
  assert(factorial(4) == 24);
  assert(factorial(5) == 120);
  assert(factorial(6) == 720);

  assert(count_permutations(3, 3) == 6);

  for (nat_t i = 0; i <= 10; ++i)
    assert(count_permutations(i, i) == factorial(i));

  for (nat_t i = 0; i <= 10; ++i)
    assert(count_permutations(i, nat_t(0)) == 1);

  assert(count_permutations(10, 5) == 30240);

  assert(count_combinations(52, 3) == 22100);

  assert(count_combinations(10, 7) == 120);

  cout << "Everything ok!\n";
  return 0;
}
