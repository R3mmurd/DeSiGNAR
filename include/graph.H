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

 /*!
    \class BaseGraph graph.H <graph.H>

    \~English
    \brief DeSIGNAR Graph Object. It contains all elemental operations
    on graphs and general set operations.

  
    \~Spanish
    \brief Grafo de DeSIGNAR. Contiene todas las operaciones elementales sobre
    grafos y operaciones generales sobre conjuntos haciendo uso de 
    cuantificadores.

  */

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
    
  public:

    static void copy_graph(const GT &, GT &);

    /*! \fn Node & nth_node(nat_t i)
     * \~English
     *
     *  \brief Get nth node of the graph in depth first order
     *  \param[in]  i  Position of the desired node
     *  \return A modifiable reference of the desired node
     * \~Spanish
     *
     *  \brief Obtiene el n-ésimo nodo del grafo
     *  \param[in]  i Posición del nodo deseado
     *  \return Una referencia modificable al nodo deseado

     */
    
    Node & nth_node(nat_t i)
    {
      return nth_it(me().nodes_begin(), me().nodes_end(), i);
    }

    /*! \fn const Node & nth_node(nat_t i) const
     * \~English
     *
     *  \brief Get nth node of the graph in depth first order
     *  \param[in]  i  Position of the desired node
     *  \return A constant reference of the desired node
     * \~Spanish
     *
     *  \brief Obtiene el n-ésimo nodo del grafo
     *  \param[in]  i Posición del nodo deseado
     *  \return Una referencia constante al nodo deseado

     */

    const Node & nth_node(nat_t i) const
    {
      return nth_it(const_me().nodes_begin(), const_me().nodes_end(), i);
    }

     /*! \fn void for_each_node(Op & op) const
     * \~English
     *
     *  \brief Apply an operation over all nodes of the graph
     *  \param[in]  op An operation to be executed for each node
     *  \return void
     * \~Spanish
     *
     *  \brief Aplica una operación sobre todos los nodos del grafo
     *  \param[in]  op Una operación a ejecutar en cada nodo
     *  \return Vacío

     */

    template <class Op>
    void for_each_node(Op & op) const
    {
      for_each_it(const_me().nodes_begin(), const_me().nodes_end(), op);
    }

