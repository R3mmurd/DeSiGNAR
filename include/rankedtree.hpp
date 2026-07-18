/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file rankedtree.hpp
    @brief RankedTree: a plain, unbalanced binary search tree set
    implementation augmented with a per-node subtree count, giving it
    order-statistics support (select-by-rank, position-of-key,
    split-by-position) on top of Tree's ordinary set operations.
    @see tree.hpp for Tree, the unranked sibling of this tree.
    @see avltree.hpp (RankedAVLTree), rbtree.hpp (RankedRBTree), treap.hpp
    (RankedTreap) for the balanced order-statistics trees in this
    library.
    @ingroup Trees
*/

#pragma once

#include <tree.hpp> // for the shared node/tree scaffolding it pulls in

namespace Designar
{
    template <typename Key>
    class RankedTreeNode
        : public BaseBinTreeNode<Key, RankedTreeNode<Key>,
                                 BinTreeNodeNullValue::SENTINEL>
    {
        using BaseNode = BaseBinTreeNode<Key, RankedTreeNode<Key>,
                                         BinTreeNodeNullValue::SENTINEL>;

        nat_t count;

    public:
        RankedTreeNode() : BaseNode()
        {
            // empty
        }

        RankedTreeNode(const Key& k) : BaseNode(k), count(1)
        {
            // empty
        }

        RankedTreeNode(Key&& k) : BaseNode(std::forward<Key>(k)), count(1)
        {
            // empty
        }

        RankedTreeNode(BinTreeNodeCtor ctor) : BaseNode(ctor), count(0)
        {
            // empty
        }

        nat_t& get_count()
        {
            return count;
        }
    };

    /** `Tree` (tree.hpp) plus a per-node subtree-size count, giving it
        `select(i)` (the `i`-th smallest key), `position(k)` (the in-order
        rank of `k`), and `split_pos(i)` (split into "smallest `i`
        elements" / "the rest") — the same order-statistics operations
        every `Ranked*` tree in this library offers, but with no
        balancing at all, exactly like `Tree` has none. `split_pos` needs
        no rebalancing step here (unlike RankedAVLTree/RankedRBTree's
        join-based split): with no invariant to preserve beyond the
        count itself, cutting the tree at the split boundary and
        reattaching the two halves is already correct on its own.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class RankedTree
        : private DefaultCmpHolder<Cmp>,
          public ContainerAlgorithms<RankedTree<Key, Cmp>, Key>,
          public SetAlgorithms<RankedTree<Key, Cmp>, Key>
    {
    public:
        using Node = RankedTreeNode<Key>;

    private:
        Node head;
        Node*& root;
        Cmp& cmp;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(RankedTree& self, const RankedTree& t)
        {
            if (&t.cmp == &t.default_cmp)
            {
                self.default_cmp = t.default_cmp;
                return self.default_cmp;
            }

            return t.cmp;
        }

        static bool verify(Node*, Cmp&);

        static Node* copy(Node*);

        static void destroy(Node*&);

        static Node* insert(Node*&, Node*, Cmp&);

        static Node* insert_dup(Node*&, Node*, Cmp&);

        static Node* search(Node*, const Key&, Cmp&);

        static Node* search_or_insert(Node*&, Node*, Cmp&);

        static void split_pos(Node*, nat_t, Node*&, Node*&);

        static Node* remove_min(Node*& r)
        {
            if (L(r) == Node::null)
            {
                Node* ret_val = r;
                r = R(r);
                return ret_val;
            }

            Node* ret_val = remove_min(L(r));
            --COUNT(r);
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

        template <class Op>
        static void preorder_rec(Node*, Op&);

        template <class Op>
        static void inorder_rec(Node*, Op&);

        template <class Op>
        static void postorder_rec(Node*, Op&);

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

        RankedTree(Cmp& _cmp) : head(), root(L(&head)), cmp(_cmp)
        {
            // empty
        }

        /** @see RankedTreap's matching constructors (treap.hpp) for why
            this cannot simply delegate to the Cmp& overload above, and
            why copy-assignment (not move) is used for `default_cmp`. */
        RankedTree(Cmp&& _cmp = Cmp())
            : head(), root(L(&head)), cmp(this->default_cmp)
        {
            this->default_cmp = _cmp;
        }

        RankedTree(const RankedTree& t)
            : head(), root(L(&head)), cmp(cmp_for_copy(*this, t))
        {
            root = copy(t.root);
        }

        RankedTree(RankedTree&& t) : RankedTree()
        {
            swap(t);
        }

        RankedTree(const std::initializer_list<Key>&);

        ~RankedTree()
        {
            clear();
        }

        RankedTree& operator=(const RankedTree& t)
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

        RankedTree& operator=(RankedTree&& t)
        {
            swap(t);
            return *this;
        }

        void swap(RankedTree& t)
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

