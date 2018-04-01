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

# ifndef DSGTREE_H
# define DSGTREE_H

# include <nodesdef.H>
# include <containeralgorithms.H>
# include <setalgorithms.H>
# include <stack.H>
# include <iterator.H>

namespace Designar
{
  template <typename Key>
  class TreapRkNode : public BaseBinTreeNode<Key, TreapRkNode<Key>,
					     BinTreeNodeNullValue::SENTINEL>
  {
    using BaseNode = BaseBinTreeNode<Key, TreapRkNode<Key>,
				     BinTreeNodeNullValue::SENTINEL>;
    
    nat_t      count;
    rng_seed_t prior;
    
  public:
    TreapRkNode()
      : BaseNode()
    {
      // empty
    }
    
    TreapRkNode(const Key & k)
      : BaseNode(k), count(1), prior(0)
    {
      // empty
    }
    
    TreapRkNode(Key && k)
      : BaseNode(std::forward<Key>(k)), count(1), prior(0)
    {
      // empty
    }
    
    TreapRkNode(BinTreeNodeCtor ctor)
      : BaseNode(ctor), count(0), prior(rng_t::max())
    {
      // empty
    }
    
    nat_t & get_count()
    {
      return count;
    }
    
    rng_seed_t & get_priority()
    {
      return prior;
    }
  };

  template <class RkNode>
  inline nat_t & COUNT(RkNode * p)
  {
    return p->get_count();
  }

