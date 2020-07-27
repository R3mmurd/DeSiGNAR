/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGNODESDEF_H
# define DSGNODESDEF_H

# include <types.H>
# include <iterator.H>

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

    const SLNode *& get_next() const
    {
      return next;
   }

    void insert_next(SLNode * p)
    {
      assert(p != nullptr);
      assert(p->is_empty());
      p->next = next;
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

    const DL *& get_next() const
    {
      return (const DL *&) next;
    }

    DL *& get_prev()
    {
      return prev;
    }

    const DL *& get_prev() const
    {
      return (const DL *&) prev;
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

    protected:
      DL * get_head() const
      {
	return head;
      }
      
      DL * get_location() const
      {
	return curr;
      }
      
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
	  return;
	
	curr = curr->get_next();
      }

      void prev()
      {
	if (curr == head->get_next())
	  return;

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

    const DLNode *& get_next() const
    {
      return (const DLNode *&) DL::get_next();
    }

    DLNode *& get_prev()
    {
      return (DLNode *&) DL::get_prev();
    }

    const DLNode *& get_prev() const
    {
      return (const DLNode *&) DL::get_prev();
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

  template <typename Key>
  class MTreeNode : private DLNode<Key>
  {
    struct SiblingInfo
    {
      unsigned int is_leftmost  : 4;
      unsigned int is_rightmost : 4;

      SiblingInfo()
	: is_leftmost(true), is_rightmost(true)
      {
	// empty
      }
    };

    MTreeNode * parent      = nullptr;
    MTreeNode * first_child = nullptr;
    SiblingInfo sibling_info;

    using Base = DLNode<Key>;

    static MTreeNode * to_treenode(Base * p)
    {
      return static_cast<MTreeNode *>(p);
    }

    void insert_first_child(MTreeNode * c)
    {
      assert(first_child == nullptr);
      c->parent = this;
      first_child = c;
    }
    
  public:
    using KeyType   = Key;
    using ValueType = Key;
    using ItemType  = Key;
    
    MTreeNode() = default;
    
    MTreeNode(const Key & k)
      : Base(k)
    {
      // empty
    }

    MTreeNode(Key && k)
      : Base(std::forward<Key>(k))
    {
      // empty
    }

    Key & get_key()
    {
      return Base::get_item();
    }

    const Key & get_key() const
    {
      return Base::get_item();
    }

    MTreeNode * get_first_child() const
    {
      return first_child;
    }

    MTreeNode * get_last_child() const
    {
      if (first_child == nullptr)
	return nullptr;

      return to_treenode(first_child->get_prev());
    }

    MTreeNode * get_right_sibling() const
    {
      if (sibling_info.is_rightmost)
	return nullptr;

      return to_treenode(const_cast<Base *>(Base::get_next()));
    }

    MTreeNode * get_left_sibling() const
    {
      if (sibling_info.is_leftmost)
	return nullptr;

      return to_treenode(const_cast<Base *>(Base::get_prev()));
    }

    bool is_leaf() const
    {
      return first_child == nullptr;
    }

    bool has_siblings() const
    {
      return not Base::is_empty();
    }

    bool has_parent() const
    {
      return parent != nullptr;
    }

    bool has_children() const
    {
      return first_child != nullptr;
    }

    void reset_sibling_info()
    {
      sibling_info.is_leftmost = sibling_info.is_rightmost = true;
    }

    void reset()
    {
      Base::reset();
      parent = first_child = nullptr;
      reset_sibling_info();
    }

    void add_right_sibling(MTreeNode * s)
    {
      assert(s != nullptr);
      assert(not s->has_siblings());
      assert(not s->has_parent());
      Base::insert_next(s);
      s->sibling_info.is_rightmost = sibling_info.is_rightmost;
      s->sibling_info.is_leftmost = false;
      sibling_info.is_rightmost = false;
      s->parent = parent;
    }

    void add_left_sibling(MTreeNode * s)
    {
      assert(s != nullptr);
      assert(not s->has_siblings());
      assert(not s->has_parent());
      Base::insert_prev(s);
      s->sibling_info.is_leftmost = sibling_info.is_leftmost;
      s->sibling_info.is_rightmost = false;
      sibling_info.is_leftmost = false;
      s->parent = parent;

      if (s->sibling_info.is_leftmost and parent)
	parent->first_child = s;
    }

    void insert_child(MTreeNode * c)
    {
      assert(c != nullptr);
      assert(not c->has_siblings());
      assert(not c->has_parent());
      
      if (first_child == nullptr)
	insert_first_child(c);
      else
	first_child->add_left_sibling(c);
    }

    void append_child(MTreeNode * c)
    {
      assert(c != nullptr);
      assert(not c->has_siblings());
      assert(not c->has_parent());
	    
      if (first_child == nullptr)
	insert_first_child(c);
      else
	to_treenode(first_child->get_prev())->add_right_sibling(c);
    }

    MTreeNode * remove_first_child()
    {
      if (first_child == nullptr)
	return nullptr;

      MTreeNode * ret_val = first_child;

      if (ret_val->sibling_info.is_rightmost)
	first_child = nullptr;
      else
	{
	  first_child = to_treenode(ret_val->get_next());
	  first_child->sibling_info.is_leftmost = true;
	  ret_val->del();
	}

      ret_val->parent = nullptr;
      ret_val->reset_sibling_info();
      return ret_val;
    }

    MTreeNode * remove_last_child()
    {
      if (first_child == nullptr)
	return nullptr;

      MTreeNode * ret_val = to_treenode(first_child->get_prev());

      if (ret_val->sibling_info.is_leftmost)
	first_child = nullptr;
      else
	{
	  ret_val->get_left_sibling()->sibling_info.is_rightmost = true;
	  ret_val->del();
	}

      ret_val->parent = nullptr;
      ret_val->reset_sibling_info();
      return ret_val;
    }

    class ChildrenIterator :
      public BidirectionalIterator<ChildrenIterator, MTreeNode *, true>
    {
      friend class BasicIterator<ChildrenIterator, MTreeNode *, true>;

      MTreeNode * first;
      MTreeNode * curr;

    protected:
      MTreeNode * get_location() const
      {
	return curr;
      }

    public:
      ChildrenIterator()
	: first(nullptr), curr(nullptr)
      {
	// empty
      }

      ChildrenIterator(const MTreeNode & node)
	: first(const_cast<MTreeNode *>(&node)->get_first_child()), curr(first)
      {
	// empty
      }

      ChildrenIterator(const ChildrenIterator & it)
	: first(it.first), curr(it.curr)
      {
	// empty
      }

      ChildrenIterator(ChildrenIterator && it)
	: ChildrenIterator()
      {
	swap(it);
      }

      ChildrenIterator & operator = (const ChildrenIterator & it)
      {
	if (this == &it)
	  return *this;

	first = it.first;
	curr  = it.curr;
	return *this;
      }

      ChildrenIterator & operator = (ChildrenIterator && it)
      {
	swap(it);
	return *this;
      }

      void swap(ChildrenIterator & it)
      {
	std::swap(first, it.first);
	std::swap(curr, it.curr);
      }

      bool has_current() const
      {
	return curr != nullptr;
      }

      MTreeNode * get_current()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return curr;
      }

      MTreeNode * get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return curr;
      }

      void next()
      {
	if (not has_current())
	  throw std::out_of_range("There is not next element");

	curr = curr->get_right_sibling();
      }

      void prev()
      {
	if (curr == first)
	  throw std::out_of_range("There is not previous element");

	if (curr == nullptr)
	  curr = to_treenode(first->get_prev());
	else
	  curr = curr->get_left_sibling();
      }
    };

    template <class Op>
    void for_each_child(Op &) const;

    template <class Op>
    void for_each_child(Op && op = Op()) const
    {
      for_each_child<Op>(op);
    }

    static void destroy_tree(MTreeNode *&);
  };

  template <typename Key>
  template <class Op>
  void MTreeNode<Key>::for_each_child(Op & op) const
  {
    MTreeNode * ptr = first_child;

    while (ptr != nullptr)
      {
	op(ptr);
	ptr = ptr->get_right_sibling();
      }
  }

  template <typename Key>
  void MTreeNode<Key>::destroy_tree(MTreeNode *& r)
  {
    if (r == nullptr)
      return;

    MTreeNode * fc = nullptr;
    
    while ((fc = r->remove_first_child()))
      destroy_tree(fc);

    delete r;
    r = nullptr;
  }

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

    void reset()
    {
      lchild = rchild = null;
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
    GraphNode * get_src_node()
    {
      return src_node;
    }
    
    GraphNode * get_src_node() const
    {
      return src_node;
    }
    
    GraphNode * get_tgt_node()
    {
      return tgt_node;
    }
    
    GraphNode * get_tgt_node() const
    {
      return tgt_node;
    }
    
    GraphNode * get_connected_node(GraphNode * node)
    {
      if (node == get_src_node())
	return get_tgt_node();
      
      if (node == get_tgt_node())
	return get_src_node();
      
      throw std::logic_error("Arc is not adjacent to node");
    }
    
    GraphNode * get_connected_node(GraphNode * node) const
    {
      if (node == get_src_node())
	return get_tgt_node();
      
      if (node == get_tgt_node())
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
