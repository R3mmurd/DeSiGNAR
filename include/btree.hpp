/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file btree.hpp
    @brief BTree: a classic (Bayer & McCreight) B-tree set implementation
    — a multi-way, always-perfectly-height-balanced search tree where
    every node holds several keys and children rather than exactly one
    key and two children, designed around minimizing the number of
    nodes touched per operation (originally to minimize disk-block
    reads; the same "wide, shallow tree" shape is also just a
    fundamentally different balancing trade-off from every binary tree
    in this library).
    @ingroup DataStructures
*/

#pragma once

#include <array.hpp>
#include <containeralgorithms.hpp>
#include <setalgorithms.hpp>
#include <iterator.hpp>
#include <typetraits.hpp>

namespace Designar
{
  /** `MinDegree` (CLRS's `t`) controls node fan-out: every non-root node
      holds between `MinDegree - 1` and `2*MinDegree - 1` keys (so
      between `MinDegree` and `2*MinDegree` children when internal).
      Both insertion and deletion use CLRS's "proactive" approach — a
      full node is split (on insert) or a too-small node is merged/
      rebalanced with a sibling (on delete) *before* recursing into it,
      never after — which means a single top-down pass suffices for
      either operation, with no separate bottom-up fixup phase and no
      parent pointers needed.

      @see DefaultCmpHolder for why this class privately derives from
      `DefaultCmpHolder<Cmp>` and why that must be its first base. */
  template <typename Key, class Cmp = std::less<Key>, nat_t MinDegree = 3>
  class BTree : private DefaultCmpHolder<Cmp>,
                public ContainerAlgorithms<BTree<Key, Cmp, MinDegree>, Key>,
                public SetAlgorithms<BTree<Key, Cmp, MinDegree>, Key>
  {
    static_assert(MinDegree >= 2, "MinDegree must be at least 2");

    static constexpr nat_t T = MinDegree;
    static constexpr nat_t MAX_KEYS = 2 * T - 1;
    static constexpr nat_t MIN_KEYS = T - 1;

    struct Node
    {
      DynArray<Key> keys;
      DynArray<Node*> children;
      bool leaf;

      Node(bool is_leaf)
          : keys(MAX_KEYS + 1), children(is_leaf ? 0 : MAX_KEYS + 2), leaf(is_leaf)
      {
        // empty
      }

      nat_t num_keys() const
      {
        return keys.size();
      }
    };

    Node* root;
    nat_t num_items;
    Cmp& cmp;

    /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
        logic and the same reason it is needed. */
    static Cmp& cmp_for_copy(BTree& self, const BTree& t)
    {
      if (&t.cmp == &t.default_cmp)
      {
        self.default_cmp = t.default_cmp;
        return self.default_cmp;
      }

      return t.cmp;
    }

    static void truncate(DynArray<Key>& arr, nat_t new_size)
    {
      while (arr.size() > new_size)
      {
        arr.remove_last();
      }
    }

    static void truncate(DynArray<Node*>& arr, nat_t new_size)
    {
      while (arr.size() > new_size)
      {
        arr.remove_last();
      }
    }

    static Node* copy(Node* n)
    {
      Node* c = new Node(n->leaf);
      c->keys = n->keys;

      if (!n->leaf)
      {
        for (nat_t i = 0; i < n->children.size(); ++i)
        {
          c->children.append(copy(n->children[i]));
        }
      }

      return c;
    }

    static void destroy(Node* n)
    {
      if (!n->leaf)
      {
        for (nat_t i = 0; i < n->children.size(); ++i)
        {
          destroy(n->children[i]);
        }
      }

      delete n;
    }

    /** Finds the first index in `n`'s keys that is not less than `k`
        (the standard "which child would `k` fall under" / "does `n`
        hold `k` at this index" position). */
    nat_t lower_bound(Node* n, const Key& k) const
    {
      nat_t i = 0;

      while (i < n->num_keys() && cmp(n->keys[i], k))
      {
        ++i;
      }

      return i;
    }

    /** Splits the full (2t-1-key) child `x->children[i]` in two: the
        median key moves up into `x` at position `i`, the smaller half
        stays in place, and the larger half becomes a new right sibling
        inserted at `i+1`. Requires `x` to not itself be full (already
        guaranteed by every call site, which only reaches a full child
        after confirming its parent has room, all the way up to the
        root — see insert()). */
    void split_child(Node* x, nat_t i)
    {
      Node* y = x->children[i];
      Node* z = new Node(y->leaf);

      for (nat_t j = T; j < y->keys.size(); ++j)
      {
        z->keys.append(y->keys[j]);
      }

      if (!y->leaf)
      {
        for (nat_t j = T; j < y->children.size(); ++j)
        {
          z->children.append(y->children[j]);
        }
      }

      Key mid = y->keys[T - 1];

      truncate(y->keys, T - 1);

      if (!y->leaf)
      {
        truncate(y->children, T);
      }

      x->children.insert(i + 1, z);
      x->keys.insert(i, mid);
    }

    void insert_nonfull(Node* x, const Key& k)
    {
      nat_t i = lower_bound(x, k);

      if (x->leaf)
      {
        x->keys.insert(i, k);
        return;
      }

      if (x->children[i]->num_keys() == MAX_KEYS)
      {
        split_child(x, i);

        if (cmp(x->keys[i], k))
        {
          ++i;
        }
      }

      insert_nonfull(x->children[i], k);
    }

    Node* search_node(Node* n, const Key& k, nat_t& pos) const
    {
      nat_t i = lower_bound(n, k);

      if (i < n->num_keys() && !cmp(k, n->keys[i]))
      {
        pos = i;
        return n;
      }

      if (n->leaf)
      {
        return nullptr;
      }

      return search_node(n->children[i], k, pos);
    }

    /** Finds and removes the maximum key in the subtree rooted at `n`
        (used to find `k`'s in-order predecessor when `k` is found at an
        internal node — see remove_from()). This must apply the same
        ensure_child_has_extra_key() fixup at *every* level while
        descending to the rightmost leaf, not just check the immediate
        child once: the caller only guarantees that `n` itself has more
        than MIN_KEYS keys, which says nothing about the nodes further
        down the rightmost path, any of which could be sitting at
        exactly MIN_KEYS and would underflow if a key were blindly
        pulled out of the leaf at the bottom. */
    Key remove_max(Node* n)
    {
      if (n->leaf)
      {
        return n->keys.remove_last();
      }

      nat_t last = n->children.size() - 1;
      ensure_child_has_extra_key(n, last);

      // ensure_child_has_extra_key() may have merged the last child into
      // its left sibling (shrinking n->children by one), so the new
      // last child is recomputed fresh rather than reusing `last`.
      return remove_max(n->children[n->children.size() - 1]);
    }

    Key remove_min(Node* n)
    {
      if (n->leaf)
      {
        return n->keys.remove_pos_closing_breach(0);
      }

      ensure_child_has_extra_key(n, 0);
      return remove_min(n->children[0]);
    }

    /** Merges `x->children[i]`, the key at `x->keys[i]`, and
        `x->children[i+1]` into a single node (this is only ever called
        when both children have exactly `MIN_KEYS` keys, so the result
        has exactly `MAX_KEYS`, never overflowing), replacing the two
        children and the key with the merged node in `x`. */
    void merge_children(Node* x, nat_t i)
    {
      Node* left = x->children[i];
      Node* right = x->children[i + 1];

      left->keys.append(x->keys.remove_pos_closing_breach(i));

      for (nat_t j = 0; j < right->keys.size(); ++j)
      {
        left->keys.append(right->keys[j]);
      }

      if (!left->leaf)
      {
        for (nat_t j = 0; j < right->children.size(); ++j)
        {
          left->children.append(right->children[j]);
        }
      }

      x->children.remove_pos_closing_breach(i + 1);
      delete right;
    }

    /** Ensures `x->children[i]` has more than MIN_KEYS keys before
        recursing into it, by borrowing a key from a sibling that has
        spare keys (rotating through the parent) or, if neither sibling
        has one to spare, merging with one of them — the "proactive"
        step that makes remove() a single top-down pass. */
    void ensure_child_has_extra_key(Node* x, nat_t i)
    {
      Node* c = x->children[i];

      if (c->num_keys() > MIN_KEYS)
      {
        return;
      }

      if (i > 0 && x->children[i - 1]->num_keys() > MIN_KEYS)
      {
        Node* left = x->children[i - 1];
        c->keys.insert(0, x->keys[i - 1]);
        x->keys[i - 1] = left->keys.remove_last();

        if (!c->leaf)
        {
          c->children.insert(0, left->children.remove_last());
        }
      }
      else if (i < x->children.size() - 1 &&
               x->children[i + 1]->num_keys() > MIN_KEYS)
      {
        Node* right = x->children[i + 1];
        c->keys.append(x->keys[i]);
        x->keys[i] = right->keys.remove_pos_closing_breach(0);

        if (!c->leaf)
        {
          c->children.append(right->children.remove_pos_closing_breach(0));
        }
      }
      else if (i > 0)
      {
        merge_children(x, i - 1);
      }
      else
      {
        merge_children(x, i);
      }
    }

    /** Removes `k` from the subtree rooted at `n`, which the caller
        guarantees already has more than MIN_KEYS keys (true for the
        root unconditionally, and for every other node because the
        caller called ensure_child_has_extra_key() first) — so this
        never needs to underflow-and-fix a node *after* the fact. */
    void remove_from(Node* n, const Key& k)
    {
      nat_t i = lower_bound(n, k);

      if (i < n->num_keys() && !cmp(k, n->keys[i]))
      {
        if (n->leaf)
        {
          n->keys.remove_pos_closing_breach(i);
          return;
        }

        if (n->children[i]->num_keys() > MIN_KEYS)
        {
          n->keys[i] = remove_max(n->children[i]);
        }
        else if (n->children[i + 1]->num_keys() > MIN_KEYS)
        {
          n->keys[i] = remove_min(n->children[i + 1]);
        }
        else
        {
          merge_children(n, i);
          remove_from(n->children[i], k);
        }

        return;
      }

      if (n->leaf)
      {
        return; // k is not present; caller already confirmed via search()
      }

      ensure_child_has_extra_key(n, i);

      // ensure_child_has_extra_key() may have merged children[i] into
      // children[i-1] (shifting indices), so recompute where k now
      // belongs rather than reusing `i`.
      remove_from(n->children[lower_bound(n, k)], k);
    }

  public:
    using ItemType = Key;
    using KeyType = Key;
    using DataType = Key;
    using ValueType = Key;
    using SizeType = nat_t;
    using CmpType = Cmp;

    bool verify() const
    {
      if (root->num_keys() == 0)
      {
        return root->leaf;
      }

      int_t leaf_depth = -1;
      return verify(root, true, leaf_depth, 0);
    }

    bool verify(Node* n, bool is_root, int_t& leaf_depth, nat_t depth) const
    {
      nat_t nk = n->num_keys();

      if (!is_root && (nk < MIN_KEYS || nk > MAX_KEYS))
      {
        return false;
      }

      if (nk > MAX_KEYS)
      {
        return false;
      }

      for (nat_t i = 1; i < nk; ++i)
      {
        if (!cmp(n->keys[i - 1], n->keys[i]))
        {
          return false;
        }
      }

      if (n->leaf)
      {
        if (leaf_depth == -1)
        {
          leaf_depth = int_t(depth);
        }
        else if (leaf_depth != int_t(depth))
        {
          return false;
        }

        return true;
      }

      if (n->children.size() != nk + 1)
      {
        return false;
      }

      for (nat_t i = 0; i < n->children.size(); ++i)
      {
        if (!verify(n->children[i], false, leaf_depth, depth + 1))
        {
          return false;
        }
      }

      return true;
    }

    BTree(Cmp& _cmp)
        : root(new Node(true)), num_items(0), cmp(_cmp)
    {
      // empty
    }

    /** @see RankedTreap's matching constructor for why this cannot
        simply delegate to the Cmp& overload above, and why
        copy-assignment (not move) is used for `default_cmp`. */
    BTree(Cmp&& _cmp = Cmp())
        : root(new Node(true)), num_items(0), cmp(this->default_cmp)
    {
      this->default_cmp = _cmp;
    }

    BTree(const BTree& t)
        : root(copy(t.root)), num_items(t.num_items), cmp(cmp_for_copy(*this, t))
    {
      // empty
    }

    BTree(BTree&& t)
        : BTree()
    {
      swap(t);
    }

    BTree(const std::initializer_list<Key>&);

    ~BTree()
    {
      destroy(root);
    }

    BTree& operator=(const BTree& t)
    {
      if (this == &t)
      {
        return *this;
      }

      destroy(root);
      root = copy(t.root);
      num_items = t.num_items;
      cmp = t.cmp;
      return *this;
    }

    BTree& operator=(BTree&& t)
    {
      swap(t);
      return *this;
    }

    void swap(BTree& t)
    {
      std::swap(root, t.root);
      std::swap(num_items, t.num_items);
      std::swap(cmp, t.cmp);
    }

    bool is_empty() const
    {
      return num_items == 0;
    }

    bool is_sorted() const
    {
      return true;
    }

    nat_t size() const
    {
      return num_items;
    }

    void clear()
    {
      destroy(root);
      root = new Node(true);
      num_items = 0;
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
      nat_t pos;

      if (search_node(root, k, pos) != nullptr)
      {
        return nullptr; // duplicate
      }

      if (root->num_keys() == MAX_KEYS)
      {
        Node* s = new Node(false);
        s->children.append(root);
        root = s;
        split_child(s, 0);
      }

      insert_nonfull(root, k);
      ++num_items;

      Node* n = search_node(root, k, pos);
      return &n->keys[pos];
    }

    Key* append(const Key& k)
    {
      return insert(k);
    }

    Key* search(const Key& k)
    {
      nat_t pos;
      Node* n = search_node(root, k, pos);
      return n == nullptr ? nullptr : &n->keys[pos];
    }

    const Key* search(const Key& k) const
    {
      nat_t pos;
      Node* n = search_node(root, k, pos);
      return n == nullptr ? nullptr : &n->keys[pos];
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
      nat_t pos;

      if (search_node(root, k, pos) == nullptr)
      {
        return false;
      }

      remove_from(root, k);
      --num_items;

      if (root->num_keys() == 0 && !root->leaf)
      {
        Node* old_root = root;
        root = root->children[0];
        delete old_root;
      }

      return true;
    }

    const Key& min() const
    {
      if (is_empty())
      {
        throw std::underflow_error("BTree is empty");
      }

      Node* n = root;

      while (!n->leaf)
      {
        n = n->children[0];
      }

      return n->keys[0];
    }

    const Key& max() const
    {
      if (is_empty())
      {
        throw std::underflow_error("BTree is empty");
      }

      Node* n = root;

      while (!n->leaf)
      {
        n = n->children[n->children.size() - 1];
      }

      return n->keys[n->keys.size() - 1];
    }

    /** Visits every key in ascending order — an in-order traversal
        generalized to "recurse into child i, visit key i" repeated for
        every key/child pair in a node. There is no incremental Iterator
        here (unlike every binary tree in this library): a resumable
        in-order cursor over a B-tree needs a stack of (node, key-index)
        pairs rather than the plain node stack a binary tree's
        InorderIterator uses, and no algorithm elsewhere in this session
        needed that; for_each() covers every use seen so far (equal(),
        map(), fold(), ...) via ContainerAlgorithms. */
    template <class Op>
    void for_each_inorder(Op& op) const
    {
      for_each_inorder_rec(root, op);
    }

    template <class Op>
    void for_each_inorder(Op&& op = Op()) const
    {
      for_each_inorder<Op>(op);
    }

  private:
    template <class Op>
    static void for_each_inorder_rec(Node* n, Op& op)
    {
      if (n->leaf)
      {
        for (nat_t i = 0; i < n->keys.size(); ++i)
        {
          op(n->keys[i]);
        }

        return;
      }

      for (nat_t i = 0; i < n->keys.size(); ++i)
      {
        for_each_inorder_rec(n->children[i], op);
        op(n->keys[i]);
      }

      for_each_inorder_rec(n->children[n->children.size() - 1], op);
    }
  };

  template <typename Key, class Cmp, nat_t MinDegree>
  BTree<Key, Cmp, MinDegree>::BTree(const std::initializer_list<Key>& l)
      : BTree()
  {
    for (const auto& item : l)
    {
      append(item);
    }
  }

} // end namespace Designar