//! \fn  void for_each_node(Op && op = Op()) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief  Similar a la anterior pero con una rvalue como parámetro
    
    template <class Op>
    void for_each_node(Op && op = Op()) const
    {
      for_each_it(const_me().nodes_begin(), const_me().nodes_end(),
		  std::forward<Op>(op));
    }

    template <class ContainerRet = SLList<NodeInfoType>, class Pred>
    ContainerRet filter_nodes(Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().nodes_begin(),
				     const_me().nodes_end(),
				     [] (const Node & p)
				     {
				       return p.get_info();
				     },
				     pred);
    }

    template <class ContainerRet = SLList<NodeInfoType>, class Pred>
    ContainerRet filter_nodes(Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().nodes_begin(),
				     const_me().nodes_end(),
				     [] (const Node & p)
				     {
				       return p.get_info();
				     },
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
      return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
				     op, pred);
    }

    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_nodes_if(Op & op, Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
				     op, std::forward<Pred>(pred));
    }
    
    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_nodes_if(Op && op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
				     std::forward<Op>(op), pred);
    }

    template <typename RetT = NodeInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_nodes_if(Op && op = Op(), Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
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
    
    /*! \fn bool all_nodes(Pred & pred) const
     * \~English
     *
     *  \brief Ask if all nodes of the graph satisfy a predicate
     *  \param[in] pred A predicate to be evaluated for each node
     *  \return `true` if for each node the predicate is true, `false` otherwise
     * \~Spanish
     *
     *  \brief Pregunta si todos los nodos del grafo satisfacen un predicado
     *  \param[in]  pred Un predicado a ser evaluado en cada nodo
     *  \return `true` si para cada nodo del grafo el predicado es verdadero, `false` en otro caso

     */
    template <class Pred>
    bool all_nodes(Pred & pred) const
    {
      return all_it(const_me().nodes_begin(), const_me().nodes_end(), pred);
    }

///! \fn   bool all_nodes(Pred && pred = Pred()) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
    template <class Pred>
    bool all_nodes(Pred && pred = Pred()) const
    {
      return all_it(const_me().nodes_begin(), const_me().nodes_end(),
		    std::forward<Pred>(pred));
    }

     /*! \fn bool exists_node(Pred & pred) const
     * \~English
     *
     *  \brief Ask if there is at least one node which satifies a predicate
     *  \param[in] pred A predicate to be evaluated for each node
     *  \return `true` if exist a node for the predicate is true, `false` otherwise
     * \~Spanish
     *
     *  \brief Pregunta si al menos un nodo del grafo satisface un predicado
     *  \param[in]  pred Un predicado a ser evaluado en cada nodo
     *  \return `true` si existe un nodo para el cual el predicado es verdadero,
     * `false` en otro caso

     */

    template <class Pred>
    bool exists_node(Pred & pred) const
    {
      return exists_it(const_me().nodes_begin(), const_me().nodes_end(), pred);
    }

/// \fn     bool exists_node(Pred && pred = Pred()) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
    template <class Pred>
    bool exists_node(Pred && pred = Pred()) const
    {
      return exists_it(const_me().nodes_begin(), const_me().nodes_end(),
		       std::forward<Pred>(pred));
    }

   /*! \fn  bool none_node(Pred & pred) const
     * \~English
     *
     *  \brief Ask if there is not a single node which satifies a predicate
     *  \param[in] pred A predicate to be evaluated for each node
     *  \return `true` if not exist a node for the predicate is true, `false` otherwise
     * \~Spanish
     *
     *  \brief Pregunta si al menos un nodo del grafo satisface un predicado
     *  \param[in]  pred Un predicado a ser evaluado en cada nodo
     *  \return `true` si no existe un nodo para el cual el predicado es verdadero,
     * `false` en otro caso

     */


    template <class Pred>
    bool none_node(Pred & pred) const
    {
      return none_it(const_me().nodes_begin(), const_me().nodes_end(), pred);
    }

///\fn bool none_node(Pred && pred = Pred()) const
//!<\~English \brief Similar to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
    template <class Pred>
    bool none_node(Pred && pred = Pred()) const
    {
      return none_it(const_me().nodes_begin(), const_me().nodes_end(),
		     std::forward<Pred>(pred));
    }
    
     /*! \fn  Node * search_node_ptr(Pred & pred)
     * \~English
     *
     *  \brief Search a node which satifies a predicate
     *  \param[in] pred A predicate to be evaluated for each node
     *  \return A pointer to the node which the predicate is true, `nullptr` otherwise
     * \~Spanish
     *
     *  \brief Busca un nodo que satisfaga un predicado
     *  \param[in]  pred Un predicado a ser evaluado en cada nodo
     *  \return Un puntero al nodo para el cual es predicado es verdadero, `nullptr` en otro caso

     */

    template <class Pred>
    Node * search_node_ptr(Pred & pred) const
    {
      return search_ptr_it<Node>(const_me().nodes_begin(),
				 const_me().nodes_end(),
				 pred);
    }

/// \fn Node * search_node_ptr(Pred && pred = Pred())
//!<\~English \brief Similar to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
    template <class Pred>
    Node * search_node_ptr(Pred && pred = Pred()) const
    {  
      return search_ptr_it(const_me().nodes_begin(), const_me().nodes_end(),
			   std::forward<Pred>(pred));
    }

  /*! \fn  bool remove_first_node_if(Pred & pred)
     * \~English
     *
     *  \brief Remove first node inserted in the graph if a predicate is true
     *  \param[in] pred A predicate to be evaluated for each node
     *  \return `true` if predicate is true for each node, `false` otherwise
     * \~Spanish
     *
     *  \brief Elimina el primer nodo insertado en el grafo si un predicado es verdadero
     *  \param[in]  pred Un predicado a ser evaluado en cada nodo
     *  \return Un puntero al nodo para el cual es predicado es verdadero, `nullptr` en otro caso

     */

    template <class Pred>
    bool remove_first_node_if(Pred & pred)
    {
      return remove_first_if_it(me().nodes_begin(), me().nodes_end(), pred);
    }

/// \fn bool remove_first_node_if(Pred && pred = Pred())
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
    template <class Pred>
    bool remove_first_node_if(Pred && pred = Pred())
    {
      return remove_first_if_it(me().nodes_begin(), me().nodes_end(),
				std::forward<Pred>(pred));
    }


  /*! \fn void remove_node_if(Pred & pred)
     * \~English
     *
     *  \brief Remove every node for which a predicate is true
     *  \param[in] pred A predicate to be evaluated for each node
     *  \return void
     * \~Spanish
     *
     *  \brief Elimina todos los nodos para los cuales un predicado es verdadero
     *  \param[in]  pred Un predicado a ser evaluado en cada nodo
     *  \return Vacío

     */

    template <class Pred>
    void remove_node_if(Pred & pred)
    {
      remove_if_it(me().nodes_begin(), me().nodes_end(), pred);
    }

/// \fn void remove_node_if(Pred && pred = Pred())
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
    template <class Pred>
    void remove_node_if(Pred && pred = Pred())
    {
      remove_if_it(me().nodes_begin(), me().nodes_end(),
		   std::forward<Pred>(pred));
    }


  /*! \fn SLList<Node *> nodes() const
     * \~English
     *
     *  \brief Get a DeSIGNAR Single Linked List containing all nodes of the graph
     *  \return A list containing all nodes of the graph
     * \~Spanish
     *
     *  \brief Devuelve una lista enlazada de DeSIGNAR que contiene todos los nodos del grafo
     *  \return Una lista conteniendo todos los nodos del grafo

     */
     
    SLList<Node *> nodes() const
    {
      return map_it<SLList<Node *>>(const_me().nodes_begin(),
				    const_me().nodes_end(),
				    [] (const Node & p)
				    {
				      return const_cast<Node *>(&p);
				    });
    }

  /*! \fn  Arc & nth_arc(nat_t i)
     * \~English
     *
     *  \brief Get the n-th arc of the graph
     *  \param[in] i The position of the arc in the E (edges) set
     *  \return A modifiable reference to the n-th arc
     * \~Spanish
     *
     *  \brief Obtiene el n-ésimo arco del grafo
     *  \param[in]  i La posición del arco en el conjunto E de los arcos
     *  \return Una referencia modificable al enésimo arco
     */

    Arc & nth_arc(nat_t i)
    {
      return nth_it(me().arcs_begin(), me().arcs_end(), i);
    }
  

    /// \fn const Arc & nth_arc(nat_t i) const
    //!<\~English \brief Similar to previous one but with a constant return value. \~Spanish \brief Similar a la anterior pero con un valor de retorno constante

    const Arc & nth_arc(nat_t i) const
    {
      return nth_it(const_me().arcs_begin(), const_me().arcs_end(), i);
    }

  /*! \fn  void for_each_arc(Op & op) const
     * \~English
     *
     *  \brief Apply an operation over every arc in the graph
     *  \param[in] op An operation to be executed over all arcs
     *  \return void
     * \~Spanish
     *
     *  \brief Aplica una operación sobre todos los arcos en el grafo
     *  \param[in]  op Una operación a ser ejecutada sobre cada arco
     *  \return Vacío
     */
  
    template <class Op>
    void for_each_arc(Op & op) const
    {
      for_each_it(const_me().arcs_begin(), const_me().arcs_end(), op);
    }
  

/// \fn void for_each_arc(Op && op) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   

    template <class Op>
    void for_each_arc(Op && op) const
    {
      for_each_it(const_me().arcs_begin(), const_me().arcs_end(),
		  std::forward<Op>(op));
    }

    template <class ContainerRet = SLList<ArcInfoType>, class Pred>
    ContainerRet filter_arcs(Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(),
				     const_me().arcs_end(),
				     [] (const Arc & a)
				     {
				       return a.get_info();
				     },
				     pred);
    }

    template <class ContainerRet = SLList<ArcInfoType>, class Pred>
    ContainerRet filter_arcs(Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(),
				     const_me().arcs_end(),
				     [] (const Arc & a)
				     {
				       return a.get_info();
				     },
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
      return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
				     op, pred);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_arcs_if(Op & op, Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
				     op, std::forward<Pred>(pred));
    }
    
    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_arcs_if(Op && op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
				     std::forward<Op>(op), pred);
    }
  
    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_arcs_if(Op && op = Op(), Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
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

 /*! \fn   bool all_arcs(Pred & pred) const
     * \~English
     *
     *  \brief Check if a predicate is true for all arcs
     *  \param[in] pred A predicate to be evaluated over all arcs
     *  \return `true` if the predicate is true for all arcs, `false` otherwise
     * \~Spanish
     *
     *  \brief Revisa si un predicado es verdadero para todos los arcos del grafo
     *  \param[in]  pred Un predicado a ser evaluado sobre todos los arcos
     *  \return `true` si el predicado es verdadero para todos los arcos, de lo contrario retorna `false`

     */
  
  
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

/// \fn bool all_arcs(Pred && pred) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
 
    template <class Pred>
    bool all_arcs(Pred && pred) const
    {
      return all_it(const_me().arcs_begin(), const_me().arcs_end(),
		    std::forward<Pred>(pred));
    }
  
/
 /*! \fn       bool exists_arc(Pred & pred) const
     * \~English
     *
     *  \brief Check if exist an arc which a given predicate is true
     *  \param[in] pred A predicate to be evaluated over all arcs
     *  \return `true` if the predicate is true for at least one arc, `false` otherwise
     * \~Spanish
     *
     *  \brief Revisa si existe un arco tal que el predicado dado sea verdadero
     *  \param[in]  pred Un predicado a ser evaluado sobre todos los arcos
     *  \return `true` si el predicado es verdadero en uno de los arcos, de lo contrario retorna `false`

     */

    template <class Pred>
    bool exists_arc(Pred & pred) const
    {
      return exists_it(const_me().arcs_begin(), const_me().arcs_end(), pred);
    }
  
/// \fn     bool exists_arc(Pred && pred) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
    template <class Pred>
    bool exists_arc(Pred && pred) const
    {
      return exists_it(const_me().arcs_begin(), const_me().arcs_end(),
		       std::forward<Pred>(pred));
    }

    /*! \fn       bool none_arc(Pred & pred) const
     * \~English
     *
     *  \brief Check if there is no arc which a given predicate is true
     *  \param[in] pred A predicate to be evaluated over all arcs
     *  \return `true` if the predicate is false for every arc, `false` otherwise
     * \~Spanish
     *
     *  \brief Revisa si no existe ningún arco tal que el predicado dado sea verdadero
     *  \param[in]  pred Un predicado a ser evaluado sobre todos los arcos
     *  \return `true` si el predicado es falso para todos los arcos, de lo contrario retorna `false`
     */


    template <class Pred>
    bool none_arc(Pred & pred) const
    {
      return none_it(const_me().arcs_begin(), const_me().arcs_end(), pred);
    }
    
/// \fn    bool none_arc(Pred && pred) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro
   
    template <class Pred>
    bool none_arc(Pred && pred) const
    {
      return none_it(const_me().arcs_begin(), const_me().arcs_end(),
		     std::forward<Pred>(pred));
    }

     /*! \fn       Arc * search_arc_ptr(Pred & pred)
     * \~English
     *
     *  \brief Search the pointer to the first arc which a given predicate is **true**
     *  \param[in] pred A predicate to be evaluated over all arcs
     *  \return A pointer to the first arc which **pred** is true
     * \~Spanish
     *
     *  \brief Busca el apuntador al primer arco tal que el predicado dado sea verdadero
     *  \param[in]  pred Un predicado a ser evaluado sobre todos los arcos
     *  \return Un apuntador al arco para el que **pred** sea verdadero
     */
  
    template <class Pred> 
    Arc * search_arc_ptr(Pred & pred) const
    {
      return search_ptr_it<Arc>(const_me().arcs_begin(),
				const_me().arcs_end(),
				pred);
    }
    /// \fn   Arc * search_arc_ptr(Pred && pred)
    //!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro 
    template <class Pred>
    Arc * search_arc_ptr(Pred && pred) const
    {
      return search_ptr_it(const_me().arcs_begin(), const_me().arcs_end(),
			   std::forward<Pred>(pred));
    }

      /*! \fn      bool remove_first_arc_if(Pred & pred)
     * \~English
     *
     *  \brief Remove the first arc of the graph if a given predicate is **true**
     *  \param[in] pred A predicate to be evaluated over all arcs
     *  \return `true` if removal was done, `false` otherwise
     * \~Spanish
     *
     *  \brief Elimina el primer arco del grafo si un predicado dado es verdadero
     *  \param[in]  pred Un predicado a ser evaluado sobre todos los arcos
     *  \return `true` si la remoción fue realizada, `false` en otro caso 
     */
  
    template <class Pred>
    bool remove_first_arc_if(Pred & pred)
    {
      return remove_first_if_it(me().arcs_begin(), me().arcs_end(), pred);
    }
  
   
    /// \fn   bool remove_first_arc_if(Pred && pred)
    //!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro  
    template <class Pred>
    bool remove_first_arc_if(Pred && pred)
    {
      return remove_first_if_it(me().arcs_begin(), me().arcs_end(),
				std::forward<Pred>(pred));
    }
    /*! \fn       void remove_arc_if(Pred & pred)
     * \~English
     *
     *  \brief Remove all arcs which predicate is **true**
     *  \param[in] pred A predicate to be evaluated over all arcs
     *  \return void
     * \~Spanish
     *
     *  \brief Elimina todos los arcos para los cuales un predicado dado sea verdadero
     *  \param[in]  pred Un predicado a ser evaluado sobre todos los arcos
     *  \return Vacío
     */
    template <class Pred>
    void remove_arc_if(Pred & pred)
    {
      remove_if_it(me().arcs_begin(), me().arcs_end(), pred);
    }

    /// \fn   void remove_arc_if(Pred && pred)
    //!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro  

    template <class Pred>
    void remove_arc_if(Pred && pred)
    {
      remove_if_it(me().arcs_begin(), me().arcs_end(), std::forward<Pred>(pred));
    }


    /*! \fn       SLList<Arc *> arcs() const
     * \~English
     *
     *  \brief Get all arc set in the graph
     *  \return A DeSIGNAR Single Linked List where are stored all arcs
     * \~Spanish
     *
     *  \brief Obtiene todos el conjunto de arcos en el grafo
     *  \return Una lista enlazada de DeSIGNAR donde están guardados todos los arcos
     */
  
    SLList<Arc *> arcs() const
    {
      return map_it<SLList<Arc *>>(const_me().arcs_begin(),
				   const_me().arcs_end(),
				   [] (const Arc & a)
				   {
				     return const_cast<Arc *>(&a);
				   });
    }

     /*! \fn   Arc & nth_adjacent_arc(Node & p, nat_t i)
     * \~English
     *
     *  \brief Get the nth adjacent arc of a given node
     *  \param[in] p A node in the graph
     *  \param[in] i The n-th adjacent arc of **p**
     *  \return A modifiable reference to the n-th adjacent arc of **p**
     * \~Spanish
     *
     *  \brief Obtiene el enésimo arco adyacente a un nodo dado
     *  \param[in] p Un nodo en el grafo
     *  \param[in] i El enésimo arco adyacente de **p**
     *  \return Una referencia modificable al enésimo arco adyacente de **p**
     */

    Arc & nth_adjacent_arc(Node & p, nat_t i)
    {
      return nth_it(me().arcs_begin(p), me().arcs_end(p), i);
    }
  

/// \fn  const Arc & nth_adjacent_arc(Node & p, nat_t i) const
//!<\~English \brief Similar  to previous one but with an non-modifiable return value. \~Spanish \brief Similar a la anterior pero con valor de retorno no modificable  
    const Arc & nth_adjacent_arc(Node & p, nat_t i) const
    {
      return nth_it(const_me().arcs_begin(p), const_me().arcs_end(p), i);
    }

      /*! \fn    void for_each_adjacent_arc(Node & p, Op & op) const
     * \~English
     *
     *  \brief Apply an specific operation over all adjacent arcs of a given node
     *  \param[in] p A node in the graph
     *  \param[in] op An operation to be executed on every adjacent arc of **p**
     *  \return void     
     * \~Spanish
     *
     *  \brief Aplica una operación específica sobre todos los arcos adyacentes de un nodo dado
     *  \param[in] p Un nodo en el grafo
     *  \param[in] op Una operación a ser ejecutada sobre todos los arcos adyacentes de **p**
     *  \return Vacío
     */

  
    template <class Op>
    void for_each_adjacent_arc(Node & p, Op & op) const
    {
      for_each_it(const_me().arcs_begin(p), const_me().arcs_end(p), op);
    }

  
/// \fn   void for_each_adjacent_arc(Node & p, Op && op) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro  

    template <class Op>
    void for_each_adjacent_arc(Node & p, Op && op) const
    {
      for_each_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		  std::forward<Op>(op));
    }

    template <class ContainerRet = SLList<ArcInfoType>, class Pred>
    ContainerRet filter_adjacent_arcs(Node & p, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(p),
				     const_me().arcs_end(p),
				     [] (const Arc & a)
				     {
				       return a.get_info();
				     },
				     pred);
    }

    template <class ContainerRet = SLList<ArcInfoType>, class Pred>
    ContainerRet filter_adjacent_arcs(Node & p, Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().arcs_begin(p),
				     const_me().arcs_end(p),
				     [] (const Arc & a)
				     {
				       return a.get_info();
				     },
				     std::forward<Pred>(pred));
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map_adjacent_arcs(Node & p, Op & op) const
    {
      return map_it<ContainerRet>(const_me().arcs_begin(p),
				  const_me().arcs_end(p), op);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>, class Op>
    ContainerRet map_adjacent_arcs(Node & p, Op && op = Op()) const
    {
      return map_it<ContainerRet>(const_me().arcs_begin(p),
				  const_me().arcs_end(p),
				  std::forward<Op>(op));
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_adjacent_arcs_if(Node & p, Op & op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().begin(p), const_me().end(p),
				     op, pred);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet
    map_adjacent_arcs_if(Node & p, Op & op, Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().begin(p), const_me().end(p),
				     op, std::forward<Pred>(pred));
    }
    
    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet map_adjacent_arcs_if(Node & p, Op && op, Pred & pred) const
    {
      return map_if_it<ContainerRet>(const_me().begin(p), const_me().end(p),
				     std::forward<Op>(op), pred);
    }

    template <typename RetT = ArcInfoType,
	      class ContainerRet = SLList<RetT>,
	      class Op, class Pred>
    ContainerRet
    map_adjacent_arcs_if(Node & p, Op && op = Op(), Pred && pred = Pred()) const
    {
      return map_if_it<ContainerRet>(const_me().begin(p), const_me().end(p),
				     std::forward<Op>(op),
				     std::forward<Pred>(pred));
    }

    template <typename RetT, class Op>
    RetT fold_adjacent_arcs(Node & p, const RetT & init_val, Op & op) const
    {
      return fold_it<RetT>(const_me().arcs_begin(p), const_me().arcs_end(p),
			   init_val, op);
    }

    template <typename RetT, class Op>
    RetT fold_adjacent_arcs(Node & p, const RetT & init_val, Op && op = Op()) const
    {
      return fold_it<RetT>(const_me().arcs_begin(p), const_me().arcs_end(p),
			   init_val, std::forward<Op>(op));
    }

    template <typename RetT, class Op>
    RetT fold_adjacent_arcs(Node & p, RetT && init_val, Op & op) const
    {
      return fold_it<RetT>(const_me().arcs_begin(p), const_me().arcs_end(p),
			   std::forward<RetT>(init_val), op);
    }

    template <typename RetT, class Op>
    RetT fold_adjacent_arcs(Node & p, RetT && init_val, Op && op = Op()) const
    {
      return fold_it<RetT>(const_me().arcs_begin(p), const_me().arcs_end(p),
			   std::forward<RetT>(init_val),
			   std::forward<Op>(op));
    }
  
 /*! \fn     bool all_adjacent_arcs(Node & p, Pred & pred) const
     * \~English
     *
     *  \brief Check if a given predicate is true for all adjacent arcs of a specific node
     *  \param[in] p A node in the graph
     *  \param[in] pred A predicate to be evaluated in every adjacent arc of **p**
     *  \return `true` if for each adjacent arc of **p** the predicate is true, `false` otherwise.     
     * \~Spanish
     *
     *  \brief Revisa si un predicado dado es verdadero para todos los arcos adyacentes de un nodo específico
     *  \param[in] p Un nodo en el grafo
     *  \param[in] pred Un predicado a ser evaluado sobre todos los arcos adyacentes de **p**
     *  \return `true` si para todos los arcos adyacentes de **p** el predicado es verdadero, `false` en otro caso.
     */

    template <class Pred>
    bool all_adjacent_arcs(Node & p, Pred & pred) const
    {
      return all_it(const_me().arcs_begin(p), const_me().arcs_end(p), pred);
    }
 
    /// \fn   bool all_adjacent_arcs(Node & p, Pred && pred) const
    //!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro  
 
    template <class Pred>
    bool all_adjacent_arcs(Node & p, Pred && pred) const
    {
      return all_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		    std::forward<Pred>(pred));
    }

    /*! \fn     bool exists_adjacent_arc(Node & p, Pred & pred) const
     * \~English
     *
     *  \brief Check if a given predicate is true for at least one adjacent arc of a specific node
     *  \param[in] p A node in the graph
     *  \param[in] pred A predicate to be evaluated in every adjacent arc of **p**
     *  \return `true` if exist at least one adjacent arc of **p** for which **pred** is true, `false` otherwise.     
     * \~Spanish
     *
     *  \brief Revisa si un predicado dado es verdadero para al menos un arco adyacente de un nodo específico
     *  \param[in] p Un nodo en el grafo
     *  \param[in] pred Un predicado a ser evaluado sobre todos los arcos adyacentes de **p**
     *  \return `true` si existe al menos un arco adyacente de **p** para el cual el predicado es verdadero, `false` en otro caso.
     */
  
    template <class Pred>
    bool exists_adjacent_arc(Node & p, Pred & pred) const
    {
      return exists_it(const_me().arcs_begin(p), const_me().arcs_end(p), pred);
    }
 
