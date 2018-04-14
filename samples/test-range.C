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

# include <cassert>
# include <iostream>

using namespace std;

# include <range.H>

using namespace Designar;

int main()
{
  Range<int> r1(10);
  assert(r1.min() == 0);
  assert(r1.max() == 10);
  assert(r1.step_size() == 1);
  assert(r1.size() == 10);

  Range<int> r2(-5, 5);
  assert(r2.min() == -5);
  assert(r2.max() == 5);
  assert(r2.step_size() == 1);
  assert(r2.size() == 10);

  Range<int> r3(0, 10, 2);
  assert(r3.min() == 0);
  assert(r3.max() == 10);
  assert(r3.step_size() == 2);
  assert(r3.size() == 5);

  Range<int> r4(-5, 5, 3);
  assert(r4.min() == -5);
  assert(r4.max() == 5);
  assert(r4.step_size() == 3);
  assert(r4.size() == 4);

  Range<int> r5(-5, 5, 4);
  assert(r5.min() == -5);
  assert(r5.max() == 5);
  assert(r5.step_size() == 4);
  assert(r5.size() == 3);

  Range<int> r6(-5, 5, 5);
  assert(r6.min() == -5);
  assert(r6.max() == 5);
  assert(r6.step_size() == 5);
  assert(r6.size() == 2);

  Range<int> r7(-5, 5, 10);
  assert(r7.min() == -5);
  assert(r7.max() == 5);
  assert(r7.step_size() == 10);
  assert(r7.size() == 1);

  Range<int> r8(5, 5);
  assert(r8.min() == 5);
  assert(r8.max() == 5);
  assert(r8.step_size() == 1);
  assert(r8.size() == 0);
  

  Range<double> r9(0, 10., 0.1);
  assert(num_equal(r9.min(), 0.));
  assert(num_equal(r9.max(), 10.));
  assert(num_equal(r9.step_size(), 0.1));
  assert(r9.size() == 100);

  Range<double> r10(-5., 5., 0.01);
  assert(num_equal(r10.min(), -5.));
  assert(num_equal(r10.max(), 5.));
  assert(num_equal(r10.step_size(), 0.01));
  assert(r10.size() == 1000);

  Range<double> r11(-5., 5., 0.02);
  assert(num_equal(r11.min(), -5.));
  assert(num_equal(r11.max(), 5.));
  assert(num_equal(r11.step_size(), 0.02));
  assert(r11.size() == 500);
  
  cout << "Everything ok!\n";
  return 0;
}
