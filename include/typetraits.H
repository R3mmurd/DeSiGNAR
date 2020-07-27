/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGTYPETRAITS_H
# define DSGTYPETRAITS_H

# include <type_traits>

namespace Designar
{

  template <typename To, typename FromHead, typename ...FromTail>
  struct AllAreConvertible
  {
    static constexpr bool value =
      std::is_convertible<FromHead, To>::value and
      AllAreConvertible<To, FromTail...>::value;
  };
  
  template <typename To, typename From>
  struct AllAreConvertible<To, From>
  {
    static constexpr bool value = std::is_convertible<From, To>::value;
  };
  
} // end namespace Designar

# endif // DSGTYPETRAITS_H
