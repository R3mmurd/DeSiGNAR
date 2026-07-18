/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file randomizedtree.hpp
    @brief RandomizedTree: a randomized binary search tree set
    implementation (Martínez & Roura, 1997/1998; the authors' own name
    for it, "ABBA" — Árbol Binario de Búsqueda Aleatorizado — is kept
    here as the term this algorithm is cited by), which balances via
    biased random choices made at insertion/deletion time rather than a
    per-node priority (Treap) or a structural invariant (AVL,
    red-black).
    @ingroup Trees
*/

#pragma once

#include <nodesdef.hpp>
#include <containeralgorithms.hpp>
#include <setalgorithms.hpp>
#include <stack.hpp>
#include <iterator.hpp>
#include <typetraits.hpp>
#include <random.hpp>

namespace Designar
{
    template <typename Key>
    class RandTreeNode : public BaseBinTreeNode<Key, RandTreeNode<Key>,
                                                BinTreeNodeNullValue::SENTINEL>
    {
        using BaseNode = BaseBinTreeNode<Key, RandTreeNode<Key>,
                                         BinTreeNodeNullValue::SENTINEL>;

        nat_t count;

    public:
        RandTreeNode() : BaseNode()
        {
            // empty
        }

        RandTreeNode(const Key& k) : BaseNode(k), count(1)
        {
            // empty
        }

        RandTreeNode(Key&& k) : BaseNode(std::forward<Key>(k)), count(1)
        {
            // empty
        }

        /** @see RankedTreap's sentinel for why this is fixed at 0. */
        RandTreeNode(BinTreeNodeCtor ctor) : BaseNode(ctor), count(0)
        {
            // empty
        }

        nat_t& get_count()
        {
            return count;
        }
    };

    /** A randomized binary search tree (Martínez & Roura, 1997/1998; the
        authors call it "ABBA" — Árbol Binario de Búsqueda Aleatorizado).

        Unlike Treap (which balances via a random priority assigned once
        per node and never touched again) or AVL/red-black (which balance
        via a deterministic structural invariant restored by rotations),
        this tree balances by making the random choices *at the moment of
        insertion and deletion themselves*, biased by subtree sizes:

          - insert_root(): performs the ordinary recursive BST insertion of
            a new key, then rotates it up one level at a time until it
            becomes the root of the subtree it was inserted into.
          - insert(): at every node visited on the way down, with
            probability 1/(subtree_size + 1) calls insert_root() right
            there instead of recursing further — i.e. every one of the
            (subtree_size + 1) possible ranks the new key could end up at
            is equally likely to make it the new root of that subtree.
          - join(l, r): merges two subtrees (all of `l` known to precede
            all of `r`) into one, picking the new root from `l` or `r`
            with probability proportional to each side's size — the same
            distribution a fresh random BST over the combined keys would
            have at its root.
          - remove(): finds the node, discards it, and replaces it with
            join(left subtree, right subtree).

        The result: at all times, the tree's shape has *exactly* the
        probability distribution of a BST built by inserting its current
        keys in a uniformly random order — regardless of the actual
        insertion/deletion history — which gives O(lg n) expected height
        without storing any per-node priority (only a subtree-size count,
        needed for the biased coin flips themselves).

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class RandomizedTree
        : private DefaultCmpHolder<Cmp>,
          public ContainerAlgorithms<RandomizedTree<Key, Cmp>, Key>,
          public SetAlgorithms<RandomizedTree<Key, Cmp>, Key>
    {
    public:
        using Node = RandTreeNode<Key>;

    private:
        Node head;
        Node*& root;
        Cmp& cmp;
        rng_t rng;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(RandomizedTree& self, const RandomizedTree& t)
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

        static Node* rotate_left(Node* r)
        {
            Node* q = generic_rotate_left(r);
            COUNT(r) = COUNT(L(r)) + COUNT(R(r)) + 1;
            COUNT(q) = COUNT(L(q)) + COUNT(R(q)) + 1;
            return q;
        }

        static Node* rotate_right(Node* r)
        {
            Node* q = generic_rotate_right(r);
            COUNT(r) = COUNT(L(r)) + COUNT(R(r)) + 1;
            COUNT(q) = COUNT(L(q)) + COUNT(R(q)) + 1;
            return q;
        }

        static Node* insert_root(Node*& r, Node* p, Cmp& cmp)
        {
            if (r == Node::null)
            {
                r = p;
                return p;
            }

            if (cmp(KEY(p), KEY(r)))
            {
                insert_root(L(r), p, cmp);
                r = rotate_right(r);
            }
            else
            {
                insert_root(R(r), p, cmp);
                r = rotate_left(r);
            }

            return p;
        }

        Node* insert(Node*& r, Node* p, Cmp& cmp)
        {
            if (r == Node::null)
            {
                r = p;
                return p;
            }

            if (random_uniform(rng, COUNT(r) + 1) == 0)
            {
                return insert_root(r, p, cmp);
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

            ++COUNT(r);
            return result;
        }

        Node* insert_dup(Node*& r, Node* p, Cmp& cmp)
        {
            if (r == Node::null)
            {
                r = p;
                return p;
            }

            if (random_uniform(rng, COUNT(r) + 1) == 0)
            {
                return insert_root(r, p, cmp);
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

            ++COUNT(r);
            return result;
        }

        static Node* search(Node* r, const Key& k, Cmp& cmp)
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

        Node* search_or_insert(Node*& r, Node* p, Cmp& cmp)
        {
            if (r == Node::null)
            {
                r = p;
                return p;
            }

            if (cmp(KEY(p), KEY(r)))
            {
                Node* result = search_or_insert(L(r), p, cmp);

                if (result == p)
                {
                    ++COUNT(r);
                }

                return result;
            }
            else if (cmp(KEY(r), KEY(p)))
            {
                Node* result = search_or_insert(R(r), p, cmp);

                if (result == p)
                {
                    ++COUNT(r);
                }

                return result;
            }

            return r;
        }

        /** Merges `l` and `r` (every key of `l` known to precede every key
            of `r`) into a single subtree, choosing the new root from `l` or
            `r` with probability proportional to each side's size — the same
            distribution a fresh random BST over the combined keyset would
            produce at its root. This is what keeps the tree's shape random
            (rather than, say, always favoring one side) across deletions. */
        Node* join(Node*& l, Node*& r)
        {
            if (l == Node::null)
            {
                return r;
            }

            if (r == Node::null)
            {
                return l;
            }

            if (random_uniform(rng, COUNT(l) + COUNT(r)) < COUNT(l))
            {
                R(l) = join(R(l), r);
                COUNT(l) = COUNT(L(l)) + COUNT(R(l)) + 1;
                return l;
            }
            else
            {
                L(r) = join(l, L(r));
                COUNT(r) = COUNT(L(r)) + COUNT(R(r)) + 1;
                return r;
            }
        }

