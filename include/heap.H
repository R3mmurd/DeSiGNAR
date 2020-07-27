/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGHEAP_H
# define DSGHEAP_H

# include <array.H>
# include <queue.H>
# include <nodesdef.H>
# include <sort.H>

namespace Designar
{

  template <typename Key, class Cmp = std::less<Key>, nat_t cap = 100>
  class FixedHeap
  {
    nat_t num_items;
    Key   array[cap];
    Cmp & cmp;
    
    void copy(const FixedHeap &);
    
    void swap(FixedHeap & h)
    {
      std::swap(num_items, h.num_items);
      std::swap(array, h.array);
      std::swap(cmp, h.cmp);
    }

  public:
    using ItemType  = Key;
    using KeyType   = Key;
    using DataType  = Key;
    using ValueType = Key;
    using SizeType  = nat_t;
    using CmpType   = Cmp;
    
    FixedHeap(Cmp & _cmp)
      : num_items(0), cmp(_cmp)
    {
      // empty
    }

    FixedHeap(Cmp && _cmp = Cmp())
      : FixedHeap(_cmp)
    {
      // empty
    }

    FixedHeap(const FixedHeap & h)
      : num_items(h.num_items), cmp(h.cmp)
    {
      copy(h);
    }

    FixedHeap(FixedHeap && h)
      : FixedHeap()
    {
      swap(h);
    }

    FixedHeap & operator = (const FixedHeap & h)
    {
      if (this == &h)
	return *this;

      num_items = h.num_items;
      cmp = h.cmp;
      copy(h);
      return *this;
    }

    FixedHeap & operator = (FixedHeap && h)
    {
      swap(h);
      return *this;
    }

    Cmp & get_cmp()
    {
      return cmp;
    }

    const Cmp & get_cmp() const
    {
      return cmp;
    }
    
    void clear()
    {
      num_items = 0;
    }

    bool is_empty() const
    {
      return num_items == 0;
    }

    bool is_full() const
    {
      return num_items == cap;
    }

    nat_t size() const
    {
      return num_items;
    }

    nat_t get_capacity() const
    {
      return cap;
    }

    void insert(const Key & item)
    {
      if (is_full())
	throw std::overflow_error("Heap is full");
      
      array[num_items] = item;
      ++num_items;
      sift_up(array - 1, 1, num_items, cmp);
    }

    void insert(Key && item)
    {
      if (is_full())
	throw std::overflow_error("Heap is full");
      
      array[num_items] = std::move(item);
      ++num_items;
      sift_up(array - 1, 1, num_items, cmp);
    }

    const Key & top() const
    {
      if (is_empty())
	throw std::underflow_error("Heap is empty");
      
      return array[0];
    }

    Key get()
    {
      if (is_empty())
	throw std::underflow_error("Heap is empty");

      Key ret_val = std::move(array[0]);
      array[0] = std::move(array[num_items - 1]);
      --num_items;
      sift_down(array - 1, 1, num_items, cmp);
      return ret_val;
    }
  };

  template <typename Key, class Cmp, nat_t cap>
  void FixedHeap<Key, Cmp, cap>::copy(const FixedHeap & h)
  {
    for (nat_t i = 1; i <= cap; ++i)
      array[i] = h.array[i];
  }

  template <typename Key, class Cmp = std::less<Key>>
  class DynHeap : private DynArray<Key>
  {
    using BaseArray = DynArray<Key>;
    
    static void sift_up(BaseArray &, nat_t, nat_t, Cmp &);

    static void sift_down(BaseArray &, nat_t, nat_t, Cmp &);

    Cmp & cmp;
    
    void swap(DynHeap & h)
    {
      BaseArray::swap(h);
      std::swap(cmp, h.cmp);
    }

  public:
    using ItemType  = Key;
    using KeyType   = Key;
    using DataType  = Key;
    using ValueType = Key;
    using SizeType  = nat_t;
    using CmpType   = Cmp;
    
    DynHeap(Cmp & _cmp)
      : BaseArray(), cmp(_cmp)
    {
      // empty
    }