/// \fn   bool exists_adjacent_arc(Node & p, Pred && pred) const
//!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro  
  
    template <class Pred>
    bool exists_adjacent_arc(Node & p, Pred && pred) const
    {
      return exists_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		       std::forward<Pred>(pred));
    }

    /*! \fn     bool none_adjacent_arc(Node & p, Pred & pred) const
     * \~English
     *
     *  \brief Check if a given predicate is false for all adjacent arcs of a specific node
     *  \param[in] p A node in the graph
     *  \param[in] pred A predicate to be evaluated in every adjacent arc of **p**
     *  \return `true` if for each adjacent arc of **p** the predicate is false, `false` otherwise.     
     * \~Spanish
     *
     *  \brief Revisa si un predicado dado es falso para todos los arcos adyacentes de un nodo específico
     *  \param[in] p Un nodo en el grafo
     *  \param[in] pred Un predicado a ser evaluado sobre todos los arcos adyacentes de **p**
     *  \return `true` si para todos los arcos adyacentes de **p** el predicado es false, `false` en otro caso.
     */

    template <class Pred>
    bool none_adjacent_arc(Node & p, Pred & pred) const
    {
      return none_it(const_me().arcs_begin(p), const_me().arcs_end(p), pred);
    }
    
    /// \fn   bool none_adjacent_arc(Node & p, Pred && pred) const
    //!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro  
  
    template <class Pred>
    bool none_adjacent_arc(Node & p, Pred && pred) const
    {
      return none_it(const_me().arcs_begin(p), const_me().arcs_end(p),
		     std::forward<Pred>(pred));
    }

    /*! \fn   Arc * search_adjacent_arc_ptr(Node & p, Pred & pred)
     * \~English
     *
     *  \brief Search for the first adjacent arc for which a given predicate is true
     *  \param[in] p A node in the graph
     *  \param[in] pred A predicate to be evaluated in every adjacent arc of **p**
     *  \return A pointer to an adjacent arc of **p** for which **pred** is true.     
     * \~Spanish
     *
     *  \brief Busca el primer arco adyacente a un nodo para el cual un predicado dado es verdadedro
     *  \param[in] p Un nodo en el grafo
     *  \param[in] pred Un predicado a ser evaluado sobre todos los arcos adyacentes de **p**
     *  \return Un apuntador a un arco adyacente de **p** para el cual **pred** es verdadero.
     */
  
    template <class Pred>
    Arc * search_adjacent_arc_ptr(Node & p, Pred & pred) const
    {
      return search_ptr_it<Arc>(const_me().arcs_begin(p),
				const_me().arcs_end(p),
				pred);
    }
  

    /// \fn   Arc * search_adjacent_arc_ptr(Node & p, Pred && pred)
    //!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro  
    template <class Pred>
    Arc * search_adjacent_arc_ptr(Node & p, Pred && pred) const
    {
      return search_ptr_it<Arc>(const_me().arcs_begin(p),
				const_me().arcs_end(p),
				std::forward<Pred>(pred));
    }

    /*! \fn      bool remove_first_adjacent_arc_if(Node & p, Pred & pred)
     * \~English
     *
     *  \brief Remove the first adjacent arc of a specific node which a given predicate is true
     *  \param[in] p A node in the graph
     *  \param[in] pred A predicate to be evaluated in every adjacent arc of **p**
     *  \return `true` if removal was done, `false` otherwise.     
     * \~Spanish
     *
     *  \brief Elimina el primer arco adjacente de un nodo específico para el cual se cumple un predicado dado
     *  \param[in] p Un nodo en el grafo
     *  \param[in] pred Un predicado a ser evaluado sobre todos los arcos adyacentes de **p**
     *  \return `true` si la eliminación fue realizada, `false` en otro caso.
     */
  
    template <class Pred>
    bool remove_first_adjacent_arc_if(Node & p, Pred & pred)
    {
      return remove_first_if_it(me().arcs_begin(p), me().arcs_end(p), pred);
    }
  

    /// \fn   bool remove_first_adjacent_arc_if(Node & p, Pred && pred)
    //!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro  
    template <class Pred>
    bool remove_first_adjacent_arc_if(Node & p, Pred && pred)
    {
      return remove_first_if_it(me().arcs_begin(p), me().arcs_end(p),
				std::forward<Pred>(pred));
    }
  
  /*! \fn      void remove_adjacent_arc_if(Node & p, Pred & pred)
     * \~English
     *
     *  \brief Remove all adjacent arcs of a specific node for which a predicate evaluated in those arcs is true
     *  \param[in] p A node in the graph
     *  \param[in] pred A predicate to be evaluated in every adjacent arc of **p**
     *  \return void   
     * \~Spanish
     *
     *  \brief Elimina todos los arcos adjacentes de un nodo específico para los cuales un predicado evaluado en esos arcos sea verdadero
     *  \param[in] p Un nodo en el grafo
     *  \param[in] pred Un predicado a ser evaluado sobre todos los arcos adyacentes de **p**
     *  \return Vacío.
     */
    template <class Pred>
    void remove_adjacent_arc_if(Node & p, Pred & pred)
    {
      remove_if_it(me().arcs_begin(p), me().arcs_end(p), pred);
    }
  
    /// \fn   void remove_adjacent_arc_if(Node & p, Pred && pred)
    //!<\~English \brief Similar  to previous one but with an rvalue parameter. \~Spanish \brief Similar a la anterior pero con una rvalue como parámetro 
    template <class Pred>
    void remove_adjacent_arc_if(Node & p, Pred && pred)
    {
      remove_if_it(me().arcs_begin(p), me().arcs_end(p), std::forward<Pred>(pred));
    }


