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
# include <cassert>
# include <vector2D.H>

using namespace std;
using namespace Designar;

int main()
{
  Vector2D v(-3.,4.);

  assert(num_equal(v.get_x(), -3.));
  assert(num_equal(v.get_y(), 4.));

  assert(not v.is_null());
  assert(Vector2D::ZERO.is_null());

  Vector2D w = v;
  w.normalize();

  assert(w.is_unitarian());

  assert(real_equal(v.square_magnitude(), 25.));
  assert(real_equal(v.magnitude(), 5.));

  assert(v);
  assert(not Vector2D(0.,0.));

  assert(v.get_opposite() == Vector2D(3.,-4.));

  Vector2D a(1.,0.), b(1.,1.), c(1.,-1.);

  assert(b.is_to_left_from(a));
  assert(c.is_to_right_from(b));

  cout << "Everything ok!\n";
  
  return 0;
}
