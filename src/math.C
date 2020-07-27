/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# include <math.H>

namespace Designar
{

  template <>
  bool num_equal<float>(float a, float b)
  {
    return real_equal(a, b);
  }

  template <>
  bool num_equal<double>(double a, double b)
  {
    return real_equal(a, b);
  }

  template <>
  bool num_equal<long double>(long double a, long double b)
  {
    return real_equal(a, b);
  }
  
} // end namespace Designar
