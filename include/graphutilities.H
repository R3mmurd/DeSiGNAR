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

# ifndef GRAPHUTILITIES_H
# define GRAPHUTILITIES_H

# include <types.H>
# include <array.H>
# include <list.H>
# include <queue.H>
# include <map.H>
# include <heap.H>

namespace Designar
{
  enum class GraphTag : nat_t
  {
    DEPTH_FIRST       = 1,
    BREADTH_FIRST     = 2,
    KRUSKAL           = 4,
    PRIM              = 8,
    DIJKSTRA          = 16,
    ASTAR             = 32,
    SPANNING_TREE     = 64,
    MIN_SPANNING_TREE = 128,
    COMPONENT         = 256,
    CUT               = 512,
    PATH              = 1024,
    MIN_PATH          = 2048,
    MIN_PATH_TREE     = 4096
  };

  class CommonNodeArc
  {
    nat_t  tag;
    lint_t  _counter;
    void * _cookie;

  public:
    CommonNodeArc();

    void visit(GraphTag);
    
    void unvisit(GraphTag);
    
    bool is_visited(GraphTag) const;

    void *& cookie();

    void reset_tag();
    
    lint_t & counter();
      
    void reset();
  };

  template <class GT> using Node    = typename GT::Node;
  template <class GT> using Arc     = typename GT::Arc;
  template <class GT> using NodeIt  = typename GT::NodeIterator;
  template <class GT> using ArcIt   = typename GT::ArcIterator;
  template <class GT> using AdArcIt = typename GT::AdjacentArcIterator;

  inline void map_graph_item(CommonNodeArc & x, CommonNodeArc & y)
  {
    if (x.cookie() == nullptr)
      {
	x.cookie() = &y;
	y.cookie() = &x;
      }
    else
      {
	y.cookie() = x.cookie();
	x.cookie() = &y;
      }
  }

  template <class SG, class TG = SG>
  void map_nodes(Node<SG> & p, Node<TG> & q)
  {
    map_graph_item(p, q);
  }

  template <class SG, class TG = SG>
  void map_arcs(Arc<SG> & a, Arc<TG> & b)
  {
    map_graph_item(a, b);
  }

  template <class SG, class TG = SG>
  typename TG::Node * mapped_node_ptr(Node<SG> & p)
  {
    return reinterpret_cast<Node<TG> *>(p.cookie());
  }

  template <class SG, class TG = SG>
  typename TG::Arc * mapped_arc_ptr(Arc<SG> & a)
  {
    return reinterpret_cast<Arc<TG> *>(a.cookie());
  }

  template <class GT>
  class Path
  {
  public:
    using NodeType = Node<GT>;
    using ArcType  = Arc<GT>;

  private:
    struct PathDesc
    {
      NodeType * node;
      ArcType  * arc;

      PathDesc(NodeType * _node = nullptr, ArcType * _arc = nullptr)
        : node(_node), arc(_arc)
      {
        // Empty
      }
    };

    using ListType = DLList<PathDesc>;

    GT     * ptr_owner_graph;
    ListType list;

    void test_for_graph()
    {
      if (ptr_owner_graph == nullptr)
	throw std::logic_error("Graph has not been specified");
    }

  public:
    Path()
      : ptr_owner_graph(nullptr), list()
    {
      // Empty
    }

    Path(GT & graph)
      : ptr_owner_graph(&graph), list()
    {
      // Empty
    }

    Path(const Path & path)
      : ptr_owner_graph(path.ptr_owner_graph), list(path.list)
    {
      // Empty
    }

    Path(Path && path)
      : ptr_owner_graph(nullptr), list()
    {
      std::swap(ptr_owner_graph, path.ptr_owner_graph);
      std::swap(list, path.list);
    }

    GT & get_graph()
    {
      test_for_graph();
      return *ptr_owner_graph;
    }

    void init(NodeType & start)
    {
      test_for_graph();
      list.clear();
      list.append(PathDesc(&start));
    }

    void set(GT & graph, NodeType * ptr_start = nullptr)
    {
      clear();

      ptr_owner_graph = &graph;

      if (ptr_start != nullptr)
	init(*ptr_start);
    }

    void clear()
    {
      list.clear();
    }

    void insert(ArcType & arc)
    {
      test_for_graph();
      
      if (list.is_empty())
        throw std::domain_error("Path is empty");

      PathDesc & first = list.get_first();

      NodeType & prev_node = arc.get_connected_node(*first.node);

      list.insert(PathDesc(&prev_node, &arc));
    }

    void insert(NodeType & s)
    {
      test_for_graph();
      
      if (list.is_empty())
	{
	  init(s);
	  return;
	}

      PathDesc & first = list.get_first();

      ArcType * ptr_arc = ptr_owner_graph->search_arc(s, *first.node);

      if (ptr_arc == nullptr)
	throw std::logic_error("There is not arc between last and current node");

      list.insert(PathDesc(&s, ptr_arc));
    }