  template <class TreapNode>
  inline rng_seed_t & PRIOR(TreapNode * p)
  {
    return p->get_priority();
  }
  
  
  template <typename Key, class Cmp = std::less<Key>>
    class TreeSet : public ContainerAlgorithms<TreeSet<Key, Cmp>, Key>,
		    public SetAlgorithms<TreeSet<Key, Cmp>, Key>
  {
  public:
    using Node = TreapRkNode<Key>;
      
  private:    
    Node    head;
    Node *& root;
    Cmp   & cmp;
    
    rng_t rng;

    static bool verify(Node *, Cmp & cmp);

    static Node * copy(Node *);

    static void destroy(Node *&);
    
    static Node * rotate_left(Node *);

    static Node * rotate_right(Node *);

    static bool split_key(Node *, const Key &, Node *&, Node *&, Cmp &);

    static Node * exclusive_join(Node *&, Node *&);
    
    static Node * insert(Node *&, Node *, Cmp &);

    static Node * search(Node *, const Key &, Cmp &);

    static Node * search(Node *, Key &&, Cmp &);

    static Node * search_or_insert(Node *&, Node *, Cmp &);

    static Node * remove_root(Node *& root)
    {
      Node * ret_val = root;
      root = exclusive_join(L(root), R(root));
      return ret_val;
    }

    static Node * remove(Node *&, const Key &, Cmp &);

    static Node * remove_pos(Node *&, nat_t);

    static Node * select(Node *, nat_t);

    static lint_t position(Node *, const Key &, Cmp &);
    
    static Node * min(Node *);

    static Node * max(Node *);

    template <class Op>
    static void preorder_rec(Node *, Op &);

    template <class Op>
    static void inorder_rec(Node *, Op &);

    template <class Op>
    static void postorder_rec(Node *, Op &);
    
    Key * insert(Node * p)
    {
      PRIOR(p) = rng();
	
      if (insert(root, p, cmp) != Node::null)
	return &KEY(p);
      
      delete p;
      return nullptr;
    }

    Key * search_or_insert(Node * p)
    {
      PRIOR(p) = rng();
	
      Node * result = search_or_insert(root, p, cmp);

      if (p != result)
	delete p;

      return &KEY(result);
    }

  public:
    using ItemType  = Key;
    using KeyType   = Key;
    using DataType  = Key;
    using ValueType = Key;
    using SizeType  = nat_t;
    using CmpType   = Cmp;
    
    bool verify() const
    {
      return verify(root, cmp);
    }
    
    TreeSet(rng_seed_t seed, Cmp & _cmp)
      : head(), root(L(&head)), cmp(_cmp), rng(seed)
    {
      // empty
    }

    TreeSet(Cmp & _cmp)
      : TreeSet(time(nullptr), _cmp)
    {
      // empty
    }

    TreeSet(Cmp && _cmp = Cmp())
      : TreeSet(_cmp)
    {
      // empty
    }

    TreeSet(rng_seed_t seed, Cmp && _cmp = Cmp())
      : TreeSet(seed, _cmp)
    {
      // empty
    }

    TreeSet(const TreeSet & t)
      : TreeSet(t.cmp)
    {
      root = copy(t.root);
    }

    TreeSet(TreeSet && t)
      : TreeSet()
    {
      swap(t);
    }

    TreeSet(const std::initializer_list<Key> &);

    ~TreeSet()
    {
      clear();
    }

    TreeSet & operator = (const TreeSet & t)
    {
      if (this == &t)
	return *this;

      clear();
      root = copy(t.root);
      cmp = t.cmp;
      return *this;
    }

    TreeSet & operator = (TreeSet && t)
    {
      swap(t);
      return *this;
    }

    void swap(TreeSet & t)
    {
      std::swap(root, t.root);
      std::swap(cmp, t.cmp);
    }

    bool is_empty() const
    {
      return root == Node::null;
    }

    bool is_sorted() const
    {
      return true;
    }

    nat_t size() const
    {
      return COUNT(root);
    }

    void clear()
    {
      destroy(root);
    }

    Cmp & get_cmp()
    {
      return cmp;
    }

    const Cmp & get_cmp() const
    {
      return cmp;
    }

    Key * insert(const Key & k)
    {
      Node * p = new Node(k);
      return insert(p);
    }

    Key * insert(Key && k)
    {
      Node * p = new Node(std::forward<Key>(k));
      return insert(p);
    }

    Key * append(const Key & k)
    {
      return insert(k);
    }

    Key * append(Key && k)
    {
      return insert(std::forward<Key>(k));
    }

    Key * search(const Key & k)
    {
      Node * result = search(root, k, cmp);
      
      if (result == Node::null)
	return nullptr;

      return &KEY(result);	  
    }

    const Key * search(const Key & k) const
    {
      Node * result = search(root, k, cmp);
      
      if (result == Node::null)
	return nullptr;

      return &KEY(result);	  
    }

    Key * search_or_insert(const Key & k)
    {
      Node * p = new Node(k);
      return search_or_insert(p);
    }

    Key * search_or_insert(Key && k)
    {
      Node * p = new Node(std::forward<Key>(k));
      return search_or_insert(p);
    }

    Key & find(const Key & k)
    {
      Key * result = search(k);

      if (result == nullptr)
	throw std::domain_error("Key not found");

      return *result;
    }

    const Key & find(const Key & k) const
    {
      const Key * result = search(k);

      if (result == nullptr)
	throw std::domain_error("Key not found");

      return *result;
    }

    bool remove(const Key & k)

    {
      Node * result = remove(root, k, cmp);

      if (result == Node::null)
	return false;

      delete result;
      return true;
    }

    Key remove_pos(nat_t i)
    {
      if (i >= size())
	throw std::out_of_range("Infix position is out of range");
      
      Node * result = remove_pos(root, i);
      Key ret_val = std::move(KEY(result));
      delete result;
      return ret_val;
    }

    const Key & min() const
    {
      if (is_empty())
	throw std::underflow_error("Tree is empty");

      return KEY(min(root));
    }

    const Key & max() const
    {
      if (is_empty())
	throw std::underflow_error("Tree is empty");

      return KEY(max(root));
    }

    std::tuple<TreeSet, TreeSet> split_key(const Key & k)
    {
      TreeSet ts, tg;

      if (split_key(root, k, ts.root, tg.root, cmp))
	root = Node::null;

      return std::make_tuple(ts, tg);
    }

    void exclusive_join(TreeSet & ts, TreeSet & tg)
    {
      root = exclusive_join(ts.root, tg.root);
      ts.root = tg.root = Node::null;
    }

    Key & select(nat_t i)
    {
      if (i >= size())
	throw std::out_of_range("Infix position is out of range");

      return KEY(select(root, i));
    }

    const Key & select(nat_t i) const
    {
      if (i >= size())
	throw std::out_of_range("Infix position is out of range");

      return KEY(select(root, i));
    }

    lint_t position(const Key & k) const
    {
      return position(root, k, cmp);
    }

    Key & operator [] (nat_t i)
    {
      return select(i);
    }

    const Key & operator [] (nat_t i) const
    {
      return select(i);
    }

    template <class Op>
    void for_each_preorder(Op & op)
    {
      preorder_rec<Op>(root, op);
    }

    template <class Op>
    void for_each_preorder(Op && op = Op())
    {
      for_each_preorder<Op>(op);
    }

    template <class Op>
    void for_each_inorder(Op & op)
    {
      inorder_rec<Op>(root, op);
    }

    template <class Op>
    void for_each_inorder(Op && op = Op())
    {
      for_each_inorder<Op>(op);
    }

    template <class Op>
    void for_each_postorder(Op & op)
    {
      postorder_rec<Op>(root, op);
    }

    template <class Op>
    void for_each_postorder(Op && op = Op())
    {
      for_each_postorder<Op>(op);
    }

    class PreorderIterator
    {
      friend class TreeSet;
      
      DynStack<Node *> stack;
      Node * root = Node::null;
      Node * curr = Node::null;
      lint_t pos = 0;

      Node * last(Node *);

    protected:
      PreorderIterator(const TreeSet & t, int)
	: root(t.root), curr(Node::null), pos(t.size())
      {
	// empty
      }
      
      Node * get_location() const
      {
	return curr;
      }

    public:
      PreorderIterator(const TreeSet & t)
	: root(t.root), curr(root)
      {
	// empty
      }

      PreorderIterator(const PreorderIterator & it)
	: stack(it.stack), root(it.root), curr(it.curr), pos(it.pos)
      {
	// empty
      }

      PreorderIterator(PreorderIterator && it)
      {
	swap(it);
      }

      PreorderIterator & operator = (const PreorderIterator & it)
      {
	if (this == &it)
	  return *this;

	stack = it.stack;
	root = it.root;
	curr = it.curr;
	pos = it.pos;

	return *this;
      }

      PreorderIterator & operator = (PreorderIterator && it)
      {
	swap(it);
	return *this;
      }
      
      void swap(PreorderIterator & it)
      {
	std::swap(stack, it.stack);
	std::swap(root, it.root);
	std::swap(curr, it.curr);
	std::swap(pos, it.pos);
      }

      void reset()
      {
	stack.clear();
	pos = 0;
	curr = root;
      }

      nat_t get_position() const
      {
	return pos;
      }

      bool has_current() const
      {
	return curr != Node::null;
      }

      Key & get_current()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return KEY(curr);
      }

      const Key & get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return KEY(curr);
      }

