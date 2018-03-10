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

# include <bitset.H>
# include <random.H>

using namespace std;
using namespace Designar;

int main()
{
  DynBitSet bs;
  assert(bs.is_empty());
  assert(bs.size() == 0);

  bs.append(true);
  assert(bs.size() == 1);
  assert(not bs.is_empty());

  assert(bs[0]);
  bs[0] = false;
  assert(not bs[0]);

  bs.remove_last();
  assert(bs.is_empty());
  assert(bs.size() == 0);

  DynBitSet bs1(64);
  assert(bs1.size() == 64);
  for (auto i = 0; i < 64; ++i)
    assert(not bs1[i]);

  DynBitSet bs2(64, true);
  assert(bs2.size() == 64);
  for (auto i = 0; i < 64; ++i)
    assert(bs2[i]);

  DynBitSet bs3{0,1,1,1,0,0};
  assert(bs3.size() == 6);
  assert(bs3[0] == 0);
  assert(bs3[1] == 0);
  assert(bs3[2] == 1);
  assert(bs3[3] == 1);
  assert(bs3[4] == 1);
  assert(bs3[5] == 0);

  assert(bs3.to_string() == "011100");

  cout << "Everything ok!\n";
  return 0;
}


