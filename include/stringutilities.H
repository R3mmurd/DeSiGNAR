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

# ifndef DSGSTRINGUTILITIES_H
# define DSGSTRINGUTILITIES_H

# include <types.H>

namespace Designar
{
  template <typename T>
  std::string to_str(const T &);

  template <typename T>
  std::string to_str(const T & value)
  {
    std::stringstream sstr;
    sstr << value;
    return sstr.str();
  }
}

# endif // DSGSTRINGUTILITIES_H
