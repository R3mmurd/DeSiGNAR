/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file btree.hpp
    @brief BTree (classic Bayer & McCreight B-tree) and BPlusTree
    (leaf-linked variant) set implementations — multi-way,
    always-perfectly-height-balanced search trees where every node holds
    several keys and children rather than exactly one key and two
    children, designed around minimizing the number of nodes touched per
    operation (originally to minimize disk-block reads; the same "wide,
    shallow tree" shape is also just a fundamentally different balancing
    trade-off from every binary tree in this library).
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
                : keys(MAX_KEYS + 1),
                  children(is_leaf ? 0 : MAX_KEYS + 2),
                  leaf(is_leaf)
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
                z->keys.append(std::move(y->keys[j]));
            }

            if (!y->leaf)
            {
                for (nat_t j = T; j < y->children.size(); ++j)
                {
                    z->children.append(y->children[j]);
                }
            }

            Key mid = std::move(y->keys[T - 1]);

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
                left->keys.append(std::move(right->keys[j]));
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
                c->keys.insert(0, std::move(x->keys[i - 1]));
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
                c->keys.append(std::move(x->keys[i]));
                x->keys[i] = right->keys.remove_pos_closing_breach(0);

                if (!c->leaf)
                {
                    c->children.append(
                        right->children.remove_pos_closing_breach(0));
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
                return; // k is not present; caller already confirmed via
                        // search()
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

        BTree(Cmp& _cmp) : root(new Node(true)), num_items(0), cmp(_cmp)
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
            : root(copy(t.root)),
              num_items(t.num_items),
              cmp(cmp_for_copy(*this, t))
        {
            // empty
        }

        BTree(BTree&& t) : BTree()
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

    /** A B+ tree: like BTree above, but only leaves hold real data —
        internal nodes hold routing keys (each a *copy* of some leaf key,
        used purely to guide search, and allowed to go "stale" after that
        leaf key is later removed elsewhere, same as any B+ tree) and
        child pointers, nothing else. Leaves are additionally linked
        left-to-right (`Node::next`), so `for_each_inorder`/iteration
        walks a flat singly-linked list instead of recursing through the
        whole tree — the actual reason B+ trees are the structure real
        databases/filesystems use for range scans over BTree, despite
        doing strictly more node touches per single-key lookup (every
        search descends all the way to a leaf; BTree can stop early at
        an internal-node match).

        Splitting a full child on the way down (same preemptive,
        single-pass insert as BTree) differs by node kind: an internal
        split moves its median key *up* into the parent (as in BTree,
        since it's not needed in two places); a leaf split *copies* its
        first surviving right-hand key up as the new separator while
        keeping that key in the leaf too (data lives only in leaves, so
        nothing may be removed from one just to satisfy the parent).
        Deletion mirrors BTree's proactive
        ensure-the-child-has-a-spare-key-before-descending shape, but
        always finishes by removing directly from a leaf (never via an
        in-order-predecessor/successor swap through an internal node —
        there is no data at internal nodes to swap into); merging two
        leaves discards the separator between them instead of pulling it
        down (a leaf doesn't need it) and relinks the leaf chain around
        the removed leaf.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>, nat_t MinDegree = 3>
    class BPlusTree
        : private DefaultCmpHolder<Cmp>,
          public ContainerAlgorithms<BPlusTree<Key, Cmp, MinDegree>, Key>,
          public SetAlgorithms<BPlusTree<Key, Cmp, MinDegree>, Key>
    {
        static_assert(MinDegree >= 2, "MinDegree must be at least 2");

        static constexpr nat_t T = MinDegree;
        static constexpr nat_t MAX_KEYS = 2 * T - 1;
        static constexpr nat_t MIN_KEYS = T - 1;

        struct Node
        {
            DynArray<Key> keys;
            DynArray<Node*> children;
            Node* next = nullptr;
            bool leaf;

            Node(bool is_leaf)
                : keys(MAX_KEYS + 1),
                  children(is_leaf ? 0 : MAX_KEYS + 2),
                  leaf(is_leaf)
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
        static Cmp& cmp_for_copy(BPlusTree& self, const BPlusTree& t)
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

        /** `copy()` above rebuilds structure only — every copied leaf's
            `next` starts null. This relinks the copy's leaves
            left-to-right in a single pass, returning the last leaf
            visited (so a nested call can continue the chain from its
            caller's last leaf), the same "carry a cursor through the
            recursion" shape join()/split_pos() elsewhere in this
            library's trees use. */
        static Node* relink_leaves(Node* n, Node* prev)
        {
            if (n->leaf)
            {
                if (prev != nullptr)
                {
                    prev->next = n;
                }

                return n;
            }

            for (nat_t i = 0; i < n->children.size(); ++i)
            {
                prev = relink_leaves(n->children[i], prev);
            }

            return prev;
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

        /** @see BTree::lower_bound — identical routing rule (first index
            whose key is not less than `k`), reused unchanged for
            descending through this tree's internal nodes too: an
            internal separator `keys[i]` is always a copy of the
            smallest key under `children[i + 1]`, so "descend into
            children[i] for the smallest such i" is exactly the same
            rule whether `n` holds real data (BTree) or pure routing
            copies (this class). */
        nat_t lower_bound(Node* n, const Key& k) const
        {
            nat_t i = 0;

            while (i < n->num_keys() && cmp(n->keys[i], k))
            {
                ++i;
            }

            return i;
        }

        /** Which child of the internal node `n` to descend into for
            `k` — the first `i` with `k < keys[i]` (strictly), i.e. the
            last child whose separator is `<= k`. Deliberately not
            `lower_bound` (which stops at the first `keys[i] >= k`,
            treating equality as "found here"): that's the right rule
            for BTree, where a key equal to an internal key really is
            stored right there, but here a separator is only ever a
            *copy* of the smallest key under `children[i + 1]` — so on
            `k == keys[i]` the actual (still-live) key is in
            `children[i + 1]`, the child to its right, not `children[i]`
            itself. */
        nat_t child_index(Node* n, const Key& k) const
        {
            nat_t i = 0;

            while (i < n->num_keys() && !cmp(k, n->keys[i]))
            {
                ++i;
            }

            return i;
        }

        /** Descends from `n` to the leaf `k` would belong in, always
            going all the way down regardless of any separator match
            along the way — unlike BTree, an internal-node key match
            here doesn't mean `k` is present (only that some leaf key
            equal to it existed at split time), so only the leaf itself
            can answer that. */
        Node* find_leaf(Node* n, const Key& k) const
        {
            while (!n->leaf)
            {
                n = n->children[child_index(n, k)];
            }

            return n;
        }

        Node* search_node(Node* n, const Key& k, nat_t& pos) const
        {
            Node* leaf = find_leaf(n, k);
            nat_t i = lower_bound(leaf, k);

            if (i < leaf->num_keys() && !cmp(k, leaf->keys[i]))
            {
                pos = i;
                return leaf;
            }

            return nullptr;
        }

        /** Splits the full child `x->children[i]`. For an internal
            child this is byte-for-byte BTree::split_child (the median
            moves up, unduplicated). For a leaf, the left half keeps the
            first `T` keys and the right half gets the remaining `T - 1`
            (`2T - 1` total, same as BTree's split point), but the
            separator promoted to `x` is a *copy* of the right half's
            first key — that key stays in the leaf, since leaves are
            where this tree's actual data lives — and the two leaves are
            spliced into the leaf chain around wherever `y` was. */
        void split_child(Node* x, nat_t i)
        {
            Node* y = x->children[i];
            Node* z = new Node(y->leaf);

            if (y->leaf)
            {
                for (nat_t j = T; j < y->keys.size(); ++j)
                {
                    z->keys.append(y->keys[j]);
                }

                truncate(y->keys, T);

                z->next = y->next;
                y->next = z;

                x->children.insert(i + 1, z);
                x->keys.insert(i, z->keys[0]);
                return;
            }

            for (nat_t j = T; j < y->keys.size(); ++j)
            {
                z->keys.append(std::move(y->keys[j]));
            }

            for (nat_t j = T; j < y->children.size(); ++j)
            {
                z->children.append(y->children[j]);
            }

            Key mid = std::move(y->keys[T - 1]);

            truncate(y->keys, T - 1);
            truncate(y->children, T);

            x->children.insert(i + 1, z);
            x->keys.insert(i, std::move(mid));
        }

        void insert_nonfull(Node* x, const Key& k)
        {
            if (x->leaf)
            {
                x->keys.insert(lower_bound(x, k), k);
                return;
            }

            nat_t i = child_index(x, k);

            if (x->children[i]->num_keys() == MAX_KEYS)
            {
                split_child(x, i);

                if (!cmp(k, x->keys[i]))
                {
                    ++i;
                }
            }

            insert_nonfull(x->children[i], k);
        }

        /** Merges `x->children[i]`, and (for an internal pair) the key
            at `x->keys[i]`, with `x->children[i + 1]`. For a leaf pair,
            `x->keys[i]` — the separator between them — is simply
            discarded rather than pulled down (a leaf's own keys already
            say everything it needs to), and the leaf chain is relinked
            around the removed leaf. */
        void merge_children(Node* x, nat_t i)
        {
            Node* left = x->children[i];
            Node* right = x->children[i + 1];

            if (left->leaf)
            {
                x->keys.remove_pos_closing_breach(i);

                for (nat_t j = 0; j < right->keys.size(); ++j)
                {
                    left->keys.append(std::move(right->keys[j]));
                }

                left->next = right->next;
            }
            else
            {
                left->keys.append(x->keys.remove_pos_closing_breach(i));

                for (nat_t j = 0; j < right->keys.size(); ++j)
                {
                    left->keys.append(std::move(right->keys[j]));
                }

                for (nat_t j = 0; j < right->children.size(); ++j)
                {
                    left->children.append(right->children[j]);
                }
            }

            x->children.remove_pos_closing_breach(i + 1);
            delete right;
        }

        /** @see BTree::ensure_child_has_extra_key — same proactive
            borrow-or-merge shape, except a leaf borrow moves a key
            directly between the two leaves (no separator rotation
            needed) and refreshes the separator in `x` to a fresh copy
            of whichever leaf key now sits on the boundary. */
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

                if (c->leaf)
                {
                    c->keys.insert(0, left->keys.remove_last());
                    x->keys[i - 1] = c->keys[0];
                }
                else
                {
                    c->keys.insert(0, std::move(x->keys[i - 1]));
                    x->keys[i - 1] = left->keys.remove_last();
                    c->children.insert(0, left->children.remove_last());
                }
            }
            else if (i < x->children.size() - 1 &&
                     x->children[i + 1]->num_keys() > MIN_KEYS)
            {
                Node* right = x->children[i + 1];

                if (c->leaf)
                {
                    c->keys.append(right->keys.remove_pos_closing_breach(0));
                    x->keys[i] = right->keys[0];
                }
                else
                {
                    c->keys.append(std::move(x->keys[i]));
                    x->keys[i] = right->keys.remove_pos_closing_breach(0);
                    c->children.append(
                        right->children.remove_pos_closing_breach(0));
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
            guarantees already has more than MIN_KEYS keys. Unlike
            BTree::remove_from, there is no in-order-predecessor/
            successor swap case: a match at an internal node is only
            ever a routing copy, so this always keeps descending —
            proactively fixing up whichever child it's about to enter —
            until it reaches the leaf actually holding `k`. */
        void remove_from(Node* n, const Key& k)
        {
            if (n->leaf)
            {
                nat_t i = lower_bound(n, k);
                n->keys.remove_pos_closing_breach(i);
                return;
            }

            nat_t i = child_index(n, k);
            ensure_child_has_extra_key(n, i);

            // ensure_child_has_extra_key() may have merged children[i] into
            // children[i-1] (shifting indices), so recompute where k now
            // belongs rather than reusing `i`.
            remove_from(n->children[child_index(n, k)], k);
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

            if (!verify(root, true, leaf_depth, 0))
            {
                return false;
            }

            return verify_leaf_chain();
        }

        bool verify(Node* n, bool is_root, int_t& leaf_depth,
                    nat_t depth) const
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

        /** Walks the leaf chain from the leftmost leaf, checking that
            keys are strictly increasing across leaf boundaries too (not
            just within one leaf, which `verify()` above already checks)
            and that the chain visits every key exactly once — this
            tree's whole reason for existing over plain BTree, so it
            gets its own dedicated check. */
        bool verify_leaf_chain() const
        {
            Node* n = root;

            while (!n->leaf)
            {
                n = n->children[0];
            }

            nat_t count = 0;
            const Key* prev = nullptr;

            while (n != nullptr)
            {
                for (nat_t i = 0; i < n->keys.size(); ++i)
                {
                    if (prev != nullptr && !cmp(*prev, n->keys[i]))
                    {
                        return false;
                    }

                    prev = &n->keys[i];
                    ++count;
                }

                n = n->next;
            }

            return count == num_items;
        }

        BPlusTree(Cmp& _cmp) : root(new Node(true)), num_items(0), cmp(_cmp)
        {
            // empty
        }

        /** @see RankedTreap's matching constructor (treap.hpp) for why
            this cannot simply delegate to the Cmp& overload above, and
            why copy-assignment (not move) is used for `default_cmp`. */
        BPlusTree(Cmp&& _cmp = Cmp())
            : root(new Node(true)), num_items(0), cmp(this->default_cmp)
        {
            this->default_cmp = _cmp;
        }

        BPlusTree(const BPlusTree& t)
            : root(copy(t.root)),
              num_items(t.num_items),
              cmp(cmp_for_copy(*this, t))
        {
            relink_leaves(root, nullptr);
        }

        BPlusTree(BPlusTree&& t) : BPlusTree()
        {
            swap(t);
        }

        BPlusTree(const std::initializer_list<Key>&);

        ~BPlusTree()
        {
            destroy(root);
        }

        BPlusTree& operator=(const BPlusTree& t)
        {
            if (this == &t)
            {
                return *this;
            }

            destroy(root);
            root = copy(t.root);
            relink_leaves(root, nullptr);
            num_items = t.num_items;
            cmp = t.cmp;
            return *this;
        }

        BPlusTree& operator=(BPlusTree&& t)
        {
            swap(t);
            return *this;
        }

        void swap(BPlusTree& t)
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

            if (!root->leaf && root->num_keys() == 0)
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
                throw std::underflow_error("BPlusTree is empty");
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
                throw std::underflow_error("BPlusTree is empty");
            }

            Node* n = root;

            while (!n->leaf)
            {
                n = n->children[n->children.size() - 1];
            }

            return n->keys[n->keys.size() - 1];
        }

        /** Unlike BTree::for_each_inorder (which recurses through every
            internal node), this walks the flat leaf chain directly —
            the whole reason this tree links its leaves in the first
            place. */
        template <class Op>
        void for_each_inorder(Op& op) const
        {
            Node* n = root;

            while (!n->leaf)
            {
                n = n->children[0];
            }

            while (n != nullptr)
            {
                for (nat_t i = 0; i < n->keys.size(); ++i)
                {
                    op(n->keys[i]);
                }

                n = n->next;
            }
        }

        template <class Op>
        void for_each_inorder(Op&& op = Op()) const
        {
            for_each_inorder<Op>(op);
        }
    };

    template <typename Key, class Cmp, nat_t MinDegree>
    BPlusTree<Key, Cmp, MinDegree>::BPlusTree(
        const std::initializer_list<Key>& l)
        : BPlusTree()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

} // end namespace Designar
