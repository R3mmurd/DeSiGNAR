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

# ifndef DSGGRAPHALGORITHMS_H
# define DSGGRAPHALGORITHMS_H

# include <relation.H>
# include <graph.H>
# include <random.H>

namespace Designar
{

  /* Depth first based algorithms */

  template <class GT, class Op>
  void depth_first_traverse_prefix_rec(GT &, Node<GT> &, Op &);

  template <class GT, class Op>
  void depth_first_traverse_prefix(GT &, Op & op);
  
  template <class GT, class Op>
  void depth_first_traverse_prefix(GT & g, Op && op = Op())
  {
    depth_first_traverse_prefix<GT, Op>(g, op);
  }
  
  template <class GT, class Op>
  void depth_first_traverse_sufix_rec(GT &, Node<GT> &, Op &);

  template <class GT, class Op>
  void depth_first_traverse_sufix(GT &, Op & op);
  
  template <class GT, class Op>
  void depth_first_traverse_sufix(GT & g, Op && op = Op())
  {
    depth_first_traverse_sufix<GT, Op>(g, op);
  }

  template <class GT, class Op>
  void depth_first_traverse(GT & g, Op & op)
  {
    depth_first_traverse_prefix<GT, Op>(g, op);
  }
  
  template <class GT, class Op>
  void depth_first_traverse(GT & g, Op && op = Op())
  {
    depth_first_traverse<GT, Op>(g, op);
  }

  template <class GT>
  bool depth_first_search_path_rec(GT &, Node<GT> &, Node<GT> &, Path<GT> &);

  template <class GT>
  Path<GT> depth_first_search_path(GT &, Node<GT> &, Node<GT> &);

  template <class GT>
  bool test_cycle_rec(GT &, Node<GT> &, bool);

  template <class GT>
  bool has_cycle(GT &, Node<GT> &);

  template <class GT>
  bool has_cycle(GT &);

  template <class GT>
  bool is_graph_acyclique(GT &, Node<GT> &);
 
  template <class GT>
  bool is_graph_acyclique(GT &);

  template <class GT>
  GT invert_digraph(GT &, bool);

  template <class GT>
  void build_subgraph_rec(GT &, Node<GT> &, GT &);

  template <class GT>
  SLList<GT> compute_connected_components(GT &);

  template <class GT>
  void add_nodes_to_component_rec(GT &, Node<GT> &, SLList<Node<GT> *> &);

  template <class GT>
  SLList<SLList<Node<GT> *>> connected_components_node_list(GT &);

  template <class GT>
  void compute_cut_nodes_rec(GT &, Node<GT> &, Arc<GT> &,
			     SLList<Node<GT> *> &, lint_t &);

  template <class GT>
  SLList<Node<GT> *> compute_cut_nodes(GT &);

  template <class GT>
  void paint_cut_nodes_connected_components_rec(GT &, Node<GT> &, lint_t);

  template <class GT>
  void paint_from_cut_node(GT &, Node<GT> &, lint_t &);

  template <class GT>
  lint_t paint_cut_nodes_subgraphs(GT &, const SLList<Node<GT> *> &);

  template <class GT>
  void build_cut_nodes_subgraph_rec(GT &, Node<GT> &, GT &, lint_t);

  template <class GT>
  SLList<GT> build_cut_nodes_subgraphs(GT &);

  template <class GT>
  std::tuple<GT, SLList<Arc<GT> *>>
    build_cut_graph(GT &, const SLList<Node<GT> *> &);

  template <class GT>
  std::tuple<SLList<GT>, GT, SLList<Arc<GT> *>>
    compute_cut_nodes_connected_components(GT &, const SLList<Node<GT> *> &);

  template <class GT>
  void Kosaraju_build_subgraph_rec(GT &, Node<GT> &, GT &, nat_t);

  template <class GT>
  SLList<GT> Kosaraju_compute_strong_connected_components(GT &);

  template <class GT>
  SLList<Node<GT> *> df_topological_sort(GT &);

  /* Breadth first based algorithms */

  template <class GT, class Op>
  void breadth_first_traverse(GT &, Node<GT> &, Op &);
  
  template <class GT, class Op>
  void breadth_first_traverse(GT & g, Node<GT> & p, Op && op = Op())
  {
    breadth_first_traverse<GT, Op>(g, p, op);
  }
  
  template <class GT, class Op>
  void breadth_first_traverse(GT & g, Op & op)
  {
    breadth_first_traverse<GT, Op>(g, g.get_first_node(), op);
  }
  
  template <class GT, class Op>
  void breadth_first_traverse(GT & g, Op && op = Op())
  {
    breadth_first_traverse<GT, Op>(g, op);
  }

  template <class GT>
  Path<GT> breadth_first_search_path(GT & g, Node<GT> &, Node<GT> &);

  template <class GT>
  SLList<Node<GT> *> bf_topological_sort(GT &);

  template <class GT>
  SLList<SLList<Node<GT> *>> topological_ranks(GT &);

