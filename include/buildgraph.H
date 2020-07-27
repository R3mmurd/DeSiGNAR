/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGBUILDGRAPH_H
# define DSGBUILDGRAPH_H

# include <graphalgorithms.H>
# include <random.H>

namespace Designar
{
  template <class GT, class NodeInit, class ArcInit>
  GT full_graph(nat_t, NodeInit &, ArcInit &);

  template <class GT,
	    class NodeInit = DftNodeInit<GT>,
	    class ArcInit = DftArcInit<GT>>
  GT full_graph(nat_t, NodeInit && node_init = NodeInit(),
		ArcInit && arc_init = ArcInit());

  template <class GT, class NodeInit, class ArcInit>
  GT ring_graph(nat_t, nat_t, NodeInit &, ArcInit &);

  template <class GT,
	    class NodeInit = DftNodeInit<GT>,
	    class ArcInit = DftArcInit<GT>>
  GT ring_graph(nat_t, nat_t, NodeInit && node_init = NodeInit(),
		ArcInit && arc_init = ArcInit());
  
  template <class GT, class NodeInit, class ArcInit>
  GT build_grid(nat_t width, nat_t height, NodeInit & node_init,
		ArcInit & arc_init, bool with_diagonal = true);

  template <class GT,
	    class NodeInit = DftGridNodeInit<GT>,
	    class ArcInit = DftGridArcInit<GT>>
  GT build_grid(nat_t width, nat_t height, NodeInit && node_init = NodeInit(),
		ArcInit && arc_init = ArcInit(), bool with_diagonal = true);

  template <class GT, class NodeInit, class ArcInit>
  GT random_graph(nat_t, nat_t, rng_seed_t, NodeInit &, ArcInit &);

  template <class GT,
	    class NodeInit = DftNodeInit<GT>,
	    class ArcInit = DftArcInit<GT>>
  GT random_graph(nat_t, nat_t, rng_seed_t,
		  NodeInit && node_init = NodeInit(),
		  ArcInit && arc_init = ArcInit());

  template <class GT,
	    class NodeInit = DftNodeInit<GT>,
	    class ArcInit = DftArcInit<GT>>
  GT random_graph(nat_t, nat_t,
		  NodeInit && node_init = NodeInit(),
		  ArcInit && arc_init = ArcInit());

  template <class GT, class NodeInit, class ArcInit>
  GT er_random_graph(nat_t, real_t, rng_seed_t, bool, NodeInit &, ArcInit &);

  template <class GT,
	    class NodeInit = DftNodeInit<GT>,
	    class ArcInit = DftArcInit<GT>>
  GT er_random_graph(nat_t, real_t, rng_seed_t, bool grant_connectivity = false,
		     NodeInit && node_init = NodeInit(),
		     ArcInit && arc_init = ArcInit());

  template <class GT, class NodeInit, class ArcInit>
  GT er_random_graph(nat_t, real_t, bool, NodeInit &, ArcInit &);

  template <class GT,
	    class NodeInit = DftNodeInit<GT>,
	    class ArcInit = DftArcInit<GT>>
  GT er_random_graph(nat_t, real_t, bool grant_connectivity = false,
		     NodeInit && node_init = NodeInit(),
		     ArcInit && arc_init = ArcInit());

  template <class GT, class NodeInit, class ArcInit>
  GT full_graph(nat_t num_nodes, NodeInit & node_init, ArcInit & arc_init)
  {
    GT g;

    for (nat_t i = 0; i < num_nodes; ++i)
      {
	Node<GT> * p = g.insert_node();
	node_init(p);
      }

    g.for_each_node_pair([&g, &arc_init] (Node<GT> * s, Node<GT> * t)
			 {
			   Arc<GT> * fa = g.insert_arc(s, t);
			   arc_init(fa);

			   if (not g.is_digraph())
			     return;
	  
			   Arc<GT> * ba = g.insert_arc(t, s);
			   arc_init(ba);
			 });
    return g;
  }

  template <class GT, class NodeInit, class ArcInit>
  GT full_graph(nat_t num_nodes, NodeInit && node_init, ArcInit && arc_init)
  {
    return full_graph<GT, NodeInit, ArcInit>(num_nodes, node_init, arc_init);
  }

  template <class GT, class NodeInit, class ArcInit>
  GT ring_graph(nat_t num_nodes, nat_t num_neighbors,
		NodeInit & node_init, ArcInit & arc_init)
  {
    GT g;

    for (nat_t i = 0; i < num_nodes; ++i)
      {
	Node<GT> * p = g.insert_node();
	node_init(p);
      }

    for (auto i = g.nodes_begin(); i != g.nodes_end(); ++i)
      {
	auto j = next_it_c(i);
	
	for (nat_t k = 0; k < num_neighbors / 2; ++k)
	  {
	    Arc<GT> * fa = g.insert_arc(*i, *j);
	    arc_init(fa);

	    if (g.is_digraph())
	      {
		Arc<GT> * ba = g.insert_arc(*j, *i);
		arc_init(ba);
	      }

	    j = next_it_c(j);
	  }
      }
    
    return g;
  }

  template <class GT, class NodeInit, class ArcInit>
  GT ring_graph(nat_t num_nodes, nat_t num_neighbors,
		NodeInit && node_init, ArcInit && arc_init)
  {
    return ring_graph<GT, NodeInit, ArcInit>(num_nodes, num_neighbors,
					     node_init, arc_init);
  }

