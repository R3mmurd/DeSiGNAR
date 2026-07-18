/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file tree.hpp
    @brief Tree: a plain, unbalanced binary search tree set implementation
    — the foundational shape every other tree in this library builds on
    (a balancing strategy layered on top of the same insert/search/remove
    recursion), and the one to reach for when the caller already knows
    insertion order won't be adversarial.
    @see rankedtree.hpp for RankedTree, the order-statistics-capable
    (select/position/split_pos) sibling of this tree.
    @see avltree.hpp, rbtree.hpp, treap.hpp, randomizedtree.hpp,
    splaytree.hpp for the balanced binary search trees in this library,
    all usable as the `TreeType` template parameter of TreeSet/TreeMap
    (set.hpp / map.hpp).
    @ingroup Trees
*/

#pragma once

#include <nodesdef.hpp>
#include <containeralgorithms.hpp>
#include <setalgorithms.hpp>
#include <stack.hpp>
#include <iterator.hpp>
#include <typetraits.hpp>

namespace Designar
{
    template <typename Key>
    class TreeNode : public BaseBinTreeNode<Key, TreeNode<Key>,
                                            BinTreeNodeNullValue::SENTINEL>
    {
        using BaseNode = BaseBinTreeNode<Key, TreeNode<Key>,
                                         BinTreeNodeNullValue::SENTINEL>;

    public:
        using BaseNode::BaseNode;
    };

    /** A plain binary search tree: no balancing at all — insertion order
        alone determines shape, so `search`/`insert`/`remove` cost O(h)
        where `h` is whatever height that insertion order happens to
        produce (`O(lg n)` for a random order, `O(n)` in the worst case,
        e.g. inserting an already-sorted sequence). Every other tree in
        this library is this same recursive insert/search/remove shape
        plus a balancing strategy layered on top (a stored priority for
        Treap, a height for AVLTree, a color for RbTree, ...); this class
        is what's left when none of that is added.

        Removal of a node with two children splices out the in-order
        successor (the minimum of the right subtree, which has at most
        one child) and moves its key into the node being removed, so the
        node actually freed is the (now key-less) successor — the same
        technique AVLTree/RbTree use before their own rebalancing step.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class Tree : private DefaultCmpHolder<Cmp>,
                 public ContainerAlgorithms<Tree<Key, Cmp>, Key>,
                 public SetAlgorithms<Tree<Key, Cmp>, Key>
    {
    public:
        using Node = TreeNode<Key>;

    private:
        Node head;
        Node*& root;
        Cmp& cmp;
        nat_t num_items;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(Tree& self, const Tree& t)
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

        static Node* remove_min(Node*& r)
        {
            if (L(r) == Node::null)
            {
                Node* ret_val = r;
                r = R(r);
                return ret_val;
            }

            return remove_min(L(r));
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

            ++num_items;
            return &KEY(p);
        }

        Key* insert_dup(Node* p)
        {
            insert_dup(root, p, cmp);
            ++num_items;
            return &KEY(p);
        }

        Key* search_or_insert(Node* p)
        {
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

        bool verify() const
        {
            return verify(root, cmp);
        }

        Tree(Cmp& _cmp) : head(), root(L(&head)), cmp(_cmp), num_items(0)
        {
            // empty
        }

        /** @see RankedTreap's matching constructors (treap.hpp) for why
            this cannot simply delegate to the Cmp& overload above, and
            why copy-assignment (not move) is used for `default_cmp`. */
        Tree(Cmp&& _cmp = Cmp())
            : head(),
              root(L(&head)),
              cmp(this->default_cmp),
              num_items(0)
        {
            this->default_cmp = _cmp;
        }

        Tree(const Tree& t)
            : head(),
              root(L(&head)),
              cmp(cmp_for_copy(*this, t)),
              num_items(t.num_items)
        {
            root = copy(t.root);
        }

        Tree(Tree&& t) : Tree()
        {
            swap(t);
        }

        Tree(const std::initializer_list<Key>&);

        ~Tree()
        {
            clear();
        }

        Tree& operator=(const Tree& t)
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

        Tree& operator=(Tree&& t)
        {
            swap(t);
            return *this;
        }

        void swap(Tree& t)
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
            friend class Tree;

            Tree* set_ptr = nullptr;
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
            InorderIterator(const Tree& t, int)
                : set_ptr(const_cast<Tree*>(&t)),
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
            InorderIterator(const Tree& t)
                : set_ptr(const_cast<Tree*>(&t)), root(set_ptr->root)
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
            friend class Tree;
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
    Tree<Key, Cmp>::Tree(const std::initializer_list<Key>& l) : Tree()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp>
    bool Tree<Key, Cmp>::verify(Node* r, Cmp& cmp)
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

        bool test = true;

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
    typename Tree<Key, Cmp>::Node* Tree<Key, Cmp>::copy(Node* r)
    {
        if (r == Node::null)
        {
            return Node::null;
        }

        Node* p = new Node(KEY(r));
        L(p) = copy(L(r));
        R(p) = copy(R(r));
        return p;
    }

    template <typename Key, class Cmp>
    void Tree<Key, Cmp>::destroy(Node*& r)
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
    typename Tree<Key, Cmp>::Node* Tree<Key, Cmp>::insert(Node*& r, Node* p,
                                                          Cmp& cmp)
    {
        if (r == Node::null)
        {
            r = p;
            return r;
        }

        if (cmp(KEY(p), KEY(r)))
        {
            return insert(L(r), p, cmp);
        }
        else if (cmp(KEY(r), KEY(p)))
        {
            return insert(R(r), p, cmp);
        }

        return Node::null;
    }

    template <typename Key, class Cmp>
    typename Tree<Key, Cmp>::Node*
    Tree<Key, Cmp>::insert_dup(Node*& r, Node* p, Cmp& cmp)
    {
        if (r == Node::null)
        {
            r = p;
            return r;
        }

        if (cmp(KEY(p), KEY(r)))
        {
            return insert_dup(L(r), p, cmp);
        }

        return insert_dup(R(r), p, cmp);
    }

    template <typename Key, class Cmp>
    typename Tree<Key, Cmp>::Node*
    Tree<Key, Cmp>::search(Node* r, const Key& k, Cmp& cmp)
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
    typename Tree<Key, Cmp>::Node*
    Tree<Key, Cmp>::search_or_insert(Node*& r, Node* p, Cmp& cmp)
    {
        if (r == Node::null)
        {
            r = p;
            return r;
        }

        if (cmp(KEY(p), KEY(r)))
        {
            return search_or_insert(L(r), p, cmp);
        }
        else if (cmp(KEY(r), KEY(p)))
        {
            return search_or_insert(R(r), p, cmp);
        }

        return r;
    }

    template <typename Key, class Cmp>
    typename Tree<Key, Cmp>::Node*
    Tree<Key, Cmp>::remove(Node*& r, const Key& k, Cmp& cmp)
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
        // successor rather than `r` itself.
        Node* succ = remove_min(R(r));
        std::swap(KEY(r), KEY(succ));
        return succ;
    }

    template <typename Key, class Cmp>
    template <class Op>
    void Tree<Key, Cmp>::preorder_rec(Node* r, Op& op)
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
    void Tree<Key, Cmp>::inorder_rec(Node* r, Op& op)
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
    void Tree<Key, Cmp>::postorder_rec(Node* r, Op& op)
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
