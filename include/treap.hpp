/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file treap.hpp
    @brief A plain (unranked) Treap set implementation.
    @see tree.hpp for RankedTreap, the order-statistics-capable sibling
    of the Treap implemented here.
    @ingroup Trees
*/

#pragma once

#include <tree.hpp> // for PRIOR<>() and the shared node/tree scaffolding it pulls in

namespace Designar
{
    template <typename Key>
    class TreapNode : public BaseBinTreeNode<Key, TreapNode<Key>,
                                             BinTreeNodeNullValue::SENTINEL>
    {
        using BaseNode = BaseBinTreeNode<Key, TreapNode<Key>,
                                         BinTreeNodeNullValue::SENTINEL>;

        rng_seed_t prior;

    public:
        TreapNode() : BaseNode()
        {
            // empty
        }

        TreapNode(const Key& k) : BaseNode(k), prior(0)
        {
            // empty
        }

        TreapNode(Key&& k) : BaseNode(std::forward<Key>(k)), prior(0)
        {
            // empty
        }

        TreapNode(BinTreeNodeCtor ctor) : BaseNode(ctor), prior(rng_t::max())
        {
            // empty
        }

        rng_seed_t& get_priority()
        {
            return prior;
        }
    };

    /** A plain (unranked) Treap: a binary search tree that is kept
        balanced *in expectation* by assigning every node a random
        priority at insertion time and maintaining the max-heap property
        on priorities via rotations (a node's priority must be <= both of
        its children's — this file uses "smaller priority wins", i.e. the
        node closest to the root has the smallest priority value). Because
        the priorities are random, the resulting shape has the same
        probability distribution as a BST built by inserting the keys in
        random order, which gives O(log n) expected height regardless of
        insertion order.

        This is the same balancing idea as RankedTreap (tree.hpp), minus
        the per-node subtree-size bookkeeping RankedTreap maintains to
        support select()/position()/split_pos(). Use this one when you
        only need ordinary set operations (insert/search/remove/iterate)
        and RankedTreap when you also need order statistics.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class Treap : private DefaultCmpHolder<Cmp>,
                  public ContainerAlgorithms<Treap<Key, Cmp>, Key>,
                  public SetAlgorithms<Treap<Key, Cmp>, Key>
    {
    public:
        using Node = TreapNode<Key>;

    private:
        Node head;
        Node*& root;
        Cmp& cmp;
        nat_t num_items;

        rng_t rng;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(Treap& self, const Treap& t)
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
            return generic_rotate_left(r);
        }

        static Node* rotate_right(Node* r)
        {
            return generic_rotate_right(r);
        }

        static Node* exclusive_join(Node*&, Node*&);

        static Node* insert(Node*&, Node*, Cmp&);

        static Node* insert_dup(Node*&, Node*, Cmp&);

        static Node* search(Node*, const Key&, Cmp&);

        static Node* search_or_insert(Node*&, Node*, Cmp&);

        static Node* remove_root(Node*& root)
        {
            Node* ret_val = root;
            root = exclusive_join(L(root), R(root));
            return ret_val;
        }

        static Node* remove(Node*&, const Key&, Cmp&);

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
            PRIOR(p) = rng();

            if (insert(root, p, cmp) == Node::null)
            {
                delete p;
                return nullptr;
            }

            ++num_items;
            return &KEY(p);
        }

        Key* insert_dup(Node* p)
        {
            PRIOR(p) = rng();
            insert_dup(root, p, cmp);
            ++num_items;
            return &KEY(p);
        }

        Key* search_or_insert(Node* p)
        {
            PRIOR(p) = rng();

            Node* result = search_or_insert(root, p, cmp);

            if (p != result)
            {
                delete p;
            }
            else
            {
                ++num_items;
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

        Treap(rng_seed_t seed, Cmp& _cmp)
            : head(), root(L(&head)), cmp(_cmp), num_items(0), rng(seed)
        {
            // empty
        }

        Treap(Cmp& _cmp) : Treap(time(nullptr), _cmp)
        {
            // empty
        }

        /** @see RankedTreap's matching constructors for why these cannot
            simply delegate to the Cmp& overloads above, and why
            copy-assignment (not move) is used for `default_cmp`. */
        Treap(rng_seed_t seed, Cmp&& _cmp = Cmp())
            : head(),
              root(L(&head)),
              cmp(this->default_cmp),
              num_items(0),
              rng(seed)
        {
            this->default_cmp = _cmp;
        }

        Treap(Cmp&& _cmp = Cmp())
            : head(),
              root(L(&head)),
              cmp(this->default_cmp),
              num_items(0),
              rng(time(nullptr))
        {
            this->default_cmp = _cmp;
        }

        Treap(const Treap& t)
            : head(),
              root(L(&head)),
              cmp(cmp_for_copy(*this, t)),
              num_items(t.num_items),
              rng(time(nullptr))
        {
            root = copy(t.root);
        }

        Treap(Treap&& t) : Treap()
        {
            swap(t);
        }

        Treap(const std::initializer_list<Key>&);

        ~Treap()
        {
            clear();
        }

