/*
  This file is part of Designar.
  Copyright (C) 2017 by Alejandro J. Mujica

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.

  Any user request of this software, write to 

  Alejandro Mujica

  aledrums@gmail.com
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
    assert(count_permutations(i,i) == factorial(i));

  for (nat_t i = 0; i <= 10; ++i)
    assert(count_permutations(i, nat_t(0)) == 1);

  assert(count_permutations(10, 5) == 30240);

  assert(count_combinations(52, 3) == 22100);

  assert(count_combinations(10, 7) == 120);

  cout << "Everything ok!\n";
  return 0;
}
