/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file splaytree.hpp
    @brief A self-adjusting (splay) binary search tree set
    implementation, using Sleator & Tarjan's top-down splay algorithm.
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
    class SplayNode : public BaseBinTreeNode<Key, SplayNode<Key>,
                                             BinTreeNodeNullValue::SENTINEL>
    {
        using BaseNode = BaseBinTreeNode<Key, SplayNode<Key>,
                                         BinTreeNodeNullValue::SENTINEL>;

    public:
        using BaseNode::BaseNode;
    };

    /** A splay tree (Sleator & Tarjan, 1985): a self-adjusting BST with no
        explicit balance invariant at all — instead, *every* access
        (search, insert, or delete) restructures the tree via rotations so
        that the accessed key ends up at the root. That single rule gives
        an amortized O(log n) bound per operation over any sequence of
        operations, without storing any extra per-node information
        (priority, height, color, size — this is the only tree in this
        library whose node carries nothing beyond the key and its two
        children), at the cost of restructuring on *every* access,
        including plain search() — a splay tree is never "just reading",
        which is why even its `const` search() overload mutates the tree
        internally (see the note on that overload below).

        This implements Sleator & Tarjan's *top-down* splay, which walks
        from the root toward the target key once, greedily rotating and
        relinking as it goes, rather than the more commonly-taught
        bottom-up version (search down to the node, then walk back up
        splaying zig/zig-zig/zig-zag steps) — top-down needs only a single
        pass and no parent pointers, matching how every other tree in this
        library is written.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class SplayTree : private DefaultCmpHolder<Cmp>,
                      public ContainerAlgorithms<SplayTree<Key, Cmp>, Key>,
                      public SetAlgorithms<SplayTree<Key, Cmp>, Key>
    {
    public:
        using Node = SplayNode<Key>;

    private:
        Node head;
        Node*& root;
        Cmp& cmp;
        nat_t num_items;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(SplayTree& self, const SplayTree& t)
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
            Node* q = R(r);
            R(r) = L(q);
            L(q) = r;
            return q;
        }

        static Node* rotate_right(Node* r)
        {
            Node* q = L(r);
            L(r) = R(q);
            R(q) = r;
            return q;
        }

        /** Sleator & Tarjan's top-down splay. Walks from `t` toward `k`,
            peeling off everything strictly less than the path into a "left
            tree" (via `l`) and everything strictly greater into a "right
            tree" (via `r`) as it goes — doing one rotation first whenever
            two consecutive steps go the same direction (the "zig-zig" case),
            which is what gives splay trees their amortized bound instead of
            just degenerating into the plain BST search path. When the walk
            stops (key found, or the correct empty spot reached), the node it
            stopped at is stitched together with the accumulated left/right
            trees and returned as the new root. */
        /** Templated on the probe type `K` (deduced as `Key` at every
            existing call site) rather than fixed to `const Key&`, purely
            so search_by() below can reuse this exact splaying logic for
            a heterogeneous probe too — `k` is only ever compared via
            `cmp`, never stored, so nothing here actually depended on `K`
            being `Key` in the first place. */
        template <typename K>
        static Node* splay(Node* t, const K& k, Cmp& cmp)
        {
            if (t == Node::null)
            {
                return t;
            }

            Node header;
            Node* l = &header;
            Node* r = &header;

            while (true)
            {
                if (cmp(k, KEY(t)))
                {
                    if (L(t) == Node::null)
                    {
                        break;
                    }

                    if (cmp(k, KEY(L(t))))
                    {
                        t = rotate_right(t);

                        if (L(t) == Node::null)
                        {
                            break;
                        }
                    }

                    L(r) = t;
                    r = t;
                    t = L(t);
                }
                else if (cmp(KEY(t), k))
                {
                    if (R(t) == Node::null)
                    {
                        break;
                    }

                    if (cmp(KEY(R(t)), k))
                    {
                        t = rotate_left(t);

                        if (R(t) == Node::null)
                        {
                            break;
                        }
                    }

                    R(l) = t;
                    l = t;
                    t = R(t);
                }
                else
                {
                    break;
                }
            }

            R(l) = L(t);
            L(r) = R(t);
            L(t) = R(&header);
            R(t) = L(&header);

            return t;
        }

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

    public:
        using ItemType = Key;
        using KeyType = Key;
        using DataType = Key;
        using ValueType = Key;
        using SizeType = nat_t;
        using CmpType = Cmp;

        SplayTree(Cmp& _cmp) : head(), root(L(&head)), cmp(_cmp), num_items(0)
        {
            // empty
        }

        /** @see RankedTreap's matching constructor for why this cannot
            simply delegate to the Cmp& overload above, and why
            copy-assignment (not move) is used for `default_cmp`. */
        SplayTree(Cmp&& _cmp = Cmp())
            : head(), root(L(&head)), cmp(this->default_cmp), num_items(0)
        {
            this->default_cmp = _cmp;
        }

        SplayTree(const SplayTree& t)
            : head(),
              root(L(&head)),
              cmp(cmp_for_copy(*this, t)),
              num_items(t.num_items)
        {
            root = copy(t.root);
        }

        SplayTree(SplayTree&& t) : SplayTree()
        {
            swap(t);
        }

        SplayTree(const std::initializer_list<Key>&);

        ~SplayTree()
        {
            clear();
        }

        SplayTree& operator=(const SplayTree& t)
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

        SplayTree& operator=(SplayTree&& t)
        {
            swap(t);
            return *this;
        }

        void swap(SplayTree& t)
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

        Key* append(const Key& k)
        {
            return insert(k);
        }

        Key* append(Key&& k)
        {
            return insert(std::forward<Key>(k));
        }

        /** Splays `k` to the root regardless of whether it is found, which
            is the entire point of a splay tree — even a failed search
            pushes the closest key it did find toward the root, making a
            future search for it (or anything near it) cheaper. Because of
            that, this is not a read-only operation on the underlying
            structure, even though it is on the logical contents; the
            `const` overload below casts that constness away deliberately,
            not as an oversight. */
        Key* search(const Key& k)
        {
            if (root == Node::null)
            {
                return nullptr;
            }

            root = splay(root, k, cmp);

            if (cmp(k, KEY(root)) || cmp(KEY(root), k))
            {
                return nullptr;
            }

            return &KEY(root);
        }

        const Key* search(const Key& k) const
        {
            return const_cast<SplayTree*>(this)->search(k);
        }

        /** Heterogeneous lookup: same splaying behavior as search(), just
            probed with anything comparable to `Key` via `cmp` rather than
            `Key` itself — see map.hpp's GenMap::search(), which needs
            exactly this to avoid constructing a full MapKey<Key, Value>
            probe pair purely to look a key up. */
        template <typename K>
        Key* search_by(const K& k)
        {
            if (root == Node::null)
            {
                return nullptr;
            }

            root = splay(root, k, cmp);

            if (cmp(k, KEY(root)) || cmp(KEY(root), k))
            {
                return nullptr;
            }

            return &KEY(root);
        }

        template <typename K>
        const Key* search_by(const K& k) const
        {
            return const_cast<SplayTree*>(this)->search_by(k);
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

        Key* search_or_insert(const Key& k)
        {
            Key* found = search(k);

            if (found != nullptr)
            {
                return found;
            }

            return insert(k);
        }

        Key* search_or_insert(Key&& k)
        {
            Key* found = search(k);

            if (found != nullptr)
            {
                return found;
            }

            return insert(std::forward<Key>(k));
        }

        bool remove(const Key& k)
        {
            if (root == Node::null)
            {
                return false;
            }

            root = splay(root, k, cmp);

            if (cmp(k, KEY(root)) || cmp(KEY(root), k))
            {
                return false;
            }

            Node* old_root = root;

            if (L(root) == Node::null)
            {
                root = R(root);
            }
            else
            {
                // Everything in L(old_root) is < k, so splaying it with k
                // drives it as far right as possible — i.e. brings
                // L(old_root)'s maximum to its top — which is exactly the node
                // that should become the new root once old_root is spliced out.
                Node* new_root = splay(L(old_root), k, cmp);
                R(new_root) = R(old_root);
                root = new_root;
            }

            delete old_root;
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
            friend class SplayTree;

            SplayTree* set_ptr = nullptr;
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
            InorderIterator(const SplayTree& t, int)
                : set_ptr(const_cast<SplayTree*>(&t)),
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
            InorderIterator(const SplayTree& t)
                : set_ptr(const_cast<SplayTree*>(&t)), root(set_ptr->root)
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
            friend class SplayTree;
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

    private:
        Key* insert(Node* p)
        {
            if (root == Node::null)
            {
                root = p;
                ++num_items;
                return &KEY(p);
            }

            root = splay(root, KEY(p), cmp);

            if (cmp(KEY(p), KEY(root)))
            {
                L(p) = L(root);
                R(p) = root;
                L(root) = Node::null;
                root = p;
            }
            else if (cmp(KEY(root), KEY(p)))
            {
                R(p) = R(root);
                L(p) = root;
                R(root) = Node::null;
                root = p;
            }
            else
            {
                delete p;
                return nullptr;
            }

            ++num_items;
            return &KEY(root);
        }
    };

    template <typename Key, class Cmp>
    SplayTree<Key, Cmp>::SplayTree(const std::initializer_list<Key>& l)
        : SplayTree()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp>
    typename SplayTree<Key, Cmp>::Node* SplayTree<Key, Cmp>::copy(Node* r)
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
    void SplayTree<Key, Cmp>::destroy(Node*& r)
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

} // end namespace Designar
