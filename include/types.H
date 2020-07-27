/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGTYPES_H
# define DSGTYPES_H

# include <cmath>
# include <cstring>
# include <cassert>
# include <iostream>
# include <string>
# include <sstream>
# include <fstream>
# include <tuple>
# include <stdexcept>
# include <limits>
# include <chrono>
# include <random>
# include <functional>
# include <regex>
# include <thread>
# include <mutex>
# include <condition_variable>

# include <typetraits.H>

namespace Designar
{
  using lint_t       = long long;
  using nat_t        = unsigned long int;
  using real_t       = double;
  using rng_t        = std::mt19937_64;
  using rng_seed_t   = rng_t::result_type;
  using clock_t      = std::chrono::high_resolution_clock;
  using time_point_t = clock_t::time_point;
  using duration_t   = clock_t::duration;

  constexpr lint_t QuicksortThreshold = 40;

  class EmptyClass
  {
  public:
    EmptyClass() { /* empty */ }

    EmptyClass(const EmptyClass &) { /* empty */ }

    EmptyClass(EmptyClass &&) { /* empty */ }

    ~EmptyClass() { /* empty */ }

    EmptyClass & operator = (const EmptyClass &) { return *this; }

    EmptyClass & operator = (EmptyClass &&) { return *this; }

    bool operator == (const EmptyClass &) const { return true; }

    bool operator != (const EmptyClass &) const { return false; }

    bool operator < (const EmptyClass &) const { return false; }

    bool operator <= (const EmptyClass &) const { return true; }

    bool operator > (const EmptyClass &) const { return false; }

    bool operator >= (const EmptyClass &) const { return true; }

    friend std::ostream & operator << (std::ostream & out, const EmptyClass &)
    {
      return out;
    }

    friend std::istream & operator >> (std::istream & in, EmptyClass &)
    {
      return in;
    }
  };
  
} // end namespace Designar

# endif // DSGTYPES_H