/*! \fn    SLList<Arc *> adjacent_arcs(Node & p) const
     * \~English
     *
     *  \brief Get all adjacent arcs of a given node
     *  \param[in] p A reference to node in the graph
     *  \return  A DeSIGNAR Linked List containing all adjacent arcs of **p** 
     * \~Spanish
     *
     *  \brief Obtiene todos los arcos adyacentes de un nodo dado
     *  \param[in] p Una referencia a un nodo en el grafo
     *  \return Una lista enlazada de DeSIGNAR conteniendo todos los nodos adyacentes de **p**.
     */
    SLList<Arc *> adjacent_arcs(Node & p) const
    {
      return map_it<SLList<Arc *>>(const_me().arcs_begin(p),
				   const_me().arcs_end(p),
				   [] (const Arc & a)
				   {
				     return const_cast<Arc *>(&a);
				   });
    }
    /*! \fn     void reset_all_node_tag(GraphTag tag)
     * \~English
     *
     *  \brief Reset all node tag info of a given graph tag
     *  \param[out] tag A GraphTag container
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de etiquetas de los nodos de un grafo de etiquetas
     *  \param[out] tag Un contenedor GraphTag
     *  \return Vacío
     */
    void reset_all_node_tag(GraphTag tag)
    {
      for_each_node([&tag](Node & node)
		    {
		      node.unvisit(tag);
		    });
    }

