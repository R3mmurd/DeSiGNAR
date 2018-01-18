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

# ifndef INTUTILITIES_H
# define INTUTILITIES_H

# include <types.H>

namespace Designar
{
  template <typename T>
  T forward_prod(T, T);

  template <typename T>
  T backward_prod(T, T);
  
  template <typename T>
  T factorial(T);

  template <typename T>
  T count_permutations(T, T);

  template <typename T>
  T count_combinations(T, T);

  template <typename T>
  T forward_prod(T a, T b)
  {
    static_assert(std::is_integral<T>::value,
		  "Argument must be an integral type");
    
    T ret_val = 1;

    while (a <= b)
      ret_val *= a++;

    return ret_val;
  }

  template <typename T>
  T backward_prod(T a, T b)
  {
    static_assert(std::is_integral<T>::value,
		  "Argument must be an integral type");
    
    T ret_val = 1;

    while (a >= b)
      ret_val *= a--;

    return ret_val;
  }

  template <typename T>
  T factorial(T n)
  {
    if (n < 0)
      throw std::domain_error("Argument must be positive");

    return forward_prod(T(1), n);
  }

  template <typename T>
  T count_permutations(T n, T r)
  {
    if (n < 0 or r < 0)
      throw std::domain_error("Arguments must be positive numbers");

    if (r > n)
      throw std::logic_error("r > n");
    
    return backward_prod(n, n - r + T(1));
  }

  template <typename T>
  T count_combinations(T n, T r)
  {
    if (n < 0 or r < 0)
      throw std::domain_error("Arguments must be positive numbers");

    if (r > n)
      throw std::logic_error("r > n");

    return backward_prod(n, n - r + T(1)) / factorial(r);
  }
  
} // end namespace Designar

# endif
