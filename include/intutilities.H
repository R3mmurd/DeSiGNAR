/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGINTUTILITIES_H
# define DSGINTUTILITIES_H

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

# endif // DSGINTUTILITIES_H
