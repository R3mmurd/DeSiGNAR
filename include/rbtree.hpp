/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file rbtree.hpp
    @brief A left-leaning red-black tree set implementation (Sedgewick's
    formulation, isomorphic to 2-3 trees).
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
    enum class RBColor : unsigned char
    {
        RED,
        BLACK
    };

    template <typename Key>
    class RbNode : public BaseBinTreeNode<Key, RbNode<Key>,
                                          BinTreeNodeNullValue::SENTINEL>
    {
        using BaseNode =
            BaseBinTreeNode<Key, RbNode<Key>, BinTreeNodeNullValue::SENTINEL>;

        RBColor color;

    public:
        RbNode() : BaseNode()
        {
            // empty
        }

        RbNode(const Key& k) : BaseNode(k), color(RBColor::RED)
        {
            // empty
        }

        RbNode(Key&& k) : BaseNode(std::forward<Key>(k)), color(RBColor::RED)
        {
            // empty
        }

        /** The sentinel null node is fixed BLACK, matching the standard
            red-black invariant that every (non-existent) leaf is black —
            exactly the same "bake the sentinel's field so callers never
            need a null check" trick RankedTreap's sentinel count and
            AVLNode's sentinel height use. */
        RbNode(BinTreeNodeCtor ctor) : BaseNode(ctor), color(RBColor::BLACK)
        {
            // empty
        }

        RBColor& get_color()
        {
            return color;
        }
    };

    template <class RbNodeType>
    inline RBColor& COLOR(RbNodeType* p)
    {
        return p->get_color();
    }

    template <class RbNodeType>
    inline bool is_red(RbNodeType* p)
    {
        return COLOR(p) == RBColor::RED;
    }

    /** A left-leaning red-black tree (Sedgewick's formulation of red-black
        trees, isomorphic to 2-3 trees): the classic red-black balancing
        invariants (no red node has a red child; every root-to-null path
        has the same number of black nodes), restricted so that a red
        link only ever leans left. That restriction is what makes both
        insertion and deletion expressible as simple top-down recursive
        functions that rotate/recolor on the way back up — the exact same
        "recurse, then patch `r` via rotations before returning" shape
        already used by AVLTree/Treap in this library — instead of the
        up-and-down two-pass algorithm (with parent pointers) that classic
        CLRS-style red-black trees need.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class RbTree : private DefaultCmpHolder<Cmp>,
                   public ContainerAlgorithms<RbTree<Key, Cmp>, Key>,
                   public SetAlgorithms<RbTree<Key, Cmp>, Key>
    {
    public:
        using Node = RbNode<Key>;

    private:
        Node head;
        Node*& root;
        Cmp& cmp;
        nat_t num_items;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(RbTree& self, const RbTree& t)
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

        static Node* rotate_left(Node* h)
        {
            Node* x = R(h);
            R(h) = L(x);
            L(x) = h;
            COLOR(x) = COLOR(h);
            COLOR(h) = RBColor::RED;
            return x;
        }

        static Node* rotate_right(Node* h)
        {
            Node* x = L(h);
            L(h) = R(x);
            R(x) = h;
            COLOR(x) = COLOR(h);
            COLOR(h) = RBColor::RED;
            return x;
        }

        static void flip(Node* h)
        {
            COLOR(h) = is_red(h) ? RBColor::BLACK : RBColor::RED;
        }

        /** Splits a temporary 4-node (`h` with two red children) into two
            2-nodes by recoloring; the exact inverse (re-forming a 4-node) is
            used by move_red_left()/move_red_right() during deletion. */
        static void flip_colors(Node* h)
        {
            flip(h);
            flip(L(h));
            flip(R(h));
        }

        /** Restores the left-leaning invariants at `h` assuming they only
            hold approximately (true right after an insertion/deletion one
            level below): a lone red right link is rotated to the left: two
            left-leaning reds in a row are rotated right; two red children
            are merged back into a (temporary) 4-node by flip_colors(). */
        static Node* balance(Node* h)
        {
            if (is_red(R(h)) && !is_red(L(h)))
            {
                h = rotate_left(h);
            }

            if (is_red(L(h)) && is_red(L(L(h))))
            {
                h = rotate_right(h);
            }

            if (is_red(L(h)) && is_red(R(h)))
            {
                flip_colors(h);
            }

            return h;
        }

        static Node* insert(Node*&, Node*, Cmp&);

        static Node* insert_dup(Node*&, Node*, Cmp&);

        static Node* search(Node*, const Key&, Cmp&);

        static Node* search_or_insert(Node*&, Node*, Cmp&);

        /** Re-forms the (already-flipped-apart) 4-node rooted at `h` so that
            `h`'s left child can safely lose a node to a deletion below it
            without leaving the tree with a null where a 2-node used to be. */
        static Node* move_red_left(Node* h)
        {
            flip_colors(h);

            if (is_red(L(R(h))))
            {
                R(h) = rotate_right(R(h));
                h = rotate_left(h);
                flip_colors(h);
            }

            return h;
        }

        static Node* move_red_right(Node* h)
        {
            flip_colors(h);

            if (is_red(L(L(h))))
            {
                h = rotate_right(h);
                flip_colors(h);
            }

            return h;
        }

        static Node* delete_min(Node*& h)
        {
            if (L(h) == Node::null)
            {
                Node* ret_val = h;
                h = Node::null;
                return ret_val;
            }

            if (!is_red(L(h)) && !is_red(L(L(h))))
            {
                h = move_red_left(h);
            }

            Node* ret_val = delete_min(L(h));
            h = balance(h);
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

        static nat_t black_height(Node* r, bool& ok)
        {
            if (r == Node::null)
            {
                return 0;
            }

            if (is_red(r) && (is_red(L(r)) || is_red(R(r))))
            {
                ok = false;
            }

            nat_t lh = black_height(L(r), ok);
            nat_t rh = black_height(R(r), ok);

            if (lh != rh)
            {
                ok = false;
            }

            return lh + (is_red(r) ? 0 : 1);
        }

        Key* insert(Node* p)
        {
            if (insert(root, p, cmp) == Node::null)
            {
                delete p;
                return nullptr;
            }

            COLOR(root) = RBColor::BLACK;
            ++num_items;
            return &KEY(p);
        }

        Key* insert_dup(Node* p)
        {
            insert_dup(root, p, cmp);
            COLOR(root) = RBColor::BLACK;
            ++num_items;
            return &KEY(p);
        }

        Key* search_or_insert(Node* p)
        {
            Node* result = search_or_insert(root, p, cmp);
            COLOR(root) = RBColor::BLACK;

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

        /** Checks every left-leaning red-black invariant (BST order, no red
            node has a red child, every root-to-null path has the same
            black-height, root is black, no red right links) — used by
            tests, not by normal operation. */
        bool verify() const
        {
            if (root != Node::null && is_red(root))
            {
                return false;
            }

            bool ok = true;
            black_height(root, ok);

            if (!ok)
            {
                return false;
            }

            return verify_order(root, cmp) && verify_no_red_right(root);
        }

        static bool verify_order(Node* r, Cmp& cmp)
        {
            if (r == Node::null)
            {
                return true;
            }

            if (L(r) != Node::null && !cmp(KEY(L(r)), KEY(r)))
            {
                return false;
            }

            if (R(r) != Node::null && !cmp(KEY(r), KEY(R(r))))
            {
                return false;
            }

            return verify_order(L(r), cmp) && verify_order(R(r), cmp);
        }

        static bool verify_no_red_right(Node* r)
        {
            if (r == Node::null)
            {
                return true;
            }

            if (is_red(R(r)))
            {
                return false;
            }

            return verify_no_red_right(L(r)) && verify_no_red_right(R(r));
        }

        RbTree(Cmp& _cmp) : head(), root(L(&head)), cmp(_cmp), num_items(0)
        {
            // empty
        }

        /** @see RankedTreap's matching constructor for why this cannot
            simply delegate to the Cmp& overload above, and why
            copy-assignment (not move) is used for `default_cmp`. */
        RbTree(Cmp&& _cmp = Cmp())
            : head(), root(L(&head)), cmp(this->default_cmp), num_items(0)
        {
            this->default_cmp = _cmp;
        }

        RbTree(const RbTree& t)
            : head(),
              root(L(&head)),
              cmp(cmp_for_copy(*this, t)),
              num_items(t.num_items)
        {
            root = copy(t.root);
        }

        RbTree(RbTree&& t) : RbTree()
        {
            swap(t);
        }

        RbTree(const std::initializer_list<Key>&);

        ~RbTree()
        {
            clear();
        }

        RbTree& operator=(const RbTree& t)
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

        RbTree& operator=(RbTree&& t)
        {
            swap(t);
            return *this;
        }

        void swap(RbTree& t)
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
            if (search(root, k, cmp) == Node::null)
            {
                return false;
            }

            if (!is_red(L(root)) && !is_red(R(root)))
            {
                COLOR(root) = RBColor::RED;
            }

            Node* result = remove(root, k, cmp);

            if (root != Node::null)
            {
                COLOR(root) = RBColor::BLACK;
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
            friend class RbTree;

            RbTree* set_ptr = nullptr;
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
            InorderIterator(const RbTree& t, int)
                : set_ptr(const_cast<RbTree*>(&t)),
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
            InorderIterator(const RbTree& t)
                : set_ptr(const_cast<RbTree*>(&t)), root(set_ptr->root)
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
            friend class RbTree;
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
    RbTree<Key, Cmp>::RbTree(const std::initializer_list<Key>& l) : RbTree()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp>
    typename RbTree<Key, Cmp>::Node* RbTree<Key, Cmp>::copy(Node* r)
    {
        if (r == Node::null)
        {
            return Node::null;
        }

        Node* p = new Node(KEY(r));
        COLOR(p) = COLOR(r);
        L(p) = copy(L(r));
        R(p) = copy(R(r));
        return p;
    }

    template <typename Key, class Cmp>
    void RbTree<Key, Cmp>::destroy(Node*& r)
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
    typename RbTree<Key, Cmp>::Node* RbTree<Key, Cmp>::insert(Node*& h, Node* p,
                                                              Cmp& cmp)
    {
        if (h == Node::null)
        {
            h = p;
            return p;
        }

        Node* result;

        if (cmp(KEY(p), KEY(h)))
        {
            result = insert(L(h), p, cmp);

            if (result == Node::null)
            {
                return Node::null;
            }
        }
        else if (cmp(KEY(h), KEY(p)))
        {
            result = insert(R(h), p, cmp);

            if (result == Node::null)
            {
                return Node::null;
            }
        }
        else
        {
            return Node::null;
        }

        h = balance(h);
        return result;
    }

    template <typename Key, class Cmp>
    typename RbTree<Key, Cmp>::Node*
    RbTree<Key, Cmp>::insert_dup(Node*& h, Node* p, Cmp& cmp)
    {
        if (h == Node::null)
        {
            h = p;
            return p;
        }

        Node* result;

        if (cmp(KEY(p), KEY(h)))
        {
            result = insert_dup(L(h), p, cmp);
        }
        else
        {
            result = insert_dup(R(h), p, cmp);
        }

        h = balance(h);
        return result;
    }

    template <typename Key, class Cmp>
    typename RbTree<Key, Cmp>::Node*
    RbTree<Key, Cmp>::search(Node* r, const Key& k, Cmp& cmp)
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
    typename RbTree<Key, Cmp>::Node*
    RbTree<Key, Cmp>::search_or_insert(Node*& h, Node* p, Cmp& cmp)
    {
        if (h == Node::null)
        {
            h = p;
            return p;
        }

        Node* result;

        if (cmp(KEY(p), KEY(h)))
        {
            result = search_or_insert(L(h), p, cmp);
        }
        else if (cmp(KEY(h), KEY(p)))
        {
            result = search_or_insert(R(h), p, cmp);
        }
        else
        {
            return h;
        }

        h = balance(h);
        return result;
    }

    /** Adapted from Sedgewick & Wayne's reference LLRB `delete` (Algorithms,
        4th ed.): recurse toward the key, calling move_red_left()/
        move_red_right() to guarantee the node about to lose an element is
        not a bare 2-node, then either splice out a leaf directly or swap
        in the in-order successor and delete it instead, rebalancing on
        the way back up via balance(). */
    template <typename Key, class Cmp>
    typename RbTree<Key, Cmp>::Node*
    RbTree<Key, Cmp>::remove(Node*& h, const Key& k, Cmp& cmp)
    {
        Node* ret_val;

        if (cmp(k, KEY(h)))
        {
            if (!is_red(L(h)) && !is_red(L(L(h))))
            {
                h = move_red_left(h);
            }

            ret_val = remove(L(h), k, cmp);
        }
        else
        {
            if (is_red(L(h)))
            {
                h = rotate_right(h);
            }

            // We already know (from the outer if/else split) that k is not
            // less than KEY(h); after the possible rotate_right() above, `h`
            // may have changed, so KEY(h) below is deliberately re-read fresh
            // rather than reusing a value captured before the rotation.
            if (!cmp(KEY(h), k) && R(h) == Node::null)
            {
                ret_val = h;
                h = Node::null;
                return ret_val;
            }

            if (!is_red(R(h)) && !is_red(L(R(h))))
            {
                h = move_red_right(h);
            }

            if (!cmp(KEY(h), k))
            {
                Node* succ = min(R(h));
                std::swap(KEY(h), KEY(succ));
                ret_val = delete_min(R(h));
            }
            else
            {
                ret_val = remove(R(h), k, cmp);
            }
        }

        h = balance(h);
        return ret_val;
    }

} // end namespace Designar
