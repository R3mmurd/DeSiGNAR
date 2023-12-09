/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <graph.hpp>
#include <buildgraph.hpp>

using namespace Designar;

using GT = Graph<nat_t>;
using DGT = Digraph<nat_t>;

using Dot = DotGraph<GT>;
using DDot = DotGraph<DGT>;

int main()
{
  nat_t i = 0;

  GT fg = full_graph<GT>(5, [&i](GT::Node *p)
                         { p->get_info() = ++i; });

  Dot().write_graph(fg, "full-graph.dot");

  i = 0;

  DGT fdg = full_graph<DGT>(3, [&i](DGT::Node *p)
                            { p->get_info() = ++i; });

  DDot().write_graph(fdg, "full-digraph.dot");

  i = 0;

  GT rg = ring_graph<GT>(10, 4, [&i](GT::Node *p)
                         { p->get_info() = ++i; });

  Dot().write_graph(rg, "ring-graph.dot");

  i = 0;

  DGT rdg = ring_graph<DGT>(10, 4, [&i](DGT::Node *p)
                            { p->get_info() = ++i; });

  DDot().write_graph(rdg, "ring-digraph.dot");

  i = 0;

  GT rgna = random_graph<GT>(5, 7, [&i](GT::Node *p)
                             { p->get_info() = ++i; });

  Dot().write_graph(rgna, "random-graph-na.dot");

  i = 0;

  DGT rdgna = random_graph<DGT>(5, 7, [&i](DGT::Node *p)
                                { p->get_info() = ++i; });

  DDot().write_graph(rdgna, "random-digraph-na.dot");

  i = 0;

  GT rghpnc = er_random_graph<GT>(5, 0.8, false, [&i](GT::Node *p)
                                  { p->get_info() = ++i; });

  Dot().write_graph(rghpnc, "random-graph-hp-nc.dot");

  i = 0;

  GT rghpc = er_random_graph<GT>(5, 0.8, true, [&i](GT::Node *p)
                                 { p->get_info() = ++i; });

  Dot().write_graph(rghpc, "random-graph-hp-c.dot");

  i = 0;

  GT rglpnc = er_random_graph<GT>(10, 0.2, false, [&i](GT::Node *p)
                                  { p->get_info() = ++i; });

  Dot().write_graph(rglpnc, "random-graph-lp-nc.dot");

  i = 0;

  GT rglpc = er_random_graph<GT>(10, 0.2, true, [&i](GT::Node *p)
                                 { p->get_info() = ++i; });

  Dot().write_graph(rglpc, "random-graph-lp-c.dot");

  i = 0;

  GT triangular_grid = build_grid<GT>(4, 4, GridType::TRIANGULAR,
                                      [&i](GT::Node *p, nat_t, nat_t)
                                      {
                                        p->get_info() = ++i;
                                      });

  Dot().write_graph(triangular_grid, "triangular-grid.dot");

  i = 0;

  GT rectangular_grid = build_grid<GT>(4, 4, GridType::RECTANGULAR,
                                       [&i](GT::Node *p, nat_t, nat_t)
                                       {
                                         p->get_info() = ++i;
                                       });

  Dot().write_graph(rectangular_grid, "rectangular-grid.dot");

  i = 0;

  GT hexagonal_grid = build_grid<GT>(4, 4,
                                     GridType::HEXAGONAL,
                                     [&i](GT::Node *p, nat_t, nat_t)
                                     {
                                       p->get_info() = ++i;
                                     });

  Dot().write_graph(hexagonal_grid, "hexagonal-grid.dot");

  i = 0;

  GT octagonal_grid = build_grid<GT>(4, 4,
                                     GridType::OCTAGONAL,
                                     [&i](GT::Node *p, nat_t, nat_t)
                                     {
                                       p->get_info() = ++i;
                                     });

  Dot().write_graph(octagonal_grid, "octagonal-grid.dot");

  return 0;
}
