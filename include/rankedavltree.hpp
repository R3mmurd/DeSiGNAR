/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file rankedavltree.hpp
    @brief An AVL tree set implementation with order-statistics support
    (select-by-rank, position-of-key), the AVL counterpart of
    RankedTreap.
    @see avltree.hpp for the plain (unranked) AVL tree this builds on.
    @see tree.hpp for RankedTreap, the same order-statistics interface
    on top of randomized (priority-based) balancing instead.
    @ingroup Trees
*/

#pragma once

#include <avltree.hpp> // for HEIGHT<>()
#include <tree.hpp>    // for COUNT<>()

namespace Designar
{
  template <typename Key>
  class AVLRkNode : public BaseBinTreeNode<Key, AVLRkNode<Key>,
                                           BinTreeNodeNullValue::SENTINEL>
  {
    using BaseNode = BaseBinTreeNode<Key, AVLRkNode<Key>,
                                     BinTreeNodeNullValue::SENTINEL>;

    int_t height;
    nat_t count;

  public:
    AVLRkNode()
        : BaseNode()
    {
      // empty
    }

    AVLRkNode(const Key& k)
        : BaseNode(k), height(0), count(1)
    {
      // empty
    }

    AVLRkNode(Key&& k)
        : BaseNode(std::forward<Key>(k)), height(0), count(1)
    {
      // empty
    }

    /** @see AVLNode's matching constructor for why the sentinel fixes
        `height` at -1; `count` is fixed at 0 for the same reason
        RankedTreap's sentinel is (so COUNT(Node::null) reads as 0 with
        no branch needed). */
    AVLRkNode(BinTreeNodeCtor ctor)
        : BaseNode(ctor), height(-1), count(0)
    {
      // empty
    }

    int_t& get_height()
    {
      return height;
    }

    nat_t& get_count()
    {
      return count;
    }
  };

