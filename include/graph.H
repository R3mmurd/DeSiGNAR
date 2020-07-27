/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGGRAPH_H
# define DSGGRAPH_H

# include <nodesdef.H>
# include <graphutilities.H>
# include <italgorithms.H>
# include <sort.H>

namespace Designar
{
  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class Graph;

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class Digraph;

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class GraphNode : public BaseGraphNode<NodeInfo, CommonNodeArc>
  {
    friend class Graph<NodeInfo, ArcInfo, GraphInfo>;
    friend class DLNode<GraphNode>;
    using Base = BaseGraphNode<NodeInfo, CommonNodeArc>;
    using Base::Base;
  };

  template <typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class DigraphNode : public BaseGraphNode<NodeInfo, CommonNodeArc>
  {
    friend class Digraph<NodeInfo, ArcInfo, GraphInfo>;
    friend class DLNode<DigraphNode>;
    using Base = BaseGraphNode<NodeInfo, CommonNodeArc>;
    using Base::Base;
  };

  template <class Node, typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class GraphArc : public BaseGraphArc<Node, ArcInfo, CommonNodeArc>
  {
    friend class Graph<NodeInfo, ArcInfo, GraphInfo>;
    friend class DLNode<GraphArc>;
    using Base = BaseGraphArc<Node, ArcInfo, CommonNodeArc>;
    using Base::Base;
    
  protected:
    DLNode<DLNode<GraphArc> *> * arc_in_src_node = nullptr;
    DLNode<DLNode<GraphArc> *> * arc_in_tgt_node = nullptr;
  };
  
  template <class Node, typename NodeInfo, typename ArcInfo, typename GraphInfo>
  class DigraphArc : public BaseGraphArc<Node, ArcInfo, CommonNodeArc>
  {
    friend class Digraph<NodeInfo, ArcInfo, GraphInfo>;
    friend class DLNode<DigraphArc>;
    using Base = BaseGraphArc<Node, ArcInfo, CommonNodeArc>;
    using Base::Base;

  protected:
    DLNode<DLNode<DigraphArc> *> * arc_in_arc_list = nullptr;
  };

  template <class GT, class Node, class Arc,
	    typename NodeInfoType, typename ArcInfoType>
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

    template <class RetT, class It>
    RetT * nth_graph_element(const It &, const It &, nat_t);
    
  public:
    static void copy_graph(const GT &, GT &);
    
    Node * nth_node(nat_t i)
    {
      return nth_graph_element<Node>(const_me().nodes_begin(),
				     const_me().nodes_end(), i);
    }

    Node * nth_node(nat_t i) const
    {
      return nth_graph_element<Node>(const_me().nodes_begin(),
				     const_me().nodes_end(), i);
    }
    
    template <class Op>
    void for_each_node(Op & op) const
    {
      for_each_it(const_me().nodes_begin(), const_me().nodes_end(), op);
    }

    template <class Op>
    void for_each_node(Op && op = Op()) const
    {
      for_each_it(const_me().nodes_begin(), const_me().nodes_end(),
		  std::forward<Op>(op));
    }

    template <class Op>
    void enum_for_each_node(Op & op) const
    {
      enum_for_each_it(const_me().nodes_begin(), const_me().nodes_end(), op);
    }

    template <class Op>
    void enum_for_each_node(Op && op = Op()) const
    {
      enum_for_each_it(const_me().nodes_begin(), const_me().nodes_end(),
		       std::forward<Op>(op));
    }

    template <class Op>
    void for_each_node_pair(Op & op) const
    {
      for_each_pair_it(const_me().nodes_begin(), const_me().nodes_end(), op);
    }

    template <class Op>
    void for_each_node_pair(Op && op = Op()) const
    {
      for_each_pair_it(const_me().nodes_begin(), const_me().nodes_end(),
		       std::forward<Op>(op));
    }

    template <class ContainerRet = SLList<Node *>, class Pred>
    ContainerRet filter_nodes(Pred & pred) const
    {
      return filter_it<ContainerRet>
	(const_me().nodes_begin(), const_me().nodes_end(), pred);
    }

