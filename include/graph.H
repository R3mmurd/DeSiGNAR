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

# ifndef DSGGRAPH_H
# define DSGGRAPH_H

# include <graphutilities.H>

namespace Designar
{
  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class Graph;

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class Digraph;
  
  template <typename NodeInfo>
  class BaseGraphNode : public CommonNodeArc
  {
  protected:
    NodeInfo info;
    nat_t    num_arcs;
    DL       adjacent_arc_list;
    
    BaseGraphNode()
      : CommonNodeArc(), info(), num_arcs(0), adjacent_arc_list()
    {
      // empty
    }
    
    BaseGraphNode(const NodeInfo & _info)
      : CommonNodeArc(), info(_info), num_arcs(0), adjacent_arc_list()
    {
      // empty
    }
    
    BaseGraphNode(NodeInfo && _info)
      : CommonNodeArc(), info(std::forward<NodeInfo>(_info)), num_arcs(0),
	adjacent_arc_list()
    {
      // empty
    }
    
    BaseGraphNode(BaseGraphNode * ptr)
      : CommonNodeArc(), info(ptr->info), num_arcs(0), adjacent_arc_list()
    {
      // empty
    }
    
  public:
    NodeInfo & get_info()
    {
      return info;
    }
    
    const NodeInfo & get_info() const
    {
      return info;
    }
    
    nat_t get_num_arcs() const
    {
      return num_arcs;
    }
  };

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class GraphNode : public BaseGraphNode<NodeInfo>
  {
    friend class Graph<NodeInfo, ArcInfo, GraphInfo>;
    friend class DLNode<GraphNode>;
    using Base = BaseGraphNode<NodeInfo>;
    using Base::Base;
  };

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class DigraphNode : public BaseGraphNode<NodeInfo>
  {
    friend class Digraph<NodeInfo, ArcInfo, GraphInfo>;
    friend class DLNode<DigraphNode>;
    using Base = BaseGraphNode<NodeInfo>;
    using Base::Base;
  };
  
  template <class Node, typename ArcInfo>
  class BaseGraphArc : public CommonNodeArc
  {    
  protected:
    Node    * src_node;
    Node    * tgt_node;
    ArcInfo   info;

    BaseGraphArc()
      : src_node(nullptr), tgt_node(nullptr), info()
    {
      // empty
    }
    
    BaseGraphArc(Node * src, Node * tgt)
      : src_node(src), tgt_node(tgt), info()
    {
      // empty
    }
    
    BaseGraphArc(Node * src, Node * tgt, const ArcInfo & _info)
      : src_node(src), tgt_node(tgt), info(_info)
    {
      // empty
    }
    
    BaseGraphArc(Node * src, Node * tgt, ArcInfo && _info)
      : src_node(src), tgt_node(tgt), info(std::forward<ArcInfo>(_info))
    {
      // empty
    }
    
  public:
    Node & get_src_node()
    {
      return *src_node;
    }
    
    const Node & get_src_node() const
    {
      return *src_node;
    }
    
    Node & get_tgt_node()
    {
      return *tgt_node;
    }
    
    const Node & get_tgt_node() const
    {
      return *tgt_node;
    }
    
    Node & get_connected_node(Node & node)
    {
      if (&node == &get_src_node())
	return get_tgt_node();
      
      if (&node == &get_tgt_node())
	return get_src_node();
      
      throw std::logic_error("Arc is not adjacent to node");
    }
    
    const Node & get_connected_node(Node & node) const
    {
      if (&node == &get_src_node())
	return get_tgt_node();
      
      if (&node == &get_tgt_node())
	return get_src_node();
      
      throw std::logic_error("Arc is not adjacent to node");
    }

    ArcInfo & get_info()
    {
      return info;
    }
    
    const ArcInfo & get_info() const
    {
      return info;
    }
  };

  template <class Node, typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class GraphArc : public BaseGraphArc<Node, ArcInfo>
  {
    friend class Graph<NodeInfo, ArcInfo, GraphInfo>;
    friend class DLNode<GraphArc>;
    using Base = BaseGraphArc<Node, ArcInfo>;
    using Base::Base;
    
  protected:
    DLNode<DLNode<GraphArc> *> * arc_in_src_node = nullptr;
    DLNode<DLNode<GraphArc> *> * arc_in_tgt_node = nullptr;
  };
  
  template <class Node, typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class DigraphArc : public BaseGraphArc<Node, ArcInfo>
  {
    friend class Digraph<NodeInfo, ArcInfo, GraphInfo>;
    friend class DLNode<DigraphArc>;
    using Base = BaseGraphArc<Node, ArcInfo>;
    using Base::Base;

  protected:
    DLNode<DLNode<DigraphArc> *> * arc_in_arc_list = nullptr;
  };

  template <class GT, class Node, class Arc>
  class BaseGraph
  {
    GT & me()
    {
      return *static_cast<GT *>(this);
    }

    const GT & const_me() const
    {
      return *static_cast<const GT *>(this);
    }
    
  public:
    static void copy_graph(const GT &, GT &);
    
    Node & nth_node(nat_t);

    const Node & nth_node(nat_t) const;

    template <class Op>
    void for_each_node(Op &) const;

    template <class Op>
    void for_each_node(Op && op = Op()) const;
    
    template <class Op>
    void mutable_for_each_node(Op &);

    template <class Op>
    void mutable_for_each_node(Op && op = Op());

    template <class Pred>
    bool all_nodes(Pred &) const;

    template <class Pred>
    bool all_nodes(Pred && pred = Pred()) const;

    template <class Pred>
    bool exists_node(Pred &) const;

    template <class Pred>
    bool exists_node(Pred && pred = Pred()) const;

    template <class Pred>
    Node * search_node_ptr(Pred &);

    template <class Pred>
    Node * search_node_ptr(Pred && pred = Pred());

    template <class Pred>
    const Node * search_node_ptr(Pred &) const;

    template <class Pred>
    const Node * search_node_ptr(Pred && pred = Pred()) const;

    template <class Pred>
    bool remove_first_node_if(Pred &);

    template <class Pred>
    bool remove_first_node_if(Pred && pred = Pred());

    template <class Pred>
    void remove_node_if(Pred &);

    template <class Pred>
    void remove_node_if(Pred && pred = Pred());
     
    SLList<Node *> nodes() const;

    Arc & nth_arc(nat_t);

    const Arc & nth_arc(nat_t) const;

    template <class Op>
    void for_each_arc(Op &) const;

    template <class Op>
    void for_each_arc(Op && op = Op()) const;

    template <class Op>
    void mutable_for_each_arc(Op &);

    template <class Op>
    void mutable_for_each_arc(Op && op = Op());
    
    template <class Pred>
    bool all_arcs(Pred &) const;

    template <class Pred>
    bool all_arcs(Pred && pred = Pred()) const;

    template <class Pred>
    bool exists_arc(Pred &) const;

