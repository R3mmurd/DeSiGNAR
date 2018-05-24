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
  template <typename T> class DynArray;
  
  template <typename ContainerType = DynArray<std::string>>
  ContainerType split_string(const std::string &, const std::string &);
  
  template <typename ContainerType>
  ContainerType split_string(const std::string & str, const std::string & sep)
  {
    ContainerType ss;

    size_t sep_sz = sep.size();

    size_t beg = 0;
    size_t end = str.find(sep);

    while (end != std::string::npos)
      {
	ss.append(str.substr(beg, end - beg));
	beg = end + sep_sz;
	end = str.find(sep, beg);
      }

    ss.append(str.substr(beg, str.size() - beg));

    return ss;
  }
  
} // end namespace Designar

# endif // DSGSTRINGUTILITIES_H
