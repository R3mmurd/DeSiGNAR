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

# include <graphutilities.H>

namespace Designar
{

  CommonNodeArc::CommonNodeArc()
    : tag(0), _counter(0), _cookie(nullptr)
  {
    // empty
  }

  void CommonNodeArc::visit(GraphTag graph_tag)
  {
    tag |= nat_t(graph_tag);
  }

  void CommonNodeArc::unvisit(GraphTag graph_tag)
  {
    tag &= ~nat_t(graph_tag);
  }

  bool CommonNodeArc::is_visited(GraphTag graph_tag) const
  {
    return (tag & nat_t(graph_tag)) == nat_t(graph_tag);
  }

  void *& CommonNodeArc::cookie()
  {
    return _cookie;
  }

  void CommonNodeArc::reset_tag()
  {
    tag = 0;
  }

  lint_t & CommonNodeArc::counter()
  {
    return _counter;
  }

  void CommonNodeArc::reset()
  {
    tag      = 0;
    _counter = 0;
    _cookie  = nullptr;
  }

} // end namespace Designar