    template <class Pred>
    bool exists_arc(Pred && pred = Pred()) const;

    template <class Pred>
    Arc * search_arc_ptr(Pred &);

    template <class Pred>
    Arc * search_arc_ptr(Pred && pred = Pred());

    template <class Pred>
    const Arc * search_arc_ptr(Pred &) const;

    template <class Pred>
    const Arc * search_arc_ptr(Pred && pred = Pred()) const;

    template <class Pred>
    bool remove_first_arc_if(Pred &);

    template <class Pred>
    bool remove_first_arc_if(Pred && pred = Pred());

    template <class Pred>
    void remove_arc_if(Pred &);

    template <class Pred>
    void remove_arc_if(Pred && pred = Pred());
     
    SLList<Arc *> arcs() const;

    Arc & nth_adjacent_arc(Node &, nat_t);

    const Arc & nth_adjacent_arc(Node &, nat_t) const;

    template <class Op>
    void for_each_adjacent_arc(Node &, Op &) const;

    template <class Op>
    void for_each_adjacent_arc(Node &, Op && op = Op()) const;

    template <class Op>
    void mutable_for_each_adjacent_arc(Node &, Op &);

    template <class Op>
    void mutable_for_each_adjacent_arc(Node &, Op && op = Op());
    
    template <class Pred>
    bool all_adjacent_arcs(Node &, Pred &) const;

    template <class Pred>
    bool all_adjacent_arcs(Node &, Pred && pred = Pred()) const;

    template <class Pred>
    bool exists_adjacent_arc(Node &, Pred &) const;

    template <class Pred>
    bool exists_adjacent_arc(Node &, Pred && pred = Pred()) const;

    template <class Pred>
    Arc * search_adjacent_arc_ptr(Node &, Pred &);

    template <class Pred>
    Arc * search_adjacent_arc_ptr(Node &, Pred && pred = Pred());

    template <class Pred>
    const Arc * search_adjacent_arc_ptr(Node &, Pred &) const;

    template <class Pred>
    const Arc * search_adjacent_arc_ptr(Node &,
					    Pred && pred = Pred()) const;

    template <class Pred>
    bool remove_first_adjacent_arc_if(Node &, Pred &);

    template <class Pred>
    bool remove_first_adjacent_arc_if(Node &, Pred && pred = Pred());

    template <class Pred>
    void remove_adjacent_arc_if(Node &, Pred &);

    template <class Pred>
    void remove_adjacent_arc_if(Node &, Pred && pred = Pred());
     
    SLList<Arc *> adjacent_arcs(Node &) const;
    
    void reset_all_node_tag(GraphTag tag)
    {
      mutable_for_each_node([&tag](Node & node)
			    {
			      node.unvisit(tag);
			    });
    }

    void reset_all_node_tag()
    {
      mutable_for_each_node([](Node & node)
			    {
			      node.reset_tag();
			    });
    }

    void reset_all_arc_tag(GraphTag tag)
    {
      mutable_for_each_arc([&tag](Arc & arc)
			    {
			      arc.unvisit(tag);
			    });
    }

    void reset_all_arc_tag()
    {
      mutable_for_each_arc([](Arc & arc)
			    {
			      arc.reset_tag();
			    });
    }

    void reset_tag(GraphTag tag)
    {
      reset_all_node_tag(tag);
      reset_all_arc_tag(tag);
    }

    void reset_all_tags()
    {
      reset_all_node_tag();
      reset_all_arc_tag();
    }

    void reset_node_cookies()
    {
      mutable_for_each_node([](Node & node)
			    {
			      node.cookie() = nullptr;
			    });
    }

    void reset_arc_cookies()
    {
      mutable_for_each_arc([](Arc & arc)
			   {
			     arc.cookie() = nullptr;
			   });
    }

   void reset_node_counter()
    {
      mutable_for_each_node([](Node & node)
			    {
			      node.counter() = 0;
			    });
    }

    void reset_arc_counter()
    {
      mutable_for_each_arc([](Arc & arc)
			   {
			     arc.counter() = 0;
			   });
    }

    void reset_counters()
    {
      reset_node_counter();
      reset_arc_counter();
    }

    void reset_cookies()
    {
      reset_node_cookies();
      reset_arc_cookies();
    }

    void reset_nodes()
    {
      mutable_for_each_node([](Node & node)
			    {
			      node.reset();
			    });
    }

    void reset_arcs()
    {
      mutable_for_each_arc([](Arc & arc)
			   {
			     arc.reset();
			   });
    }
  };

  template <typename GT, class Node, class Arc>
  void BaseGraph<GT, Node, Arc>::copy_graph(const GT & src, GT & tgt)
  {
    HashMap<Node *, Node *> map_nodes;

    for (auto it = src.nodes_begin(); it != src.nodes_end(); ++it)
      {
        Node * p = &it.get_current();
        Node * q = &tgt.insert_node(p->get_info());
        map_nodes[p] = q;
      }

    for (auto it = src.arcs_begin(); it != src.arcs_end(); ++it)
      {
        Arc  & a    = it.get_current();
        Node & ssrc = a.get_src_node();
        Node & stgt = a.get_tgt_node();
        Node & tsrc = *map_nodes[&ssrc];
        Node & ttgt = *map_nodes[&stgt];
        tgt.insert_arc(tsrc, ttgt, a.get_info());
      }
  }

  template <class GT, class Node, class Arc>
  Node & BaseGraph<GT, Node, Arc>::nth_node(nat_t i)
  {
    for (auto it = me().nodes_begin(); it != me().nodes_end(); ++it)
      {
	if (i == 0)
	  return *it;
	--i;
      }
    
    throw std::overflow_error("index too large");
  }
  
