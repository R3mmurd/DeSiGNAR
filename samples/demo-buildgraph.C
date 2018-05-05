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

# include <graph.H>
# include <buildgraph.H>

using namespace Designar;

using GT = Graph<nat_t>;
using DGT = Digraph<nat_t>;

using Dot = DotGraph<GT>;
using DDot = DotGraph<DGT>;

int main()
{
  nat_t i = 0;
  
  GT fg = full_graph<GT>(5, [&i] (GT::Node * p)
			     {
			       p -> get_info() = ++i;
			     });

  Dot().write_graph(fg, "full-graph.dot");

  i = 0;

  DGT fdg = full_graph<DGT>(3, [&i] (DGT::Node * p)
				{
				  p -> get_info() = ++i;
				});
  
  DDot().write_graph(fdg, "full-digraph.dot");

  i = 0;
  
  GT rgna = random_graph<GT>(5, 7, [&i] (GT::Node * p)
				 {
				   p -> get_info() = ++i;
				 });
  
  Dot().write_graph(rgna, "random-graph-na.dot");

  i = 0;
  
  DGT rdgna = random_graph<DGT>(5, 7, [&i] (DGT::Node * p)
				    {
				      p -> get_info() = ++i;
				    });
  
  DDot().write_graph(rdgna, "random-digraph-na.dot");

  i = 0;
  
  GT rghpnc = p_random_graph<GT>(5, 0.8, false, [&i] (GT::Node * p)
				     {
				       p -> get_info() = ++i;
				     });
  
  Dot().write_graph(rghpnc, "random-graph-hp-nc.dot");

  i = 0;

  GT rghpc = p_random_graph<GT>(5, 0.8, true, [&i] (GT::Node * p)
				    {
				      p -> get_info() = ++i;
				    });
  
  Dot().write_graph(rghpc, "random-graph-hp-c.dot");

  i = 0;
  
  GT rglpnc = p_random_graph<GT>(10, 0.2, false, [&i] (GT::Node * p)
				     {
				       p -> get_info() = ++i;
				     });
  
  Dot().write_graph(rglpnc, "random-graph-lp-nc.dot");

  i = 0;

  GT rglpc = p_random_graph<GT>(10, 0.2, true, [&i] (GT::Node * p)
				    {
				      p -> get_info() = ++i;
				    });
  
  Dot().write_graph(rglpc, "random-graph-lp-c.dot");
  
  return 0;
}