    DynHeap(Cmp && _cmp = Cmp())
      : DynHeap(_cmp)
    {
      // empty
    }

    DynHeap(const DynHeap & h)
      : BaseArray(h), cmp(h.cmp)
    {
      // empty
    }

    DynHeap(DynHeap && h)
      : DynHeap()
    {
      swap(h);
    }

    DynHeap & operator = (const DynHeap & h)
    {
      if (this == &h)
	return *this;

      (BaseArray &) *this = h;
      cmp = h.cmp;
      return *this;
    }

    DynHeap & operator = (DynHeap && h)
    {
      swap(h);
      return *this;
    }

    Cmp & get_cmp()
    {
      return cmp;
    }

    const Cmp & get_cmp() const
    {
      return cmp;
    }
    
    void clear()
    {
      BaseArray::clear();
    }

    bool is_empty() const
    {
      return BaseArray::is_empty();
    }

    nat_t size() const
    {
      return BaseArray::size();
    }

    void insert(const Key & item)
    {
      BaseArray::append(item);
      sift_up(*this, 1, size(), cmp);
    }

    void insert(Key && item)
    {
      BaseArray::append(std::forward<Key>(item));
      sift_up(*this, 1, size(), cmp);
    }

    const Key & top() const
    {
      if (is_empty())
	throw std::underflow_error("Heap is empty");
      
      return BaseArray::get_first();
    }

    Key get()
    {
      if (is_empty())
	throw std::underflow_error("Heap is empty");

      Key ret_val = std::move((*this)[0]);
      (*this)[0] = std::move(BaseArray::get_last());
      BaseArray::remove_last();
      sift_down(*this, 1, size(), cmp);
      return ret_val;
    }
  };

  template <typename Key, class Cmp>
  void DynHeap<Key, Cmp>::sift_up(BaseArray & a, nat_t l, nat_t r, Cmp & cmp)
  {
    nat_t i = r;

    nat_t u = i / 2;

    while (u >= l and cmp(a[i - 1], a[u - 1]))
      {
	std::swap(a[i - 1], a[u - 1]);
	i = u;
	u = i / 2;
      }
  }

  template <typename Key, class Cmp>
  void DynHeap<Key, Cmp>::sift_down(BaseArray & a, nat_t l, nat_t r, Cmp & cmp)
  {
    nat_t i = l;

    nat_t c = i * 2;

    while (c <= r)
      {
	if (c < r)
	  if (cmp(a[c], a[c - 1]))
	    ++c;

	if (not cmp(a[c - 1], a[i - 1]))
	  break;

	std::swap(a[c - 1], a[i - 1]);
	i = c;
	c = i * 2;
      }
  }

  template <typename Key>
  class HeapNode :
    public BaseBinTreeNode<Key, HeapNode<Key>, BinTreeNodeNullValue::NULLPTR>
  {
    using BaseNode =
      BaseBinTreeNode<Key, HeapNode<Key>, BinTreeNodeNullValue::NULLPTR>;
    
    struct HeapNodeBits
    {
      unsigned int is_left : 4;
      unsigned int is_leaf : 4;
      
      HeapNodeBits()
	: is_left(true), is_leaf(true)
      {
	// empty
      }
    };
    
    HeapNode   * parent = nullptr;
    HeapNodeBits bits;

  public:
    HeapNode()
      : BaseNode()
    {
      // empty
    }

    HeapNode(const Key & k)
      : BaseNode(k)
    {
      // empty
    }

    HeapNode(Key && k)
      : BaseNode(std::forward<Key>(k))
    {
      // empty
    }

    HeapNode(BinTreeNodeCtor ctor)
      : BaseNode(ctor)
    {
      // empty
    }

    HeapNode *& get_parent()
    {
      return parent;
    }

    HeapNodeBits & get_bits()
    {
      return bits;
    }

    unsigned int is_leaf() const
    {
      return bits.is_leaf;
    }

    void set_leaf(unsigned int value)
    {
      bits.is_leaf = value;
    }

    unsigned int is_left() const
    {
      return bits.is_left;
    }

    void set_left(unsigned int value)
    {
      bits.is_left = value;
    }

    void reset()
    {
      bits.is_leaf = true;
      bits.is_left = true;
    }
  };

