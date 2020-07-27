/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
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

    void reset()
    {
      BaseNode::reset();
      count = 1;
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
    class RankedTreap : public ContainerAlgorithms<RankedTreap<Key, Cmp>, Key>,
			public SetAlgorithms<RankedTreap<Key, Cmp>, Key>
    {
    public:
      using Node = TreapRkNode<Key>;
      
    private:    
      Node    head;
      Node *& root;
      Cmp   & cmp;
    
      rng_t rng;

      static bool verify(Node *, Cmp & cmp);

      static bool verify_dup(Node *, Cmp & cmp);

      static Node * copy(Node *);

      static void destroy(Node *&);
    
      static Node * rotate_left(Node *);

      static Node * rotate_right(Node *);

      static void split_pos(Node *, nat_t, Node *&, Node *&);

      static bool split_key(Node *, const Key &, Node *&, Node *&, Cmp &);

      static void split_key_dup(Node *, const Key &, Node *&, Node *&,
				Cmp &);

      static Node * exclusive_join(Node *&, Node *&);

      static void join_dup(Node *&, Node *&, Cmp &);
    
      static Node * insert(Node *&, Node *, Cmp &);
      
      static Node * insert_dup(Node *&, Node *, Cmp &);
      
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
      
      Key * insert_dup(Node * p)
      {
	PRIOR(p) = rng();
	return &KEY(insert_dup(root, p, cmp));
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

      bool verify_dup() const
      {
	return verify_dup(root, cmp);
      }
      
      RankedTreap(rng_seed_t seed, Cmp & _cmp)
	: head(), root(L(&head)), cmp(_cmp), rng(seed)
      {
	// empty
      }

      RankedTreap(Cmp & _cmp)
	: RankedTreap(time(nullptr), _cmp)
      {
	// empty
      }

      RankedTreap(Cmp && _cmp = Cmp())
	: RankedTreap(_cmp)
      {
	// empty
      }

      RankedTreap(rng_seed_t seed, Cmp && _cmp = Cmp())
	: RankedTreap(seed, _cmp)
      {
	// empty
      }

      RankedTreap(const RankedTreap & t)
	: RankedTreap(t.cmp)
      {
	root = copy(t.root);
      }

      RankedTreap(RankedTreap && t)
	: RankedTreap()
      {
	swap(t);
      }

      RankedTreap(const std::initializer_list<Key> &);

      ~RankedTreap()
      {
	clear();
      }

      RankedTreap & operator = (const RankedTreap & t)
      {
	if (this == &t)
	  return *this;

	clear();
	root = copy(t.root);
	cmp = t.cmp;
	return *this;
      }

      RankedTreap & operator = (RankedTreap && t)
      {
	swap(t);
	return *this;
      }

      void swap(RankedTreap & t)
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
      
      Key * insert_dup(const Key & k)
      {
	Node * p = new Node(k);
	return insert_dup(p);
      }
      
      Key * insert_dup(Key && k)
      {
	Node * p = new Node(std::forward<Key>(k));
	return insert_dup(p);
      }
      
      Key * append(const Key & k)
      {
	return insert(k);
      }
      
      Key * append(Key && k)
      {
	return insert(std::forward<Key>(k));
      }
      
      Key * append_dup(const Key & k)
      {
	return insert_dup(k);
      }
      
      Key * append_dup(Key && k)
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

      std::tuple<RankedTreap, RankedTreap> split_pos(nat_t i)
      {
	if (i >= size())
	  throw std::out_of_range("Infix position is out of range");
	
	RankedTreap ts, tg;
	split_pos(root, i, ts.root, tg.root);
	root = Node::null;
	return std::make_tuple(std::move(ts), std::move(tg));
      }

      std::tuple<RankedTreap, RankedTreap> split_key(const Key & k)
      {
	RankedTreap ts, tg;

	if (split_key(root, k, ts.root, tg.root, cmp))
	  root = Node::null;

	return std::make_tuple(std::move(ts), std::move(tg));
      }

      std::tuple<RankedTreap, RankedTreap> split_key_dup(const Key & k)
      {
	RankedTreap ts, tg;
	split_key_dup(root, k, ts.root, tg.root, cmp);
	root = Node::null;
	return std::make_tuple(std::move(ts), std::move(tg));
      }

      void exclusive_join(RankedTreap & ts, RankedTreap & tg)
      {
	root = exclusive_join(ts.root, tg.root);
	ts.root = tg.root = Node::null;
      }

      void join_dup(RankedTreap & ts, RankedTreap & tg)
      {
	join_dup(ts.root, tg.root, cmp);
	root = ts.root;
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
	friend class RankedTreap;
      
	DynStack<Node *> stack;
	Node *    root = Node::null;
	Node *    curr = Node::null;

	Node * last(Node *);

      protected:
	PreorderIterator(const RankedTreap & t, int)
	  : root(t.root), curr(Node::null)
	{
	  // empty
	}
      
	Node * get_location() const
	{
	  return curr;
	}

      public:
	PreorderIterator(const RankedTreap & t)
	  : root(t.root), curr(root)
	{
	  // empty
	}

	PreorderIterator(const PreorderIterator & it)
	  : stack(it.stack), root(it.root), curr(it.curr)
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
	}

	void reset_first()
	{
	  stack.clear();
	  curr = root;
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
	    return;

	  stack.push(curr);

	  curr = L(curr);

	  if (curr != Node::null)
	    return;

	  while (not stack.is_empty() and curr == Node::null)
	    curr = R(stack.pop());	  
	}
      };

      class InorderIterator
      {
	friend class RankedTreap;
	
	RankedTreap * set_ptr = nullptr;
	DynStack<Node *> stack;
	Node * root = Node::null;
	Node * curr = Node::null;
	
	Node * search_min(Node *);
	
	Node * search_max(Node *);
	
	void init()
	{
	  if (root == Node::null)
	    return;

	  curr = search_min(root);
	}

      protected:
	InorderIterator(const RankedTreap & t, int)
	  : set_ptr(const_cast<RankedTreap *>(&t)), root(set_ptr->root),
	    curr(Node::null)
	{
	  // empty
	}
	
	Node * get_location() const
	{
	  return curr;
	}
	
      public:
	InorderIterator(const RankedTreap & t)
	  : set_ptr(const_cast<RankedTreap *>(&t)), root(set_ptr->root)
	{
	  init();
	}

	InorderIterator(const InorderIterator & it)
	  : set_ptr(it.set_ptr), stack(it.stack), root(it.root), curr(it.curr)
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
	}

	void reset_first()
	{
	  stack.clear();
	  init();
	}

	void reset_last()
	{
	  stack.clear();
	  curr = search_max(root);
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

	  curr = R(curr);
	
	  if (curr != Node::null)
	    curr = search_min(curr);
	  else
	    if (not stack.is_empty())
	      curr = stack.pop();
	}

	Key del()
	{
	  if (not has_current())
	    throw std::logic_error("There is not current element");
	  
	  Key to_remove = KEY(curr);
	  next();
	  set_ptr->remove(to_remove);

	  return to_remove;
	}
      };

      class PostorderIterator
      {
	friend class RankedTreap;
      
	DynStack<Node *> stack;
	Node * root = Node::null;
	Node * curr = Node::null;

	Node * first(Node *);

	void init()
	{
	  if (root == Node::null)
	    return;

	  curr = first(root);
	}

      protected:
	PostorderIterator(const RankedTreap & t, int)
	  : root(t.root), curr(Node::null)
	{
	  // empty
	}

	Node * get_location() const
	{
	  return curr;
	}

      public:
	PostorderIterator(const RankedTreap & t)
	  : root(t.root)
	{
	  init();
	}

	PostorderIterator(const PostorderIterator & it)
	  : stack(it.stack), root(it.root), curr(it.curr)
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
	}

	void reset_first()
	{
	  stack.clear();
	  init();
	}

	void reset_last()
	{
	  stack.clear();
	  curr = root;
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
      };

      class Iterator : public InorderIterator,
		       public ForwardIterator<Iterator, Key>
      {
	friend class RankedTreap;
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
  RankedTreap<Key, Cmp>::RankedTreap(const std::initializer_list<Key> & l)
    : RankedTreap()
  {
    for (const auto & item : l)
      append(item);
  }

  template <typename Key, class Cmp>
  bool RankedTreap<Key, Cmp>::verify(Node * r, Cmp & cmp)
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
  bool RankedTreap<Key, Cmp>::verify_dup(Node * r, Cmp & cmp)
  {
    if (r == Node::null)
      return true;

    Node * lc = L(r);
    Node * rc = R(r);

    if (not verify_dup(lc, cmp))
      return false;

    if (not verify_dup(rc, cmp))
      return false;

    bool test = (COUNT(r) == (COUNT(lc) + COUNT(rc) + 1)) and
      (PRIOR(r) <= PRIOR(lc)) and (PRIOR(r) <= PRIOR(rc));

    if (lc != Node::null)
      test = test and not cmp(KEY(r), KEY(lc));

    if (rc != Node::null)
      test = test and not cmp(KEY(rc), KEY(r));

    return test;
  }

  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node * RankedTreap<Key, Cmp>::copy(Node * r)
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
  void RankedTreap<Key, Cmp>::destroy(Node *& r)
  {
    if (r == Node::null)
      return;
    
    destroy(L(r));
    destroy(R(r));
    delete r;
    r = Node::null;
  }
  
  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::rotate_left(Node * r)
  {
    Node * q = R(r);
    R(r) = L(q);
    L(q) = r;
    
    COUNT(r) = COUNT(L(r)) + COUNT(R(r)) + 1;
    COUNT(q) = COUNT(L(q)) + COUNT(R(q)) + 1;
    
    return q;
  }

  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::rotate_right(Node * r)
  {
    Node * q = L(r);
    L(r) = R(q);
    R(q) = r;
      
    COUNT(r) = COUNT(L(r)) + COUNT(R(r)) + 1;
    COUNT(q) = COUNT(L(q)) + COUNT(R(q)) + 1;
    
    return q;
  }

  template <typename Key, class Cmp>
  void RankedTreap<Key, Cmp>::split_pos(Node * r, nat_t i,
					Node *& ts, Node *& tg)
  {
    if (i == COUNT(L(r)))
      {
	ts = L(r);
	tg = r;
	L(tg) = Node::null;
	COUNT(tg) -= COUNT(ts);
	return;
      }

    if (i < COUNT(L(r)))
      {
	split_pos(L(r), i, ts, L(r));
	tg = r;
	COUNT(r) -= COUNT(ts);
      }
    else
      {
	split_pos(R(r), i - (COUNT(L(r)) + 1), R(r), tg);
	ts = r;
	COUNT(r) -= COUNT(tg);
      }
  }

  template <typename Key, class Cmp>
  bool RankedTreap<Key, Cmp>::split_key(Node * r, const Key & k,
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
    else if (cmp(KEY(r), k))
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
  void RankedTreap<Key, Cmp>::split_key_dup(Node * r, const Key & k,
					    Node *& ts, Node *& tg,
					    Cmp & cmp)
  {
    if (r == Node::null)
      {
	ts = tg = Node::null;
	return;
      }
    
    if (cmp(k, KEY(r)))
      {
	split_key_dup(L(r), k, ts, L(r), cmp);
	tg = r;
	COUNT(tg) -= COUNT(ts);
      }
    else
      {
	split_key_dup(R(r), k, R(r), tg, cmp);
	ts = r;
	COUNT(ts) -= COUNT(tg);
      }
  }
  
  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::exclusive_join(Node *& ts, Node *& tg)
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
  void RankedTreap<Key, Cmp>::join_dup(Node *& t1, Node *& t2, Cmp & cmp)
  {
    if (t2 == Node::null)
      return;

    Node * l = L(t2);
    Node * r = R(t2);
    
    t2->reset();
    insert_dup(t1, t2, cmp);
    join_dup(t1, l, cmp);
    join_dup(t1, r, cmp);
  }

  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::insert(Node *& r, Node * p, Cmp & cmp)
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
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::insert_dup(Node *& r, Node * p, Cmp & cmp)
  {
    if (r == Node::null)
      {
	r = p;
	return r;
      }
    
    if (cmp(KEY(p), KEY(r)))
      {
	Node * result = insert_dup(L(r), p, cmp);

	if (result == Node::null)
	  return Node::null;
	
	++COUNT(r);
	    
	if (PRIOR(L(r)) < PRIOR(r))
	  r = result = rotate_right(r);
	
	return result;
      }
    
    Node * result = insert_dup(R(r), p, cmp);
    
    if (result == Node::null)
      return Node::null;
    
    ++COUNT(r);
    
    if (PRIOR(R(r)) < PRIOR(r))
      r = result = rotate_left(r);
    
    return result;	
  }

  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::search(Node * r, const Key & k, Cmp & cmp)
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
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::search(Node * r, Key && k, Cmp & cmp)
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
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::search_or_insert(Node *& r, Node * p, Cmp & cmp)
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
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::remove(Node *& r, const Key & k, Cmp & cmp)
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
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::remove_pos(Node *& r, nat_t i)
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
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::select(Node * r, nat_t i)
  {
    if (COUNT(L(r)) == i)
      return r;

    if (i < COUNT(L(r)))
      return select(L(r), i);

    return select(R(r), i - COUNT(L(r)) - 1);
  }

  template <typename Key, class Cmp>
  lint_t RankedTreap<Key, Cmp>::position(Node * r, const Key & k, Cmp & cmp)
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
  typename RankedTreap<Key, Cmp>::Node * RankedTreap<Key, Cmp>::min(Node * r)
  {
    while (L(r) != Node::null)
      r = L(r);

    return r;
  }

  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node * RankedTreap<Key, Cmp>::max(Node * r)
  {
    while (R(r) != Node::null)
      r = R(r);

    return r;
  }
  
  template <typename Key, class Cmp>
  template <class Op>
  void RankedTreap<Key, Cmp>::preorder_rec(Node * r, Op & op)
  {
    if (r == Node::null)
      return;
    
    op(KEY(r));
    preorder_rec(L(r), op);
    preorder_rec(R(r), op);
  }

  template <typename Key, class Cmp>
  template <class Op>
  void RankedTreap<Key, Cmp>::inorder_rec(Node * r, Op & op)
  {
    if (r == Node::null)
      return;
    
    inorder_rec(L(r), op);
    op(KEY(r));
    inorder_rec(R(r), op);
  }

  template <typename Key, class Cmp>
  template <class Op>
  void RankedTreap<Key, Cmp>::postorder_rec(Node * r, Op & op)
  {
    if (r == Node::null)
      return;
    
    postorder_rec(L(r), op);
    postorder_rec(R(r), op);
    op(KEY(r));
  }

  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::PreorderIterator::last(Node * r)
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
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::InorderIterator::search_min(Node * r)
  {
    while (L(r) != Node::null)
      {
	stack.push(r);
	r = L(r);
      }

    return r;
  }

  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::InorderIterator::search_max(Node * r)
  {
    while (R(r) != Node::null)
      r = R(r);

    return r;
  }

  template <typename Key, class Cmp>
  typename RankedTreap<Key, Cmp>::Node *
  RankedTreap<Key, Cmp>::PostorderIterator::first(Node * r)
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