  /** An AVL tree (see avltree.hpp) that additionally maintains a
      subtree-size count per node, giving it the same order-statistics
      operations as RankedTreap (tree.hpp) — select(i)/position(k)/
      operator[] — but with deterministic height-based balancing instead
      of randomized-priority balancing.

      @see DefaultCmpHolder for why this class privately derives from
      `DefaultCmpHolder<Cmp>` and why that must be its first base. */
  template <typename Key, class Cmp = std::less<Key>>
  class RankedAVLTree : private DefaultCmpHolder<Cmp>,
                        public ContainerAlgorithms<RankedAVLTree<Key, Cmp>, Key>,
                        public SetAlgorithms<RankedAVLTree<Key, Cmp>, Key>
  {
  public:
    using Node = AVLRkNode<Key>;

  private:
    Node head;
    Node*& root;
    Cmp& cmp;

    /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
        logic and the same reason it is needed. */
    static Cmp& cmp_for_copy(RankedAVLTree& self, const RankedAVLTree& t)
    {
      if (&t.cmp == &t.default_cmp)
      {
        self.default_cmp = t.default_cmp;
        return self.default_cmp;
      }

      return t.cmp;
    }

    static Node* copy(Node*);

    static void destroy(Node*&);

    static Node* update(Node* r)
    {
      HEIGHT(r) = 1 + std::max(HEIGHT(L(r)), HEIGHT(R(r)));
      COUNT(r) = COUNT(L(r)) + COUNT(R(r)) + 1;
      return r;
    }

    static int_t balance_factor(Node* r)
    {
      return HEIGHT(L(r)) - HEIGHT(R(r));
    }

    static Node* rotate_left(Node* r)
    {
      Node* q = R(r);
      R(r) = L(q);
      L(q) = r;
      update(r);
      update(q);
      return q;
    }

    static Node* rotate_right(Node* r)
    {
      Node* q = L(r);
      L(r) = R(q);
      R(q) = r;
      update(r);
      update(q);
      return q;
    }

    static Node* balance(Node* r)
    {
      update(r);

      int_t bf = balance_factor(r);

      if (bf > 1)
      {
        if (balance_factor(L(r)) < 0)
        {
          L(r) = rotate_left(L(r));
        }

        return rotate_right(r);
      }

      if (bf < -1)
      {
        if (balance_factor(R(r)) > 0)
        {
          R(r) = rotate_right(R(r));
        }

        return rotate_left(r);
      }

      return r;
    }

    static Node* insert(Node*&, Node*, Cmp&);

    static Node* insert_dup(Node*&, Node*, Cmp&);

    static Node* search(Node*, const Key&, Cmp&);

    static Node* search_or_insert(Node*&, Node*, Cmp&);

    static Node* remove_min(Node*& r)
    {
      if (L(r) == Node::null)
      {
        Node* ret_val = r;
        r = R(r);
        return ret_val;
      }

      Node* ret_val = remove_min(L(r));
      r = balance(r);
      return ret_val;
    }

    static Node* remove(Node*&, const Key&, Cmp&);

    static Node* remove_pos(Node*&, nat_t);

    static Node* select(Node*, nat_t);

    static int_t position(Node*, const Key&, Cmp&);

    static Node* min(Node* r)
    {
      while (L(r) != Node::null)
      {
        r = L(r);
      }

      return r;
    }

    static Node* max(Node* r)
    {
      while (R(r) != Node::null)
      {
        r = R(r);
      }

      return r;
    }

    Key* insert(Node* p)
    {
      if (insert(root, p, cmp) == Node::null)
      {
        delete p;
        return nullptr;
      }

      return &KEY(p);
    }

    Key* insert_dup(Node* p)
    {
      insert_dup(root, p, cmp);
      return &KEY(p);
    }

    Key* search_or_insert(Node* p)
    {
      Node* result = search_or_insert(root, p, cmp);

      if (p != result)
      {
        delete p;
      }

      return &KEY(result);
    }

  public:
    using ItemType = Key;
    using KeyType = Key;
    using DataType = Key;
    using ValueType = Key;
    using SizeType = nat_t;
    using CmpType = Cmp;

    RankedAVLTree(Cmp& _cmp)
        : head(), root(L(&head)), cmp(_cmp)
    {
      // empty
    }

    /** @see RankedTreap's matching constructor for why this cannot
        simply delegate to the Cmp& overload above, and why
        copy-assignment (not move) is used for `default_cmp`. */
    RankedAVLTree(Cmp&& _cmp = Cmp())
        : head(), root(L(&head)), cmp(this->default_cmp)
    {
      this->default_cmp = _cmp;
    }

    RankedAVLTree(const RankedAVLTree& t)
        : head(), root(L(&head)), cmp(cmp_for_copy(*this, t))
    {
      root = copy(t.root);
    }

    RankedAVLTree(RankedAVLTree&& t)
        : RankedAVLTree()
    {
      swap(t);
    }

    RankedAVLTree(const std::initializer_list<Key>&);

    ~RankedAVLTree()
    {
      clear();
    }

    RankedAVLTree& operator=(const RankedAVLTree& t)
    {
      if (this == &t)
      {
        return *this;
      }

      clear();
      root = copy(t.root);
      cmp = t.cmp;
      return *this;
    }

    RankedAVLTree& operator=(RankedAVLTree&& t)
    {
      swap(t);
      return *this;
    }

    void swap(RankedAVLTree& t)
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

    Cmp& get_cmp()
    {
      return cmp;
    }

    const Cmp& get_cmp() const
    {
      return cmp;
    }

    Key* insert(const Key& k)
    {
      return insert(new Node(k));
    }

    Key* insert(Key&& k)
    {
      return insert(new Node(std::forward<Key>(k)));
    }

    Key* insert_dup(const Key& k)
    {
      return insert_dup(new Node(k));
    }

    Key* insert_dup(Key&& k)
    {
      return insert_dup(new Node(std::forward<Key>(k)));
    }

    Key* append(const Key& k)
    {
      return insert(k);
    }

    Key* append(Key&& k)
    {
      return insert(std::forward<Key>(k));
    }

    Key* append_dup(const Key& k)
    {
      return insert_dup(k);
    }

    Key* append_dup(Key&& k)
    {
      return insert_dup(std::forward<Key>(k));
    }

    Key* search(const Key& k)
    {
      Node* result = search(root, k, cmp);
      return result == Node::null ? nullptr : &KEY(result);
    }

    const Key* search(const Key& k) const
    {
      Node* result = search(root, k, cmp);
      return result == Node::null ? nullptr : &KEY(result);
    }

    Key* search_or_insert(const Key& k)
    {
      return search_or_insert(new Node(k));
    }

    Key* search_or_insert(Key&& k)
    {
      return search_or_insert(new Node(std::forward<Key>(k)));
    }

    Key& find(const Key& k)
    {
      Key* result = search(k);

      if (result == nullptr)
      {
        throw std::domain_error("Key not found");
      }

      return *result;
    }

    const Key& find(const Key& k) const
    {
      const Key* result = search(k);

      if (result == nullptr)
      {
        throw std::domain_error("Key not found");
      }

      return *result;
    }

    bool remove(const Key& k)
    {
      Node* result = remove(root, k, cmp);

      if (result == Node::null)
      {
        return false;
      }

      delete result;
      return true;
    }

    Key remove_pos(nat_t i)
    {
      if (i >= size())
      {
        throw std::out_of_range("Infix position is out of range");
      }

      Node* result = remove_pos(root, i);
      Key ret_val = std::move(KEY(result));
      delete result;
      return ret_val;
    }

    const Key& min() const
    {
      if (is_empty())
      {
        throw std::underflow_error("Tree is empty");
      }

      return KEY(min(root));
    }

    const Key& max() const
    {
      if (is_empty())
      {
        throw std::underflow_error("Tree is empty");
      }

      return KEY(max(root));
    }

    Key& select(nat_t i)
    {
      if (i >= size())
      {
        throw std::out_of_range("Infix position is out of range");
      }

      return KEY(select(root, i));
    }

    const Key& select(nat_t i) const
    {
      if (i >= size())
      {
        throw std::out_of_range("Infix position is out of range");
      }

      return KEY(select(root, i));
    }

    int_t position(const Key& k) const
    {
      return position(root, k, cmp);
    }

    Key& operator[](nat_t i)
    {
      return select(i);
    }

    const Key& operator[](nat_t i) const
    {
      return select(i);
    }

    class InorderIterator
    {
      friend class RankedAVLTree;

      RankedAVLTree* set_ptr = nullptr;
      DynStack<Node*> stack;
      Node* root = Node::null;
      Node* curr = Node::null;

      Node* search_min(Node* r)
      {
        while (L(r) != Node::null)
        {
          stack.push(r);
          r = L(r);
        }

        return r;
      }

      Node* search_max(Node* r)
      {
        while (R(r) != Node::null)
        {
          r = R(r);
        }

        return r;
      }

      void init()
      {
        if (root == Node::null)
        {
          return;
        }

        curr = search_min(root);
      }

    protected:
      InorderIterator(const RankedAVLTree& t, int)
          : set_ptr(const_cast<RankedAVLTree*>(&t)), root(set_ptr->root),
            curr(Node::null)
      {
        // empty
      }

      Node* get_location() const
      {
        return curr;
      }

    public:
      InorderIterator(const RankedAVLTree& t)
          : set_ptr(const_cast<RankedAVLTree*>(&t)), root(set_ptr->root)
      {
        init();
      }

      InorderIterator(const InorderIterator& it)
          : set_ptr(it.set_ptr), stack(it.stack), root(it.root), curr(it.curr)
      {
        // empty
      }

      InorderIterator(InorderIterator&& it)
      {
        swap(it);
      }

      InorderIterator& operator=(const InorderIterator& it)
      {
        if (this == &it)
        {
          return *this;
        }

        stack = it.stack;
        root = it.root;
        curr = it.curr;

        return *this;
      }

      InorderIterator& operator=(InorderIterator&& it)
      {
        swap(it);
        return *this;
      }

      void swap(InorderIterator& it)
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

      Key& get_current()
      {
        if (!has_current())
        {
          throw std::overflow_error("There is not current element");
        }

        return KEY(curr);
      }

      const Key& get_current() const
      {
        if (!has_current())
        {
          throw std::overflow_error("There is not current element");
        }

        return KEY(curr);
      }

      void next()
      {
        if (!has_current())
        {
          throw std::overflow_error("There is not current element");
        }

        curr = R(curr);

        if (curr != Node::null)
        {
          curr = search_min(curr);
        }
        else if (!stack.is_empty())
        {
          curr = stack.pop();
        }
      }

      Key del()
      {
        if (!has_current())
        {
          throw std::logic_error("There is not current element");
        }

        Key to_remove = KEY(curr);
        next();
        set_ptr->remove(to_remove);

        return to_remove;
      }
    };

    class Iterator : public InorderIterator,
                     public ForwardIterator<Iterator, Key>
    {
      friend class RankedAVLTree;
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
  RankedAVLTree<Key, Cmp>::RankedAVLTree(const std::initializer_list<Key>& l)
      : RankedAVLTree()
  {
    for (const auto& item : l)
    {
      append(item);
    }
  }

  template <typename Key, class Cmp>
  typename RankedAVLTree<Key, Cmp>::Node*
  RankedAVLTree<Key, Cmp>::copy(Node* r)
  {
    if (r == Node::null)
    {
      return Node::null;
    }

    Node* p = new Node(KEY(r));
    L(p) = copy(L(r));
    R(p) = copy(R(r));
    update(p);
    return p;
  }

  template <typename Key, class Cmp>
  void RankedAVLTree<Key, Cmp>::destroy(Node*& r)
  {
    if (r == Node::null)
    {
      return;
    }

    destroy(L(r));
    destroy(R(r));
    delete r;
    r = Node::null;
  }

  template <typename Key, class Cmp>
  typename RankedAVLTree<Key, Cmp>::Node*
  RankedAVLTree<Key, Cmp>::insert(Node*& r, Node* p, Cmp& cmp)
  {
    if (r == Node::null)
    {
      r = p;
      return r;
    }

    Node* result;

    if (cmp(KEY(p), KEY(r)))
    {
      result = insert(L(r), p, cmp);

      if (result == Node::null)
      {
        return Node::null;
      }
    }
    else if (cmp(KEY(r), KEY(p)))
    {
      result = insert(R(r), p, cmp);

      if (result == Node::null)
      {
        return Node::null;
      }
    }
    else
    {
      return Node::null;
    }

    r = balance(r);
    return result;
  }

  template <typename Key, class Cmp>
  typename RankedAVLTree<Key, Cmp>::Node*
  RankedAVLTree<Key, Cmp>::insert_dup(Node*& r, Node* p, Cmp& cmp)
  {
    if (r == Node::null)
    {
      r = p;
      return r;
    }

    Node* result;

    if (cmp(KEY(p), KEY(r)))
    {
      result = insert_dup(L(r), p, cmp);
    }
    else
    {
      result = insert_dup(R(r), p, cmp);
    }

    r = balance(r);
    return result;
  }

  template <typename Key, class Cmp>
  typename RankedAVLTree<Key, Cmp>::Node*
  RankedAVLTree<Key, Cmp>::search(Node* r, const Key& k, Cmp& cmp)
  {
    if (r == Node::null)
    {
      return Node::null;
    }

    if (cmp(k, KEY(r)))
    {
      return search(L(r), k, cmp);
    }
    else if (cmp(KEY(r), k))
    {
      return search(R(r), k, cmp);
    }

    return r;
  }

  template <typename Key, class Cmp>
  typename RankedAVLTree<Key, Cmp>::Node*
  RankedAVLTree<Key, Cmp>::search_or_insert(Node*& r, Node* p, Cmp& cmp)
  {
    if (r == Node::null)
    {
      r = p;
      return r;
    }

    Node* result;

    if (cmp(KEY(p), KEY(r)))
    {
      result = search_or_insert(L(r), p, cmp);
    }
    else if (cmp(KEY(r), KEY(p)))
    {
      result = search_or_insert(R(r), p, cmp);
    }
    else
    {
      return r;
    }

    r = balance(r);
    return result;
  }

  template <typename Key, class Cmp>
  typename RankedAVLTree<Key, Cmp>::Node*
  RankedAVLTree<Key, Cmp>::remove(Node*& r, const Key& k, Cmp& cmp)
  {
    if (r == Node::null)
    {
      return Node::null;
    }

    Node* ret_val;

    if (cmp(k, KEY(r)))
    {
      ret_val = remove(L(r), k, cmp);

      if (ret_val == Node::null)
      {
        return Node::null;
      }
    }
    else if (cmp(KEY(r), k))
    {
      ret_val = remove(R(r), k, cmp);

      if (ret_val == Node::null)
      {
        return Node::null;
      }
    }
    else
    {
      ret_val = r;

      if (L(r) == Node::null)
      {
        r = R(r);
        return ret_val;
      }

      if (R(r) == Node::null)
      {
        r = L(r);
        return ret_val;
      }

      Node* succ = remove_min(R(r));
      std::swap(KEY(r), KEY(succ));
      ret_val = succ;
    }

    r = balance(r);
    return ret_val;
  }

  template <typename Key, class Cmp>
  typename RankedAVLTree<Key, Cmp>::Node*
  RankedAVLTree<Key, Cmp>::remove_pos(Node*& r, nat_t i)
  {
    Node* ret_val;

    if (COUNT(L(r)) == i)
    {
      ret_val = r;

      if (L(r) == Node::null)
      {
        r = R(r);
        return ret_val;
      }

      if (R(r) == Node::null)
      {
        r = L(r);
        return ret_val;
      }

      Node* succ = remove_min(R(r));
      std::swap(KEY(r), KEY(succ));
      ret_val = succ;
    }
    else if (i < COUNT(L(r)))
    {
      ret_val = remove_pos(L(r), i);
    }
    else
    {
      ret_val = remove_pos(R(r), i - COUNT(L(r)) - 1);
    }

    r = balance(r);
    return ret_val;
  }

  template <typename Key, class Cmp>
  typename RankedAVLTree<Key, Cmp>::Node*
  RankedAVLTree<Key, Cmp>::select(Node* r, nat_t i)
  {
    if (COUNT(L(r)) == i)
    {
      return r;
    }

    if (i < COUNT(L(r)))
    {
      return select(L(r), i);
    }

    return select(R(r), i - COUNT(L(r)) - 1);
  }

  template <typename Key, class Cmp>
  int_t RankedAVLTree<Key, Cmp>::position(Node* r, const Key& k, Cmp& cmp)
  {
    if (r == Node::null)
    {
      return -1;
    }

    if (cmp(k, KEY(r)))
    {
      return position(L(r), k, cmp);
    }
    else if (cmp(KEY(r), k))
    {
      int_t p = position(R(r), k, cmp);

      if (p == -1)
      {
        return p;
      }

      return p + COUNT(L(r)) + 1;
    }

    return COUNT(L(r));
  }

} // end namespace Designar