    void append(ArcType & arc)
    {
      test_for_graph();
      
      if (list.is_empty())
        throw std::domain_error("Path is empty");

      PathDesc & last = list.get_last();

      last.arc = &arc;

      NodeType & next_node = arc.get_connected_node(*last.node);

      list.append(PathDesc(&next_node));
    }

    void append(NodeType & t)
    {
      test_for_graph();
      
      if (list.is_empty())
	{
	  init(t);
	  return;
	}

      PathDesc & last = list.get_last();

      ArcType * ptr_arc = ptr_owner_graph->search_arc(*last.node, t);

      if (ptr_arc == nullptr)
        throw std::logic_error("There is not arc between last and current node");

      last.arc = ptr_arc;

      list.append(PathDesc(&t));
    }

    void remove_last_node()
    {
      test_for_graph();

      if (list.is_empty())
	throw std::underflow_error("Path is empty");
      
      list.remove_last();

      if (not list.is_empty())
	list.get_last().arc = nullptr;
    }

    NodeType & get_last_node()
    {
      test_for_graph();
      
      if (list.is_empty())
        throw std::overflow_error("Path is empty");

      return *list.get_last().node;
    }

    const NodeType & get_last_node() const
    {
      test_for_graph();
      
      if (list.is_empty())
        throw std::overflow_error("Path is empty");

      return *list.get_last().node;
    }

    NodeType & get_first_node()
    {
      if (list.is_empty())
        throw std::underflow_error("Path is empty");

      return *list.get_first().node;
    }
    
    const NodeType & get_first_node() const
    {
      if (list.is_empty())
        throw std::underflow_error("Path is empty");

      return *list.get_first().node;
    }

    ArcType & get_last_arc()
    {
      if (list.is_empty())
        throw std::overflow_error("Path is empty");

      if (list.size() == 1)
        throw std::overflow_error("Path has only one node (without arcs)");


      typename ListType::Iterator it(list);
      it.reset_last();
      it.prev();
      return *it.get_current().arc;
    }

    const ArcType & get_last_arc() const
    {
      if (list.is_empty())
        throw std::overflow_error("Path is empty");

      if (list.size() == 1)
        throw std::overflow_error("Path has only one node (without arcs)");


      typename ListType::Iterator it(list);
      it.reset_last();
      it.prev();
      return *it.get_current().arc;
    }

    ArcType & get_first_arc()
    {
      if (list.is_empty())
        throw std::underflow_error("Path is empty");

      if (list.is_unitarian())
        throw std::underflow_error("Path has only one node (without arcs)");

      return *list.get_first().arc;
    }

    template <class Op>
    void for_each(Op & op) const
    {
      for (const PathDesc & path_desc : list)
        op(path_desc.node, path_desc.arc);
    }

    template <class Op>
    void for_each(Op && op = Op()) const
    {
      for_each<Op>(op);
    }

    template <class Op>
    void for_each(Op & op)
    {
      for (const PathDesc & path_desc : list)
        op(path_desc.node, path_desc.arc);
    }

    template <class Op>
    void for_each(Op && op = Op())
    {
      for_each<Op>(op);
    }

    Path & operator = (const Path & path)
    {
      if (&path == this)
        return *this;

      ptr_owner_graph = path.ptr_owner_graph;
      list = path.list;

      return *this;
    }

    Path & operator = (Path && path)
    {
      std::swap(ptr_owner_graph, path.ptr_owner_graph);
      std::swap(list, path.list);

      return *this;
    }

    bool is_empty() const
    {
      return list.is_empty();
    }
  };

  // For compute cut nodes and Tarjan algorithm
  template <class GT>
  inline lint_t & df(Node<GT> & p)
  {
    return p.counter();
  }

  template <class GT>
  inline lint_t & low(Node<GT> & p)
  {
    return (lint_t &) p.cookie();
  }

  
  // For Dijkstra and Astar algorithms
  template <class GT, class Distance>
  class MinPathNodeInfo
  {
  public:
    Node<GT>     * tree_node;
    typename Distance::Type accumulated_distance;
    
    MinPathNodeInfo()
      : tree_node(nullptr), accumulated_distance(Distance::ZERO)
    {
      // empty
    }
  };

  template <class GT, class Distance>
  class MinPathArcInfo
  {
  public:
    Arc<GT>      * tree_arc;
    typename Distance::Type potential;
    bool                    is_in_queue;
    
    MinPathArcInfo()
      : tree_arc(nullptr), potential(Distance::ZERO), is_in_queue(false)
    {
      // empty
    }
  };

  template <class GT, class Distance>
  inline MinPathNodeInfo<GT, Distance> *& NI(Node<GT> & p)
  {
    return (MinPathNodeInfo<GT, Distance> *&) p.cookie();
  }