  template <class GT, class NodeInit, class ArcInit>
  GT build_grid(nat_t width, nat_t height, NodeInit & node_init,
		ArcInit & arc_init, bool with_diagonal)
  {
    if (width < 2 or height < 2)
      throw std::length_error("The minimun size must be 2 x 2");
    
    MultiDimArray<Node<GT> *, 2> mat(height, width);
    
    GT g;

    for (nat_t i = 0; i < height; ++i)
      for (nat_t j = 0; j < width; ++j)
	{
	  mat(i, j) = g.insert_node();
	  node_init(mat(i ,j), i, j);
	  
	  if (j > 0)
	    {
	      auto a = g.insert_arc(mat(i, j - 1), mat(i, j));
	      arc_init(a);
	    }
	  
	  if (i > 0)
	    {
	      auto a = g.insert_arc(mat(i - 1, j), mat(i, j));
	      arc_init(a);
	    }
	  
	  if (with_diagonal)
	    {
	      if (i > 0 and j > 0)
		{
		  auto a = g.insert_arc(mat(i - 1, j - 1), mat(i, j));
		  arc_init(a);
		}
	      
	      if (i > 0 and j + 1 < width)
		{
		  auto a = g.insert_arc(mat(i - 1, j + 1), mat(i, j));
		  arc_init(a);
		}
	    }
	}
    
    return g;
  }

  template <class GT, class NodeInit, class ArcInit>
  GT build_grid(nat_t width, nat_t height, NodeInit && node_init,
		ArcInit && arc_init, bool with_diagonal)
  {
    return build_grid<GT, NodeInit, ArcInit>
      (width, height, node_init, arc_init, with_diagonal);
  }

  template <class GT, class NodeInit, class ArcInit>
  GT random_graph(nat_t num_nodes, nat_t num_arcs, rng_seed_t seed,
		  NodeInit & node_init, ArcInit & arc_init)
  {
    GT g;

    FixedArray<Node<GT> *> nodes(num_nodes);

    rng_t rng(seed);

    for (nat_t i = 0; i < num_nodes; ++i)
      {
	Node<GT> * p = g.insert_node();
	node_init(p);
	nodes[i] = p;
      }

    for (nat_t i = 0; i < num_arcs; ++i)
      {
	Node<GT> * s = nodes[random_uniform(rng, nodes.size())];
	Node<GT> * t = nodes[random_uniform(rng, nodes.size())];

	if (s == t)
	  continue;

	if (g.search_arc(s, t) != nullptr)
	  continue;

	Arc<GT> * a = g.insert_arc(s, t);
	arc_init(a);
      }

    return g;
  }

  template <class GT, class NodeInit, class ArcInit>
  GT random_graph(nat_t num_nodes, nat_t num_arcs, rng_seed_t seed,
		  NodeInit && node_init, ArcInit && arc_init)
  {
    return random_graph<GT, NodeInit, ArcInit>
      (num_nodes, num_arcs, seed, node_init, arc_init);
  }

  template <class GT, class NodeInit, class ArcInit>
  GT random_graph(nat_t num_nodes, nat_t num_arcs,
		  NodeInit && node_init, ArcInit && arc_init)
  {
    return random_graph<GT, NodeInit, ArcInit>
      (num_nodes, num_arcs, get_random_seed(), node_init, arc_init);
  }

  template <class GT, class NodeInit, class ArcInit>
  GT er_random_graph(nat_t num_nodes, real_t prob_arc, rng_seed_t seed,
		     bool grant_connectivity, NodeInit & node_init,
		     ArcInit & arc_init)
  {
    GT g;

    rng_t rng(seed);

    for (nat_t i = 0; i < num_nodes; ++i)
      {
	Node<GT> * p = g.insert_node();
	node_init(p);
      }

    g.for_each_node_pair([&g, &arc_init, prob_arc, &rng] (Node<GT> * s,
							  Node<GT> * t)
			 {
			   if (flip(rng, prob_arc))
			     {
			       Arc<GT> * fa = g.insert_arc(s, t);
			       arc_init(fa);
			     }
			   
			   if (not g.is_digraph())
			     return;

			   if (flip(rng, prob_arc))
			     {
			       Arc<GT> * ba = g.insert_arc(t, s);
			       arc_init(ba);
			     }
			 });
    
    if (not grant_connectivity)
      return g;

    auto l = connected_components_node_list(g);

    if (l.size() == 1)
      return g;
    
    auto it1 = l.begin();
    auto it2 = l.begin();
    it2.next();
    
    for ( ; it2.has_current(); it1.next(), it2.next())
      {
	auto & l1 = *it1;
	auto & l2 = *it2;
	
	Arc<GT> * a = g.insert_arc(l1.get_first(), l2.get_last());
	arc_init(a);
      }
    
    return g;
  }

  template <class GT, class NodeInit, class ArcInit>
  GT er_random_graph(nat_t num_nodes, real_t prob_arc, rng_seed_t seed,
		     bool grant_connectivity, NodeInit && node_init,
		     ArcInit && arc_init)
  {
    return er_random_graph<GT, NodeInit, ArcInit>
      (num_nodes, prob_arc, seed, grant_connectivity, node_init, arc_init);
  }

  template <class GT, class NodeInit, class ArcInit>
  GT er_random_graph(nat_t num_nodes, real_t prob_arc, bool grant_connectivity,
		     NodeInit & node_init, ArcInit & arc_init)
  {
    return er_random_graph<GT, NodeInit, ArcInit>
      (num_nodes, prob_arc, get_random_seed(), grant_connectivity,
       node_init, arc_init);
  }
  
  template <class GT, class NodeInit, class ArcInit>
  GT er_random_graph(nat_t num_nodes, real_t prob_arc, bool grant_connectivity,
		     NodeInit && node_init, ArcInit && arc_init)
  {
    return er_random_graph<GT, NodeInit, ArcInit>
      (num_nodes, prob_arc, grant_connectivity, node_init, arc_init);
  }
} // end namespace Designar

# endif // DSGBUILDGRAPH_H