        Node* remove(Node*& r, const Key& k, Cmp& cmp)
        {
            if (r == Node::null)
            {
                return Node::null;
            }

            if (cmp(k, KEY(r)))
            {
                Node* result = remove(L(r), k, cmp);

                if (result != Node::null)
                {
                    --COUNT(r);
                }

                return result;
            }
            else if (cmp(KEY(r), k))
            {
                Node* result = remove(R(r), k, cmp);

                if (result != Node::null)
                {
                    --COUNT(r);
                }

                return result;
            }

            Node* ret_val = r;
            r = join(L(r), R(r));
            return ret_val;
        }

        static void split_pos(Node*, nat_t, Node*&, Node*&);

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

        bool verify() const
        {
            return verify(root, cmp);
        }

        static bool verify(Node* r, Cmp& cmp)
        {
            if (r == Node::null)
            {
                return true;
            }

            Node* lc = L(r);
            Node* rc = R(r);

            if (!verify(lc, cmp) || !verify(rc, cmp))
            {
                return false;
            }

            if (COUNT(r) != COUNT(lc) + COUNT(rc) + 1)
            {
                return false;
            }

            if (lc != Node::null && !cmp(KEY(lc), KEY(r)))
            {
                return false;
            }

            if (rc != Node::null && !cmp(KEY(r), KEY(rc)))
            {
                return false;
            }

            return true;
        }

        RandomizedTree(rng_seed_t seed, Cmp& _cmp)
            : head(), root(L(&head)), cmp(_cmp), rng(seed)
        {
            // empty
        }

        RandomizedTree(Cmp& _cmp) : RandomizedTree(time(nullptr), _cmp)
        {
            // empty
        }

        /** @see RankedTreap's matching constructors for why these cannot
            simply delegate to the Cmp& overloads above, and why
            copy-assignment (not move) is used for `default_cmp`. */
        RandomizedTree(rng_seed_t seed, Cmp&& _cmp = Cmp())
            : head(), root(L(&head)), cmp(this->default_cmp), rng(seed)
        {
            this->default_cmp = _cmp;
        }

        RandomizedTree(Cmp&& _cmp = Cmp())
            : head(), root(L(&head)), cmp(this->default_cmp), rng(time(nullptr))
        {
            this->default_cmp = _cmp;
        }

        RandomizedTree(const RandomizedTree& t)
            : head(),
              root(L(&head)),
              cmp(cmp_for_copy(*this, t)),
              rng(time(nullptr))
        {
            root = copy(t.root);
        }