  template <class GT, class Distance>
  inline Node<GT> *& TREE_NODE(Node<GT> & p)
  {
    return NI<GT, Distance>(p)->tree_node;
  }

  template <class GT, class Distance>
  inline typename Distance::Type & ACC(Node<GT> & p)
  {
    return NI<GT, Distance>(p)->accumulated_distance;
  }

  template <class GT, class Distance>
  inline MinPathArcInfo<GT, Distance> *& AI(Arc<GT> & a)
  {
    return (MinPathArcInfo<GT, Distance> *&) a.cookie();
  }

  template <class GT, class Distance>
  inline Arc<GT> *& TREE_ARC(Arc<GT> & a)
  {
    return AI<GT, Distance>(a)->tree_arc;
  }

  template <class GT, class Distance>
  inline typename Distance::Type & POT(Arc<GT> & a)
  {
    return AI<GT, Distance>(a)->potential;
  }

  template <class GT, class Distance>
  inline bool & IS_IN_QUEUE(Arc<GT> & a)
  {
    return AI<GT, Distance>(a)->is_in_queue;
  }
  
  template <class GT, class Distance>
  inline void allocate_node_info(GT & g)
  {
    g.for_each_node([&](auto & node)
		    {
		      NI<GT, Distance>(node) =
			new MinPathNodeInfo<GT, Distance>;
		    });
  }
  
  template <class GT, class Distance>
  inline void allocate_arc_info(GT & g)
  {
    g.for_each_arc([&](auto & arc)
		   {
		     AI<GT, Distance>(arc) =
		       new MinPathArcInfo<GT, Distance>;
		   });
  }
  
  template <class GT, class Distance>
  void destroy_node_info(GT & g)
  {
    g.for_each_node([&](auto & node)
		    {
		      auto to_destroy = NI<GT, Distance>(node);
		      auto ptr_tree_node =
			TREE_NODE<GT, Distance>(node);
			      
		      if (ptr_tree_node == nullptr)
			node.cookie() = nullptr;
		      else
			{
			  node.cookie()           = ptr_tree_node;
			  ptr_tree_node->cookie() = &node;
			}
			      
		      delete to_destroy;
		    });
  }

  template <class GT, class Distance>
  void destroy_arc_info(GT & g)
  {
    g.for_each_arc([&](auto & arc)
		   {
		     auto to_destroy = AI<GT, Distance>(arc);
		     auto ptr_tree_arc =
		       TREE_ARC<GT, Distance>(arc);
			     
		     if (ptr_tree_arc == nullptr)
		       arc.cookie() = nullptr;
		     else
		       {
			 arc.cookie()           = ptr_tree_arc;
			 ptr_tree_arc->cookie() = &arc;
		       }
			     
		     delete to_destroy;
		   });
  }

  template <class GT, class Distance, class Heap>
  void put_in_heap(Arc<GT> & a, Node<GT> & t, Heap & h)
  {
    if (IS_IN_QUEUE<GT, Distance>(a))
      return;

    IS_IN_QUEUE<GT, Distance>(a) = true;
    h.insert_arc(a, t);
  }

  template <class GT, class Distance, class Heap>
  Arc<GT> & get_from_heap(Heap & h)
  {
    Arc<GT> & ret_val         = *h.get_min_arc();
    IS_IN_QUEUE<GT, Distance>(ret_val) = false;
    return ret_val;
  }
  
  template <class GT, class Distance>
  class GetPot
  {
  public:
    typename Distance::Type operator () (Arc<GT> & a)
    {
      return POT<GT, Distance>(a);
    }
  };

  template <class GT>
  class DftGridNodeInit
  {
  public:
    void operator () (Node<GT> &, nat_t, nat_t)
    {
      // empty
    }
  };

  template <class GT>
  class DftGridArcInit
  {
  public:
    void operator () (Arc<GT> &, nat_t, nat_t)
    {
      // empty
    }
  };

  template <class GT>
  class DftNodeInit
  {
  public:
    void operator () (Node<GT> &)
    {
      // empty
    }
  };

  template <class GT>
  class DftArcInit
  {
  public:
    void operator () (Arc<GT> &)
    {
      // empty
    }
  };


  template <class GT>
  class DftDotNodeAttr
  {
  public:
    std::string operator () (const Node<GT> & p) 
    {
      std::stringstream s;
      s << "label = \"" << p.get_info() << "\"";
      return s.str();
    }
  };

  template <class GT>
  struct DftDotArcAttr
  {
    std::string operator () (const Arc<GT> & a)
    {
      std::stringstream s;
      s << "label = \"" << a.get_info() << "\"";
      return s.str();
    }
  };

  template <class GT>
  class DftDotGraphAttr
  {
  public:
    std::string operator () (const GT &)
    {
      return "  // Without graph attributes";
    }
  };
  
} // end namespace Designar

# endif // GRAPHUTILITIES_H