            return KEY(generic_select(root, i));
        }

        const Key& select(nat_t i) const
        {
            if (i >= size())
            {
                throw std::out_of_range("Infix position is out of range");
            }

            return KEY(generic_select(root, i));
        }

        int_t position(const Key& k) const
        {
            return generic_position(root, k, cmp);
        }

        Key& operator[](nat_t i)
        {
            return select(i);
        }

        const Key& operator[](nat_t i) const
        {
            return select(i);
        }

        std::tuple<RankedTree, RankedTree> split_pos(nat_t i)
        {
            if (i >= size())
            {
                throw std::out_of_range("Infix position is out of range");
            }

            RankedTree ts, tg;
            split_pos(root, i, ts.root, tg.root);
            root = Node::null;
            return std::make_tuple(std::move(ts), std::move(tg));
        }

        template <class Op>
        void for_each_preorder(Op& op)
        {
            preorder_rec<Op>(root, op);
        }

        template <class Op>
        void for_each_preorder(Op&& op = Op())
        {
            for_each_preorder<Op>(op);
        }

        template <class Op>
        void for_each_inorder(Op& op)
        {
            inorder_rec<Op>(root, op);
        }

        template <class Op>
        void for_each_inorder(Op&& op = Op())
        {
            for_each_inorder<Op>(op);
        }

        template <class Op>
        void for_each_postorder(Op& op)
        {
            postorder_rec<Op>(root, op);
        }

        template <class Op>
        void for_each_postorder(Op&& op = Op())
        {
            for_each_postorder<Op>(op);
        }

        class InorderIterator
        {
            friend class RankedTree;

            RankedTree* set_ptr = nullptr;
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
            InorderIterator(const RankedTree& t, int)
                : set_ptr(const_cast<RankedTree*>(&t)),
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
            InorderIterator(const RankedTree& t)
                : set_ptr(const_cast<RankedTree*>(&t)), root(set_ptr->root)
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
            friend class RankedTree;
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
    RankedTree<Key, Cmp>::RankedTree(const std::initializer_list<Key>& l)
        : RankedTree()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp>
    bool RankedTree<Key, Cmp>::verify(Node* r, Cmp& cmp)
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

        bool test = COUNT(r) == (COUNT(lc) + COUNT(rc) + 1);

        if (lc != Node::null)
        {
            test = test && cmp(KEY(lc), KEY(r));
        }

        if (rc != Node::null)
        {
            test = test && cmp(KEY(r), KEY(rc));
        }

        return test;
    }

    template <typename Key, class Cmp>
    typename RankedTree<Key, Cmp>::Node* RankedTree<Key, Cmp>::copy(Node* r)
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
    void RankedTree<Key, Cmp>::destroy(Node*& r)
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
    typename RankedTree<Key, Cmp>::Node*
    RankedTree<Key, Cmp>::insert(Node*& r, Node* p, Cmp& cmp)
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
        }
        else if (cmp(KEY(r), KEY(p)))
        {
            result = insert(R(r), p, cmp);
        }
        else
        {
            return Node::null;
        }

        if (result != Node::null)
        {
            ++COUNT(r);
        }

        return result;
    }

    template <typename Key, class Cmp>
    typename RankedTree<Key, Cmp>::Node*
    RankedTree<Key, Cmp>::insert_dup(Node*& r, Node* p, Cmp& cmp)
    {
        if (r == Node::null)
        {
            r = p;
            return r;
        }

        Node* result = cmp(KEY(p), KEY(r)) ? insert_dup(L(r), p, cmp)
                                           : insert_dup(R(r), p, cmp);
        ++COUNT(r);
        return result;
    }

    template <typename Key, class Cmp>
    typename RankedTree<Key, Cmp>::Node*
    RankedTree<Key, Cmp>::search(Node* r, const Key& k, Cmp& cmp)
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
    typename RankedTree<Key, Cmp>::Node*
    RankedTree<Key, Cmp>::search_or_insert(Node*& r, Node* p, Cmp& cmp)
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

        if (result == p)
        {
            ++COUNT(r);
        }

        return result;
    }

    template <typename Key, class Cmp>
    void RankedTree<Key, Cmp>::split_pos(Node* r, nat_t i, Node*& ts,
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

    template <typename Key, class Cmp>
    typename RankedTree<Key, Cmp>::Node*
    RankedTree<Key, Cmp>::remove(Node*& r, const Key& k, Cmp& cmp)
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

        // Two children: splice out the in-order successor (the minimum of
        // the right subtree, which has at most one child) and move its
        // key into `r`, so the node actually freed is the (now key-less)
        // successor rather than `r` itself — same technique as Tree's
        // remove, plus the count decrement remove_min() already applies
        // along the successor's path, plus one more for `r` itself now
        // that its subtree has one fewer node overall.
        Node* succ = remove_min(R(r));
        std::swap(KEY(r), KEY(succ));
        --COUNT(r);
        return succ;
    }

    template <typename Key, class Cmp>
    template <class Op>
    void RankedTree<Key, Cmp>::preorder_rec(Node* r, Op& op)
    {
        if (r == Node::null)
        {
            return;
        }

        op(KEY(r));
        preorder_rec(L(r), op);
        preorder_rec(R(r), op);
    }

    template <typename Key, class Cmp>
    template <class Op>
    void RankedTree<Key, Cmp>::inorder_rec(Node* r, Op& op)
    {
        if (r == Node::null)
        {
            return;
        }

        inorder_rec(L(r), op);
        op(KEY(r));
        inorder_rec(R(r), op);
    }

    template <typename Key, class Cmp>
    template <class Op>
    void RankedTree<Key, Cmp>::postorder_rec(Node* r, Op& op)
    {
        if (r == Node::null)
        {
            return;
        }

        postorder_rec(L(r), op);
        postorder_rec(R(r), op);
        op(KEY(r));
    }

} // end namespace Designar