      void next()
      {
	if (not has_current())
	  throw std::out_of_range("There is not next element");

	stack.push(curr);

	curr = L(curr);

	++pos;

	if (curr != Node::null)
	  return;

	while (not stack.is_empty() and curr == Node::null)
	  curr = R(stack.pop());	  
      }
      
      Key del()
      {
	return Key();
      }
    };

    class InorderIterator
    {
      friend class TreeSet;
      
      DynStack<Node *> stack;
      Node * root = Node::null;
      Node * curr = Node::null;
      lint_t pos = 0;

      Node * search_min(Node *);

      Node * search_max(Node *);

      void init()
      {
	if (root == Node::null)
	  return;

	pos = 0;
	curr = search_min(root);
      }

    protected:
      InorderIterator(const TreeSet & t, int)
	: root(t.root), curr(Node::null), pos(t.size())
      {
	// empty
      }
      
      Node * get_location() const
      {
	return curr;
      }

    public:
      InorderIterator(const TreeSet & t)
	: root(t.root)
      {
	init();
      }

      InorderIterator(const InorderIterator & it)
	: stack(it.stack), root(it.root), curr(it.curr), pos(it.pos)
      {
	// empty
      }

      InorderIterator(InorderIterator && it)
      {
	swap(it);
      }

      InorderIterator & operator = (const InorderIterator & it)
      {
	if (this == &it)
	  return *this;

	stack = it.stack;
	root = it.root;
	curr = it.curr;
	pos = it.pos;

	return *this;
      }

      InorderIterator & operator = (InorderIterator && it)
      {
	swap(it);
	return *this;
      }
      
      void swap(InorderIterator & it)
      {
	std::swap(stack, it.stack);
	std::swap(root, it.root);
	std::swap(curr, it.curr);
	std::swap(pos, it.pos);
      }

      void reset()
      {
	stack.clear();
	init();
      }

      void reset_last()
      {
	pos = -1;
	stack.clear();
	curr = search_max(root);
      }

      nat_t get_position() const
      {
	return pos;
      }

      bool has_current() const
      {
	return curr != Node::null;
      }

      Key & get_current()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return KEY(curr);
      }

      const Key & get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return KEY(curr);
      }

      void next()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	++pos;
	curr = R(curr);
	
	if (curr != Node::null)
	  curr = search_min(curr);
	else
	  if (not stack.is_empty())
	    curr = stack.pop();
      }

      Key del()
      {
	return Key();
      }
    };

    class PostorderIterator
    {
      friend class TreeSet;
      
      DynStack<Node *> stack;
      Node * root = Node::null;
      Node * curr = Node::null;
      lint_t pos = 0;

      Node * first(Node *);

      void init()
      {
	if (root == Node::null)
	  return;

	pos = 0;
	curr = first(root);
      }

    protected:
      PostorderIterator(const TreeSet & t, int)
	: root(t.root), curr(Node::null), pos(t.size())
      {
	// empty
      }

      Node * get_location() const
      {
	return curr;
      }

    public:
      PostorderIterator(const TreeSet & t)
	: root(t.root)
      {
	init();
      }

      PostorderIterator(const PostorderIterator & it)
	: stack(it.stack), root(it.root), curr(it.curr), pos(it.pos)
      {
	// empty
      }

      PostorderIterator(PostorderIterator && it)
      {
	swap(it);
      }

      PostorderIterator & operator = (const PostorderIterator & it)
      {
	if (this == &it)
	  return *this;

	stack = it.stack;
	root = it.root;
	curr = it.curr;
	pos = it.pos;

	return *this;
      }

      PostorderIterator & operator = (PostorderIterator && it)
      {
	swap(it);
	return *this;
      }
      
      void swap(PostorderIterator & it)
      {
	std::swap(stack, it.stack);
	std::swap(root, it.root);
	std::swap(curr, it.curr);
	std::swap(pos, it.pos);
      }

      void reset()
      {
	stack.clear();
	init();
      }

      void reset_last()
      {
	pos = -1;
	stack.clear();
	curr = root;
      }

      nat_t get_position() const
      {
	return pos;
      }

      bool has_current() const
      {
	return curr != Node::null;
      }

      Key & get_current()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return KEY(curr);
      }

      const Key & get_current() const
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");
	
	return KEY(curr);
      }

      void next()
      {
	if (not has_current())
	  throw std::overflow_error("There is not current element");

	++pos;

	if (stack.is_empty())
	  {
	    curr = Node::null;
	    return;
	  }

	Node *& parent = stack.top();

	if (curr == R(parent) or R(parent) == Node::null)
	  {
	    curr = stack.pop();
	    return;
	  }

	curr = first(R(parent));
      }

      Key del()
      {
	return Key();
      }
    };

    class Iterator : public InorderIterator,
		     public ForwardIterator<Iterator, Key>
    {
      friend class TreeSet;
      friend class BasicIterator<Iterator, Key>;
      using Base = InorderIterator;
      using Base::Base;
    };

    Iterator begin()
    {
      return Iterator(*this);
    }

    Iterator begin() const
    {
      return Iterator(*this);
    }

    Iterator end()
    {
      return Iterator(*this, 0);
    }

    Iterator end() const
    {
      return Iterator(*this, 0);
    }
  };

  template <typename Key, class Cmp>
  TreeSet<Key, Cmp>::TreeSet(const std::initializer_list<Key> & l)
    : TreeSet()
  {
    for (const auto & item : l)
      append(item);
  }

  template <typename Key, class Cmp>
  bool TreeSet<Key, Cmp>::verify(Node * r, Cmp & cmp)
  {
    if (r == Node::null)
      return true;

    Node * lc = L(r);
    Node * rc = R(r);

    if (not verify(lc, cmp))
      return false;

    if (not verify(rc, cmp))
      return false;

    bool test = (COUNT(r) == (COUNT(lc) + COUNT(rc) + 1)) and
      (PRIOR(r) <= PRIOR(lc)) and (PRIOR(r) <= PRIOR(rc));

    if (lc != Node::null)
      test = test and cmp(KEY(lc), KEY(r));

    if (rc != Node::null)
      test = test and cmp(KEY(r), KEY(rc));

    return test;
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node * TreeSet<Key, Cmp>::copy(Node * r)
  {
    if (r == Node::null)
      return Node::null;

    Node * p = new Node(KEY(r));
    COUNT(p) = COUNT(r);
    PRIOR(p) = PRIOR(r);
    L(p) = copy(L(r));
    R(p) = copy(R(r));
    return p;
  }
  
  template <typename Key, class Cmp>
  void TreeSet<Key, Cmp>::destroy(Node *& r)
  {
    if (r == Node::null)
      return;
    
    destroy(L(r));
    destroy(R(r));
    delete r;
    r = Node::null;
  }
  
  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node * TreeSet<Key, Cmp>::rotate_left(Node * r)
  {
    Node * q = R(r);
    R(r) = L(q);
    L(q) = r;
    
    COUNT(r) = COUNT(L(r)) + COUNT(R(r)) + 1;
    COUNT(q) = COUNT(L(q)) + COUNT(R(q)) + 1;
    
    return q;
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node * TreeSet<Key, Cmp>::rotate_right(Node * r)
  {
    Node * q = L(r);
    L(r) = R(q);
    R(q) = r;
      
    COUNT(r) = COUNT(L(r)) + COUNT(R(r)) + 1;
    COUNT(q) = COUNT(L(q)) + COUNT(R(q)) + 1;
    
    return q;
  }

    template <typename Key, class Cmp>
  bool TreeSet<Key, Cmp>::split_key(Node * r, const Key & k,
				    Node *& ts, Node *& tg, Cmp & cmp)
  {
    if (r == Node::null)
      {
	ts = tg = Node::null;
	return true;
      }

    if (cmp(k, KEY(r)))
      {
	if (split_key(L(r), k, ts, L(r), cmp))
	  {
	    tg = r;
	    COUNT(tg) -= COUNT(ts);
	    return true;
	  }
      }
    else
      {
	if (split_key(R(r), k, R(r), tg, cmp))
	  {
	    ts = r;
	    COUNT(ts) -= COUNT(tg);
	    return true;
	  }
      }

    return false;
  }
  
  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::exclusive_join(Node *& ts, Node *& tg)
  {
    if (ts == Node::null)
      return tg;

    if (tg == Node::null)
      return ts;

    if (PRIOR(ts) < PRIOR(tg))
      {
	COUNT(ts) += COUNT(tg);
	R(ts) = exclusive_join(R(ts), tg);
	return ts;
      }
    else
      {
	COUNT(tg) += COUNT(ts);
	L(tg) = exclusive_join(ts, L(tg));
	return tg;
      }
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::insert(Node *& r, Node * p, Cmp & cmp)
  {
    if (r == Node::null)
      {
	r = p;
	return r;
      }
    
    if (cmp(KEY(p), KEY(r)))
      {
	Node * result = insert(L(r), p, cmp);

	if (result == Node::null)
	  return Node::null;
	
	++COUNT(r);
	    
	if (PRIOR(L(r)) < PRIOR(r))
	  r = result = rotate_right(r);
	
	return result;
      }
    else if (cmp(KEY(r), KEY(p)))
      {
	Node * result = insert(R(r), p, cmp);
	
	if (result == Node::null)
	  return Node::null;
	
	++COUNT(r);
	    
	if (PRIOR(R(r)) < PRIOR(r))
	  r = result = rotate_left(r);
	
	return result;	
      }
    
    return Node::null;
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::search(Node * r, const Key & k, Cmp & cmp)
  {
    if (r == Node::null)
      return Node::null;
    
    if (cmp(k, KEY(r)))
      return search(L(r), k, cmp);
    else if (cmp(KEY(r), k))
      return search(R(r), k, cmp);
    
    return r;
  }
  
  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::search(Node * r, Key && k, Cmp & cmp)
  {
    if (r == Node::null)
      return Node::null;
    
    if (cmp(k, KEY(r)))
      return search(L(r), std::forward<Key>(k), cmp);
    else if (cmp(KEY(r), k))
      return search(R(r), std::forward<Key>(k), cmp);
    
    return r;
  }


  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::search_or_insert(Node *& r, Node * p, Cmp & cmp)
  {
    if (r == Node::null)
      {
	r = p;
	return r;
      }
    
    if (cmp(KEY(p), KEY(r)))
      {
	Node * result = search_or_insert(L(r), p, cmp);

	if (result == p)
	  {	
	    ++COUNT(r);
	    
	    if (PRIOR(L(r)) < PRIOR(r))
	      r = result = rotate_right(r);
	  }
	
	return result;
      }
    else if (cmp(KEY(r), KEY(p)))
      {
	Node * result = search_or_insert(R(r), p, cmp);
	
	if (result == p)
	  {
	    ++COUNT(r);
	    
	    if (PRIOR(R(r)) < PRIOR(r))
	      r = result = rotate_left(r);
	  }
	
	return result;	
      }
    
    return r;
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::remove(Node *& r, const Key & k, Cmp & cmp)
  {
    if (r == Node::null)
      return Node::null;
    
    if (cmp(k, KEY(r)))
      {
	Node * result = remove(L(r), k, cmp);

	if (result != Node::null)
	  --COUNT(r);

	return result;
      }
    else if (cmp(KEY(r), k))
      {
	Node * result = remove(R(r), k, cmp);

	if (result != Node::null)
	  --COUNT(r);

	return result;
      }

    return remove_root(r);
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::remove_pos(Node *& r, nat_t i)
  {
    if (COUNT(L(r)) == i)
      return remove_root(r);

    Node * result = Node::null;

    if (i < COUNT(L(r)))
      result = remove_pos(L(r), i);
    else
      result = remove_pos(R(r), i - COUNT(L(r)) - 1);

    --COUNT(r);    
    return result;
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::select(Node * r, nat_t i)
  {
    if (COUNT(L(r)) == i)
      return r;

    if (i < COUNT(L(r)))
      return select(L(r), i);

    return select(R(r), i - COUNT(L(r)) - 1);
  }

  template <typename Key, class Cmp>
  lint_t TreeSet<Key, Cmp>::position(Node * r, const Key & k, Cmp & cmp)
  {
    if (r == Node::null)
      return -1;

    if (cmp(k, KEY(r)))
      return position(L(r), k, cmp);
    else if (cmp(KEY(r), k))
      {
	lint_t p = position(R(r), k, cmp);

	if (p == -1)
	  return p;

	return p + COUNT(L(r)) + 1;
      }

    return COUNT(L(r));
  }
  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node * TreeSet<Key, Cmp>::min(Node * r)
  {
    while (L(r) != Node::null)
      r = L(r);

    return r;
  }

    template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node * TreeSet<Key, Cmp>::max(Node * r)
  {
    while (R(r) != Node::null)
      r = R(r);

    return r;
  }
  
  template <typename Key, class Cmp>
  template <class Op>
  void TreeSet<Key, Cmp>::preorder_rec(Node * r, Op & op)
  {
    if (r == Node::null)
      return;
    
    op(KEY(r));
    preorder_rec(L(r), op);
    preorder_rec(R(r), op);
  }

  template <typename Key, class Cmp>
  template <class Op>
  void TreeSet<Key, Cmp>::inorder_rec(Node * r, Op & op)
  {
    if (r == Node::null)
      return;
    
    inorder_rec(L(r), op);
    op(KEY(r));
    inorder_rec(R(r), op);
  }

  template <typename Key, class Cmp>
  template <class Op>
  void TreeSet<Key, Cmp>::postorder_rec(Node * r, Op & op)
  {
    if (r == Node::null)
      return;
    
    postorder_rec(L(r), op);
    postorder_rec(R(r), op);
    op(KEY(r));
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::PreorderIterator::last(Node * r)
  {
    while (true)
      {
	while (R(r) != Node::null)
	  r = R(r);

	if (L(r) == Node::null)
	  break;

	r = L(r);
      }

    return r;
  }
  
  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::InorderIterator::search_min(Node * r)
  {
    while (L(r) != Node::null)
      {
	stack.push(r);
	r = L(r);
      }

    return r;
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::InorderIterator::search_max(Node * r)
  {
    while (R(r) != Node::null)
      r = R(r);

    return r;
  }

  template <typename Key, class Cmp>
  typename TreeSet<Key, Cmp>::Node *
  TreeSet<Key, Cmp>::PostorderIterator::first(Node * r)
  {
    while (true)
      {
	while (L(r) != Node::null)
	  {
	    stack.push(r);
	    r = L(r);
	  }

	if (R(r) == Node::null)
	  break;

	stack.push(r);
	r = R(r);
      }

    return r;
  }
  
} // end namespace Designar

# endif // DSGBINTREE_H