  template <class GT, class Node, class Arc>
  const Node & BaseGraph<GT, Node, Arc>::nth_node(nat_t i) const
  {
    for (auto it = const_me().nodes_begin(); it != const_me().nodes_end(); ++it)
      {
	if (i == 0)
	  return *it;
	--i;
      }
    
    throw std::overflow_error("index too large");
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::for_each_node(Op & op) const
  {
    for (auto it = const_me().nodes_begin(); it != const_me().nodes_end(); ++it)
      op(*it);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::for_each_node(Op && op) const
  {
    for_each_node<Op>(op);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::mutable_for_each_node(Op & op)
  {
    for (auto it = me().nodes_begin(); it != me().nodes_end(); ++it)
      op(*it);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::mutable_for_each_node(Op && op)
  {
    mutable_for_each_node<Op>(op);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::all_nodes(Pred & pred) const
  {
    for (auto it = const_me().nodes_begin(); it != const_me().nodes_end(); ++it)
      if (not pred(*it))
	return false;

    return true;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::all_nodes(Pred && pred) const
  {
    return all_nodes<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::exists_node(Pred & pred) const
  {
    for (auto it = const_me().nodes_begin(); it != const_me().nodes_end(); ++it)
      if (pred(*it))
	return true;

    return false;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::exists_node(Pred && pred) const
  {
    return exists_node<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  Node * BaseGraph<GT, Node, Arc>::search_node_ptr(Pred & pred)
  {
    for (auto it = me().nodes_begin(); it != me().nodes_end(); ++it)
      if (pred(*it))
	return &*it;

    return nullptr;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  Node * BaseGraph<GT, Node, Arc>::search_node_ptr(Pred && pred)
  {
    return search_node_ptr<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  const Node * BaseGraph<GT, Node, Arc>::search_node_ptr(Pred & pred) const
  {
    for (auto it = const_me().nodes_begin(); it != const_me().nodes_end(); ++it)
      if (pred(*it))
	return &*it;

    return nullptr;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  const Node * BaseGraph<GT, Node, Arc>::search_node_ptr(Pred && pred) const
  {
    return search_node_ptr<Pred>(pred);
  }
  
  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::remove_first_node_if(Pred & pred)
  {
    for (auto it = me().nodes_begin(); it != me().nodes_end(); ++it)
      if (pred(*it))
	{
	  it.del();
	  return true;
	}

    return false;
  }
  
  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::remove_first_node_if(Pred && pred)
  {
    return remove_first_node_if<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  void BaseGraph<GT, Node, Arc>::remove_node_if(Pred & pred)
  {
    for (auto it = me().nodes_begin(); it != me().nodes_end(); ++it)
      if (pred(*it))
	it.del();
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  void BaseGraph<GT, Node, Arc>::remove_node_if(Pred && pred)
  {
    remove_node_if<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  SLList<Node *> BaseGraph<GT, Node, Arc>::nodes() const
  {
    SLList<Node *> ret_val;

    for (auto it = me().nodes_begin(); it != me().nodes_end(); ++it)
      ret_val.append(&*it);

    return ret_val;
  }

  template <class GT, class Node, class Arc>
  Arc & BaseGraph<GT, Node, Arc>::nth_arc(nat_t i)
  {
    for (auto it = me().arcs_begin(); it != me().arcs_end(); ++it)
      {
	if (i == 0)
	  return *it;
	--i;
      }
    
    throw std::overflow_error("index too large");
  }
  
  template <class GT, class Node, class Arc>
  const Arc & BaseGraph<GT, Node, Arc>::nth_arc(nat_t i) const
  {
    for (auto it = const_me().arcs_begin(); it != const_me().arcs_end(); ++it)
      {
	if (i == 0)
	  return *it;
	--i;
      }
    
    throw std::overflow_error("index too large");
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::for_each_arc(Op & op) const
  {
    for (auto it = const_me().arcs_begin(); it != const_me().arcs_end(); ++it)
      op(*it);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::for_each_arc(Op && op) const
  {
    for_each_arc<Op>(op);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::mutable_for_each_arc(Op & op)
  {
    for (auto it = me().arcs_begin(); it != me().arcs_end(); ++it)
      op(*it);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::mutable_for_each_arc(Op && op)
  {
    mutable_for_each_arc<Op>(op);
  }
  
  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::all_arcs(Pred & pred) const
  {
    for (auto it = const_me().arcs_begin(); it != const_me().arcs_end(); ++it)
      if (not pred(*it))
	return false;

    return true;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::all_arcs(Pred && pred) const
  {
    return all_arcs<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::exists_arc(Pred & pred) const
  {
    for (auto it = const_me().arcs_begin(); it != const_me().arcs_end(); ++it)
      if (pred(*it))
	return true;

    return false;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::exists_arc(Pred && pred) const
  {
    return exists_arc<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  Arc * BaseGraph<GT, Node, Arc>::search_arc_ptr(Pred & pred)
  {
    for (auto it = me().arcs_begin(); it != me().arcs_end(); ++it)
      if (pred(*it))
	return &*it;

    return nullptr;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  Arc * BaseGraph<GT, Node, Arc>::search_arc_ptr(Pred && pred)
  {
    return search_arc_ptr<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  const Arc * BaseGraph<GT, Node, Arc>::search_arc_ptr(Pred & pred) const
  {
    for (ArcIt<GT> it(*this); it.has_current(); it.next())
      if (pred(*it))
	return &*it;

    return nullptr;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  const Arc * BaseGraph<GT, Node, Arc>::search_arc_ptr(Pred && pred) const
  {
    return search_arc_ptr<Pred>(pred);
  }
  
  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::remove_first_arc_if(Pred & pred)
  {
    for (ArcIt<GT> it(*this); it.has_current(); it.next())
      if (pred(*it))
	{
	  it.del();
	  return true;
	}

    return false;
  }
  
  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::remove_first_arc_if(Pred && pred)
  {
    return remove_first_arc_if<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  void BaseGraph<GT, Node, Arc>::remove_arc_if(Pred & pred)
  {
    for (ArcIt<GT> it(*this); it.has_current(); it.next())
      if (pred(*it))
	it.del();
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  void BaseGraph<GT, Node, Arc>::remove_arc_if(Pred && pred)
  {
    remove_arc_if<Pred>(pred);
  }

  template <class GT, class Node, class Arc>
  SLList<Arc *> BaseGraph<GT, Node, Arc>::arcs() const
  {
    SLList<Arc *> ret_val;
    
    for (ArcIt<GT> it(*this); it.has_current(); it.next())
      ret_val.append(&*it);

    return ret_val;
  }

  template <class GT, class Node, class Arc>
  Arc & BaseGraph<GT, Node, Arc>::nth_adjacent_arc(Node & p, nat_t i)
  {
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); ++it)
      {
	if (i == 0)
	  return *it;
	--i;
      }
    
    throw std::overflow_error("index too large");
  }
  
  template <class GT, class Node, class Arc>
  const Arc & BaseGraph<GT, Node, Arc>::nth_adjacent_arc(Node & p, nat_t i) const
  {
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); ++it)
      {
	if (i == 0)
	  return *it;
	--i;
      }
    
    throw std::overflow_error("index too large");
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::for_each_adjacent_arc(Node & p, Op & op) const
  {
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); ++it)
      op(*it);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::for_each_adjacent_arc(Node & p, Op && op) const
  {
    for_each_adjacent_arc<Op>(p, op);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void BaseGraph<GT, Node, Arc>::mutable_for_each_adjacent_arc(Node & p, Op & op)
  {
    for (auto it = me().arcs_begin(p); it != me().arcs_end(p); ++it)
      op(*it);
  }

  template <class GT, class Node, class Arc>
  template <class Op>
  void
  BaseGraph<GT, Node, Arc>::mutable_for_each_adjacent_arc(Node & p, Op && op)
  {
    mutable_for_each_adjacent_arc<Op>(p, op);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::all_adjacent_arcs(Node & p, Pred & pred) const
  {
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); ++it)
      if (not pred(*it))
	return false;

    return true;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::all_adjacent_arcs(Node & p, Pred && pred) const
  {
    return all_adjacent_arcs<Pred>(p, pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool BaseGraph<GT, Node, Arc>::exists_adjacent_arc(Node & p, Pred & pred) const
  {
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); ++it)
      if (pred(*it))
	return true;

    return false;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  bool
  BaseGraph<GT, Node, Arc>::exists_adjacent_arc(Node & p, Pred && pred) const
  {
    return exists_adjacent_arc<Pred>(p, pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  Arc * BaseGraph<GT, Node, Arc>::search_adjacent_arc_ptr(Node & p, Pred & pred)
  {
    for (auto it = me().arcs_begin(p); it != me().arcs_end(p); ++it)
      if (pred(*it))
	return &*it;

    return nullptr;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  Arc * BaseGraph<GT, Node, Arc>::search_adjacent_arc_ptr(Node & p, Pred && pred)
  {
    return search_adjacent_arc_ptr<Pred>(p, pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  const Arc *
  BaseGraph<GT, Node, Arc>::search_adjacent_arc_ptr(Node & p, Pred & pred) const
  {
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); ++it)
      if (pred(*it))
	return &*it;

    return nullptr;
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  const Arc *
  BaseGraph<GT, Node, Arc>::search_adjacent_arc_ptr(Node & p, Pred && pred) const
  {
    return search_adjacent_arc_ptr<Pred>(p, pred);
  }
  
  template <class GT, class Node, class Arc>
  template <class Pred>
  bool
  BaseGraph<GT, Node, Arc>::remove_first_adjacent_arc_if(Node & p, Pred & pred)
  {
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); )
      {
	Arc & arc = *it;
	it.next();
	if (pred(arc))
	{
	  remove_arc(arc);
	  return true;
	}
      }

    return false;
  }
  
  template <class GT, class Node, class Arc>
  template <class Pred>
  bool
  BaseGraph<GT, Node, Arc>::remove_first_adjacent_arc_if(Node & p, Pred && pred)
  {
    return remove_first_adjacent_arc_if<Pred>(p, pred);
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  void BaseGraph<GT, Node, Arc>::remove_adjacent_arc_if(Node & p, Pred & pred)
  {
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); )
      {
	Arc & arc = *it;
	it.next();
	if (pred(arc))
	  remove_arc(arc);
      }
  }

  template <class GT, class Node, class Arc>
  template <class Pred>
  void BaseGraph<GT, Node, Arc>::
  remove_adjacent_arc_if(Node & p, Pred && pred)
  {
    remove_adjacent_arc_if<Pred>(p, pred);
  }

  template <class GT, class Node, class Arc>
  SLList<Arc *>BaseGraph<GT, Node, Arc>::adjacent_arcs(Node & p) const
  {
    SLList<Arc *> ret_val;
    
    for (auto it = const_me().arcs_begin(p); it != const_me().arcs_end(p); ++it)
      ret_val.append(&*it);
    
    return ret_val;
  }
  
  template <typename NodeInfo,
	    typename ArcInfo   = EmptyClass,
	    typename GraphInfo = EmptyClass>
  class Graph : public BaseGraph<Graph<NodeInfo, ArcInfo, GraphInfo>,
				 GraphNode<NodeInfo, ArcInfo, GraphInfo>,
				 GraphArc<
				   GraphNode<NodeInfo, ArcInfo, GraphInfo>,
				   NodeInfo, ArcInfo, GraphInfo>>
  {
    using BaseGraph = BaseGraph<Graph<NodeInfo, ArcInfo, GraphInfo>,
				GraphNode<NodeInfo, ArcInfo, GraphInfo>,
				GraphArc<
				  GraphNode<NodeInfo, ArcInfo, GraphInfo>,
				  NodeInfo, ArcInfo, GraphInfo>>;
    
  public:
    using NodeInfoType  = NodeInfo;
    using ArcInfoType   = ArcInfo;
    using GraphInfoType = GraphInfo;

    using Node = GraphNode<NodeInfo, ArcInfo, GraphInfo>;
    using Arc  = GraphArc<Node, NodeInfo, ArcInfo, GraphInfo>;

  protected:
    using GNode  = DLNode<Node>;
    using GArc   = DLNode<Arc>;
    using GAdArc = DLNode<GArc *>;

    static GNode * dl_to_node(DL * ptr)
    {
      return static_cast<GNode *>(ptr);
    }

    static GArc * dl_to_arc(DL * ptr)
    {
      return static_cast<GArc *>(ptr);
    }

    static GAdArc * dl_to_adjacent_arc(DL * ptr)
    {
      return static_cast<GAdArc *>(ptr);
    }

    static GNode * to_gnode(Node & node)
    {
      GNode * node_zero    = 0;
      nat_t   off_set      = (nat_t) &node_zero->get_item();
      nat_t   node_address = (nat_t) &node;
      return (GNode *) (node_address - off_set); 
    }

    static GArc * to_garc(Arc & arc)
    {
      GArc * arc_zero    = 0;
      nat_t  off_set     = (nat_t) &arc_zero->get_item();
      nat_t  arc_address = (nat_t) &arc;
      return (GArc *) (arc_address - off_set); 
    }
    
    GraphInfo info;
    nat_t     num_nodes;
    DL        node_list;
    nat_t     num_arcs;
    DL        arc_list;

    GNode * insert_node(GNode * p)
    {
      node_list.insert_prev(p);
      ++num_nodes;
      return p;
    }
    
    GArc * insert_arc(Node * src, Node * tgt)
    {
      GArc * arc = new GArc(Arc(src, tgt));

      GAdArc * arc_in_src_node = new GAdArc(arc);

      arc->get_item().arc_in_src_node = arc_in_src_node;
      src->adjacent_arc_list.insert_prev(arc_in_src_node);
      ++src->num_arcs;

      if (src == tgt)
	arc->get_item().arc_in_tgt_node = arc_in_src_node;
      else
	{
	  GAdArc * arc_in_tgt_node = new GAdArc(arc);
	  arc->get_item().arc_in_tgt_node = arc_in_tgt_node;
	  tgt->adjacent_arc_list.insert_prev(arc_in_tgt_node);
	  ++tgt->num_arcs;
	}

      arc_list.insert_prev(arc);
      ++num_arcs;
      return arc;
    }
    
    void remove_arc(GArc * arc)
    {
      Node * src_node = arc->get_item().src_node;

      GAdArc * arc_in_src_node = arc->get_item().arc_in_src_node;
      arc_in_src_node->del();
      --src_node->num_arcs;
      delete arc_in_src_node;

      Node * tgt_node = arc->get_item().tgt_node;
	  
      if (src_node != tgt_node)
	{
	  GAdArc * arc_in_tgt_node = arc->get_item().arc_in_tgt_node;
	  arc_in_tgt_node->del();
	  --tgt_node->num_arcs;  
	  delete arc_in_tgt_node;
	}

      arc->del();
      --num_arcs;
      delete arc;
    }

    void remove_node(GNode *);

  public:
    Graph()
      : info(), num_nodes(0), node_list(), num_arcs(0), arc_list()
    {
      // empty
    }
    
    Graph(const GraphInfo & _info)
      : info(_info), num_nodes(0), num_arcs(0)
    {
      // empty
    }

    Graph(GraphInfo && _info)
      : info(std::move(_info)), num_nodes(0), num_arcs(0)
    {
      // empty
    }

    Graph(const Graph & g)
      : info(g.info), num_nodes(0), num_arcs(0)
    {
      BaseGraph::copy_graph(g, *this);
    }

    Graph(Graph && g)
      : Graph()
    {
      swap(g);
    }

    ~Graph()
    {
      clear();
    }

    Graph & operator = (const Graph & g)
    {
      if (this == &g)
	return *this;

      clear();
      BaseGraph::copy_graph(g, *this);
      info = g.info;
  
      return *this;
    }

    Graph & operator = (Graph && g)
    {
      swap(g);
      return *this;
    }

    void swap(Graph & g)
    {
      std::swap(info, g.info);
      std::swap(num_nodes, g.num_nodes);
      node_list.swap(g.node_list);
      std::swap(num_arcs, g.num_arcs);
      arc_list.swap(g.arc_list);
    }

    void clear();

    GraphInfo & get_info()
    {
      return info;
    }

    const GraphInfo & get_info() const
    {
      return info;
    }

    Node & get_first_node()
    {
      if (node_list.is_empty())
	throw std::underflow_error("Graph has not nodes");

      return dl_to_node(node_list.get_next())->get_item();
    }

    const Node & get_first_node() const
    {
      if (node_list.is_empty())
	throw std::underflow_error("Graph has not nodes");

      return dl_to_node(node_list.get_next())->get_item();
    }

    Arc & get_first_arc()
    {
      if (arc_list.is_empty())
	throw std::underflow_error("Graph has not arcs");

      return dl_to_arc(arc_list.get_next())->get_item();
    }

    const Arc & get_first_arc() const
    {
      if (arc_list.is_empty())
	throw std::underflow_error("Graph has not arcs");

      return dl_to_arc(arc_list.get_next())->get_item();
    }

    nat_t get_num_nodes() const
    {
      return num_nodes;
    }

    nat_t get_num_arcs() const
    {
      return num_arcs;
    }

    Node & insert_node()
    {
      GNode * node = insert_node(new GNode);
      return node->get_item();
    }

    Node & insert_node(const NodeInfo & info)
    {
      GNode * node = insert_node(new GNode(Node(info)));
      return node->get_item();
    }

    Node & insert_node(NodeInfo && info)
    {
      GNode * node = insert_node(new GNode(Node(std::forward<NodeInfo>(info))));
      return node->get_item();
    }

    Arc & insert_arc(Node & s, Node & t)
    {
      Node * src = &s;
      Node * tgt = &t;
      GArc * arc = insert_arc(src, tgt);
      return arc->get_item();
    }
    
    Arc & insert_arc(Node & src, Node & tgt, const ArcInfo & info)
    {
      Arc & ref_arc = insert_arc(src, tgt);
      ref_arc.get_info() = info;
      return ref_arc;
    }

    Arc & insert_arc(Node & src, Node & tgt, ArcInfo && info)
    {
      Arc & ref_arc = insert_arc(src, tgt);
      ref_arc.get_info() = std::move(info);
      return ref_arc;
    }

    void remove_arc(Arc & a)
    {
      GArc * arc = to_garc(a);
      remove_arc(arc);
    }

    void remove_node(Node & n)
    {
      GNode * node = to_gnode(n);
      remove_node(node);
    }
    
    class NodeIterator : public DL::Iterator
    {
      using Base = DL::Iterator;
      
      Graph * graph_ptr;

    public:
      NodeIterator()
	: Base(), graph_ptr(nullptr)
      {
	// empty
      }

      NodeIterator(const Graph & g)
	: Base(const_cast<DL *>(&g.node_list)),
	  graph_ptr(const_cast<Graph *>(&g))
      {
	// empty
      }

      NodeIterator(const NodeIterator & it)
	: Base(it), graph_ptr(it.graph_ptr)
      {
	// empty
      }

      NodeIterator(NodeIterator && it)
	: NodeIterator()
      {
	swap(it);
      }

      NodeIterator & operator = (const NodeIterator & it)
      {
	if (this == &it)
	  return *this;

	(Base &) *this = it;
	graph_ptr = it.graph_ptr;
	return *this;
      }

      NodeIterator & operator = (NodeIterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(NodeIterator & it)
      {
	Base::swap(it);
	std::swap(graph_ptr, it.graph_ptr);
      }

      Node & get_current()
      {
	return dl_to_node(Base::get_current())->get_item();
      }

      const Node & get_current() const
      {
	return dl_to_node(Base::get_current())->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	GNode * p = dl_to_node(Base::get_current());
	Base::next();
	graph_ptr->remove_node(p);
      }

      Node & operator * ()
      {
	return get_current();
      }

      const Node & operator * () const
      {
	return get_current();
      }
    };
    
    class ArcIterator : public DL::Iterator
    {
      using Base = DL::Iterator;
      
      Graph * graph_ptr;

    public:
      ArcIterator()
	: Base(), graph_ptr(nullptr)
      {
	// empty
      }

      ArcIterator(const Graph & g)
	: Base(const_cast<DL *>(&g.arc_list)),
	  graph_ptr(const_cast<Graph *>(&g))
      {
	// empty
      }

      ArcIterator(const ArcIterator & it)
	: Base(it), graph_ptr(it.graph_ptr)
      {
	// empty
      }

      ArcIterator(ArcIterator && it)
	: ArcIterator()
      {
	swap(it);
      }

      ArcIterator & operator = (const ArcIterator & it)
      {
	if (this == &it)
	  return *this;

	(Base &) *this = it;
	graph_ptr = it.graph_ptr;
	return *this;
      }

      ArcIterator & operator = (ArcIterator && it)
      {
	swap(it);
	return *this;
      }
      
      void swap(ArcIterator & it)
      {
	Base::swap(it);
	std::swap(graph_ptr, it.graph_ptr);
      }
      
      Arc & get_current()
      {
	return dl_to_arc(Base::get_current())->get_item();
      }
      
      const Arc & get_current() const
      {
	return dl_to_arc(Base::get_current())->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	GArc * a = dl_to_arc(Base::get_current());
	Base::next();
	graph_ptr->remove_arc(a);
      }

      Arc & operator * ()
      {
	return get_current();
      }

      const Arc & operator * () const
      {
	return get_current();
      }
    };
    
    class AdjacentArcIterator : public DL::Iterator
    {
      using Base = DL::Iterator;
      
      Graph * graph_ptr;
      Node  * node_ptr;
      
    public:
      AdjacentArcIterator()
	: Base(), graph_ptr(nullptr), node_ptr(nullptr)
      {
	// empty
      }
      
      AdjacentArcIterator(const Graph & g, const Node & n)
	: Base(const_cast<DL *>(&n.adjacent_arc_list)),
	  graph_ptr(const_cast<Graph *>(&g)), node_ptr(const_cast<Node *>(&n))
      {
	// empty
      }
      
      AdjacentArcIterator(const AdjacentArcIterator & it)
	: Base(it), graph_ptr(it.graph_ptr), node_ptr(it.node_ptr)
      {
	// empty
      }
      
      AdjacentArcIterator(AdjacentArcIterator && it)
	: AdjacentArcIterator()
      {
	swap(it);
      }

      AdjacentArcIterator & operator = (const AdjacentArcIterator & it)
      {
	if (this == &it)
	  return *this;

	(Base &) *this = it;
	graph_ptr = it.graph_ptr;
	node_ptr  = it.node_ptr;
	return *this;
      }

      AdjacentArcIterator & operator = (AdjacentArcIterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(AdjacentArcIterator & it)
      {
	Base::swap(it);
	std::swap(graph_ptr, it.graph_ptr);
	std::swap(node_ptr, it.node_ptr);
      }
      
      Arc & get_current()
      {
	return dl_to_adjacent_arc(Base::get_current())->get_item()->get_item();
      }

      const Arc & get_current() const
      {
	return dl_to_adjacent_arc(Base::get_current())->get_item()->get_item();
      }

      Arc & operator * ()
      {
	return get_current();
      }

      const Arc & operator * () const
      {
	return get_current();
      }

      Node & get_src_node()
      {
	return *node_ptr;
      }

      const Node & get_src_node() const
      {
	return *node_ptr;
      }

      Node & get_tgt_node()
      {
	return get_current().get_connected_node(*node_ptr);
      }

      const Node & get_tgt_node() const
      {
	return get_current().get_connected_node(*node_ptr);
      }
    };

    NodeIterator nodes_begin()
    {
      return NodeIterator(*this);
    }

    const NodeIterator nodes_begin() const
    {
      return NodeIterator(*this);
    }

    NodeIterator nodes_end()
    {
      NodeIterator it(*this);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    const NodeIterator nodes_end() const
    {
      NodeIterator it(*this);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    ArcIterator arcs_begin()
    {
      return ArcIterator(*this);
    }

    const ArcIterator arcs_begin() const
    {
      return ArcIterator(*this);
    }

    ArcIterator arcs_end()
    {
      ArcIterator it(*this);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    const ArcIterator arcs_end() const
    {
      ArcIterator it(*this);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    AdjacentArcIterator arcs_begin(Node & p)
    {
      return AdjacentArcIterator(*this, p);
    }

    const AdjacentArcIterator arcs_begin(Node & p) const
    {
      return AdjacentArcIterator(*this, p);
    }

    AdjacentArcIterator arcs_end(Node & p)
    {
      AdjacentArcIterator it(*this, p);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    const AdjacentArcIterator arcs_end(Node & p) const
    {
      AdjacentArcIterator it(*this, p);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    Arc * search_arc(Node &, Node &);
    
    template <class Cmp>
    void sort_nodes(Cmp & cmp)
    {
      quicksort<Node, Cmp>(*dl_to_node(&node_list), cmp);
    }

    template <class Cmp>
    void sort_nodes(Cmp && cmp = Cmp())
    {
      sort_nodes<Cmp>(cmp);
    }

    template <class Cmp>
    void sort_arcs(Cmp & cmp)
    {
      quicksort<Arc, Cmp>(*dl_to_arc(&arc_list), cmp);
    }

    template <class Cmp>
    void sort_arcs(Cmp && cmp = Cmp())
    {
      sort_arcs<Cmp>(cmp);
    }

    bool is_digraph() const { return false; }
  };

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  void Graph<NodeInfo, ArcInfo, GraphInfo>::remove_node(GNode * node)
  {
    DL & l = node->get_item().adjacent_arc_list;
    
    while (not l.is_empty())
      {
	GAdArc * adjacent_arc = dl_to_adjacent_arc(l.get_next());
	GArc   * arc = adjacent_arc->get_item();
	remove_arc(arc);
      }
    
    node->del();
    --num_nodes;
    delete node;
  }
  
  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  void Graph<NodeInfo, ArcInfo, GraphInfo>::clear()
  {
    while (not node_list.is_empty())
      {
        GNode * node = dl_to_node(node_list.get_next());
        remove_node(node);
      }
  }

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  typename Graph<NodeInfo, ArcInfo, GraphInfo>::Arc *
  Graph<NodeInfo, ArcInfo, GraphInfo>::search_arc(Node & s, Node & t)
  {
    for (AdjacentArcIterator it(*this, s); it.has_current(); it.next())
      if (&it.get_tgt_node() == &t)
	return &*it;

    for (AdjacentArcIterator it(*this, t); it.has_current(); it.next())
      if (&it.get_tgt_node() == &s)
	return &*it;

    return nullptr;
  }

  template <typename NodeInfo,
	    typename ArcInfo   = EmptyClass,
	    typename GraphInfo = EmptyClass>
  class Digraph : public BaseGraph<Digraph<NodeInfo, ArcInfo, GraphInfo>,
				   DigraphNode<NodeInfo, ArcInfo, GraphInfo>,
				   DigraphArc<
				     DigraphNode<NodeInfo, ArcInfo, GraphInfo>,
				     NodeInfo, ArcInfo, GraphInfo>>
  {
    using BaseGraph = BaseGraph<Digraph<NodeInfo, ArcInfo, GraphInfo>,
				DigraphNode<NodeInfo, ArcInfo, GraphInfo>,
				DigraphArc<
				  DigraphNode<NodeInfo, ArcInfo, GraphInfo>,
				  NodeInfo, ArcInfo, GraphInfo>>;
    
  public:
    using NodeInfoType  = NodeInfo;
    using ArcInfoType   = ArcInfo;
    using GraphInfoType = GraphInfo;

    using Node = DigraphNode<NodeInfo, ArcInfo, GraphInfo>;
    using Arc  = DigraphArc<Node, NodeInfo, ArcInfo, GraphInfo>;

  protected:
    using GNode  = DLNode<Node>;
    using GAdArc = DLNode<Arc>;
    using GArc   = DLNode<GAdArc *>;
    
    static GNode * dl_to_node(DL * ptr)
    {
      return static_cast<GNode *>(ptr);
    }

    static GArc * dl_to_arc(DL * ptr)
    {
      return static_cast<GArc *>(ptr);
    }

    static GAdArc * dl_to_adjacent_arc(DL * ptr)
    {
      return static_cast<GAdArc *>(ptr);
    }

    static GNode * to_gnode(Node & node)
    {
      GNode * node_zero    = 0;
      nat_t   off_set      = (nat_t) &node_zero->get_item();
      nat_t   node_address = (nat_t) &node;
      return (GNode *) (node_address - off_set); 
    }

    static GAdArc * to_garc(Arc & arc)
    {
      GAdArc * arc_zero    = 0;
      nat_t    off_set     = (nat_t) &arc_zero->get_item();
      nat_t    arc_address = (nat_t) &arc;
      return (GAdArc *) (arc_address - off_set); 
    }
    
    GraphInfo info;
    nat_t     num_nodes;
    DL        node_list;
    nat_t     num_arcs;
    DL        arc_list;

    GNode * insert_node(GNode * p)
    {
      node_list.insert_prev(p);
      ++num_nodes;
      return p;
    }
    
    GAdArc * insert_arc(Node * src, Node * tgt)
    {
      GAdArc * arc = new GAdArc(Arc(src, tgt));
      
      GArc * arc_in_arc_list = new GArc(arc);
      
      arc->get_item().arc_in_arc_list = arc_in_arc_list;
      src->adjacent_arc_list.insert_prev(arc);
      ++src->num_arcs;
      
      arc_list.insert_prev(arc_in_arc_list);
      ++num_arcs;
      return arc;
    }
    
    void remove_arc(GAdArc * arc)
    {
      GArc * arc_in_arc_list = arc->get_item().arc_in_arc_list;

      arc_in_arc_list->del();
      --num_arcs;
      delete arc_in_arc_list;
      
      Node * src_node = arc->get_item().src_node;
      
      arc->del();
      --src_node->num_arcs;
      delete arc;
    }

    void remove_node(GNode *);

  public:
    Digraph()
      : info(), num_nodes(0), node_list(), num_arcs(0), arc_list()
    {
      // empty
    }
    
    Digraph(const GraphInfo & _info)
      : info(_info), num_nodes(0), num_arcs(0)
    {
      // empty
    }

    Digraph(GraphInfo && _info)
      : info(std::move(_info)), num_nodes(0), num_arcs(0)
    {
      // empty
    }

    Digraph(const Digraph & g)
      : info(g.info), num_nodes(0), num_arcs(0)
    {
      copy_graph(g, *this);
    }

    Digraph(Digraph && g)
      : Digraph()
    {
      swap(g);
    }

    ~Digraph()
    {
      clear();
    }

    Digraph & operator = (const Digraph & g)
    {
      if (this == &g)
	return *this;

      clear();
      BaseGraph::copy_graph(g, *this);
      info = g.info;
  
      return *this;
    }

    Digraph & operator = (Digraph && g)
    {
      swap(g);
      return *this;
    }

    void swap(Digraph & g)
    {
      std::swap(info, g.info);
      std::swap(num_nodes, g.num_nodes);
      node_list.swap(g.node_list);
      std::swap(num_arcs, g.num_arcs);
      arc_list.swap(g.arc_list);
    }

    void clear();

    GraphInfo & get_info()
    {
      return info;
    }

    const GraphInfo & get_info() const
    {
      return info;
    }

    Node & get_first_node()
    {
      if (node_list.is_empty())
	throw std::underflow_error("Graph has not nodes");

      return dl_to_node(node_list.get_next())->get_item();
    }

    const Node & get_first_node() const
    {
      if (node_list.is_empty())
	throw std::underflow_error("Graph has not nodes");

      return dl_to_node(node_list.get_next())->get_item();
    }

    Arc & get_first_arc()
    {
      if (arc_list.is_empty())
	throw std::underflow_error("Graph has not arcs");

      return dl_to_arc(arc_list.get_next())->get_item()->get_item();
    }

    const Arc & get_first_arc() const
    {
      if (arc_list.is_empty())
	throw std::underflow_error("Graph has not arcs");

      return dl_to_arc(arc_list.get_next())->get_item()->get_item();
    }

    nat_t get_num_nodes() const
    {
      return num_nodes;
    }

    nat_t get_num_arcs() const
    {
      return num_arcs;
    }

    Node & insert_node()
    {
      GNode * node = insert_node(new GNode);
      return node->get_item();
    }

    Node & insert_node(const NodeInfo & info)
    {
      GNode * node = insert_node(new GNode(Node(info)));
      return node->get_item();
    }

    Node & insert_node(NodeInfo && info)
    {
      GNode * node = insert_node(new GNode(Node(std::forward<NodeInfo>(info))));
      return node->get_item();
    }

    Arc & insert_arc(Node & s, Node & t)
    {
      Node   * src = &s;
      Node   * tgt = &t;
      GAdArc * arc = insert_arc(src, tgt);
      return arc->get_item();
    }
    
    Arc & insert_arc(Node & src, Node & tgt, const ArcInfo & info)
    {
      Arc & ref_arc = insert_arc(src, tgt);
      ref_arc.get_info() = info;
      return ref_arc;
    }

    Arc & insert_arc(Node & src, Node & tgt, ArcInfo && info)
    {
      Arc & ref_arc = insert_arc(src, tgt);
      ref_arc.get_info() = std::move(info);
      return ref_arc;
    }

    void remove_arc(Arc & a)
    {
      GAdArc * arc = to_garc(a);
      remove_arc(arc);
    }

    void remove_node(Node & n)
    {
      GNode * node = to_gnode(n);
      remove_node(node);
    }
    
    class NodeIterator : public DL::Iterator
    {
      using Base = DL::Iterator;
      
      Digraph * graph_ptr;

    public:
      NodeIterator()
	: Base(), graph_ptr(nullptr)
      {
	// empty
      }

      NodeIterator(const Digraph & g)
	: Base(const_cast<DL *>(&g.node_list)),
	  graph_ptr(const_cast<Digraph *>(&g))
      {
	// empty
      }

      NodeIterator(const NodeIterator & it)
	: Base(it), graph_ptr(it.graph_ptr)
      {
	// empty
      }

      NodeIterator(NodeIterator && it)
	: NodeIterator()
      {
	swap(it);
      }

      NodeIterator & operator = (const NodeIterator & it)
      {
	if (this == &it)
	  return *this;

	(Base &) *this = it;
	graph_ptr = it.graph_ptr;
	return *this;
      }

      NodeIterator & operator = (NodeIterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(NodeIterator & it)
      {
	Base::swap(it);
	std::swap(graph_ptr, it.graph_ptr);
      }

      Node & get_current()
      {
	return dl_to_node(Base::get_current())->get_item();
      }

      const Node & get_current() const
      {
	return dl_to_node(Base::get_current())->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	GNode * p = dl_to_node(Base::get_current());
	Base::next();
	graph_ptr->remove_node(p);
      }

      Node & operator * ()
      {
	return get_current();
      }

      const Node & operator * () const
      {
	return get_current();
      }
    };
    
    class ArcIterator : public DL::Iterator
    {
      using Base = DL::Iterator;
      
      Digraph * graph_ptr;

    public:
      ArcIterator()
	: Base(), graph_ptr(nullptr)
      {
	// empty
      }

      ArcIterator(const Digraph & g)
	: Base(const_cast<DL *>(&g.arc_list)),
	  graph_ptr(const_cast<Digraph *>(&g))
      {
	// empty
      }

      ArcIterator(const ArcIterator & it)
	: Base(it), graph_ptr(it.graph_ptr)
      {
	// empty
      }

      ArcIterator(ArcIterator && it)
	: ArcIterator()
      {
	swap(it);
      }

      ArcIterator & operator = (const ArcIterator & it)
      {
	if (this == &it)
	  return *this;

	(Base &) *this = it;
	graph_ptr = it.graph_ptr;
	return *this;
      }

      ArcIterator & operator = (ArcIterator && it)
      {
	swap(it);
	return *this;
      }
      
      void swap(ArcIterator & it)
      {
	Base::swap(it);
	std::swap(graph_ptr, it.graph_ptr);
      }
      
      Arc & get_current()
      {
	return dl_to_arc(Base::get_current())->get_item()->get_item();
      }
      
      const Arc & get_current() const
      {
	return dl_to_arc(Base::get_current())->get_item()->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	GArc * a = dl_to_arc(Base::get_current());
	Base::next();
	graph_ptr->remove_arc(a->get_item());
      }

      Arc & operator * ()
      {
	return get_current();
      }

      const Arc & operator * () const
      {
	return get_current();
      }
    };
    
    class AdjacentArcIterator : public DL::Iterator
    {
      using Base = DL::Iterator;
      
      Digraph * graph_ptr;
      Node    * node_ptr;
      
    public:
      AdjacentArcIterator()
	: Base(), graph_ptr(nullptr), node_ptr(nullptr)
      {
	// empty
      }
      
      AdjacentArcIterator(const Digraph & g, const Node & n)
	: Base(const_cast<DL *>(&n.adjacent_arc_list)),
	  graph_ptr(const_cast<Digraph *>(&g)), node_ptr(const_cast<Node *>(&n))
      {
	// empty
      }
      
      AdjacentArcIterator(const AdjacentArcIterator & it)
	: Base(it), graph_ptr(it.graph_ptr), node_ptr(it.node_ptr)
      {
	// empty
      }
      
      AdjacentArcIterator(AdjacentArcIterator && it)
	: AdjacentArcIterator()
      {
	swap(it);
      }

      AdjacentArcIterator & operator = (const AdjacentArcIterator & it)
      {
	if (this == &it)
	  return *this;

	(Base &) *this = it;
	graph_ptr = it.graph_ptr;
	node_ptr  = it.node_ptr;
	return *this;
      }

      AdjacentArcIterator & operator = (AdjacentArcIterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(AdjacentArcIterator & it)
      {
	Base::swap(it);
	std::swap(graph_ptr, it.graph_ptr);
	std::swap(node_ptr, it.node_ptr);
      }
      
      Arc & get_current()
      {
	return dl_to_adjacent_arc(Base::get_current())->get_item();
      }

      const Arc & get_current() const
      {
	return dl_to_adjacent_arc(Base::get_current())->get_item();
      }

      Arc & operator * ()
      {
	return get_current();
      }

      const Arc & operator * () const
      {
	return get_current();
      }

      Node & get_src_node()
      {
	return *node_ptr;
      }

      const Node & get_src_node() const
      {
	return *node_ptr;
      }

      Node & get_tgt_node()
      {
	return get_current().get_tgt_node();
      }

      const Node & get_tgt_node() const
      {
	return get_current().get_tgt_node();
      }
    };

    NodeIterator nodes_begin()
    {
      return NodeIterator(*this);
    }

    const NodeIterator nodes_begin() const
    {
      return NodeIterator(*this);
    }

    NodeIterator nodes_end()
    {
      NodeIterator it(*this);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    const NodeIterator nodes_end() const
    {
      NodeIterator it(*this);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    ArcIterator arcs_begin()
    {
      return ArcIterator(*this);
    }

    const ArcIterator arcs_begin() const
    {
      return ArcIterator(*this);
    }

    ArcIterator arcs_end()
    {
      ArcIterator it(*this);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    const ArcIterator arcs_end() const
    {
      ArcIterator it(*this);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    AdjacentArcIterator arcs_begin(Node & p)
    {
      return AdjacentArcIterator(*this, p);
    }

    const AdjacentArcIterator arcs_begin(Node & p) const
    {
      return AdjacentArcIterator(*this, p);
    }

    AdjacentArcIterator arcs_end(Node & p)
    {
      AdjacentArcIterator it(*this, p);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    const AdjacentArcIterator arcs_end(Node & p) const
    {
      AdjacentArcIterator it(*this, p);

      if (not it.has_current())
	return it;

      it.reset_last();
      it.next();
      return it;
    }

    Arc * search_arc(Node &, Node &);
    
    template <class Cmp>
    void sort_nodes(Cmp & cmp)
    {
      quicksort<Node, Cmp>(*dl_to_node(&node_list), cmp);
    }

    template <class Cmp>
    void sort_nodes(Cmp && cmp = Cmp())
    {
      sort_nodes<Cmp>(cmp);
    }

    template <class Cmp>
    void sort_arcs(Cmp & cmp)
    {
      quicksort<Arc, Cmp>(*dl_to_arc(&arc_list), cmp);
    }

    template <class Cmp>
    void sort_arcs(Cmp && cmp = Cmp())
    {
      sort_arcs<Cmp>(cmp);
    }

    bool is_digraph() const { return true; }
  };

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  void Digraph<NodeInfo, ArcInfo, GraphInfo>::remove_node(GNode * node)
  {
    DL * curr_link = arc_list.get_next();
    
    while (curr_link != &arc_list)
      {
	GArc * arc = dl_to_arc(curr_link);
	curr_link = curr_link->get_next(); // Muevo antes de eliminar
	if (arc->get_item()->get_item().src_node == &node->get_item() or
	    arc->get_item()->get_item().tgt_node == &node->get_item())
	  remove_arc(arc->get_item());
      }
    
    node->del();
    --num_nodes;
    delete node;
  }
  
  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  void Digraph<NodeInfo, ArcInfo, GraphInfo>::clear()
  {
    while (not arc_list.is_empty())
      {
        GAdArc * arc = dl_to_arc(arc_list.get_next())->get_item();
        remove_arc(arc);
      }

    while (not node_list.is_empty())
      {
        GNode * node = dl_to_node(node_list.get_next());
        remove_node(node);
      }
  }
  
  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  typename Digraph<NodeInfo, ArcInfo, GraphInfo>::Arc *
  Digraph<NodeInfo, ArcInfo, GraphInfo>::search_arc(Node & s, Node & t)
  {
    for (AdjacentArcIterator it(*this, s); it.has_current(); it.next())
      if (&it.get_tgt_node() == &t)
	return &*it;

    return nullptr;
  }
  
} // end namespace Designar
    
# endif // DSGGRAPH_H