  template <class HeapNode>
  inline HeapNode *& U(HeapNode * p)
  {
    return p->get_parent();
  }

  template <typename Key, class Cmp = std::less<Key>>
  class LHeap
  {
    using Node = HeapNode<Key>;

    static Node * key_to_node(Key & k)
    {
      Node * node_zero = 0;
      size_t offset = (size_t) &(KEY(node_zero));
      unsigned long addr = (unsigned long)(&k);
      return (Node*) (addr - offset);
    }

    static bool is_in_list(Node * p)
    {
      if (p->is_leaf())
	return true;

      return U(L(p)) == R(L(p));
    }

    static bool has_sibling(Node * p)
    {
      return U(p) != R(p);
    }

    static void destroy_rec(Node *, Node *);

    void swap_with_parent(Node * p)
    {
      assert(num_items >= 2);
      assert(p != root);

      Node * pp = U(p);

      const bool p_has_sibling = has_sibling(p); 
      const bool p_is_in_list  = is_in_list(p);
      const bool pp_is_in_list = is_in_list(pp);
      const bool p_has_child   = not p->is_leaf();

      std::swap(pp->get_bits(), p->get_bits());

      if (pp == root) 
	root = p;
      
      Node * ppp = U(pp);

      U(pp) = p;
      U(p)  = ppp;
      
      if (L(ppp) == pp)
	L(ppp) = p;
      else
	R(ppp) = p;
      
      Node * sp = nullptr;
      
      if (p_has_sibling) 
	{
	  sp = p == L(pp) ? R(pp) : L(pp);
	  assert(U(sp) == pp);
	  U(sp) = p;
	}

      if (p == last)
	last = pp;
    
      if (num_items == 2) 
	return;

      Node * lcp = L(p);
      Node * rcp = R(p);
   
      if (num_items == 3)
	{
	  if (R(pp) == p)
	    {
	      L(lcp) = R(lcp) = pp;
	      R(pp)  = lcp;
	      R(p)   = pp;
	    }
	  else
	    {
	      L(rcp) = R(rcp) = pp;
	      L(pp)  = rcp;
	      L(p)   = pp;
	    }
	  
	  return;
	}

      if (not p_is_in_list)
	{
	  U(lcp) = U(rcp) = pp;
	  
	  if (L(pp) == p)
	    {
	      assert(R(pp) == sp);
	      L(p) = pp;
	      R(p) = R(pp);
	    }
	  else
	    {
	      assert(L(pp) == sp);
	      R(p) = pp;
	      L(p) = L(pp);
	    }
	  
	  L(pp) = lcp;
	  R(pp) = rcp;
	  
	  return;
	}
      
      if (not pp_is_in_list)
	{
	  if (p_has_child)
	    U(L(p)) = pp;
	  
	  R(lcp) = L(rcp) = pp;
	  
	  if (L(pp) == p)
	    {
	      assert(R(pp) == sp);
	      L(p) = pp;
	      R(p) = R(pp);
	    }
	  else
	    {
	      assert(L(pp) == sp);
	      R(p)  = pp;
	      L(p)  = L(pp);
	    }
	  
	  L(pp) = lcp;
	  R(pp) = rcp;
	  
	  return;
	}
      
      R(lcp)       = pp;
      L(R(pp)) = p;
      L(pp)        = lcp;
      R(p)         = R(pp);
      R(pp)        = p;
      L(p)         = pp;
    }
    
