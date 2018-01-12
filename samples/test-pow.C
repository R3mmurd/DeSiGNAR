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

# include <math.H>

int main()
{
  assert(Designar::pow(0UL,0UL) == 1);
  assert(Designar::pow(10UL,0UL) == 1);
  assert(Designar::pow(10UL,1UL) == 10);
  assert(Designar::pow(2UL,3UL) == 8);
  assert(Designar::pow(3UL,3UL) == 27);
  assert(Designar::pow(-3,2UL) == 9);
  assert(Designar::pow(-3,3UL) == -27);
  auto r = Designar::pow(-3,3UL);  
  assert(Designar::pow(-3,-3) == 1 / r);
  Designar::pow(-3.5,-3.);
  Designar::pow(-3.5,2);
  Designar::pow(-3.5,2UL);
  Designar::pow(3UL,2.);
  Designar::pow(3,2.);
  cout << "Everything ok!\n";

  return 0;
};