/*! \fn     void reset_all_node_tag()
     * \~English
     *
     *  \brief Reset all node tag info 
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de etiquetas de los nodos
     *  \return Vacío
     */

    void reset_all_node_tag()
    {
      for_each_node([](Node & node)
		    {
		      node.reset_tag();
		    });
    }
/*! \fn     void reset_all_arc_tag(GraphTag tag)
     * \~English
     *
     *  \brief Reset all arc tag info of a given graph tag
     *  \param[out] tag A GraphTag container
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de etiquetas de los arcos de un grafo de etiquetas
     *  \param[out] tag Un contenedor GraphTag
     *  \return Vacío
     */
    void reset_all_arc_tag(GraphTag tag)
    {
      for_each_arc([&tag](Arc & arc)
		   {
		     arc.unvisit(tag);
		   });
    }
/*! \fn     void reset_all_arc_tag()
     * \~English
     *
     *  \brief Reset all arc tag info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de etiquetas de los arcos
     *  \return Vacío
     */
    void reset_all_arc_tag()
    {
      for_each_arc([](Arc & arc)
		   {
		     arc.reset_tag();
		   });
    }
/*! \fn     void reset_tag(GraphTag tag)
     * \~English
     *
     *  \brief Reset all tag info from a given GraphTag
     *  \param[out] tag A GraphTag container
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de etiquetas
     *  \param[out] tag Un contenedor GraphTag
     *  \return Vacío
     */
    void reset_tag(GraphTag tag)
    {
      reset_all_node_tag(tag);
      reset_all_arc_tag(tag);
    }