        RandomizedTree(RandomizedTree&& t) : RandomizedTree()
        {
            swap(t);
        }

        RandomizedTree(const std::initializer_list<Key>&);

        ~RandomizedTree()
        {
            clear();
        }

        RandomizedTree& operator=(const RandomizedTree& t)
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

        RandomizedTree& operator=(RandomizedTree&& t)
        {
            swap(t);
            return *this;
        }

        void swap(RandomizedTree& t)
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

        /** @see generic_bst_search_by (nodesdef.hpp) for why this exists:
            heterogeneous lookup by anything comparable to `Key` via
            `cmp`, not necessarily `Key` itself. */
        template <typename K>
        Key* search_by(const K& k)
        {
            Node* result = generic_bst_search_by<Node>(root, k, cmp);
            return result == Node::null ? nullptr : &KEY(result);
        }

        template <typename K>
        const Key* search_by(const K& k) const
        {
            Node* result = generic_bst_search_by<Node>(root, k, cmp);
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

            Node* r = root;

            while (COUNT(L(r)) != i)
            {
                if (i < COUNT(L(r)))
                {
                    r = L(r);
                }
                else
                {
                    i -= COUNT(L(r)) + 1;
                    r = R(r);
                }
            }

            return KEY(r);
        }

        const Key& select(nat_t i) const
        {
            return const_cast<RandomizedTree*>(this)->select(i);
        }

        int_t position(const Key& k) const
        {
            return generic_position(root, k, cmp);
        }

        /** Splits into "the smallest `i` elements" / "the rest". Unlike
            `remove()`, which re-randomizes via `join()` after cutting a
            single key out, this just cuts the tree along the search
            path for position `i` and reattaches the two sides directly
            — the same technique RankedTreap's `split_pos` uses, and for
            the same reason it needs no rebalancing here either: neither
            tree's balance invariant is a *structural* property of the
            post-split shape (Treap's is a per-node priority; this tree
            has no per-node balancing data at all, only `COUNT`), so
            simply partitioning the existing structure at the cut point
            already leaves both sides exactly as valid as the whole tree
            was. Every operation performed on either half afterward
            (insert/remove) re-randomizes exactly as it always does,
            regardless of how that half's current shape came to be. */
        std::tuple<RandomizedTree, RandomizedTree> split_pos(nat_t i)
        {
            if (i >= size())
            {
                throw std::out_of_range("Infix position is out of range");
            }

            RandomizedTree ts, tg;
            split_pos(root, i, ts.root, tg.root);
            root = Node::null;
            return std::make_tuple(std::move(ts), std::move(tg));
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
            friend class RandomizedTree;

            RandomizedTree* set_ptr = nullptr;
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
            InorderIterator(const RandomizedTree& t, int)
                : set_ptr(const_cast<RandomizedTree*>(&t)),
                  root(set_ptr->root),
                  curr(Node::null)
            {
                // empty
            }

            Node* get_location() const
            {
                return curr;
            }

        public:
            InorderIterator(const RandomizedTree& t)
                : set_ptr(const_cast<RandomizedTree*>(&t)), root(set_ptr->root)
            {
                init();
            }

            InorderIterator(const InorderIterator& it)
                : set_ptr(it.set_ptr),
                  stack(it.stack),
                  root(it.root),
                  curr(it.curr)
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
            friend class RandomizedTree;
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
    RandomizedTree<Key, Cmp>::RandomizedTree(
        const std::initializer_list<Key>& l)
        : RandomizedTree()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp>
    typename RandomizedTree<Key, Cmp>::Node*
    RandomizedTree<Key, Cmp>::copy(Node* r)
    {
        if (r == Node::null)
        {
            return Node::null;
        }

        Node* p = new Node(KEY(r));
        COUNT(p) = COUNT(r);
        L(p) = copy(L(r));
        R(p) = copy(R(r));
        return p;
    }

    template <typename Key, class Cmp>
    void RandomizedTree<Key, Cmp>::destroy(Node*& r)
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
    void RandomizedTree<Key, Cmp>::split_pos(Node* r, nat_t i, Node*& ts,
                                             Node*& tg)
    {
        nat_t left_count = COUNT(L(r));

        if (i == left_count)
        {
            ts = L(r);
            tg = r;
            L(tg) = Node::null;
            COUNT(tg) -= COUNT(ts);
            return;
        }

        if (i < left_count)
        {
            split_pos(L(r), i, ts, L(r));
            tg = r;
            COUNT(r) -= COUNT(ts);
        }
        else
        {
            split_pos(R(r), i - left_count - 1, R(r), tg);
            ts = r;
            COUNT(r) -= COUNT(tg);
        }
    }

} // end namespace Designar