  template <class GT, class Op>
  void depth_first_traverse_prefix_rec(GT & g, Node<GT> & p, Op & op)
  {
    if (p.is_visited(GraphTag::DEPTH_FIRST))
      return;
    
    op(p);
    p.visit(GraphTag::DEPTH_FIRST);
    
    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = *it;

	if (a.is_visited(GraphTag::DEPTH_FIRST))
	  continue;

	a.visit(GraphTag::DEPTH_FIRST);

	Node<GT> & q = it.get_tgt_node();

	depth_first_traverse_prefix_rec(g, q, op);
      }
  }

  template <class GT, class Op>
  void depth_first_traverse_sufix_rec(GT & g, Node<GT> & p, Op & op)
  {
    if (p.is_visited(GraphTag::DEPTH_FIRST))
      return;

    p.visit(GraphTag::DEPTH_FIRST);

    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = *it;

	if (a.is_visited(GraphTag::DEPTH_FIRST))
	  continue;

	a.visit(GraphTag::DEPTH_FIRST);

	Node<GT> & q = it.get_tgt_node();

	depth_first_traverse_sufix_rec(g, q, op);
      }

    op(p);
  }

  template <class GT>
  bool depth_first_search_path_rec(GT & g, Node<GT> & p, Node<GT> & end,
				   Path<GT> & path)
  {
    if (p.is_visited(GraphTag::DEPTH_FIRST))
      return false;
    
    p.visit(GraphTag::DEPTH_FIRST);
    
    path.append(p);
    
    if (&p == &end)
      return true;
    
    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = *it;

	if (a.is_visited(GraphTag::DEPTH_FIRST))
	  continue;

        a.visit(GraphTag::DEPTH_FIRST);

	Node<GT> & q = it.get_tgt_node();
       
	if (depth_first_search_path_rec(g, q, end, path))
	  return true;
      }

    path.remove_last_node();
    return false;
  }

  template <class GT>
  bool test_cycle_rec(GT & g, Node<GT> & p, bool has)
  {
    if (p.is_visited(GraphTag::DEPTH_FIRST))
      return has;
    
    p.visit(GraphTag::DEPTH_FIRST);

    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = *it;

	if (a.is_visited(GraphTag::DEPTH_FIRST))
	  continue;

	a.visit(GraphTag::DEPTH_FIRST);

	Node<GT> & q = it.get_tgt_node();

	if (test_cycle_rec(g, q, has) == has)
	  return has;
      }

    return not has;
  }

  template <class GT>
  bool has_cycle(GT & g, Node<GT> & start)
  {
    g.reset_tag(GraphTag::DEPTH_FIRST);
    return test_cycle_rec(g, start, true);
  }

  template <class GT>
  bool has_cycle(GT & g)
  {
    if (g.get_num_arcs() >= g.get_num_nodes())
      return true;
    
    return g.exists_node([&g] (Node<GT> & p)
			 {
			   return has_cycle(g, p);
			 });
  }


  template <class GT>
  bool is_acyclique(GT & g, Node<GT> & start)
  {
    g.reset_tag(GraphTag::DEPTH_FIRST);
    return test_cycle_rec(g, start, false);
  }

  template <class GT>
  bool is_acyclique(GT & g)
  {
    if (g.get_num_arcs() >= g.get_num_nodes())
      return false;
    
    return g.all_nodes([&g] (Node<GT> & p)
		       {
			 return is_acyclique(g, p);
		       });
  }

  template <class GT, class Op>
  void depth_first_traverse_prefix(GT & g, Op & op)
  {
    g.reset_tag(GraphTag::DEPTH_FIRST);

    for (NodeIt<GT> it(g); it.has_current(); it.next())
      {
	Node<GT> & p = it.get_current();

	if (p.is_visited(GraphTag::DEPTH_FIRST))
	  continue;

	depth_first_traverse_prefix_rec(g, p, op);
      }
  }

  template <class GT, class Op>
  void depth_first_traverse_sufix(GT & g, Op & op)
  {
    g.reset_tag(GraphTag::DEPTH_FIRST);

    for (NodeIt<GT> it(g); it.has_current(); it.next())
      {
	Node<GT> & p = it.get_current();

	if (p.is_visited(GraphTag::DEPTH_FIRST))
	  continue;

	depth_first_traverse_sufix_rec(g, p, op);
      }
  }

  template <class GT>
  Path<GT> depth_first_search_path(GT & g, Node<GT> & begin, Node<GT> & end)
  {
    Path<GT> ret_val(g);

    if (&begin == &end)
      {
	ret_val.init(begin);
	return ret_val;
      }

    g.reset_tag(GraphTag::DEPTH_FIRST);
    
    depth_first_search_path_rec(g, begin, end, ret_val);
    return ret_val;
  }

  template <class GT>
  GT invert_digraph(GT & g)
  {
    if (not g.is_digraph())
      throw std::domain_error("Argument must be a directed graph");
    
    g.reset_node_cookies();
    
    GT i;

    g.for_each_arc([&i] (auto & a)
		   {
		     auto & s = a.get_src_node();
		     auto & t = a.get_tgt_node();
		     auto sp = mapped_node_ptr<GT>(s);

		     if (sp == nullptr)
		       {
			 sp = &i.insert_node(s.get_info());
			 map_nodes<GT>(s, *sp);
		       }

		     auto tp = mapped_node_ptr<GT>(t);

		     if (tp == nullptr)
		       {
			 tp = &i.insert_node(t.get_info());
			 map_nodes<GT>(t, *tp);
		       }

		     auto & ap = i.insert_arc(*tp, *sp, a.get_info());
		     map_arcs<GT>(a, ap);
		   });
    
    return i;
  }

  
  template <class GT>
  void build_subgraph_rec(GT & g, Node<GT> & p, GT & t)
  {
    if (p.is_visited(GraphTag::COMPONENT))
      return;

    p.visit(GraphTag::COMPONENT);

    Node<GT> & pp = t.insert_node(p.get_info());
    map_nodes<GT>(p, pp);

    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = it.get_current();

	if (a.is_visited(GraphTag::COMPONENT))
	  continue;

	a.visit(GraphTag::COMPONENT);

	Node<GT> & q = it.get_tgt_node();

	build_subgraph_rec(g, q, t);

	Node<GT> & qq = *mapped_node_ptr<GT>(q);

	Arc<GT> & aa = t.insert_arc(pp, qq, a.get_info());
	map_arcs<GT>(a, aa);
      }
  }

  template <class GT>
  SLList<GT> compute_connected_components(GT & g)
  {
    if (g.is_digraph())
      throw std::domain_error("Argument must be an undirected graph");
    
    g.reset_tag(GraphTag::COMPONENT);
    g.reset_cookies();
    
    SLList<GT> ret;

    g.for_each_node([&g, &ret] (Node<GT> & p)
		    {
		      if (p.is_visited(GraphTag::COMPONENT))
			return;
			      
		      GT h;
		      build_subgraph_rec(g, p, h);
		      ret.append(std::move(h));
		    });
    
    
    return ret;
  }

  template <class GT>
  void add_nodes_to_component_rec(GT & g, Node<GT> & p,
				  SLList<Node<GT> *> & list)
  {
    if (p.is_visited(GraphTag::COMPONENT))
      return;

    p.visit(GraphTag::COMPONENT);

    list.append(&p);

    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = it.get_current();

	if (a.is_visited(GraphTag::COMPONENT))
	  continue;

	a.visit(GraphTag::COMPONENT);

	Node<GT> & q = it.get_tgt_node();

        add_nodes_to_component_rec(g, q, list);
      }
  }

  template <class GT>
  SLList<SLList<Node<GT> *>> connected_components_node_list(GT & g)
  {
    if (g.is_digraph())
      throw std::domain_error("Argument must be an undirected graph");
    
    g.reset_tag(GraphTag::COMPONENT);
    g.reset_cookies();
    
    SLList<SLList<Node<GT> *>> ret;

    g.for_each_node([&g, &ret] (Node<GT> & p)
		    {
		      if (p.is_visited(GraphTag::COMPONENT))
			return;
			      
		      SLList<Node<GT> *> l;
		      add_nodes_to_component_rec(g, p, l);
		      ret.append(std::move(l));
		    });
    
    
    return ret;
  }

  template <class GT>
  void compute_cut_nodes_rec(GT & g, Node<GT> & p, Arc<GT> & a,
			     SLList<Node<GT> *> & l, lint_t & cdf)
  {
    p.visit(GraphTag::DEPTH_FIRST);
    low<GT>(p) = df<GT>(p) = cdf++;

    bool is_cut = false;

    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & arc = it.get_current();

	if (&arc == &a)
	  continue;

	Node<GT> & q = it.get_tgt_node();

	if (q.is_visited(GraphTag::DEPTH_FIRST))
	  {
	    if (not arc.is_visited(GraphTag::DEPTH_FIRST))
	      low<GT>(p) = std::min(low<GT>(p), df<GT>(q));
	    
	    continue;
	  }

	if (arc.is_visited(GraphTag::DEPTH_FIRST))
	  continue;

	arc.visit(GraphTag::DEPTH_FIRST);

	compute_cut_nodes_rec(g, q, arc, l, cdf);

	low<GT>(p) = std::min(low<GT>(p), low<GT>(q));

	is_cut = low<GT>(q) >= df<GT>(p) and df<GT>(q) != 0;
      }

    if (is_cut)
      {
	p.visit(GraphTag::CUT);
	l.append(&p);
      }
    
  }

  template <class GT>
  SLList<Node<GT> *> compute_cut_nodes(GT & g)
  {
    g.for_each_node([] (Node<GT> & p)
		    {
		      p.reset_tag();
		      df<GT>(p)  =  0;
		      low<GT>(p) = -1;
		    });
    g.reset_arcs();

    nat_t call_counter = 0;
    lint_t current_df   = 0;

    Node<GT> & start = g.get_first_node();
    start.visit(GraphTag::DEPTH_FIRST);
    df<GT>(start) = current_df++;

    SLList<Node<GT> *> list;

    for (AdArcIt<GT> it(g, start); it.has_current(); it.next())
      {
	Node<GT> & t = it.get_tgt_node();

	if (t.is_visited(GraphTag::DEPTH_FIRST))
	  continue;

	Arc<GT> & a = it.get_current();

	if (a.is_visited(GraphTag::DEPTH_FIRST))
	  continue;
	
	a.visit(GraphTag::DEPTH_FIRST);

	compute_cut_nodes_rec(g, t, a, list, current_df);
	++call_counter;
      }

    if (call_counter > 1)
      {
	start.visit(GraphTag::CUT);
	list.append(&start);
      }

    return list;
  }

  template <class GT>
  void paint_cut_nodes_connected_components_rec(GT & g, Node<GT> & p, lint_t c)
  {
    if (p.counter() > 0)
      return;
    
    p.counter() = c;
    
    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = it.get_current();
	
	if (a.counter() > 0)
	  continue;
	
	Node<GT> & q = it.get_tgt_node();
	
	if (q.is_visited(GraphTag::CUT))
	  continue;
	
	a.counter() = c;
	
	paint_cut_nodes_connected_components_rec(g, q, c);
      }
  }
  
  template <class GT>
  void paint_from_cut_node(GT & g, Node<GT> & p, lint_t & color)
  {
    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT>  & a = it.get_current();
	Node<GT> & q = it.get_tgt_node();

	if (q.is_visited(GraphTag::CUT))
	  {
	    a.visit(GraphTag::CUT);
	    continue;
	  }
	else
	  {
	    a.counter() = -1;

	    if (q.counter() > 0)
	      continue;
	  }

	paint_cut_nodes_connected_components_rec(g, q, color);
	++color;
      }
  }

  template <class GT>
  lint_t paint_cut_nodes_subgraphs(GT & g, const SLList<Node<GT> *> & l)
  {
    g.reset_counters();

    lint_t color = 1;

    l.for_each([&g, &color] (Node<GT> * curr)
	       {
		 paint_from_cut_node(g, *curr, color);
	       });
    
    return color;
  }

  template <class GT>
  void build_cut_nodes_subgraph_rec(GT & g, Node<GT> & p, GT & t, lint_t color)
  {
    Node<GT> * pp = mapped_node_ptr<GT>(p);

    for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = it.get_current();

	if (a.counter() != color or a.is_visited(GraphTag::COMPONENT))
	  continue;

	a.visit(GraphTag::COMPONENT);

	Node<GT> & q = it.get_tgt_node();
	Node<GT> * qp = nullptr;

	if (q.is_visited(GraphTag::COMPONENT))
	  qp = mapped_node_ptr<GT>(q);
	else
	  {
	    qp = &t.insert_node(q.get_info());
	    map_nodes<GT>(q, *qp);
	    q.visit(GraphTag::COMPONENT);
	  }

	Arc<GT> & ap = t.insert_arc(*pp, *qp, a.get_info());
	map_arcs<GT>(a, ap);

	build_cut_nodes_subgraph_rec(g, q, t, color);
      }
  }

  template <class GT>
  SLList<GT> build_cut_nodes_subgraphs(GT & g)
  {
    SLList<GT> list;

    for (NodeIt<GT> it(g); it.has_current(); it.next())
      {
	Node<GT> & p = it.get_current();

	if (p.counter() == 0 or p.is_visited(GraphTag::COMPONENT))
	  continue;

	GT t;
	Node<GT> & q = t.insert_node(p.get_info());
	map_nodes<GT>(p, q);
	p.visit(GraphTag::COMPONENT);
	build_cut_nodes_subgraph_rec(g, p, t, p.counter());
	list.append(std::move(t));
      }

    return list;
  }

  template <class GT>
  std::tuple<GT, SLList<Arc<GT> *>>
    build_cut_graph(GT & g, const SLList<Node<GT> *> & cut_nodes)
  {
    GT cut_graph;
    SLList<Arc<GT> *> cross_arcs;

    cut_nodes.for_each([&] (Node<GT> * p)
		       {
			 Node<GT> & q = cut_graph.insert_node(p->get_info());
			 map_nodes<GT>(*p, q);
		       });


    g.for_each_arc([&] (Arc<GT> & a)
		   {
		     if (a.counter() == -1)
		       {
			 cross_arcs.append(&a);
			 return;
		       }
			     
		     if (not a.is_visited(GraphTag::CUT))
		       return;

		     auto p = mapped_node_ptr<GT>(a.get_src_node());
		     auto q = mapped_node_ptr<GT>(a.get_tgt_node());
		     Arc<GT> & ap =
		       cut_graph.insert_arc(*p, *q, a.get_info());
		     map_arcs<GT>(a, ap);
		   });
    
    return std::make_tuple(std::move(cut_graph), std::move(cross_arcs));
  }

  template <class GT>
  std::tuple<SLList<GT>, GT, SLList<Arc<GT> *>>
    compute_cut_nodes_connected_components(GT & g,
					   const SLList<Node<GT> *> & cut_nodes)
  {
    g.reset_cookies();
    paint_cut_nodes_subgraphs(g, cut_nodes);
    auto subgraphs = build_cut_nodes_subgraphs(g);
    auto t = build_cut_graph(g, cut_nodes);
    return std::make_tuple(std::move(subgraphs), std::move(std::get<0>(t)),
			   std::move(std::get<1>(t)));
  }

  template <class GT>
  void Kosaraju_build_subgraph_rec(GT & inv_g, Node<GT> & p, GT & sg, nat_t num)
  { 
    if (p.is_visited(GraphTag::COMPONENT))
      return;
    
    Node<GT> & pp = sg.insert_node(p.get_info());
    map_nodes<GT>(p, pp);

    p.visit(GraphTag::COMPONENT);
    p.counter() = num;

    for (AdArcIt<GT> it(inv_g, p); it.has_current(); it.next())
      {
	Arc<GT> & a = it.get_current();

	if (a.is_visited(GraphTag::COMPONENT))
	  continue;

	Node<GT> & q = a.get_tgt_node();

	Kosaraju_build_subgraph_rec(inv_g, q, sg, num);

	Node<GT> & qp = *mapped_node_ptr<GT>(q);

	if (p.counter() != q.counter())
	  continue;

       	Arc<GT> & ap = sg.insert_arc(qp, pp, a.get_info());

	map_arcs<GT>(a, ap);
      }
  }

  template <class GT>
  SLList<GT> Kosaraju_compute_strong_connected_components(GT & g)
  {
    if (not g.is_digraph())
      throw std::domain_error("Argument must be a directed graph");

    g.reset_cookies();
    g.reset_tag(GraphTag::COMPONENT);

    FixedArray<Node<GT> *> df(g.get_num_nodes());
    nat_t i = 0;
    depth_first_traverse_sufix(g, [&df, &i] (Node<GT> & p)
			       {
				 df[i++] = &p;
			       });
    
    GT ig = invert_digraph(g);

    SLList<GT> ret;
    nat_t num_component = 1;

    for (i = g.get_num_nodes(); i > 0; --i)
      {
	Node<GT> & v = *mapped_node_ptr<GT>(*df[i-1]);

	if (v.is_visited(GraphTag::COMPONENT))
	  continue;

	GT sc;
	Kosaraju_build_subgraph_rec(ig, v, sc, num_component++);
	ret.append(move(sc));
      }

    return ret;
  }

  template <class GT>
  SLList<Node<GT> *> df_topological_sort(GT & g)
  {
    if (not g.is_digraph())
      throw std::domain_error("Argument must be a directed graph");
    
    SLList<Node<GT> *> ret;

    depth_first_traverse_sufix(g, [&ret] (Node<GT> & p)
			       {
				 ret.insert(&p);
			       });

    return ret;
  }

  template <class GT, class Op>
  void breadth_first_traverse(GT & g, Node<GT> & begin, Op & op)
  {
    g.reset_tag(GraphTag::BREADTH_FIRST);
    
    op(begin);
    begin.visit(GraphTag::BREADTH_FIRST);
    ListQueue<Node<GT> *> queue;
    queue.put(&begin);

    while (not queue.is_empty())
      {
	Node<GT> & p = *queue.get();

	for (AdArcIt<GT> it(g, p); it.has_current(); it.next())
	  {
	    Arc<GT> & a = *it;

	    if (a.is_visited(GraphTag::BREADTH_FIRST))
	      continue;

	    a.visit(GraphTag::BREADTH_FIRST);

	    Node<GT> & q = it.get_tgt_node();

	    if (q.is_visited(GraphTag::BREADTH_FIRST))
	      continue;

	    op(q);
	    q.visit(GraphTag::BREADTH_FIRST);
	    queue.put(&q);
	  }
      }
  }

  template <class GT>
  Path<GT> breadth_first_search_path(GT & g, Node<GT> & begin, Node<GT> & end)
  {
    Path<GT> ret_val(g);
    
    if (&begin == &end)
      {
	ret_val.init(begin);
	return ret_val;
      }

    g.reset_tag(GraphTag::BREADTH_FIRST);
    g.reset_node_cookies();

    begin.visit(GraphTag::BREADTH_FIRST);
    ListQueue<Node<GT> *> queue;
    queue.put(&begin);

    Node<GT> * ptr = nullptr;

    while (not queue.is_empty() and (ptr = queue.get()) != &end)
      {
	for (AdArcIt<GT> it(g, *ptr); it.has_current(); it.next())
	  {
	    Arc<GT> & a = *it;

	    if (a.is_visited(GraphTag::BREADTH_FIRST))
	      continue;

	    a.visit(GraphTag::BREADTH_FIRST);

	    Node<GT> & q = it.get_tgt_node();

	    if (q.is_visited(GraphTag::BREADTH_FIRST))
	      continue;

	    q.cookie() = ptr;
	    q.visit(GraphTag::BREADTH_FIRST);
	    queue.put(&q);
	  }
      }

    if (end.cookie() == nullptr)
      return ret_val;

    Node<GT> * aux = &end;

    while (aux != &begin)
      {
	ret_val.insert(*aux);
	aux = reinterpret_cast<Node<GT> *>(aux->cookie());
      }

    ret_val.insert(begin);

    return ret_val;
  }

  template <class GT>
  SLList<Node<GT> *> bf_topological_sort(GT & g)
  {
    if (not g.is_digraph())
      throw std::domain_error("Argument must be a directed graph");

    g.reset_node_counter();

    g.for_each_arc([] (Arc<GT> & a)
		   {
		     ++a.get_tgt_node().counter();
		   });

    ListQueue<Node<GT> *> queue;

    g.for_each_node([&queue] (Node<GT> & p)
		    {
		      if (p.counter() == 0)
			queue.put(&p);
		    });

    SLList<Node<GT> *> ret;
    
    while (not queue.is_empty())
      {
	Node<GT> * p = queue.get();

	ret.append(p);

	for (AdArcIt<GT> it(g, *p); it.has_current(); it.next())
	  {
	    Node<GT> & q = it.get_tgt_node();

	    if (--q.counter() == 0)
	      queue.put(&q);
	  }
      }

    return ret;
  }

  template <class GT>
  SLList<SLList<Node<GT> *>> topological_ranks(GT & g)
  {
    if (not g.is_digraph())
      throw std::domain_error("Argument must be a directed graph");

    g.reset_node_counter();

    g.for_each_arc([] (Arc<GT> & a)
		   {
		     ++a.get_tgt_node().counter();
		   });

    ListQueue<Node<GT> *> queue;

    g.for_each_node([&queue] (Node<GT> & p)
		    {
		      if (p.counter() == 0)
			queue.put(&p);
		    });

    SLList<SLList<Node<GT> *>> ret;
    
    while (not queue.is_empty())
      {
	ListQueue<Node<GT> *> aqueue;

	SLList<Node<GT> *> & l = ret.append(SLList<Node<GT> *>());

	while (not queue.is_empty())
	  {
	    Node<GT> * p = queue.get();
	    
	    l.append(p);
	    
	    for (AdArcIt<GT> it(g, *p); it.has_current(); it.next())
	      {
		Node<GT> & q = it.get_tgt_node();
		
		if (--q.counter() == 0)
		  aqueue.put(&q);
	      }
	  }

	queue = std::move(aqueue);
      }

    return ret;
  }
  
  template<class GT>
  class DefaultDistance
  {
  public:
    using Type = typename GT::ArcInfoType;

    static constexpr Type ZERO = 0;
    static constexpr Type MAX  = std::numeric_limits<Type>::max();

    Type & operator () (typename GT::Arc & a)
    {
      return a.get_info();
    }
    
    const Type & operator () (const typename GT::Arc & a) const
    {
      return a.get_info();
    }
  };

  template <class GT, class Distance, class Cmp>
  class DistanceCmp
  {
    Distance & distance;
    Cmp      & cmp;
    
  public:
    DistanceCmp(Distance & d, Cmp & _cmp)
      : distance(d), cmp(_cmp)
    {
      // empty
    }

    DistanceCmp(Distance && d = Distance(), Cmp && _cmp = Cmp())
      : distance(d), cmp(_cmp)
    {
      // empty
    }

    bool operator () (typename GT::Arc & a, typename GT::Arc & b) const
    {
      return cmp(distance(a), distance(b));
    }
  };

  template <class GT,
	    class Distance = DefaultDistance<GT>,
	    class Cmp      = std::less<typename Distance::Type>>
    class Kruskal
    {
    public:
      using Node = typename GT::Node;
      using Arc  = typename GT::Arc;

    private:
      using DistCmp      = DistanceCmp<GT, Distance, Cmp>;
      using DistanceType = typename Distance::Type;
    
      static constexpr GraphTag TAG = GraphTag::KRUSKAL;
    
      Distance & distance;
      DistCmp    cmp;

    public:
      Kruskal(Distance & _distance, Cmp & _cmp)
	: distance(_distance), cmp(distance, _cmp)
      {
	// empty
      }

      Kruskal(Distance && _distance = Distance(),
	      Cmp && _cmp = Cmp())
	: distance(_distance),
	  cmp(std::forward<Distance>(_distance), std::forward<Cmp>(_cmp))
      {
	// Empty
      }

      GT build_min_spanning_tree(GT &);

      void paint_min_spanning_tree(GT & g)
      {
	GT tree = build_min_spanning_tree(g);

	g.reset_tag(GraphTag::MIN_SPANNING_TREE);
      
	tree.for_each_node([] (const Node & t_node)
			   {
			     Node * ptr_g_node = mapped_node_ptr<GT>(t_node);  
			     ptr_g_node->visit(GraphTag::MIN_SPANNING_TREE);
			   });
      
	tree.for_each_arc([](const Arc & t_arc)
			  {
			    Arc * ptr_g_arc = mapped_arc_ptr<GT>(t_arc);
			    ptr_g_arc->visit(GraphTag::MIN_SPANNING_TREE);
			  });
      }  };

  template <class GT, class Distance, class DistanceCmp>
  GT Kruskal<GT, Distance, DistanceCmp>::build_min_spanning_tree(GT & g)
  {
    g.reset_tag(TAG);
    g.reset_cookies();

    g.template sort_arcs(cmp);

    GT tree;

    TRelation<Node *> nodes_rel(g.get_num_nodes());
    
    for (ArcIt<GT> it(g); it.has_current() and
	   tree.get_num_arcs() < g.get_num_nodes() - 1; it.next())
      {
	Arc  & ga = *it;
	Node & gs = ga.get_src_node();
	Node & gt = ga.get_tgt_node();

	Node * ts_ptr = mapped_node_ptr<GT>(gs);

	if (ts_ptr == nullptr)
	  {
	    gs.visit(TAG);
	    ts_ptr = &tree.insert_node(gs.get_info());
	    map_nodes<GT>(gs, *ts_ptr);
	  }

	Node * tt_ptr = mapped_node_ptr<GT>(gt);

	if (tt_ptr == nullptr)
	  {
	    gt.visit(TAG);
	    tt_ptr = &tree.insert_node(gt.get_info());
	    map_nodes<GT>(gt, *tt_ptr);
	  }

	if (nodes_rel.are_connected(ts_ptr, tt_ptr))
	  continue;

	nodes_rel.join(ts_ptr, tt_ptr);

	Arc & ta = tree.insert_arc(*ts_ptr, *tt_ptr, ga.get_info());
	ga.visit(TAG);
	map_arcs<GT>(ga, ta);
      }
    
    return tree;
  }
  
  template <class GT, class Distance, class Cmp>
  class ArcHeapCmp
  {
    Distance & distance;
    Cmp      & cmp;

  public:
    ArcHeapCmp(Distance & _distance, Cmp & _cmp)
      : distance(_distance), cmp(_cmp)
    {
      // empty
    }
    ArcHeapCmp(Distance && _distance = Distance(), Cmp && _cmp = Cmp())
      : distance(_distance), cmp(_cmp)
    {
      // empty
    }
    
    bool operator () (typename GT::Arc * a, typename GT::Arc * b) const
    {
      return cmp(distance(*a), distance(*b));
    }
  };
  
  template <class GT,
	    class Distance = DefaultDistance<GT>,
	    class Cmp      = std::less<typename Distance::Type>>
    class ArcHeap : public LHeap<typename GT::Arc *,
				 ArcHeapCmp<GT, Distance, Cmp>>
    {
      using Node     = typename GT::Node;
      using Arc      = typename GT::Arc;
      using Table    = HashMap<Node *, Arc **>;
      using BaseHeap = LHeap<Arc *, ArcHeapCmp<GT, Distance, Cmp>>;
      using BaseHeap::BaseHeap;

      Table tgt_nodes;

    public:
      void insert_arc(Arc & a, Node & t)
      {
	Arc *** result = tgt_nodes.search(&t);

	if (result == nullptr)
	  {
	    tgt_nodes[&t] = const_cast<Arc **>(&BaseHeap::insert(&a));
	    return;
	  }

	Arc ** ap = *result;

	if (BaseHeap::get_cmp()(*ap, &a))
	  return;

	BaseHeap::remove(*ap);
	tgt_nodes[&t] = const_cast<Arc **>(&BaseHeap::insert(&a));
      }

      Arc * get_min_arc()
      {
	Arc   * arc_ptr = BaseHeap::top();
	Node  * tgt_ptr = &arc_ptr->get_src_node();
	Arc *** result =  tgt_nodes.search(tgt_ptr);

	if (result == nullptr or **result != arc_ptr)
	  result = tgt_nodes.search(tgt_ptr = &arc_ptr->get_tgt_node());

	assert(**result == arc_ptr);

	BaseHeap::get();
	tgt_nodes.remove(tgt_ptr);
      
	return arc_ptr;
      }
    };
  
  template <class GT,
	    class Distance = DefaultDistance<GT>,
	    class Cmp      = std::less<typename Distance::Type>>
    class Prim
    {
    public:
      using Node = typename GT::Node;
      using Arc  = typename GT::Arc;

    private:
      Distance & distance;
      Cmp      & cmp;

      using DistanceType = typename Distance::Type;

      static constexpr GraphTag TAG = GraphTag::PRIM;

    public:
      Prim(Distance & _distance, Cmp & _cmp)
	: distance(_distance), cmp(_cmp)
      {
	// empty
      }

      Prim(Distance && _distance = Distance(), Cmp && _cmp = Cmp())
	: distance(_distance), cmp(_cmp)
      {
	// empty
      }

      GT build_min_spanning_tree(GT &, Node &);

      GT build_min_spanning_tree(GT & g)
      {
	return build_min_spanning_tree(g, g.get_first_node());
      }

      void paint_min_spanning_tree(GT & g, GT & start)
      {
	GT tree = build_min_spanning_tree(g, start);

	g.reset_tag(GraphTag::MIN_SPANNING_TREE);
      
        tree.for_each_node([] (Node & t_node)
	      {
		Node * ptr_g_node = mapped_node_ptr<GT>(t_node);
		ptr_g_node->visit(GraphTag::MIN_SPANNING_TREE);
		ptr_g_node->cookie() = nullptr;
	      });
      
        tree.for_each_arc([](Arc & t_arc)
	      {
		Arc * ptr_g_arc = mapped_arc_ptr<GT>(t_arc);
		ptr_g_arc->visit(GraphTag::MIN_SPANNING_TREE);
		ptr_g_arc->cookie() = nullptr;
	      });
      }

      void paint_min_spanning_tree(GT & g)
      {
	paint_min_spanning_tree(g, g.get_first_node());
      }
    };

  template <class GT, class Distance, class Cmp>
  GT Prim<GT, Distance, Cmp>::build_min_spanning_tree(GT & g, Node & start)
  {
    g.reset_tag(TAG);
    g.reset_cookies();

    GT tree;
    Node & t_start = tree.insert_node(start.get_info());
    map_nodes<GT>(start, t_start);
    start.visit(TAG);

    ArcHeapCmp<GT, Distance, Cmp> arc_heap_cmp(distance, cmp);
    ArcHeap<GT, Distance, Cmp> arc_heap(arc_heap_cmp);

    for (typename GT::AdjacentArcIterator it(g, start); it.has_current();
	 it.next())
      {
	Arc  & a = *it;
	Node & t =  a.get_connected_node(start);
	arc_heap.insert_arc(a, t);
      }

    while (not arc_heap.is_empty() and tree.get_num_nodes() < g.get_num_nodes())
      {
	Arc & a = *arc_heap.get_min_arc();

	if (a.is_visited(TAG))
	  continue;

	a.visit(TAG);

	Node & p = a.get_src_node().is_visited(TAG) ?
	  a.get_tgt_node() : a.get_src_node();

	if (p.is_visited(TAG))
	  continue;

	p.visit(TAG);

	Node & pp = tree.insert_node(p.get_info());
	map_nodes<GT>(p, pp);

	Node & ts = *mapped_node_ptr<GT>(a.get_src_node());
	Node & tt = *mapped_node_ptr<GT>(a.get_tgt_node());
	Arc  & ap =  tree.insert_arc(ts, tt, a.get_info());

	map_arcs<GT>(a, ap);
	
	for (typename GT::AdjacentArcIterator it(g, p); it.has_current();
	     it.next())
	  {
	    Arc  & pa = *it;
	    Node & pt =  pa.get_connected_node(p);
	    arc_heap.insert_arc(pa, pt);
	  }
      }

    return tree;
  }

  template <class GT,
	    class Distance = DefaultDistance<GT>,
            class Cmp      = std::less<typename Distance::Type>,
            class Plus     = std::plus<typename Distance::Type>>
    class Dijkstra
    {
    public:
      using Node = typename GT::Node;
      using Arc  = typename GT::Arc;
    
    private:
      Distance & distance;
      Cmp      & cmp;
      Plus     & plus;

      using DistanceType = typename Distance::Type;

      static constexpr GraphTag TAG = GraphTag::DIJKSTRA;

      using PotHeap = ArcHeap<GT, GetPot<GT, Distance>, Cmp>;

      GT build_partial_min_path_tree(GT &, Node &, Node &);

    public:
      Dijkstra(Distance & _distance, Cmp & _cmp, Plus & _plus)
	: distance(_distance), cmp(_cmp), plus(_plus)
      {
	// empty
      }

      Dijkstra(Distance && _distance = Distance(),
	       Cmp && _cmp = Cmp(), Plus && _plus = Plus())
	: distance(_distance), cmp(_cmp), plus(_plus)
      {
	// Empty
      }

      GT build_min_path_tree(GT &, Node &);

      void paint_min_path_tree(GT & g, Node & start)
      {
	GT tree = build_min_path_tree(g, start);

	g.reset_tag(GraphTag::MIN_PATH_TREE);
      
	tree.for_each_node([](const Node & t_node)
			   {
			     Node * ptr_g_node = mapped_node_ptr<GT>(t_node);
			     ptr_g_node->visit(GraphTag::MIN_PATH_TREE);
			     ptr_g_node->cookie() = nullptr;
			   });
      
	tree.for_each_arc([](const Arc & t_arc)
			  {
			    Arc * ptr_g_arc = mapped_arc_ptr<GT>(t_arc);
			    ptr_g_arc->visit(GraphTag::MIN_PATH_TREE);
			    ptr_g_arc->cookie() = nullptr;
			  });
      }
    
      Path<GT> search_min_path(GT & g, Node & start, Node & end)
      {
	GT tree = build_partial_min_path_tree(g, start, end);

	Node & t_start = *mapped_node_ptr<GT>(start);
	Node & t_end   = *mapped_node_ptr<GT>(end);

	Path<GT> t_path = depth_first_search_path(tree, t_start, t_end);
	Path<GT> path(g);
      
	if (t_path.is_empty())
	  return path;

	path.init(start);

	t_path.for_each([&](Node *, Arc * ptr_arc)
	      {
		if (ptr_arc == nullptr)
		  return;
		
		Arc & arc = *mapped_arc_ptr<GT>(*ptr_arc);
		path.append(arc);
	      });
      
	return path;
      }

      void paint_min_path(GT & g, Node & start, Node & end)
      {
	Path<GT> path = search_min_path(g, start, end);

	g.reset_tag(GraphTag::MIN_PATH);

	path.for_each([](Node * ptr_node, Arc * ptr_arc)
	      {
		ptr_node->visit(GraphTag::MIN_PATH);
		
		if (ptr_arc != nullptr)
		  ptr_arc->visit(GraphTag::MIN_PATH);
	      });
      }
    };

  template <class GT, class Distance, class Cmp, class Plus>
  GT Dijkstra<GT, Distance, Cmp, Plus>::
  build_partial_min_path_tree(GT & g, Node & start, Node & end)
  {
    g.reset_tag(TAG);
    allocate_node_info<GT, Distance>(g);
    allocate_arc_info<GT, Distance>(g);

    GT tree;

    start.visit(TAG);
    ACC<GT, Distance>(start) = Distance::ZERO;
    TREE_NODE<GT, Distance>(start) = &tree.insert_node(start.get_info());
    TREE_NODE<GT, Distance>(start)->cookie() = &start;

    GetPot<GT, Distance> get_pot;
    ArcHeapCmp<GT, GetPot<GT, Distance>, Cmp> arc_heap_cmp(get_pot, cmp);
    PotHeap heap(arc_heap_cmp);

    for (typename GT::AdjacentArcIterator it(g, start); it.has_current();
	 it.next())
      {
	Arc & arc = *it;
	POT<GT, Distance>(arc) = distance(arc);
	arc.visit(TAG);
	put_in_heap<GT, Distance>(arc, arc.get_connected_node(start), heap);
      }

    while (tree.get_num_nodes() < g.get_num_nodes())
      {
	Arc  & g_arc = get_from_heap<GT, Distance>(heap);
	Node & g_src = g_arc.get_src_node();
	Node & g_tgt = g_arc.get_tgt_node();

	if (g_src.is_visited(TAG) and g_tgt.is_visited(TAG))
	  continue;

	Node & new_node = g_src.is_visited(TAG) ? g_tgt : g_src;

	Node & t_tgt = tree.insert_node(new_node.get_info());

	TREE_NODE<GT, Distance>(new_node) = &t_tgt;

	new_node.visit(TAG);

	Arc & t_arc = tree.insert_arc(*TREE_NODE<GT, Distance>(g_src),
				      *TREE_NODE<GT, Distance>(g_tgt),
				      g_arc.get_info());

	TREE_ARC<GT, Distance>(g_arc) = &t_arc;

	if (&new_node ==  &end)
	  break;

	ACC<GT, Distance>(new_node) = POT<GT, Distance>(g_arc);

	const DistanceType & acc = ACC<GT, Distance>(new_node);

	for (typename GT::AdjacentArcIterator it(g, new_node);
	     it.has_current(); it.next())
	  {
	    Arc & arc = *it;

	    if (arc.is_visited(TAG))
	      continue;

	    arc.visit(TAG);

	    Node & tgt = it.get_tgt_node();

	    if (tgt.is_visited(TAG))
	      continue;

	    POT<GT, Distance>(arc) = plus(acc, distance(arc));

	    put_in_heap<GT, Distance>(arc, tgt, heap);
	  }
      }

    destroy_node_info<GT, Distance>(g);
    destroy_arc_info<GT, Distance>(g);

    return tree;
  }

  template <class GT, class Distance, class Cmp, class Plus>
  GT Dijkstra<GT, Distance, Cmp, Plus>::
  build_min_path_tree(GT & g, Node & start)
  {
    g.reset_tag(TAG);
    allocate_node_info<GT, Distance>(g);
    allocate_arc_info<GT, Distance>(g);
    
    GT tree;
    
    start.visit(TAG);
    ACC<GT, Distance>(start) = Distance::ZERO;
    TREE_NODE<GT, Distance>(start) = &tree.insert_node(start.get_info());
    TREE_NODE<GT, Distance>(start)->cookie() = &start;

    GetPot<GT, Distance> get_pot;
    ArcHeapCmp<GT, GetPot<GT, Distance>, Cmp> arc_heap_cmp(get_pot, cmp);
    PotHeap heap(arc_heap_cmp);
    
    for (typename GT::AdjacentArcIterator it(g, start); it.has_current();
	 it.next())
      {
	Arc & arc = *it;
	
	POT<GT, Distance>(arc) = distance(arc);
	arc.visit(TAG);
	put_in_heap<GT, Distance>(arc, arc.get_connected_node(start), heap);
      }
    
    while (tree.get_num_nodes() < g.get_num_nodes())
      {
	Arc  & g_arc = get_from_heap<GT, Distance>(heap);
	Node & g_src = g_arc.get_src_node();
	Node & g_tgt = g_arc.get_tgt_node();
	
	if (g_src.is_visited(TAG) and g_tgt.is_visited(TAG))
	  continue;
	
	Node & new_node = g_src.is_visited(TAG) ? g_tgt : g_src;
	
	Node & t_tgt = tree.insert_node(new_node.get_info());
	
	TREE_NODE<GT, Distance>(new_node) = &t_tgt;
	
        new_node.visit(TAG);
	
	Arc & t_arc = tree.insert_arc(*TREE_NODE<GT, Distance>(g_src),
				      *TREE_NODE<GT, Distance>(g_tgt),
				      g_arc.get_info());
	
	TREE_ARC<GT, Distance>(g_arc) = &t_arc;
	
	ACC<GT, Distance>(new_node) = POT<GT, Distance>(g_arc);
	
	const DistanceType & acc = ACC<GT, Distance>(new_node);
	
	for (typename GT::AdjacentArcIterator it(g, new_node);
	     it.has_current(); it.next())
	  {
	    Arc & arc = *it;
	    
	    if (arc.is_visited(TAG))
	      continue;
	    
	    arc.visit(TAG);
	    
	    Node & tgt = it.get_tgt_node();
	    
	    if (tgt.is_visited(TAG))
	      continue;
	    
	    POT<GT, Distance>(arc) = plus(acc, distance(arc));
	    
	    put_in_heap<GT, Distance>(arc, tgt, heap);
	  }
      }
    
    destroy_node_info<GT, Distance>(g);
    destroy_arc_info<GT, Distance>(g);
    
    return tree;
  }

  template <class GT, class Distance>
  class DefaultHeuristic
  {
  public:
    typename Distance::Type operator () (typename GT::Node &,
					 typename GT::Node &)
    {
      return Distance::ZERO;
    }
  };

  template <class GT,
	    class Distance  = DefaultDistance<GT>,
	    class Heuristic = DefaultHeuristic<GT, Distance>,
	    class Cmp       = std::less<typename Distance::Type>,
            class Plus      = std::plus<typename Distance::Type>>
    class Astar
    {
    public:
      using Node = typename GT::Node;
      using Arc  = typename GT::Arc;
    
    private:
      Distance  & distance;
      Heuristic & heuristic;
      Cmp       & cmp;
      Plus      & plus;

      using DistanceType = typename Distance::Type;

      static constexpr GraphTag TAG = GraphTag::ASTAR;

      using PotHeap = ArcHeap<GT, GetPot<GT, Distance>, Cmp>;

      GT build_partial_min_path_tree(GT &, Node &, Node &);

    public:
      Astar(Distance & _distance, Heuristic & _heuristic,
	    Cmp & _cmp, Plus & _plus)
	: distance(_distance), heuristic(_heuristic), cmp(_cmp),
	  plus(_plus)
      {
	// empty
      }

      Astar(Distance && _distance = Distance(),
	    Heuristic && _heuristic = Heuristic(), Cmp && _cmp = Cmp(),
	    Plus && _plus = Plus())
	: distance(_distance), heuristic(_heuristic), cmp(_cmp),
	  plus(_plus)
      {
	// Empty
      }
      
      Path<GT> search_min_path(GT & g, Node & start, Node & end)
      {
	GT tree = build_partial_min_path_tree(g, start, end);

	Node & t_start = *mapped_node_ptr<GT>(start);
	Node & t_end   = *mapped_node_ptr<GT>(end);

	Path<GT> t_path = depth_first_search_path(tree, t_start, t_end);
	Path<GT> path(g);
      
	if (t_path.is_empty())
	  return path;

	path.init(start);

	t_path.for_each([&](Node *, Arc * ptr_arc)
	      {
		if (ptr_arc == nullptr)
		  return;
		
		Arc & arc = *mapped_arc_ptr<GT>(*ptr_arc);
		path.append(arc);
	      });
      
	return path;
      }

      void paint_min_path(GT & g, Node & start, Node & end)
      {
	Path<GT> path = search_min_path(g, start, end);

	g.reset_tag(GraphTag::MIN_PATH);

	path.for_each([](Node * ptr_node, Arc * ptr_arc)
	      {
		ptr_node->visit(GraphTag::MIN_PATH);
		
		if (ptr_arc != nullptr)
		  ptr_arc->visit(GraphTag::MIN_PATH);
	      });
      }
    };

  template <class GT, class Distance, class Heuristic, class Cmp, class Plus>
  GT Astar<GT, Distance, Heuristic, Cmp, Plus>::
  build_partial_min_path_tree(GT & g, Node & start, Node & end)
  {
    g.reset_tag(TAG);
    allocate_node_info<GT, Distance>(g);
    allocate_arc_info<GT, Distance>(g);
    
    GT tree;
    
    start.visit(TAG);
    ACC<GT, Distance>(start) = Distance::ZERO;
    TREE_NODE<GT, Distance>(start) = &tree.insert_node(start.get_info());
    TREE_NODE<GT, Distance>(start)->cookie() = &start;
    
    GetPot<GT, Distance> get_pot;
    ArcHeapCmp<GT, GetPot<GT, Distance>, Cmp> arc_heap_cmp(get_pot, cmp);
    PotHeap heap(arc_heap_cmp);
    
    for (typename GT::AdjacentArcIterator it(g, start); it.has_current();
	 it.next())
      {
	Arc & arc = *it;
	POT<GT, Distance>(arc) = plus(distance(arc),
				      heuristic(arc.get_tgt_node(), end));
        arc.visit(TAG);
	put_in_heap<GT, Distance>(arc, arc.get_connected_node(start), heap);
      }
    
    while (tree.get_num_nodes() < g.get_num_nodes())
      {
	Arc  & g_arc = get_from_heap<GT, Distance>(heap);
	Node & g_src = g_arc.get_src_node();
	Node & g_tgt = g_arc.get_tgt_node();
	
	if (g_src.is_visited(TAG) and g_tgt.is_visited(TAG))
	  continue;
	
	Node & new_node = g_src.is_visited(TAG) ? g_tgt : g_src;
	
	Node & t_tgt = tree.insert_node(new_node.get_info());
	
	TREE_NODE<GT, Distance>(new_node) = &t_tgt;
	
	new_node.visit(TAG);
	
	Arc & t_arc = tree.insert_arc(*TREE_NODE<GT, Distance>(g_src),
				      *TREE_NODE<GT, Distance>(g_tgt),
				      g_arc.get_info());
	
	TREE_ARC<GT, Distance>(g_arc) = &t_arc;
	
	if (&new_node ==  &end)
	  break;
	
	ACC<GT, Distance>(new_node) = POT<GT, Distance>(g_arc);
	
	const DistanceType & acc = ACC<GT, Distance>(new_node);
	
	for (typename GT::AdjacentArcIterator it(g, new_node);
	     it.has_current(); it.next())
	  {
	    Arc & arc = *it;
	    
	    if (arc.is_visited(TAG))
	      continue;
	    
	    arc.visit(TAG);
	    
	    Node & tgt = it.get_tgt_node();
	    
	    if (tgt.is_visited(TAG))
	      continue;
	    
	    POT<GT, Distance>(arc) = plus(plus(acc, distance(arc)),
					  heuristic(arc.get_tgt_node(), end));
	    
	    put_in_heap<GT, Distance>(arc, tgt, heap);
	  }
      }
    
    destroy_node_info<GT, Distance>(g);
    destroy_arc_info<GT, Distance>(g);
    
    return tree;
  }

  template <class GT,
	    class Distance = DefaultDistance<GT>,
            class Cmp      = std::less<typename Distance::Type>,
            class Plus     = std::plus<typename Distance::Type>>
    class BellmanFord
    {
      using Node = typename GT::Node;
      using Arc  = typename GT::Arc;

      Distance & distance;
      Cmp      & cmp;
      Plus     & plus;
    
      struct BFNodeInfo
      {
	nat_t                   idx;
	typename Distance::Type accum;
      };
    
      static BFNodeInfo *& BFNI(Node & p)
      {
	return (BFNodeInfo *&) p.cookie();
      }

      static typename Distance::Type & ACC(Node & p)
      {
	return BFNI(p)->accum;
      }

      static nat_t & IDX(Node & p)
      {
	return BFNI(p)->idx;
      }
    
      static void init_node_info(GT & g,
				 FixedArray<Node *> & pred,
				 FixedArray<Arc *> & arcs)
      {
	nat_t i = 0;
      
	g.for_each_node([&] (Node & p)
	      {
		pred[i] = nullptr;
		arcs[i] = nullptr;
		BFNI(p) = new BFNodeInfo;
	        IDX(p) = i;
		ACC(p) = Distance::MAX;
		++i;
	      });
      }

      static void destroy_node_info(GT & g)
      {
	g.for_each_node([] (Node & p)
	      {
		delete BFNI(p);
		BFNI(p) = nullptr;
	      });
      }

      bool relax(Arc & a, FixedArray<Node *> & pred, FixedArray<Arc *> & arcs)
      {
	Node & s = a.get_src_node();
	Node & t = a.get_tgt_node();

	if (ACC(s) == Distance::MAX)
	  return false;

	typename Distance::Type sum = plus(ACC(s), distance(a));
 
	if (cmp(sum, ACC(t)))
	  {
	    const nat_t & i = IDX(t);
	    pred[i] = &s;
	    arcs[i] = &a;
	    ACC(t) = sum;
	    return true;
	  }
      
	return false;
      }

      bool generic_algorithm(GT &, FixedArray<Node *> &, FixedArray<Arc *> &);

    public:
      BellmanFord(Distance & _distance, Cmp & _cmp, Plus & _plus)
	: distance(_distance), cmp(_cmp), plus(_plus)
      {
	// empty
      }

      BellmanFord(Distance && _distance = Distance(), Cmp && _cmp = Cmp(),
		  Plus && _plus = Plus())
	: distance(_distance), cmp(_cmp), plus(_plus)
      {
	// empty
      }

      std::tuple<bool, GT> build_min_path_tree(GT &, Node &);

      bool paint_min_path_tree(GT & g, Node & start)
      {
	auto t = build_min_path_tree(g, start);

	if (not std::get<0>(t))
	  return false;
      
	g.reset_tag(GraphTag::MIN_SPANNING_TREE);

	GT & tree = std::get<1>(t);
      
	tree.for_each_node([] (const Node & t_node)
			   {
			     Node * ptr_g_node = mapped_node_ptr<GT>(t_node);  
			     ptr_g_node->visit(GraphTag::MIN_SPANNING_TREE);
			     ptr_g_node->cookie() = nullptr;
			   });
      
	tree.for_each_arc([](const Arc & t_arc)
			  {
			    Arc * ptr_g_arc = mapped_arc_ptr<GT>(t_arc);
			    ptr_g_arc->visit(GraphTag::MIN_SPANNING_TREE);
			    ptr_g_arc->cookie() = nullptr;
			  });
	return true;
      }

      std::tuple<bool, Path<GT>> search_min_path(GT & g, Node & start, Node & end)
      {
	auto t = build_min_path_tree(g, start, end);
      
	if (not std::get<0>(t))
	  return make_tuple(false, Path<GT>());
      
	GT & tree = std::get<1>(t);
      
	Node & t_start = *mapped_node_ptr<GT>(start);
	Node & t_end   = *mapped_node_ptr<GT>(end);
      
	Path<GT> t_path = depth_first_search_path(tree, t_start, t_end);
	Path<GT> path(g);
      
	if (t_path.is_empty())
	  return path;
      
	path.init(start);
      
	t_path.for_each([&](Node *, Arc * ptr_arc)
	      {
		if (ptr_arc == nullptr)
		  return;
			
		Arc & arc = *mapped_arc_ptr<GT>(*ptr_arc);
		path.append(arc);
	      });
      
	return make_tuple(true, path);
      }

      bool paint_min_path(GT & g, Node & start, Node & end)
      {
	auto t = search_min_path(g, start, end);

	if (not std::get<0>(t))
	  return false;
      
	Path<GT> & path = std::get<1>(t);
      
	g.reset_tag(GraphTag::MIN_PATH);
      
	path.for_each([](Node * ptr_node, Arc * ptr_arc)
	      {
		ptr_node->visit(GraphTag::MIN_PATH);
		
		if (ptr_arc != nullptr)
		  ptr_arc->visit(GraphTag::MIN_PATH);
	      });

	return true;
      }
    
    };
  
  template <class GT, class Distance, class Cmp, class Plus>
  bool BellmanFord<GT, Distance, Cmp, Plus>::
  generic_algorithm(GT & g,  FixedArray<Node *> & pred, FixedArray<Arc *> & arcs)
  {
    nat_t n = g.get_num_nodes() - 1;
    
    bool had_relaxation = true;

    nat_t i;
    
    for (i = 0; had_relaxation and i < n; ++i)
      {
	had_relaxation = false;

	g.for_each_arc([&] (Arc & a)
		       {
			 if (relax(a, pred, arcs))
			   had_relaxation = true;
		       });
      }
	
    if (i < n)
      return true;

    had_relaxation = false;
    
    g.for_each_arc([&] (Arc & a)
		   {
		     if (relax(a, pred, arcs))
		       had_relaxation = true;
		   });
    
    return not had_relaxation;
  }

  template <class GT, class Distance, class Cmp, class Plus>
  std::tuple<bool, GT> BellmanFord<GT, Distance, Cmp, Plus>::
  build_min_path_tree(GT & g, Node & start)
  {
    if (not g.is_digraph())
      throw std::domain_error("Argument must be a directed graph");
    
    FixedArray<Node *> pred(g.get_num_nodes());
    FixedArray<Arc *>  arcs(g.get_num_nodes());
    init_node_info(g, pred, arcs);
    ACC(start) = Distance::ZERO;

    bool result = generic_algorithm(g, pred, arcs);

    destroy_node_info(g);

    if (not result)
      return std::make_tuple(false, GT());

    GT tree;

    for (nat_t i = 0; i < arcs.size(); ++i)
      {
	Arc * a = arcs[i];

	if (a == nullptr)
	  continue;

	Node & s = a->get_src_node();

	Node * st = mapped_node_ptr<GT>(s);

	if (st == nullptr)
	  {
	    st = &tree.insert_node(s.get_info());
	    map_nodes<GT>(s, *st);
	  }
	
	Node & t = a->get_tgt_node();

	Node * tt = mapped_node_ptr<GT>(t);

	if (tt == nullptr)
	  {
	    tt = &tree.insert_node(t.get_info());
	    map_nodes<GT>(t, *tt);
	  }

	Arc & at = tree.insert_arc(*st, *tt, a->get_info());
	map_arcs<GT>(*a, at);
      }
    
    return std::make_tuple(true, std::move(tree));
  }

  template <class GT>
  class KargerMinCut
  {
    using Node      = typename GT::Node;
    using Arc       = typename GT::Arc;
    using MNodeInfo = SLList<Node *>;
    using MArcInfo  = Arc *;
    using MGraph    = Graph<MNodeInfo, MArcInfo>;
    using MNode     = typename MGraph::Node;
    using MArc      = typename MGraph::Arc;
    using ArcSet    = TreeSet<MArc *>;

    static constexpr real_t SQ_TWO = 1.414213562;

    rng_t rng;

    nat_t num_iterations_hint(nat_t n)
    {
      return n * n * std::log10(n);
    }
    
    MGraph build_mgraph(GT & g)
    {
      g.reset_nodes();
      g.reset_arcs();

      MGraph mg;

      g.for_each_node([&] (Node & p)
		      {
			MNode * mp = &mg.insert_node();
			mp->get_info().append(&p);
			map_nodes<GT, MGraph>(p, *mp);
		      });

      g.for_each_arc([&] (Arc & a)
		     {
		       Node & p = a.get_src_node();
		       Node & q = a.get_tgt_node();
			       
		       MNode & mp = *mapped_node_ptr<GT, MGraph>(p);
		       MNode & mq = *mapped_node_ptr<GT, MGraph>(q);

		       mg.insert_arc(mp, mq, &a);
		     });

      return mg;
    }

    ArcSet build_arcs(MGraph & mg)
    {
      ArcSet ret;
      mg.for_each_arc([&] (MArc & a) { ret.insert(&a); });
      return ret;
    }

    void contract_arc(MGraph &, MNode &, MNode &, MNode &, ArcSet &);

    void contract(MGraph &, ArcSet &, nat_t);

    std::tuple<SLList<Node *>, SLList<Node *>, SLList<Arc *>>
      compute_min_cut(MGraph &, nat_t);

    std::tuple<SLList<Node *>, SLList<Node *>, SLList<Arc *>>
      compute_min_cut(MGraph & mg)
    {
      return compute_min_cut(mg, num_iterations_hint(mg.get_num_nodes()));
    }

    std::tuple<SLList<Node *>, SLList<Node *>, SLList<Arc *>>
      compute_min_cut_fast_rec(MGraph &);
    
  public:
    KargerMinCut(rng_seed_t seed)
      : rng(seed)
    {
      // empty
    }

    KargerMinCut()
      : KargerMinCut(get_random_seed())
    {
      // empty
    }
    
    std::tuple<SLList<Node *>, SLList<Node *>, SLList<Arc *>>
      compute_min_cut(GT & g, nat_t num_it)
    {
      if (g.get_num_arcs() == 0)
	throw std::domain_error("Graph has not arcs");

      MGraph mg = build_mgraph(g);
      return compute_min_cut(mg, num_it);
    }

    std::tuple<SLList<Node *>, SLList<Node *>, SLList<Arc *>>
      compute_min_cut(GT & g)
    {
      return compute_min_cut(g, num_iterations_hint(g.get_num_nodes()));
    }
    
    std::tuple<SLList<Node *>, SLList<Node *>, SLList<Arc *>>
      compute_min_cut_fast(GT & g)
    {
      MGraph mg = build_mgraph(g);
      return compute_min_cut_fast_rec(mg);
    }    

    std::tuple<SLList<Node *>, SLList<Node *>, SLList<Arc *>>
      operator () (GT & g)
    {
      return compute_min_cut_fast(g);
    }
  };

  template <class GT>
  void KargerMinCut<GT>::contract_arc(MGraph & mg, MNode & p, MNode & q,
				      MNode & r, ArcSet & arcs)
  {
    for (AdArcIt<MGraph> it(mg, p); it.has_current(); it.next())
      {
	MArc  & a = it.get_current();
	MNode & t = it.get_tgt_node();
	
	arcs.remove(&a);
	
	if (&t == &q)
	  continue;
	
	MArc & aa = mg.insert_arc(t, r, a.get_info());
	arcs.insert(&aa);
      }
  }
  
  template <class GT>
  void KargerMinCut<GT>::contract(MGraph & mg, ArcSet & arcs, nat_t limit)
  {
    while (mg.get_num_nodes() > limit)
      {
	auto i = random_uniform(rng, arcs.size());
	MArc * ma = arcs.select(i);
	arcs.remove(ma);

	MNode & x = ma->get_src_node();
	MNode & y = ma->get_tgt_node();
	
	MNode & z = mg.insert_node();
	z.get_info().swap(x.get_info());
	z.get_info().concat(y.get_info());

	contract_arc(mg, x, y, z, arcs);
	contract_arc(mg, y, x, z, arcs);

	mg.remove_node(x);
	mg.remove_node(y);
      }
  }

  template <class GT>
  std::tuple<SLList<Node<GT> *>, SLList<Node<GT> *>, SLList<Arc<GT> *>>
    KargerMinCut<GT>::compute_min_cut(MGraph & _mg, nat_t num_it)
  {
    nat_t min_cut = std::numeric_limits<nat_t>::max();

    SLList<Node *> ss;
    SLList<Node *> ts;
    SLList<Arc *> cut;
    
    for (int i = 0; i < num_it; ++i)
      {
	MGraph mg(_mg);
        ArcSet arcs = build_arcs(mg);

	contract(mg, arcs, 2);

	assert(mg.get_num_nodes() == 2);

	nat_t cut_size = mg.get_num_arcs();

	if (cut_size >= min_cut)
	  continue;

	SLList<Arc *> tmp_cut;

	for (ArcIt<MGraph> it(mg); it.has_current(); it.next())
	  tmp_cut.append(it.get_current().get_info());

	cut.swap(tmp_cut);

	MArc & first_arc = mg.get_first_arc();

	MNode & src = first_arc.get_src_node();
	ss.swap(src.get_info());
	
	MNode & tgt = first_arc.get_tgt_node();
	ts.swap(tgt.get_info());
      }

    return std::make_tuple(std::move(ss), std::move(ts), std::move(cut));
  }

  template <class GT>
  std::tuple<SLList<Node<GT> *>, SLList<Node<GT> *>, SLList<Arc<GT> *>>
    KargerMinCut<GT>::compute_min_cut_fast_rec(MGraph & mg)
  {
    nat_t n = mg.get_num_nodes();

    if (n <= 6)
      return compute_min_cut(mg);

    nat_t t = std::ceil(1 + real_t(n) / SQ_TWO);

    MGraph h1(mg);
    ArcSet arcs1 = build_arcs(h1);
    contract(h1, arcs1, t);
    auto r1 = compute_min_cut_fast_rec(h1);
    
    MGraph h2(mg);
    ArcSet arcs2 = build_arcs(h2);
    contract(h2, arcs2, t);
    auto r2 = compute_min_cut_fast_rec(h2);

    if (std::get<2>(r1).size() < std::get<2>(r2).size())
      return r1;

    return r2;
  }

  template <class GT,
	    class NodeAttr  = DftDotNodeAttr<GT>,
	    class ArcAttr   = DftDotArcAttr<GT>,
            class GraphAttr = DftDotGraphAttr<GT>>
    class DotGraph
    {
      NodeAttr  & node_attr;
      ArcAttr   & arc_attr;
      GraphAttr & graph_attr;

    public:
      using Node  = typename GT::Node;
      using Arc   = typename GT::Arc;
    
      DotGraph(NodeAttr & _node_attr, ArcAttr & _arc_attr,
	       GraphAttr & _graph_attr)
	: node_attr(_node_attr), arc_attr(_arc_attr), graph_attr(_graph_attr)
      {
	// empty
      }

      DotGraph(NodeAttr && _node_attr = NodeAttr(),
	       ArcAttr && _arc_attr = ArcAttr(),
	       GraphAttr && _graph_attr = GraphAttr())
	: node_attr(_node_attr), arc_attr(_arc_attr), graph_attr(_graph_attr)
      {
	// empty
      }

      void write_graph(const GT &, std::ofstream &,
		       const std::string & rankdir = "LR");
      
      void write_graph(const GT & g, const std::string & file_name)
      {
	std::ofstream file(file_name.c_str());
	write_graph(g, file);
	file.close();
      }
    };

  template <class GT, class NodeAttr, class ArcAttr, class GraphAttr>
  void DotGraph<GT, NodeAttr, ArcAttr, GraphAttr>::
  write_graph(const GT & g, std::ofstream & output, const std::string & rankdir)
  {
    output << "// "
	   << "File generated automatically by write_graph in DotGraph"
	   << "\n\n";
    
    if (g.is_digraph())
      output << "di";
    
    output << "graph\n"
	   << "{\n";
    
    output << "  rankdir = " << rankdir << ";\n\n";
    
    output << graph_attr(g) << "\n\n";
    
    output << "  // Nodes \n\n";
    
    TreeMap<Node *, nat_t> map;

    const std::string arc_connector = g.is_digraph() ? "->" : "--";
    
    nat_t i = 0;
    
    g.for_each_node([&](const Node & node)
		    {
		      output << "  " << i << "[" << node_attr(node) << "];\n";
		      map[const_cast<Node *>(&node)] = i;
		      ++i;
		    });
    
    output << "\n  // Arcs \n\n";
    
    g.for_each_arc([&](const Arc & arc)
		   {
		     Arc  & arc_ref  = const_cast<Arc &>(arc);
		     Node & src_node = arc_ref.get_src_node();
		     Node & tgt_node = arc_ref.get_tgt_node();
		     nat_t src_index = map[&src_node];
		     nat_t tgt_index = map[&tgt_node];
		     
		     output << "  "
			    << src_index << arc_connector << tgt_index
			    << "["
			    << arc_attr(arc)
			    << "];\n";
		   });
    
    output << "}\n";
  }

} // end namespace Designar

# endif // DSGGRAPHALGORITHMS_H