    void swap_last_with_root()
    {
      assert(num_items > 1);
      assert(U(root) == head);
      assert(not root->is_leaf());
      assert(last->is_leaf());

      if (num_items > 3)
	{
	  Node * lRoot     = L(root);
	  Node * rRoot     = R(root);
	  Node * f_last    = U(last);
	  Node * prev_last = L(last);
	  Node * next_last = R(last);

	  if (L(f_last) == last)
	    L(f_last) = root;
	  else
	    R(f_last) = root;       
          
	  if (R(root) != last)
	    std::swap(U(root), U(last));
	  else
	    {
	      U(root) = last;
	      U(root) = head;
	    }

	  std::swap(L(root), L(last));
	  std::swap(R(root), R(last));
        
	  U(lRoot) = U(rRoot) = last;

	  L(last) = lRoot;
	  R(last) = rRoot;

	  L(root) = prev_last;
	  R(root) = next_last;

	  R(prev_last) = L(next_last) = root;
	}      
      else if (num_items == 3)
	{
	  assert(R(root) == last);
	  assert(L(last) == L(root) and R(last) == L(root));

	  U(last) = U(root);
	  U(root) = last;

	  Node *s_last  = L(last);
	  U(s_last) = last;

	  L(last) = s_last;
	  R(last) = root;

	  L(root) = R(root) = s_last;
	  R(s_last) = L(s_last) = root;
	}
      else
	{
	  assert(L(root) == last);

	  U(last) = U(root);
	  U(root) = last;
	  R(last) = L(last) = root;
	  R(root) = L(root) = last;
	}

      std::swap(root->get_bits(), last->get_bits());
      std::swap(root, last);
    }

    void replace_node(Node * node, Node * new_node)
    {
      assert(node != new_node);
      assert(node != last);

      Node * parent      = U(node);
      Node * left_child  = L(node);
      Node * right_child = R(node);
      
      U(new_node) = parent;
      L(new_node) = left_child;
      R(new_node) = right_child;

      if (node->is_left())
	{
	  assert(L(parent) == node);
	  L(parent) = new_node;
	}
      else 
	{
	  assert(R(parent) == node);
	  R(parent) = new_node;
	}

      if (node->is_leaf())
	{
	  R(left_child)  = new_node;
	  L(right_child) = new_node;
	}
      else 
	{
	  U(left_child) = new_node;

	  if (U(right_child) == node) // node pudiera tener sólo un hijo
	    U(right_child) = new_node;
	  else
	    {
	      assert(left_child == last);
	      R(left_child)  = new_node;
	      L(right_child) = new_node;
	    }
	}
      
      new_node->get_bits() = node->get_bits();
    }
    
    Node * insert_node(Node * p)
    {
      assert(p->is_leaf());

      if (root == nullptr)
	{
	  assert(num_items == 0);

	  root       = p;
	  L(p)       = R(p) = p;
	  U(p)       = head;
	  p->set_leaf(true);
	  p->set_left(false);
	  last       = root;
	  num_items  = 1;
	  return p;
	}

      Node * pp = R(last);
      L(p) = last;
      U(p) = pp;

      if (last->is_left()) 
	{
	  p->set_left(false);
	  R(p)       = R(pp);
	  L(R(pp))   = p;
	  R(pp)      = p;
	}
      else
	{
	  p->set_left(true);
	  R(p)        = pp;
	  pp->set_leaf(false);
	  L(pp)       = p;
	}
      
      assert(not pp->is_leaf());

      R(last) = p;
      last    = p;
      ++num_items;
      sift_up(last);
      return p;
    }

    Node * remove_top()
    {
      if (root == nullptr)
	throw std::underflow_error("Heap is empty");
      
      Node * p = root;
      
      if (num_items == 1)
	{
	  root = nullptr;
	  num_items = 0;
	}
      else
	{
	  swap_last_with_root();
	  remove_last();
	  sift_down(root);
	}

      p->reset();
      return p;
    }
    
    Node * remove_last()
    {
      assert(last != root and num_items > 0);
      assert(last->is_leaf());
      
      Node * ret_val  = last;
      Node * pp       = U(last);
      Node * new_last = L(last);
      
      if (last->is_left())
	{
	  pp->set_leaf(true);
	  L(pp)       = new_last;
	}
      else
	{
	  R(pp)      = R(last);
	  L(R(last)) = pp;
	}
      
      R(L(last)) = pp;
      last = new_last;
      --num_items;
      ret_val->reset();
      
      return ret_val;
    }