    template <class ContainerRet = SLList<Node *>, class Pred>
    ContainerRet filter_nodes(Pred && pred = Pred()) const
    {
      return filter_it<ContainerRet>
	(const_me().nodes_begin(), const_me().nodes_end(),
	 std::forward<Pred>(pred));
    }

    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map_nodes(Op & op) const
    {
      return map_it<ContainerRet>(const_me().nodes_begin(),
				  const_me().nodes_end(), op);
    }

    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map_nodes(Op && op = Op()) const
    {
      return map_it<ContainerRet>(const_me().nodes_begin(),
				  const_me().nodes_end(), std::forward<Op>(op));
    }

    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_nodes_if(Op & op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().nodes_begin(),
				     const_me().nodes_end(),
				     op, pred);
    }

    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_nodes_if(Op & op, Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().nodes_begin(),
				     const_me().nodes_end(),
				     op, std::forward<Pred>(pred));
    }
    
    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_nodes_if(Op && op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().nodes_begin(),
				     const_me().nodes_end(),
				     std::forward<Op>(op), pred);
    }

    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_nodes_if(Op && op = Op(), Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().nodes_begin(),
				     const_me().nodes_end(),
				     std::forward<Op>(op),
				     std::forward<Pred>(pred));
    }

    template <typename RetT, class Op>
    RetT fold_nodes(const RetT & init_val, Op & op) const
    {
      return fold_it<RetT>(const_me().nodes_begin(), const_me().nodes_end(),
			   init_val, op);
    }

    template <typename RetT, class Op>
    RetT fold_nodes(const RetT & init_val, Op && op = Op()) const
    {
      return fold_it<RetT>(const_me().nodes_begin(), const_me().nodes_end(),
			   init_val, std::forward<Op>(op));
    }

    template <typename RetT, class Op>
    RetT fold_nodes(RetT && init_val, Op & op) const
    {
      return fold_it<RetT>(const_me().nodes_begin(), const_me().nodes_end(),
			   std::forward<RetT>(init_val), op);
    }

    template <typename RetT, class Op>
    RetT fold_nodes(RetT && init_val, Op && op = Op()) const
    {
      return fold_it<RetT>(const_me().nodes_begin(), const_me().nodes_end(),
			   std::forward<RetT>(init_val),
			   std::forward<Op>(op));
    }
    
    template <class Pred>
    bool all_nodes(Pred & pred) const
    {
      return all_it(const_me().nodes_begin(), const_me().nodes_end(), pred);
    }

    template <class Pred>
    bool all_nodes(Pred && pred = Pred()) const
    {
      return all_it(const_me().nodes_begin(), const_me().nodes_end(),
		    std::forward<Pred>(pred));
    }

    template <class Pred>
    bool exists_node(Pred & pred) const
    {
      return exists_it(const_me().nodes_begin(), const_me().nodes_end(), pred);
    }

    template <class Pred>
    bool exists_node(Pred && pred = Pred()) const
    {
      return exists_it(const_me().nodes_begin(), const_me().nodes_end(),
		       std::forward<Pred>(pred));
    }

    template <class Pred>
    bool none_node(Pred & pred) const
    {
      return none_it(const_me().nodes_begin(), const_me().nodes_end(), pred);
    }

    template <class Pred>
    bool none_node(Pred && pred = Pred()) const
    {
      return none_it(const_me().nodes_begin(), const_me().nodes_end(),
		     std::forward<Pred>(pred));
    }
    
    template <class Pred>
    Node * search_node(Pred & pred) const
    {
      for (auto it = const_me().nodes_begin();
	   it != const_me().nodes_end(); ++it)
	if (pred(*it))
	  return *it;
      return nullptr;
    }
    
    template <class Pred>
    Node * search_node(Pred && pred = Pred()) const
    {  
      return search_node<Pred>(pred);
    }
    
    template <class Pred>
    bool remove_first_node_if(Pred & pred)
    {
      return remove_first_if_it(me().nodes_begin(), me().nodes_end(), pred);
    }

    template <class Pred>
    bool remove_first_node_if(Pred && pred = Pred())
    {
      return remove_first_if_it(me().nodes_begin(), me().nodes_end(),
				std::forward<Pred>(pred));
    }

    template <class Pred>
    void remove_node_if(Pred & pred)
    {
      remove_if_it(me().nodes_begin(), me().nodes_end(), pred);
    }

    template <class Pred>
    void remove_node_if(Pred && pred = Pred())
    {
      remove_if_it(me().nodes_begin(), me().nodes_end(),
		   std::forward<Pred>(pred));
    }
     
    SLList<Node *> nodes() const
    {
      return to_list_it<Node *>(const_me().nodes_begin(),
				const_me().nodes_end());
    }

    Arc * nth_arc(nat_t i)
    {
      return nth_graph_element<Arc>(const_me().arcs_begin(),
				    const_me().arcs_end(), i);
    }
  
    Arc * nth_arc(nat_t i) const
    {
      return nth_graph_element<Arc>(const_me().arcs_begin(),
				    const_me().arcs_end(), i);
    }
  
    template <class Op>
    void for_each_arc(Op & op) const
    {
      for_each_it(const_me().arcs_begin(), const_me().arcs_end(), op);
    }
  
    template <class Op>
    void for_each_arc(Op && op) const
    {
      for_each_it(const_me().arcs_begin(), const_me().arcs_end(),
		  std::forward<Op>(op));
    }

    template <class Op>
    void enum_for_each_arc(Op & op) const
    {
      enum_for_each_it(const_me().arcs_begin(), const_me().arcs_end(), op);
    }
  
    template <class Op>
    void enum_for_each_arc(Op && op) const
    {
      enum_for_each_it(const_me().arcs_begin(), const_me().arcs_end(),
		       std::forward<Op>(op));
    }

    template <class ContainerRet = SLList<Arc *>, class Pred>
    ContainerRet filter_arcs(Pred & pred) const
    {
      return filter_it<ContainerRet>
	(const_me().arcs_begin(), const_me().arcs_end(), pred);
    }

    template <class ContainerRet = SLList<Arc *>, class Pred>
    ContainerRet filter_arcs(Pred && pred = Pred()) const
    {
      return filter_it<ContainerRet>
	(const_me().arcs_begin(), const_me().arcs_end(),
	 std::forward<Pred>(pred));
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map_arcs(Op & op) const
    {
      return map_it<ContainerRet>(const_me().arcs_begin(),
				  const_me().arcs_end(), op);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map_arcs(Op && op = Op()) const
    {
      return map_it<ContainerRet>(const_me().arcs_begin(),
				  const_me().arcs_end(), std::forward<Op>(op));
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_arcs_if(Op & op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(),
				     const_me().arcs_end(),
				     op, pred);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_arcs_if(Op & op, Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(),
				     const_me().arcs_end(),
				     op, std::forward<Pred>(pred));
    }
    
    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_arcs_if(Op && op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(),
				     const_me().arcs_end(),
				     std::forward<Op>(op), pred);
    }
  
    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_arcs_if(Op && op = Op(), Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(),
				     const_me().arcs_end(),
				     std::forward<Op>(op),
				     std::forward<Pred>(pred));
    }
    
    template <typename RetT, class Op>
    RetT fold_arcs(const RetT & init_val, Op & op) const
    {
      return fold_it<RetT>(const_me().arcs_begin(), const_me().arcs_end(),
			   init_val, op);
    }
    
    template <typename RetT, class Op>
    RetT fold_arcs(const RetT & init_val, Op && op = Op()) const
    {
      return fold_it<RetT>(const_me().arcs_begin(), const_me().arcs_end(),
			   init_val, std::forward<Op>(op));
    }
  
    template <typename RetT, class Op>
    RetT fold_arcs(RetT && init_val, Op & op) const
    {
      return fold_it<RetT>(const_me().arcs_begin(), const_me().arcs_end(),
			   std::forward<RetT>(init_val), op);
    }

    template <typename RetT, class Op>
    RetT fold_arcs(RetT && init_val, Op && op = Op()) const
    {
      return fold_it<RetT>(const_me().arcs_begin(), const_me().arcs_end(),
			   std::forward<RetT>(init_val),
			   std::forward<Op>(op));
    }
    
    template <class Pred>
    bool all_arcs(Pred & pred) const
    {
      return all_it(const_me().arcs_begin(), const_me().arcs_end(), pred);
    }
  
    template <class Pred>
    bool all_arcs(Pred && pred) const
    {
      return all_it(const_me().arcs_begin(), const_me().arcs_end(),
		    std::forward<Pred>(pred));
    }
  
    template <class Pred>
    bool exists_arc(Pred & pred) const
    {
      return exists_it(const_me().arcs_begin(), const_me().arcs_end(), pred);
    }
  
    template <class Pred>
    bool exists_arc(Pred && pred) const
    {
      return exists_it(const_me().arcs_begin(), const_me().arcs_end(),
		       std::forward<Pred>(pred));
    }

    template <class Pred>
    bool none_arc(Pred & pred) const
    {
      return none_it(const_me().arcs_begin(), const_me().arcs_end(), pred);
    }
  
    template <class Pred>
    bool none_arc(Pred && pred) const
    {
      return none_it(const_me().arcs_begin(), const_me().arcs_end(),
		     std::forward<Pred>(pred));
    }
  
    template <class Pred> 
    Arc * search_arc(Pred & pred) const
    {
      for (auto it = const_me().arcs_begin();
	   it != const_me().arcs_end(); ++it)
	if (pred(*it))
	  return *it;
      return nullptr;
    }
  
    template <class Pred>
    Arc * search_arc(Pred && pred) const
    {
      return search_arc<Pred>(pred);
    }
  
    template <class Pred>
    bool remove_first_arc_if(Pred & pred)
    {
      return remove_first_if_it(me().arcs_begin(), me().arcs_end(), pred);
    }
  
    template <class Pred>
    bool remove_first_arc_if(Pred && pred)
    {
      return remove_first_if_it(me().arcs_begin(), me().arcs_end(),
				std::forward<Pred>(pred));
    }
  
    template <class Pred>
    void remove_arc_if(Pred & pred)
    {
      remove_if_it(me().arcs_begin(), me().arcs_end(), pred);
    }
  
    template <class Pred>
    void remove_arc_if(Pred && pred)
    {
      remove_if_it(me().arcs_begin(), me().arcs_end(), std::forward<Pred>(pred));
    }
  
    SLList<Arc *> arcs() const
    {
      return to_list_it<Arc *>(const_me().arcs_begin(), const_me().arcs_end());
    }
  
    Arc * nth_adjacent_arc(Node * p, nat_t i)
    {
      return nth_graph_element<Arc>(const_me().arcs_begin(p),
				    const_me().arcs_end(p), i);
    }
  
    Arc * nth_adjacent_arc(Node * p, nat_t i) const
    {
      return nth_graph_element<Arc>(const_me().arcs_begin(p),
				    const_me().arcs_end(p), i);
    }
  
    template <class Op>
    void for_each_adjacent_arc(Node * p, Op & op) const
    {
      for_each_it(const_me().arcs_begin(p), const_me().arcs_end(p), op);
    }
  
    template <class Op>
    void for_each_adjacent_arc(Node * p, Op && op) const
    {
      for_each_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		  std::forward<Op>(op));
    }

    template <class Op>
    void enum_for_each_adjacent_arc(Node * p, Op & op) const
    {
      enum_for_each_it(const_me().arcs_begin(p), const_me().arcs_end(p), op);
    }
  
    template <class Op>
    void enum_for_each_adjacent_arc(Node * p, Op && op) const
    {
      enum_for_each_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		       std::forward<Op>(op));
    }

    template <class ContainerRet = SLList<Arc *>, class Pred>
    ContainerRet filter_adjacent_arcs(Node * p, Pred & pred) const
    {
      return filter_it<ContainerRet>
	(const_me().arcs_begin(p), const_me().arcs_end(p), pred);
    }

    template <class ContainerRet = SLList<Arc *>, class Pred>
    ContainerRet filter_adjacent_arcs(Node * p, Pred && pred = Pred()) const
    {
      return filter_it<ContainerRet>
	(const_me().arcs_begin(p), const_me().arcs_end(p),
	 std::forward<Pred>(pred));
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map_adjacent_arcs(Node * p, Op & op) const
    {
      return map_it<ContainerRet>(const_me().arcs_begin(p),
				  const_me().arcs_end(p), op);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map_adjacent_arcs(Node * p, Op && op = Op()) const
    {
      return map_it<ContainerRet>(const_me().arcs_begin(p),
				  const_me().arcs_end(p),
				  std::forward<Op>(op));
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_adjacent_arcs_if(Node * p, Op & op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(p),
				     const_me().arcs_end(p),
				     op, pred);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet
    map_adjacent_arcs_if(Node * p, Op & op, Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(p),
				     const_me().arcs_end(p),
				     op, std::forward<Pred>(pred));
    }
    
    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_adjacent_arcs_if(Node * p, Op && op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(p),
				     const_me().arcs_end(p),
				     std::forward<Op>(op), pred);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet
    map_adjacent_arcs_if(Node * p, Op && op = Op(), Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(p),
				     const_me().arcs_end(p),
				     std::forward<Op>(op),
				     std::forward<Pred>(pred));
    }

    template <typename RetT, class Op>
    RetT fold_adjacent_arcs(Node * p, const RetT & init_val, Op & op) const
    {
      return fold_it<RetT>(const_me().arcs_begin(p), const_me().arcs_end(p),
			   init_val, op);
    }

    template <typename RetT, class Op>
    RetT fold_adjacent_arcs(Node * p, const RetT & init_val, Op && op = Op()) const
    {
      return fold_it<RetT>(const_me().arcs_begin(p), const_me().arcs_end(p),
			   init_val, std::forward<Op>(op));
    }

    template <typename RetT, class Op>
    RetT fold_adjacent_arcs(Node * p, RetT && init_val, Op & op) const
    {
      return fold_it<RetT>(const_me().arcs_begin(p), const_me().arcs_end(p),
			   std::forward<RetT>(init_val), op);
    }

    template <typename RetT, class Op>
    RetT fold_adjacent_arcs(Node * p, RetT && init_val, Op && op = Op()) const
    {
      return fold_it<RetT>(const_me().arcs_begin(p), const_me().arcs_end(p),
			   std::forward<RetT>(init_val),
			   std::forward<Op>(op));
    }
  
    template <class Pred>
    bool all_adjacent_arcs(Node * p, Pred & pred) const
    {
      return all_it(const_me().arcs_begin(p), const_me().arcs_end(p), pred);
    }
  
    template <class Pred>
    bool all_adjacent_arcs(Node * p, Pred && pred) const
    {
      return all_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		    std::forward<Pred>(pred));
    }
  
    template <class Pred>
    bool exists_adjacent_arc(Node * p, Pred & pred) const
    {
      return exists_it(const_me().arcs_begin(p), const_me().arcs_end(p), pred);
    }
  
    template <class Pred>
    bool exists_adjacent_arc(Node * p, Pred && pred) const
    {
      return exists_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		       std::forward<Pred>(pred));
    }

    template <class Pred>
    bool none_adjacent_arc(Node * p, Pred & pred) const
    {
      return none_it(const_me().arcs_begin(p), const_me().arcs_end(p), pred);
    }
  
    template <class Pred>
    bool none_adjacent_arc(Node * p, Pred && pred) const
    {
      return none_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		     std::forward<Pred>(pred));
    }
  
    template <class Pred>
    Arc * search_adjacent_arc(Node * p, Pred & pred) const
    {
      for (auto it = const_me().arcs_begin(p);
	   it != const_me().arcs_end(p); ++it)
	if (pred(*it))
	  return *it;
      return nullptr;
    }
  
    template <class Pred>
    Arc * search_adjacent_arc(Node * p, Pred && pred) const
    {
      return search_adjacent_arc<Pred>(p, pred);
    }
  
    template <class Pred>
    bool remove_first_adjacent_arc_if(Node * p, Pred & pred)
    {
      return remove_first_if_it(me().arcs_begin(p), me().arcs_end(p), pred);
    }
  
    template <class Pred>
    bool remove_first_adjacent_arc_if(Node * p, Pred && pred)
    {
      return remove_first_if_it(me().arcs_begin(p), me().arcs_end(p),
				std::forward<Pred>(pred));
    }
  
    template <class Pred>
    void remove_adjacent_arc_if(Node * p, Pred & pred)
    {
      remove_if_it(me().arcs_begin(p), me().arcs_end(p), pred);
    }
  
    template <class Pred>
    void remove_adjacent_arc_if(Node * p, Pred && pred)
    {
      remove_if_it(me().arcs_begin(p), me().arcs_end(p),
		   std::forward<Pred>(pred));
    }

    SLList<Arc *> adjacent_arcs(Node * p) const
    {
      return to_list_it<Arc *>(const_me().arcs_begin(p), const_me().arcs_end(p));
    }
    
    void reset_all_node_tag(GraphTag tag) const
    {
      for_each_node([&tag](Node * node)
		    {
		      node->unvisit(tag);
		    });
    }

    void reset_all_node_tag() const
    {
      for_each_node([](Node * node)
		    {
		      node->reset_tag();
		    });
    }

    void reset_all_arc_tag(GraphTag tag) const
    {
      for_each_arc([&tag](Arc * arc)
		   {
		     arc->unvisit(tag);
		   });
    }

    void reset_all_arc_tag() const
    {
      for_each_arc([](Arc * arc)
		   {
		     arc->reset_tag();
		   });
    }

    void reset_tag(GraphTag tag) const
    {
      reset_all_node_tag(tag);
      reset_all_arc_tag(tag);
    }

    void reset_all_tags() const
    {
      reset_all_node_tag();
      reset_all_arc_tag();
    }

    void reset_node_cookies() const
    {
      for_each_node([](Node * node)
		    {
		      node->cookie() = nullptr;
		    });
    }

    void reset_arc_cookies() const
    {
      for_each_arc([](Arc * arc)
		   {
		     arc->cookie() = nullptr;
		   });
    }

   void reset_node_counter() const
    {
      for_each_node([](Node * node)
		    {
		      node->counter() = 0;
		    });
    }

    void reset_arc_counter() const
    {
      for_each_arc([](Arc * arc)
		   {
		     arc->counter() = 0;
		   });
    }

    void reset_counters() const
    {
      reset_node_counter();
      reset_arc_counter();
    }

    void reset_cookies() const
    {
      reset_node_cookies();
      reset_arc_cookies();
    }

    void reset_nodes() const
    {
      for_each_node([](Node * node)
		    {
		      node->reset();
		    });
    }

    void reset_arcs() const
    {
      for_each_arc([](Arc * arc)
		   {
		     arc->reset();
		   });
    }
  };
  
  template <typename GT, class Node, class Arc,
	    typename NodeInfoType, typename ArcInfoType>
  template <class RetT, class It>
  RetT * BaseGraph<GT, Node, Arc, NodeInfoType, ArcInfoType>::
  nth_graph_element(const It & a, const It & b, nat_t pos)
  {
    for (It i = a; i != b; ++i)
      {
	if (pos == 0)
	  return *i;
	--pos;
      }
    
    return nullptr;
  }

  template <typename GT, class Node, class Arc,
	    typename NodeInfoType, typename ArcInfoType>
  void BaseGraph<GT, Node, Arc, NodeInfoType, ArcInfoType>::
  copy_graph(const GT & src, GT & tgt)
  {
    HashMap<Node *, Node *> map_nodes;

    for (auto it = src.nodes_begin(); it != src.nodes_end(); ++it)
      {
        Node * p = it.get_current();
        Node * q = tgt.insert_node(p->get_info());
        map_nodes[p] = q;
      }

    for (auto it = src.arcs_begin(); it != src.arcs_end(); ++it)
      {
        Arc  * a    = it.get_current();
        Node * ssrc = a->get_src_node();
        Node * stgt = a->get_tgt_node();
        Node * tsrc = map_nodes[ssrc];
        Node * ttgt = map_nodes[stgt];
        tgt.insert_arc(tsrc, ttgt, a->get_info());
      }
  }
  
  template <typename NodeInfo,
	    typename ArcInfo   = EmptyClass,
	    typename GraphInfo = EmptyClass>
  class Graph : public BaseGraph<Graph<NodeInfo, ArcInfo, GraphInfo>,
				 GraphNode<NodeInfo, ArcInfo, GraphInfo>,
				 GraphArc<
				   GraphNode<NodeInfo, ArcInfo, GraphInfo>,
				   NodeInfo, ArcInfo, GraphInfo>,
				 NodeInfo, ArcInfo>
  {
    using Base = BaseGraph<Graph<NodeInfo, ArcInfo, GraphInfo>,
			   GraphNode<NodeInfo, ArcInfo, GraphInfo>,
			   GraphArc<
			     GraphNode<NodeInfo, ArcInfo, GraphInfo>,
			     NodeInfo, ArcInfo, GraphInfo>,
			   NodeInfo, ArcInfo>;
    
  public:
    using NodeInfoType  = NodeInfo;
    using ArcInfoType   = ArcInfo;
    using GraphInfoType = GraphInfo;
    using SetSizeType   = nat_t;

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

    static GNode * to_gnode(Node * node)
    {
      GNode * node_zero    = 0;
      nat_t   off_set      = (nat_t) &node_zero->get_item();
      nat_t   node_address = (nat_t) node;
      return (GNode *) (node_address - off_set); 
    }

    static GArc * to_garc(Arc * arc)
    {
      GArc * arc_zero    = 0;
      nat_t  off_set     = (nat_t) &arc_zero->get_item();
      nat_t  arc_address = (nat_t) arc;
      return (GArc *) (arc_address - off_set); 
    }
    
    GraphInfo info;
    nat_t     num_nodes;
    DL        node_list;
    nat_t     num_arcs;
    DL        arc_list;

    GNode * insert_gnode(GNode * p)
    {
      node_list.insert_prev(p);
      ++num_nodes;
      return p;
    }
    
    GArc * insert_garc(Node * src, Node * tgt)
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
      Base::copy_graph(g, *this);
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
      Base::copy_graph(g, *this);
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

    Node * get_first_node()
    {
      if (node_list.is_empty())
	throw std::underflow_error("Graph has not nodes");

      return &dl_to_node(node_list.get_next())->get_item();
    }

    Node * get_first_node() const
    {
      if (node_list.is_empty())
	throw std::underflow_error("Graph has not nodes");

      return &dl_to_node(const_cast<DL &>(node_list).get_next())->get_item();
    }

    Arc * get_first_arc()
    {
      if (arc_list.is_empty())
	throw std::underflow_error("Graph has not arcs");

      return &dl_to_arc(arc_list.get_next())->get_item();
    }

    Arc * get_first_arc() const
    {
      if (arc_list.is_empty())
	throw std::underflow_error("Graph has not arcs");

      return &dl_to_arc(const_cast<DL &>(arc_list).get_next())->get_item();
    }

    nat_t get_num_nodes() const
    {
      return num_nodes;
    }

    nat_t get_num_arcs() const
    {
      return num_arcs;
    }

    Node * insert_node()
    {
      GNode * node = insert_gnode(new GNode);
      return &node->get_item();
    }

    Node * insert_node(const NodeInfo & info)
    {
      GNode * node = insert_gnode(new GNode(Node(info)));
      return &node->get_item();
    }

    Node * insert_node(NodeInfo && info)
    {
      GNode * node = insert_gnode(new GNode(Node(std::forward<NodeInfo>(info))));
      return &node->get_item();
    }

    Arc * insert_arc(Node * s, Node * t)
    {
      GArc * arc = insert_garc(s, t);
      return &arc->get_item();
    }
    
    Arc * insert_arc(Node * src, Node * tgt, const ArcInfo & info)
    {
      Arc * arc = insert_arc(src, tgt);
      arc->get_info() = info;
      return arc;
    }

    Arc * insert_arc(Node * src, Node * tgt, ArcInfo && info)
    {
      Arc * arc = insert_arc(src, tgt);
      arc->get_info() = std::move(info);
      return arc;
    }

    void remove_arc(Arc * a)
    {
      GArc * arc = to_garc(a);
      remove_arc(arc);
    }

    void remove_node(Node * n)
    {
      GNode * node = to_gnode(n);
      remove_node(node);
    }
    
    class NodeIterator : public DL::Iterator,
                         public BidirectionalIterator<NodeIterator, Node *, true>
    {
      friend class BasicIterator<NodeIterator, Node *, true>;
      
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

      NodeIterator(const Graph & g, DL * curr)
	: Base(const_cast<DL *>(&g.node_list), curr),
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

      Node * get_current()
      {
	return &dl_to_node(Base::get_current())->get_item();
      }

      Node * get_current() const
      {
	return &dl_to_node(Base::get_current())->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	GNode * p = dl_to_node(Base::get_current());
	Base::next();
	graph_ptr->remove_node(p);
      }
    };
    
    class ArcIterator : public DL::Iterator,
			public BidirectionalIterator<ArcIterator, Arc *, true>
    {
      friend class BasicIterator<ArcIterator, Arc *, true>;
      
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

      ArcIterator(const Graph & g, DL * curr)
	: Base(const_cast<DL *>(&g.arc_list), curr),
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
      
      Arc * get_current()
      {
	return &dl_to_arc(Base::get_current())->get_item();
      }
      
      Arc * get_current() const
      {
	return &dl_to_arc(Base::get_current())->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	GArc * a = dl_to_arc(Base::get_current());
	Base::next();
	graph_ptr->remove_arc(a);
      }
    };
    
    class AdjacentArcIterator : public DL::Iterator,
				public BidirectionalIterator<AdjacentArcIterator,
							     Arc *, true>
    {
      friend class BasicIterator<AdjacentArcIterator, Arc *, true>;
      
      using Base = DL::Iterator;
      
      Graph * graph_ptr;
      Node  * node_ptr;
      
    public:
      AdjacentArcIterator()
	: Base(), graph_ptr(nullptr), node_ptr(nullptr)
      {
	// empty
      }
      
      AdjacentArcIterator(const Graph & g, Node * n)
	: Base(const_cast<DL *>(&n->adjacent_arc_list)),
	  graph_ptr(const_cast<Graph *>(&g)), node_ptr(n)
      {
	// empty
      }

      AdjacentArcIterator(const Graph & g, Node * n, DL * curr)
	: Base(const_cast<DL *>(&n->adjacent_arc_list), curr),
	  graph_ptr(const_cast<Graph *>(&g)), node_ptr(n)
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
      
      Arc * get_current()
      {
	return &dl_to_adjacent_arc(Base::get_current())->get_item()->get_item();
      }

      Arc * get_current() const
      {
	return &dl_to_adjacent_arc(Base::get_current())->get_item()->get_item();
      }

      Node * get_src_node()
      {
	return node_ptr;
      }

      Node * get_src_node() const
      {
	return node_ptr;
      }

      Node * get_tgt_node()
      {
	return get_current()->get_connected_node(node_ptr);
      }

      const Node * get_tgt_node() const
      {
	return get_current()->get_connected_node(node_ptr);
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
      return NodeIterator(*this, &node_list);
    }

    const NodeIterator nodes_end() const
    {
      return NodeIterator(*this, const_cast<DL *>(&node_list));
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
      return ArcIterator(*this, &arc_list);
    }

    const ArcIterator arcs_end() const
    {
      return ArcIterator(*this, const_cast<DL *>(&arc_list));
    }

    AdjacentArcIterator arcs_begin(Node * p)
    {
      return AdjacentArcIterator(*this, p);
    }

    const AdjacentArcIterator arcs_begin(Node * p) const
    {
      return AdjacentArcIterator(*this, p);
    }

    AdjacentArcIterator arcs_end(Node * p)
    {
      return AdjacentArcIterator(*this, p, &p->adjacent_arc_list);
    }

    const AdjacentArcIterator arcs_end(Node * p) const
    {
      return AdjacentArcIterator(*this, p,
				 const_cast<DL *>(&p->adjacent_arc_list));
    }

    Arc * search_arc(Node *, Node *);
    
    template <class Cmp>
    void sort_nodes(Cmp & cmp)
    {
      using C = PtrCmp<Node, Cmp>;
      C c(cmp);
      quicksort<Node, C>(*dl_to_node(&node_list), c);
    }

    template <class Cmp>
    void sort_nodes(Cmp && cmp = Cmp())
    {
      sort_nodes<Cmp>(cmp);
    }

    template <class Cmp>
    void sort_arcs(Cmp & cmp)
    {
      using C = PtrCmp<Arc, Cmp>;
      C c(cmp);
      quicksort<Arc, C>(*dl_to_arc(&arc_list), c);
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
  Graph<NodeInfo, ArcInfo, GraphInfo>::search_arc(Node * s, Node * t)
  {
    for (AdjacentArcIterator it(*this, s); it.has_current(); it.next())
      if (it.get_tgt_node() == t)
	return *it;

    for (AdjacentArcIterator it(*this, t); it.has_current(); it.next())
      if (it.get_tgt_node() == s)
	return *it;

    return nullptr;
  }

  template <typename NodeInfo,
	    typename ArcInfo   = EmptyClass,
	    typename GraphInfo = EmptyClass>
  class Digraph : public BaseGraph<Digraph<NodeInfo, ArcInfo, GraphInfo>,
				   DigraphNode<NodeInfo, ArcInfo, GraphInfo>,
				   DigraphArc<
				     DigraphNode<NodeInfo, ArcInfo, GraphInfo>,
				     NodeInfo, ArcInfo, GraphInfo>,
				   NodeInfo, ArcInfo>
  {
    using Base = BaseGraph<Digraph<NodeInfo, ArcInfo, GraphInfo>,
			   DigraphNode<NodeInfo, ArcInfo, GraphInfo>,
			   DigraphArc<
			     DigraphNode<NodeInfo, ArcInfo, GraphInfo>,
			     NodeInfo, ArcInfo, GraphInfo>,
			   NodeInfo, ArcInfo>;
    
  public:
    using NodeInfoType  = NodeInfo;
    using ArcInfoType   = ArcInfo;
    using GraphInfoType = GraphInfo;
    using SetSizeType   = nat_t;

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

    static GNode * to_gnode(Node * node)
    {
      GNode * node_zero    = 0;
      nat_t   off_set      = (nat_t) &node_zero->get_item();
      nat_t   node_address = (nat_t) node;
      return (GNode *) (node_address - off_set); 
    }

    static GAdArc * to_garc(Arc * arc)
    {
      GAdArc * arc_zero    = 0;
      nat_t    off_set     = (nat_t) &arc_zero->get_item();
      nat_t    arc_address = (nat_t) arc;
      return (GAdArc *) (arc_address - off_set); 
    }
    
    GraphInfo info;
    nat_t     num_nodes;
    DL        node_list;
    nat_t     num_arcs;
    DL        arc_list;

    GNode * insert_gnode(GNode * p)
    {
      node_list.insert_prev(p);
      ++num_nodes;
      return p;
    }
    
    GAdArc * insert_garc(Node * src, Node * tgt)
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
      Base::copy_graph(g, *this);
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

    Node * get_first_node()
    {
      if (node_list.is_empty())
	throw std::underflow_error("Graph has not nodes");

      return &dl_to_node(node_list.get_next())->get_item();
    }

    Node * get_first_node() const
    {
      if (node_list.is_empty())
	throw std::underflow_error("Graph has not nodes");

      return &dl_to_node(const_cast<DL &>(node_list).get_next())->get_item();
    }

    Arc * get_first_arc()
    {
      if (arc_list.is_empty())
	throw std::underflow_error("Graph has not arcs");

      return &dl_to_arc(arc_list.get_next())->get_item()->get_item();
    }

    Arc * get_first_arc() const
    {
      if (arc_list.is_empty())
	throw std::underflow_error("Graph has not arcs");

      return &dl_to_arc(const_cast<DL &>(arc_list).get_next())->get_item()
	->get_item();
    }

    nat_t get_num_nodes() const
    {
      return num_nodes;
    }

    nat_t get_num_arcs() const
    {
      return num_arcs;
    }

    Node * insert_node()
    {
      GNode * node = insert_gnode(new GNode);
      return &node->get_item();
    }

    Node * insert_node(const NodeInfo & info)
    {
      GNode * node = insert_gnode(new GNode(Node(info)));
      return &node->get_item();
    }

    Node * insert_node(NodeInfo && info)
    {
      GNode * node = insert_gnode(new GNode(Node(std::forward<NodeInfo>(info))));
      return &node->get_item();
    }

    Arc * insert_arc(Node * s, Node * t)
    {
      GAdArc * arc = insert_garc(s, t);
      return &arc->get_item();
    }
    
    Arc * insert_arc(Node * src, Node * tgt, const ArcInfo & info)
    {
      Arc * arc = insert_arc(src, tgt);
      arc->get_info() = info;
      return arc;
    }

    Arc * insert_arc(Node * src, Node * tgt, ArcInfo && info)
    {
      Arc * arc = insert_arc(src, tgt);
      arc->get_info() = std::move(info);
      return arc;
    }

    void remove_arc(Arc * a)
    {
      GAdArc * arc = to_garc(a);
      remove_arc(arc);
    }

    void remove_node(Node * n)
    {
      GNode * node = to_gnode(n);
      remove_node(node);
    }
    
    class NodeIterator : public DL::Iterator,
                         public BidirectionalIterator<NodeIterator, Node *, true>
    {
      friend class BasicIterator<NodeIterator, Node *, true>;
      
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

      NodeIterator(const Digraph & g, DL * curr)
	: Base(const_cast<DL *>(&g.node_list), curr),
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

      Node * get_current()
      {
	return &dl_to_node(Base::get_current())->get_item();
      }

      Node * get_current() const
      {
	return &dl_to_node(Base::get_current())->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	GNode * p = dl_to_node(Base::get_current());
	Base::next();
	graph_ptr->remove_node(p);
      }
    };
    
    class ArcIterator : public DL::Iterator,      
                        public BidirectionalIterator<ArcIterator, Arc *, true>
    {
      friend class BasicIterator<ArcIterator, Arc *, true>;
      
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

      ArcIterator(const Digraph & g, DL * curr)
	: Base(const_cast<DL *>(&g.arc_list), curr),
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
      
      Arc * get_current()
      {
	return &dl_to_arc(Base::get_current())->get_item()->get_item();
      }
      
      Arc * get_current() const
      {
	return &dl_to_arc(Base::get_current())->get_item()->get_item();
      }

      void del()
      {
	if (not Base::has_current())
	  throw std::overflow_error("There is not current element");

	GArc * a = dl_to_arc(Base::get_current());
	Base::next();
	graph_ptr->remove_arc(a->get_item());
      }
    };
    
    class AdjacentArcIterator
      : public DL::Iterator,
	public BidirectionalIterator<AdjacentArcIterator, Arc *, true>
    {
      friend class BasicIterator<AdjacentArcIterator, Arc *, true>;
      
      using Base = DL::Iterator;
      
      Digraph * graph_ptr;
      Node    * node_ptr;
      
    public:
      AdjacentArcIterator()
	: Base(), graph_ptr(nullptr), node_ptr(nullptr)
      {
	// empty
      }
      
      AdjacentArcIterator(const Digraph & g, Node * n)
	: Base(const_cast<DL *>(&n->adjacent_arc_list)),
	  graph_ptr(const_cast<Digraph *>(&g)), node_ptr(n)
      {
	// empty
      }

      AdjacentArcIterator(const Digraph & g, Node * n, DL * curr)
	: Base(const_cast<DL *>(&n->adjacent_arc_list), curr),
	  graph_ptr(const_cast<Digraph *>(&g)), node_ptr(n)
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
      
      Arc * get_current()
      {
	return &dl_to_adjacent_arc(Base::get_current())->get_item();
      }

      const Arc * get_current() const
      {
	return &dl_to_adjacent_arc(Base::get_current())->get_item();
      }

      Node * get_src_node()
      {
	return node_ptr;
      }

      Node * get_src_node() const
      {
	return node_ptr;
      }

      Node * get_tgt_node()
      {
	return get_current()->get_tgt_node();
      }

      const Node * get_tgt_node() const
      {
	return get_current()->get_tgt_node();
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
      return NodeIterator(*this, &node_list);
    }

    const NodeIterator nodes_end() const
    {
      return NodeIterator(*this, const_cast<DL *>(&node_list));
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
      return ArcIterator(*this, &arc_list);
    }

    const ArcIterator arcs_end() const
    {
      return ArcIterator(*this, const_cast<DL *>(&arc_list));
    }

    AdjacentArcIterator arcs_begin(Node * p)
    {
      return AdjacentArcIterator(*this, p);
    }

    const AdjacentArcIterator arcs_begin(Node * p) const
    {
      return AdjacentArcIterator(*this, p);
    }

    AdjacentArcIterator arcs_end(Node * p)
    {
      return AdjacentArcIterator(*this, p, &p->adjacent_arc_list);
    }

    const AdjacentArcIterator arcs_end(Node * p) const
    {
      return AdjacentArcIterator(*this, p,
				 const_cast<DL *>(&p->adjacent_arc_list));
    }

    Arc * search_arc(Node *, Node *);
    
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
  Digraph<NodeInfo, ArcInfo, GraphInfo>::search_arc(Node * s, Node * t)
  {
    for (AdjacentArcIterator it(*this, s); it.has_current(); it.next())
      if (it.get_tgt_node() == t)
	return *it;

    return nullptr;
  }
  
} // end namespace Designar
    
# endif // DSGGRAPH_H
