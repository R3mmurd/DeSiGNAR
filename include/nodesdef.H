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

# ifndef DSGNODESDEF_H
# define DSGNODESDEF_H

# include <types.H>

namespace Designar
{

  template<typename T>
  class SLNode
  {
    T        item;
    SLNode * next;

  public:
    SLNode()
      : item(), next(nullptr)
    {
      // empty
    }

    SLNode(const T & i)
      : item(i), next(nullptr)
    {
      // empty
    }

    SLNode(T && i)
      : item(std::forward<T>(i)), next(nullptr)
    {
      // empty
    }

    SLNode(const SLNode &) = delete;

    SLNode & operator = (const SLNode &) = delete;

    bool is_empty() const
    {
      return next == nullptr;
    }

    void reset()
    {
      next = nullptr;
    }
    
    T & get_item()
    {
      return item;
    }

    const T & get_item() const
    {
      return item;
    }

    SLNode *& get_next()
    {
      return next;
    }

    void insert_next(SLNode * p)
    {
      assert(p != nullptr);
      assert(p->is_empty());
      next = p;
    }

    SLNode * remove_next()
    {
      if (next == nullptr)
	return nullptr;

      SLNode * ret_val = next;
      next = ret_val->next;
      ret_val->reset();
      return ret_val;
    }
  };

  class DL
  {
    DL * next;
    DL * prev;

  public:
    DL()
      : next(this), prev(this)
    {
      // empty
    }

    DL(const DL &)
      : DL()
    {
      // empty
    }

    DL(DL && l)
      : DL()
    {
      swap(l);
    }

    DL & operator = (const DL &)
    {
      return *this;
    }

    DL & operator = (DL && l)
    {
      swap(l);
      return *this;
    }

    void reset()
    {
      next = prev = this;
    }

    bool is_empty() const
    {
      return next == prev and next == this;
    }

    bool is_unitarian_or_empty() const
    {
      return next == prev;
    }

    bool is_unitarian() const
    {
      return next == prev and next != this;
    }
    
    DL *& get_next()
    {
      return next;
    }

    DL *& get_prev()
    {
      return prev;
    }

    void insert_next(DL * node)
    {
      assert(node != nullptr);
      assert(node->is_empty());
      node->next = next;
      node->prev = this;
      next->prev = node;
      next = node;
    }

    void insert_prev(DL * node)
    {
      assert(node != nullptr);
      assert(node->is_empty());
      node->next = this;
      node->prev = prev;
      prev->next = node;
      prev = node;
    }

    void del()
    {
      next->prev = prev;
      prev->next = next;
      reset();
    }

    DL * remove_next()
    {
      DL * ret_val = next;
      ret_val->del();
      return ret_val;
    }
    
    DL * remove_prev()
    {
      DL * ret_val = prev;
      ret_val->del();
      return ret_val;
    }

    void swap(DL * node)
    {
      if (is_empty() and node->is_empty())
	return;

      if (is_empty())
	{
	  next = node->next;
	  prev = node->prev;
	  node->next->prev = node->prev->next = this;
	  node->reset();
	  return;
	}

      if (node->is_empty())
	{
	  node->next = next;
	  node->prev = prev;
	  next->prev = prev->next = node;
	  reset();
	  return;
	}

      std::swap(next->prev, node->next->prev);
      std::swap(prev->next, node->prev->next);
      std::swap(next, node->next);
      std::swap(prev, node->prev);
    }

    void swap(DL & node)
    {
      swap(&node);
    }

    void concat(DL * l)
    {
      if (l->is_empty())
	return;

      if (this->is_empty())
	{
	  this->next = l->next;
	  l->next->prev = this;
	  this->prev = l->prev;
	  l->prev->next = this;
	}
      else
	{
	  this->prev->next = l->next;
	  l->next->prev = this->prev;
	  l->prev->next = this;
	  this->prev = l->prev;
	}

      l->reset();
    }

    void concat(DL & l)
    {
      concat(&l);
    }

    void split(DL &, DL &);

    class Iterator
    {
      DL * head;
      DL * curr;
      
    public:
      Iterator()
	: head(nullptr), curr(nullptr)
      {
	// empty
      }
      
      Iterator(DL * h)
	: head(h), curr(h->get_next())
      {
	// empty
      }