    Node * remove(Node * p)
    {
      if (root == nullptr)
	throw std::underflow_error ("Heap is empty");
      
      if (p == root) 
	return remove_top();
      
      if (p == last) 
	return remove_last();
      
      Node * q = remove_last();
      
      if (p == last)
	{
	  remove_last();
	  insert_node(q);
	  
	  return p;
	}
      
      replace_node(p, q);
      update(q);
      p->reset();
      
      return p;
    }
    
    void sift_up(Node *);
    
    void sift_down(Node *);

    void update(Node * p)
    {
      sift_down(p);
      sift_up(p);
    }
      
    
    Node    head_node;
    Node *  head;
    Node *& root;
    Node *  last;
    nat_t   num_items;
    Cmp   & cmp;

  public:
    using ItemType  = Key;
    using KeyType   = Key;
    using DataType  = Key;
    using ValueType = Key;
    using SizeType  = nat_t;
    using CmpType   = Cmp;
    
    LHeap(Cmp & _cmp)
      : head(&head_node), root(R(head)), last(nullptr), num_items(0), cmp(_cmp)
    {
      // empty
    }

    LHeap(Cmp && _cmp = Cmp())
      : LHeap(_cmp)
    {
      // empty
    }

    LHeap(LHeap && h)
      : LHeap()
    {
      swap(h);
    }

    ~LHeap()
    {
      clear();
    }

    LHeap & operator = (LHeap && h)
    {
      swap(h);
      return *this;
    }

    void swap(LHeap & h)
    {
      std::swap(root, h.root);
      std::swap(last, h.last);
      std::swap(num_items, h.num_items);
      std::swap(cmp, h.cmp);
    }

    Cmp & get_cmp()
    {
      return cmp;
    }

    const Cmp & get_cmp() const
    {
      return cmp;
    }

    void clear();

    nat_t size() const
    {
      return num_items;
    }

    bool is_empty() const
    {
      return num_items == 0;
    }

    const Key & insert(const Key & k)
    {
      Node * p = new Node(k);
      return KEY(insert_node(p));
    }

    const Key & insert(Key && k)
    {
      Node * p = new Node(std::forward<Key>(k));
      return KEY(insert_node(p));
    }

    const Key & top() const
    {
      if (is_empty())
	throw std::underflow_error("Heap is empty");

      return KEY(root);
    }

    Key get()
    {
      Node * p = remove_top();
      Key ret_val = std::move(KEY(p));
      delete p;
      return ret_val;
    }

    void remove(Key & item)
    {
      Node * p = remove(key_to_node(item));
      delete p;
    }
  };

  template <typename Key, class Cmp>
  void LHeap<Key, Cmp>::destroy_rec(Node * p, Node * n)
  {
    if (p->is_leaf())
      {
	delete p;
	return;
      }

    destroy_rec(L(p), n);

    if (p != n)
      destroy_rec(R(p), n);

    delete p;
  }

  template <typename Key, class Cmp>
  void LHeap<Key, Cmp>::sift_up(Node * p)
  {
    while (p != root and cmp(KEY(p), KEY(U(p))))
      swap_with_parent(p);
  }

  template <typename Key, class Cmp>
  void LHeap<Key, Cmp>::sift_down(Node * p)
  {
    while (not p->is_leaf())
      {
	Node * c = L(p);
	
	if (has_sibling(c))
	  if (cmp(KEY(R(p)), KEY(L(p))))
	    c = R(p);

	if (not cmp(KEY(c), KEY(p)))
	  return;

	swap_with_parent(c);
      }
  }

  template <typename Key, class Cmp>
  void LHeap<Key, Cmp>::clear()
  {
    if (root == nullptr)
      return;

    if (num_items <= 3)
      {
	while (not is_empty())
	  get();
	return;
      }

    if (last->is_left())
      destroy_rec(root, U(last));
    else
      destroy_rec(root, nullptr);

    root = nullptr;
    last = head;
    num_items = 0;
  }
  
} // end namespace Designar

# endif // DSGHEAP_H
