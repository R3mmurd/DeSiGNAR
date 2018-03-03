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
