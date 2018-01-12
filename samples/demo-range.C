/*
  This file is part of Designar Library.
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

# include <range.H>

using namespace Designar;

int main()
{
  cout << "IntRange(10, 20, 3)\n";
  for (lint_t item : IntRange(10, 20, 3))
    cout << item << endl;

  cout << "IntRange(-10, 10, 1)\n";
  for (lint_t item : IntRange(-10, 10))
    cout << item << endl;

  cout << "IntRange(0, 10, 1)\n";
  for (lint_t item : IntRange(10))
    cout << item << endl;

  cout << "UIntRange(10, 20, 3)\n";
  for (nat_t item : UIntRange(10, 20, 3))
    cout << item << endl;

  cout << "UIntRange(10, 10, 1)\n";
  for (nat_t item : UIntRange(10, 10))
    cout << item << endl;

  cout << "IntRange(0, 10, 1)\n";
  for (nat_t item : UIntRange(10))
    cout << item << endl;

  cout << "RealRange(10, 20, 3)\n";
  for (real_t item : RealRange(10, 20, 3))
    cout << item << endl;

  cout << "RealRange(1, 2, 0.1)\n";
  for (real_t item : RealRange(1, 2, 0.1))
    cout << item << endl;

  cout << "RealRange(1, 2, 0.05)\n";
  for (real_t item : range(1., 2., 0.05))
    cout << item << endl;
  
  return 0;
}
