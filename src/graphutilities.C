/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
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
