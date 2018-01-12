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

# include <list.H>

namespace Designar
{
  
  void DL::split(DL & l, DL & r)
  {
    assert(l.is_empty());
    assert(r.is_empty());
    
    while (not this->is_empty())
      {
	l.insert_prev(this->remove_next());
	
	if (not this->is_empty())
	  r.insert_next(this->remove_prev());
      }
  }

} // end namespace Designar