/*! \fn     void reset_all_tags()
     * \~English
     *
     *  \brief Reset all tag info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de etiquetas
     *  \return Vacío
     */
    void reset_all_tags()
    {
      reset_all_node_tag();
      reset_all_arc_tag();
    }
/*! \fn     void reset_node_cookies()
     * \~English
     *
     *  \brief Reset node cookies info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de cookies de los nodos
     *  \return Vacío
     */
    void reset_node_cookies()
    {
      for_each_node([](Node & node)
		    {
		      node.cookie() = nullptr;
		    });
    }

/*! \fn     void reset_arc_cookies()
     * \~English
     *
     *  \brief Reset arc cookies info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de cookies de los arcos
     *  \return Vacío
     */

    void reset_arc_cookies()
    {
      for_each_arc([](Arc & arc)
		   {
		     arc.cookie() = nullptr;
		   });
    }
/*! \fn     void reset_node_counter()
     * \~English
     *
     *  \brief Reset node counter info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información del contador de nodos
     *  \return Vacío
     */
   void reset_node_counter()
    {
      for_each_node([](Node & node)
		    {
		      node.counter() = 0;
		    });
    }
/*! \fn     void reset_arc_counter()
     * \~English
     *
     *  \brief Reset arc counter info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información del contador de arcos
     *  \return Vacío
     */
    void reset_arc_counter()
    {
      for_each_arc([](Arc & arc)
		   {
		     arc.counter() = 0;
		   });
    }
