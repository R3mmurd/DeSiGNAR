/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file avltree.hpp
    @brief A height-balanced (AVL) binary search tree set implementation.
    @see rankedavltree.hpp for the order-statistics-capable sibling of
    this tree (subtree-size tracking added on top of the same
    height-balancing logic).
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
    class AVLNode : public BaseBinTreeNode<Key, AVLNode<Key>,
                                           BinTreeNodeNullValue::SENTINEL>
    {
        using BaseNode =
            BaseBinTreeNode<Key, AVLNode<Key>, BinTreeNodeNullValue::SENTINEL>;

        int_t height;

    public:
        AVLNode() : BaseNode()
        {
            // empty
        }

        AVLNode(const Key& k) : BaseNode(k), height(0)
        {
            // empty
        }

        AVLNode(Key&& k) : BaseNode(std::forward<Key>(k)), height(0)
        {
            // empty
        }

        /** The sentinel null node's height is fixed at -1 so that
            HEIGHT(Node::null) always reads as -1 with no branch needed —
            the same trick RankedTreap's sentinel uses to make
            COUNT(Node::null) read as 0 for free. That makes a leaf's height
            come out to `1 + max(-1, -1) == 0`, the usual convention. */
        AVLNode(BinTreeNodeCtor ctor) : BaseNode(ctor), height(-1)
        {
            // empty
        }

        int_t& get_height()
        {
            return height;
        }
    };

    template <class AVLNodeType>
    inline int_t& HEIGHT(AVLNodeType* p)
    {
        return p->get_height();
    }

    /** A height-balanced binary search tree (the classic Adelson-Velsky
        and Landis tree): after every insertion/removal, every node's left
        and right subtree heights differ by at most 1, restored via
        rotations when violated. Unlike Treap/RankedTreap's randomized
        balancing, AVL trees are balanced *deterministically* — the same
        sequence of insertions always produces the same shape — at the
        cost of storing and maintaining an exact height per node instead
        of a single random priority.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class AVLTree : private DefaultCmpHolder<Cmp>,
                    public ContainerAlgorithms<AVLTree<Key, Cmp>, Key>,
                    public SetAlgorithms<AVLTree<Key, Cmp>, Key>
    {
    public:
        using Node = AVLNode<Key>;

    private:
        Node head;
        Node*& root;
        Cmp& cmp;
        nat_t num_items;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(AVLTree& self, const AVLTree& t)
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

        static Node* update_height(Node* r)
        {
            HEIGHT(r) = 1 + std::max(HEIGHT(L(r)), HEIGHT(R(r)));
            return r;
        }

        static int_t balance_factor(Node* r)
        {
            return HEIGHT(L(r)) - HEIGHT(R(r));
        }

        static Node* rotate_left(Node* r)
        {
            Node* q = generic_rotate_left(r);
            update_height(r);
            update_height(q);
            return q;
        }

        static Node* rotate_right(Node* r)
        {
            Node* q = generic_rotate_right(r);
            update_height(r);
            update_height(q);
            return q;
        }

        /** Restores the AVL invariant (|balance factor| <= 1) at `r`,
            assuming both of `r`'s children are already themselves valid AVL
            subtrees (true right after a single insertion/removal one level
            below `r`). The two "outer" cases (left-left, right-right) are a
            single rotation; the two "inner" cases (left-right, right-left)
            need a rotation on the child first to turn them into an outer
            case, then the same rotation at `r`. */
        static Node* balance(Node* r)
        {
            update_height(r);

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

            if (std::abs(balance_factor(r)) > 1)
            {
                return false;
            }

            if (HEIGHT(r) != 1 + std::max(HEIGHT(lc), HEIGHT(rc)))
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

        AVLTree(Cmp& _cmp) : head(), root(L(&head)), cmp(_cmp), num_items(0)
        {
            // empty
        }

        /** @see RankedTreap's matching constructor for why this cannot
            simply delegate to the Cmp& overload above, and why
            copy-assignment (not move) is used for `default_cmp`. */
        AVLTree(Cmp&& _cmp = Cmp())
            : head(), root(L(&head)), cmp(this->default_cmp), num_items(0)
        {
            this->default_cmp = _cmp;
        }

        AVLTree(const AVLTree& t)
            : head(),
              root(L(&head)),
              cmp(cmp_for_copy(*this, t)),
              num_items(t.num_items)
        {
            root = copy(t.root);
        }

        AVLTree(AVLTree&& t) : AVLTree()
        {
            swap(t);
        }

        AVLTree(const std::initializer_list<Key>&);

        ~AVLTree()
        {
            clear();
        }

        AVLTree& operator=(const AVLTree& t)
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

        AVLTree& operator=(AVLTree&& t)
        {
            swap(t);
            return *this;
        }

        void swap(AVLTree& t)
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
            friend class AVLTree;

            AVLTree* set_ptr = nullptr;
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
            InorderIterator(const AVLTree& t, int)
                : set_ptr(const_cast<AVLTree*>(&t)),
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
            InorderIterator(const AVLTree& t)
                : set_ptr(const_cast<AVLTree*>(&t)), root(set_ptr->root)
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
            friend class AVLTree;
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
    AVLTree<Key, Cmp>::AVLTree(const std::initializer_list<Key>& l) : AVLTree()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp>
    typename AVLTree<Key, Cmp>::Node* AVLTree<Key, Cmp>::copy(Node* r)
    {
        if (r == Node::null)
        {
            return Node::null;
        }

        Node* p = new Node(KEY(r));
        L(p) = copy(L(r));
        R(p) = copy(R(r));
        update_height(p);
        return p;
    }

    template <typename Key, class Cmp>
    void AVLTree<Key, Cmp>::destroy(Node*& r)
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
    typename AVLTree<Key, Cmp>::Node*
    AVLTree<Key, Cmp>::insert(Node*& r, Node* p, Cmp& cmp)
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
    typename AVLTree<Key, Cmp>::Node*
    AVLTree<Key, Cmp>::insert_dup(Node*& r, Node* p, Cmp& cmp)
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
    typename AVLTree<Key, Cmp>::Node*
    AVLTree<Key, Cmp>::search(Node* r, const Key& k, Cmp& cmp)
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
    typename AVLTree<Key, Cmp>::Node*
    AVLTree<Key, Cmp>::search_or_insert(Node*& r, Node* p, Cmp& cmp)
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
    typename AVLTree<Key, Cmp>::Node*
    AVLTree<Key, Cmp>::remove(Node*& r, const Key& k, Cmp& cmp)
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

            // Two children: splice out the in-order successor (the minimum of
            // the right subtree, which has at most one child) and move its
            // key into `r`, so the node actually freed is the (now key-less)
            // successor rather than `r` itself.
            Node* succ = remove_min(R(r));
            std::swap(KEY(r), KEY(succ));
            ret_val = succ;
        }

        r = balance(r);
        return ret_val;
    }

} // end namespace Designar
