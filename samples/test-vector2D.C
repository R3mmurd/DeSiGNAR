/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
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