/*! \fn     void reset_counters()
     * \~English
     *
     *  \brief Reset all counters info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de contadores
     *  \return Vacío
     */
    void reset_counters()
    {
      reset_node_counter();
      reset_arc_counter();
    }
/*! \fn     void reset_cookies()
     * \~English
     *
     *  \brief Reset all cookies info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información de cookies
     *  \return Vacío
     */
    void reset_cookies()
    {
      reset_node_cookies();
      reset_arc_cookies();
    }
/*! \fn     void reset_nodes()
     * \~English
     *
     *  \brief Reset all nodes info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información en los nodos
     *  \return Vacío
     */
    void reset_nodes()
    {
      for_each_node([](Node & node)
		    {
		      node.reset();
		    });
    }
/*! \fn     void reset_arcs()
     * \~English
     *
     *  \brief Reset all arcs info
     *  \return void     
     * \~Spanish
     *
     *  \brief Reestablece toda la información en los arcos
     *  \return Vacío
     */
    void reset_arcs()
    {
      for_each_arc([](Arc & arc)
		   {
		     arc.reset();
		   });
    }
  };

/*! \fn    void BaseGraph<GT, Node, Arc>::copy_graph(const GT & src, GT & tgt)
     * \~English
     *
     *  \brief Copy a given graph into another one
     *  \param[in] src The soource graph to be copied
     *  \param[out] tgt A target graph where src copy will be stored
     *  \return void.     
     * \~Spanish
     *
     *  \brief Copia un grafo dado en otro
     *  \param[in] src El grafo a ser copiado
     *  \param[out] tgt Un grafo objetivo donde se almacenará la copia
     *  \return Vacío.
     */

  template <typename GT, class Node, class Arc,
	    typename NodeInfoType, typename ArcInfoType>
  void BaseGraph<GT, Node, Arc, NodeInfoType, ArcInfoType>::
  copy_graph(const GT & src, GT & tgt)
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
    
    class NodeIterator : public DL::Iterator,
                         public BidirectionalIterator<NodeIterator, Node>
    {
      friend class BasicIterator<NodeIterator, Node>;
      
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
    };
    
    class ArcIterator : public DL::Iterator,
			public BidirectionalIterator<ArcIterator, Arc>
    {
      friend class BasicIterator<ArcIterator, Arc>;
      
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
    };
    
    class AdjacentArcIterator : public DL::Iterator,
				public BidirectionalIterator<AdjacentArcIterator,
							     Arc>
    {
      friend class BasicIterator<AdjacentArcIterator, Arc>;
      
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

      AdjacentArcIterator(const Graph & g, const Node & n, DL * curr)
	: Base(const_cast<DL *>(&n.adjacent_arc_list), curr),
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
      return AdjacentArcIterator(*this, p, &p.adjacent_arc_list);
    }

    const AdjacentArcIterator arcs_end(Node & p) const
    {
      return AdjacentArcIterator(*this, p,
				 const_cast<DL *>(&p.adjacent_arc_list));
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
    
    class NodeIterator : public DL::Iterator,
                         public BidirectionalIterator<NodeIterator, Node>
    {
      friend class BasicIterator<NodeIterator, Node>;
      
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
    };
    
    class ArcIterator : public DL::Iterator,      
                        public BidirectionalIterator<ArcIterator, Arc>
    {
      friend class BasicIterator<ArcIterator, Arc>;
      
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
    };
    
    class AdjacentArcIterator
      : public DL::Iterator,
	public BidirectionalIterator<AdjacentArcIterator, Arc>
    {
      friend class BasicIterator<AdjacentArcIterator, Arc>;
      
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

      AdjacentArcIterator(const Digraph & g, const Node & n, DL * curr)
	: Base(const_cast<DL *>(&n.adjacent_arc_list), curr),
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
      return AdjacentArcIterator(*this, p, &p.adjacent_arc_list);
    }

    const AdjacentArcIterator arcs_end(Node & p) const
    {
      return AdjacentArcIterator(*this, p,
				 const_cast<DL *>(&p.adjacent_arc_list));
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