        Treap& operator=(const Treap& t)
        {
            if (this == &t)
            {
                return *this;
            }

            clear();
            root = copy(t.root);
            num_items = t.num_items;
            cmp = t.cmp;
            return *this;
        }

        Treap& operator=(Treap&& t)
        {
            swap(t);
            return *this;
        }

        void swap(Treap& t)
        {
            std::swap(root, t.root);
            std::swap(num_items, t.num_items);
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
            return num_items;
        }

        void clear()
        {
            destroy(root);
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
            --num_items;
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

        class InorderIterator
        {
            friend class Treap;

            Treap* set_ptr = nullptr;
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
            InorderIterator(const Treap& t, int)
                : set_ptr(const_cast<Treap*>(&t)),
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
            InorderIterator(const Treap& t)
                : set_ptr(const_cast<Treap*>(&t)), root(set_ptr->root)
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
            friend class Treap;
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
    Treap<Key, Cmp>::Treap(const std::initializer_list<Key>& l) : Treap()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp>
    typename Treap<Key, Cmp>::Node* Treap<Key, Cmp>::copy(Node* r)
    {
        if (r == Node::null)
        {
            return Node::null;
        }

        Node* p = new Node(KEY(r));
        PRIOR(p) = PRIOR(r);
        L(p) = copy(L(r));
        R(p) = copy(R(r));
        return p;
    }

    template <typename Key, class Cmp>
    void Treap<Key, Cmp>::destroy(Node*& r)
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
    typename Treap<Key, Cmp>::Node* Treap<Key, Cmp>::exclusive_join(Node*& ts,
                                                                    Node*& tg)
    {
        if (ts == Node::null)
        {
            return tg;
        }

        if (tg == Node::null)
        {
            return ts;
        }

        if (PRIOR(ts) < PRIOR(tg))
        {
            R(ts) = exclusive_join(R(ts), tg);
            return ts;
        }
        else
        {
            L(tg) = exclusive_join(ts, L(tg));
            return tg;
        }
    }

    template <typename Key, class Cmp>
    typename Treap<Key, Cmp>::Node* Treap<Key, Cmp>::insert(Node*& r, Node* p,
                                                            Cmp& cmp)
    {
        if (r == Node::null)
        {
            r = p;
            return r;
        }

        if (cmp(KEY(p), KEY(r)))
        {
            Node* result = insert(L(r), p, cmp);

            if (result == Node::null)
            {
                return Node::null;
            }

            if (PRIOR(L(r)) < PRIOR(r))
            {
                r = rotate_right(r);
            }

            return result;
        }
        else if (cmp(KEY(r), KEY(p)))
        {
            Node* result = insert(R(r), p, cmp);

            if (result == Node::null)
            {
                return Node::null;
            }

            if (PRIOR(R(r)) < PRIOR(r))
            {
                r = rotate_left(r);
            }

            return result;
        }

        return Node::null;
    }

    template <typename Key, class Cmp>
    typename Treap<Key, Cmp>::Node*
    Treap<Key, Cmp>::insert_dup(Node*& r, Node* p, Cmp& cmp)
    {
        if (r == Node::null)
        {
            r = p;
            return r;
        }

        if (cmp(KEY(p), KEY(r)))
        {
            Node* result = insert_dup(L(r), p, cmp);

            if (PRIOR(L(r)) < PRIOR(r))
            {
                r = rotate_right(r);
            }

            return result;
        }

        Node* result = insert_dup(R(r), p, cmp);

        if (PRIOR(R(r)) < PRIOR(r))
        {
            r = rotate_left(r);
        }

        return result;
    }

    template <typename Key, class Cmp>
    typename Treap<Key, Cmp>::Node*
    Treap<Key, Cmp>::search(Node* r, const Key& k, Cmp& cmp)
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
    typename Treap<Key, Cmp>::Node*
    Treap<Key, Cmp>::search_or_insert(Node*& r, Node* p, Cmp& cmp)
    {
        if (r == Node::null)
        {
            r = p;
            return r;
        }

        if (cmp(KEY(p), KEY(r)))
        {
            Node* result = search_or_insert(L(r), p, cmp);

            if (result == p && PRIOR(L(r)) < PRIOR(r))
            {
                r = rotate_right(r);
            }

            return result;
        }
        else if (cmp(KEY(r), KEY(p)))
        {
            Node* result = search_or_insert(R(r), p, cmp);

            if (result == p && PRIOR(R(r)) < PRIOR(r))
            {
                r = rotate_left(r);
            }

            return result;
        }

        return r;
    }

    template <typename Key, class Cmp>
    typename Treap<Key, Cmp>::Node*
    Treap<Key, Cmp>::remove(Node*& r, const Key& k, Cmp& cmp)
    {
        if (r == Node::null)
        {
            return Node::null;
        }

        if (cmp(k, KEY(r)))
        {
            return remove(L(r), k, cmp);
        }
        else if (cmp(KEY(r), k))
        {
            return remove(R(r), k, cmp);
        }

        return remove_root(r);
    }

} // end namespace Designar