      Iterator(DL * h, DL * c)
	: head(h), curr(c)
      {
	// empty
      }

      Iterator(const Iterator & it)
	: head(it.head), curr(it.curr)
      {
	// empty
      }

      Iterator(Iterator && it)
	: Iterator()
      {
	swap(it);
      }

      Iterator & operator = (const Iterator & it)
      {
	if (this == &it)
	  return *this;

	head = it.head;
	curr = it.curr;
	return *this;
      }

      Iterator & operator = (Iterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(Iterator & it)
      {
	std::swap(head, it.head);
	std::swap(curr, it.curr);
      }

      bool has_current() const
      {
	return curr != head;
      }

      DL * get_current()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return curr;
      }

      DL * get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return curr;
      }

      void next()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	curr = curr->get_next();
      }

      void prev()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	curr = curr->get_prev();
      }

      void reset_first()
      {
	curr = head->get_next(); 
      }

      void reset_last()
      {
	curr = head->get_prev();
      }

      DL * del()
      {
	DL * ret_val = curr;
	curr = curr->get_next();
	ret_val->del();
	return ret_val;
      }

      void operator ++ ()
      {
	next();
      }

      void operator -- ()
      {
	prev();
      }

      bool operator == (const Iterator & it) const
      {
	return head == it.head and curr == it.curr;
      }

      bool operator != (const Iterator & it) const
      {
	return not (it == *this);
      }

      DL * operator * ()
      {
	return get_current();
      }

      DL * operator * () const
      {
	return get_current();
      }
    };
  };

  template <typename T>
  class DLNode : public DL
  {
    T item;
    
  public:
    DLNode()
      : DL(), item()
    {
      // item
    }

    DLNode(const T & i)
      : DL(), item(i)
    {
      // empty
    }

    DLNode(T && i)
      : DL(), item(std::forward<T>(i))
    {
      // empty
    }

    DLNode(const DLNode &) = delete;

    DLNode(DLNode && n)
      : DLNode()
    {
      DL::swap(n);
    }

    DLNode & operator = (const DLNode &) = delete;

    DLNode & operator = (DLNode && n)
    {
      DL::swap(n);
      return *this;
    }

    T & get_item()
    {
      return item;
    }

    const T & get_item() const
    {
      return item;
    }

    DLNode *& get_next()
    {
      return (DLNode *&) DL::get_next();
    }

    DLNode *& get_prev()
    {
      return (DLNode *&) DL::get_prev();
    }

    DLNode * remove_next()
    {
      return static_cast<DLNode *>(DL::remove_next());
    }

    DLNode * remove_prev()
    {
      return static_cast<DLNode *>(DL::remove_prev());
    }

    class Iterator : public DL::Iterator
    {
      using Base = DL::Iterator;
      using Base::Base;
      
    public:
      DLNode * get_current()
      {
	return static_cast<DLNode *>(Base::get_current());
      }

      DLNode * get_current() const
      {
	return static_cast<DLNode *>(Base::get_current());
      }

      DLNode * del()
      {
	return static_cast<DLNode *>(Base::del());
      }

      DLNode * operator * ()
      {
	return get_current();
      }

      DLNode * operator * () const
      {
	return get_current();
      }
    };
  };

  enum class BinTreeNodeCtor { SENTINEL_CTOR };

  enum class BinTreeNodeNullValue { NULLPTR, SENTINEL };

  template <typename Key, class DerivedNodeType, BinTreeNodeNullValue NULL_VALUE>
  class BaseBinTreeNode
  {
    static DerivedNodeType sentinel_node;
    
  public:
    using KeyType = Key;

    static DerivedNodeType * const null;
    
  private:
    Key               key;
    DerivedNodeType * lchild;
    DerivedNodeType * rchild;

  public:
    BaseBinTreeNode()
      : key(), lchild(null), rchild(null)
    {
      // empty
    }

    BaseBinTreeNode(const Key & k)
      : key(k), lchild(null), rchild(null)
    {
      // empty
    }

    BaseBinTreeNode(Key && k)
      : key(std::forward<Key>(k)), lchild(null), rchild(null)
    {
      // empty
    }

    BaseBinTreeNode(BinTreeNodeCtor)
      : BaseBinTreeNode()
    {
      // empty
    }

    BaseBinTreeNode(const BaseBinTreeNode &) = delete;

    BaseBinTreeNode & operator = (const BaseBinTreeNode &) = delete;
    
    Key & get_key()
    {
      return key;
    }

    const Key & get_key() const
    {
      return key;
    }

    DerivedNodeType *& get_lchild()
    {
      return lchild;
    }

    DerivedNodeType *& get_rchild()
    {
      return rchild;
    }
  };

  template <typename Key, class DerivedNodeType, BinTreeNodeNullValue NULL_VALUE>
  DerivedNodeType BaseBinTreeNode<Key, DerivedNodeType, NULL_VALUE>::
  sentinel_node(BinTreeNodeCtor::SENTINEL_CTOR);

  template <typename Key, class DerivedNodeType, BinTreeNodeNullValue NULL_VALUE>
  DerivedNodeType * const
  BaseBinTreeNode<Key, DerivedNodeType, NULL_VALUE>::null =
    NULL_VALUE == BinTreeNodeNullValue::NULLPTR ? nullptr : &sentinel_node;

  template <class BinTreeNode>
  inline typename BinTreeNode::KeyType & KEY(BinTreeNode * p)
  {
    return p->get_key();
  }

  template <class BinTreeNode>
  inline BinTreeNode *& L(BinTreeNode * p)
  {
    return p->get_lchild();
  }

  template <class BinTreeNode>
  inline BinTreeNode *& R(BinTreeNode * p)
  {
    return p->get_rchild();
  }

  template <typename NodeInfo, class CommonGraphNodeArc>
  class BaseGraphNode : public CommonGraphNodeArc
  {
  protected:
    NodeInfo info;
    nat_t    num_arcs;
    DL       adjacent_arc_list;
    
    BaseGraphNode()
      : CommonGraphNodeArc(), info(), num_arcs(0), adjacent_arc_list()
    {
      // empty
    }
    
    BaseGraphNode(const NodeInfo & _info)
      : CommonGraphNodeArc(), info(_info), num_arcs(0), adjacent_arc_list()
    {
      // empty
    }
    
    BaseGraphNode(NodeInfo && _info)
      : CommonGraphNodeArc(), info(std::forward<NodeInfo>(_info)), num_arcs(0),
	adjacent_arc_list()
    {
      // empty
    }
    
    BaseGraphNode(BaseGraphNode * ptr)
      : CommonGraphNodeArc(), info(ptr->info), num_arcs(0), adjacent_arc_list()
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

  template <class GraphNode, typename ArcInfo, class CommonGraphNodeArc>
  class BaseGraphArc : public CommonGraphNodeArc
  {    
  protected:
    GraphNode    * src_node;
    GraphNode    * tgt_node;
    ArcInfo   info;

    BaseGraphArc()
      : src_node(nullptr), tgt_node(nullptr), info()
    {
      // empty
    }
    
    BaseGraphArc(GraphNode * src, GraphNode * tgt)
      : src_node(src), tgt_node(tgt), info()
    {
      // empty
    }
    
    BaseGraphArc(GraphNode * src, GraphNode * tgt, const ArcInfo & _info)
      : src_node(src), tgt_node(tgt), info(_info)
    {
      // empty
    }
    
    BaseGraphArc(GraphNode * src, GraphNode * tgt, ArcInfo && _info)
      : src_node(src), tgt_node(tgt), info(std::forward<ArcInfo>(_info))
    {
      // empty
    }
    
  public:
    GraphNode & get_src_node()
    {
      return *src_node;
    }
    
    const GraphNode & get_src_node() const
    {
      return *src_node;
    }
    
    GraphNode & get_tgt_node()
    {
      return *tgt_node;
    }
    
    const GraphNode & get_tgt_node() const
    {
      return *tgt_node;
    }
    
    GraphNode & get_connected_node(GraphNode & node)
    {
      if (&node == &get_src_node())
	return get_tgt_node();
      
      if (&node == &get_tgt_node())
	return get_src_node();
      
      throw std::logic_error("Arc is not adjacent to node");
    }
    
    const GraphNode & get_connected_node(GraphNode & node) const
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
  
} // end namespace Designar

# endif // DSGNODESDEF_H
