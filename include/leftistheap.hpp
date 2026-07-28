/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file leftistheap.hpp
    @brief LeftistHeap: a mergeable min-heap kept balanced by the
    "leftist property" — every node's null path length (npl, the
    length of the shortest path to a missing child; npl(nullptr) = -1)
    is at least its right child's npl, which forces the right spine to
    be the short one (O(lg n)), so merge() can always recurse down the
    right spine and stay logarithmic.
    @ingroup DataStructures
*/

#pragma once

#include <array.hpp>
#include <typetraits.hpp>

namespace Designar
{
    /** @see PairingHeap (pairingheap.hpp) for why this privately derives
        from `DefaultCmpHolder<Cmp>` and why that must be the first
        base — same reasoning applies verbatim. */
    template <typename Key, class Cmp = std::less<Key>>
    class LeftistHeap : private DefaultCmpHolder<Cmp>
    {
    public:
        class Node
        {
            friend class LeftistHeap;

            Key key;
            Node* left;
            Node* right;
            int_t npl;

            Node(const Key& k)
                : key(k), left(nullptr), right(nullptr), npl(0)
            {
                // empty
            }

            Node(Key&& k)
                : key(std::forward<Key>(k)),
                  left(nullptr),
                  right(nullptr),
                  npl(0)
            {
                // empty
            }

        public:
            const Key& get_key() const
            {
                return key;
            }
        };

    private:
        Node* root;
        nat_t num_items;
        Cmp& cmp;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving
            copy logic. */
        static Cmp& cmp_for_copy(LeftistHeap& self, const LeftistHeap& h)
        {
            if (&h.cmp == &h.default_cmp)
            {
                self.default_cmp = h.default_cmp;
                return self.default_cmp;
            }

            return h.cmp;
        }

        static int_t npl_of(Node* n)
        {
            return n == nullptr ? int_t(-1) : n->npl;
        }

        /** Merges two leftist heaps in O(lg n): the smaller-rooted tree
            wins and keeps its left child untouched, merging its right
            child with the other tree instead (the recursive step always
            follows the short — right — spine); afterward, swapping
            left/right if the leftist property would otherwise be
            violated, and recomputing `npl` from the (possibly just
            swapped-in) right child, is what keeps the property
            invariant true again at this node once its subtrees already
            satisfy it. */
        Node* merge(Node* a, Node* b)
        {
            if (a == nullptr)
            {
                return b;
            }

            if (b == nullptr)
            {
                return a;
            }

            if (cmp(b->key, a->key))
            {
                std::swap(a, b);
            }

            a->right = merge(a->right, b);

            if (npl_of(a->left) < npl_of(a->right))
            {
                std::swap(a->left, a->right);
            }

            a->npl = npl_of(a->right) + 1;
            return a;
        }

        static void destroy(Node* n)
        {
            if (n == nullptr)
            {
                return;
            }

            destroy(n->left);
            destroy(n->right);
            delete n;
        }

        static Node* copy(Node* n)
        {
            if (n == nullptr)
            {
                return nullptr;
            }

            Node* c = new Node(n->key);
            c->npl = n->npl;
            c->left = copy(n->left);
            c->right = copy(n->right);
            return c;
        }

    public:
        LeftistHeap(Cmp& _cmp) : root(nullptr), num_items(0), cmp(_cmp)
        {
            // empty
        }

        LeftistHeap(Cmp&& _cmp = Cmp())
            : root(nullptr), num_items(0), cmp(this->default_cmp)
        {
            this->default_cmp = _cmp;
        }

        LeftistHeap(const LeftistHeap& h)
            : root(copy(h.root)),
              num_items(h.num_items),
              cmp(cmp_for_copy(*this, h))
        {
            // empty
        }

        LeftistHeap(LeftistHeap&& h) : LeftistHeap()
        {
            swap(h);
        }

        ~LeftistHeap()
        {
            clear();
        }

        LeftistHeap& operator=(const LeftistHeap& h)
        {
            if (this == &h)
            {
                return *this;
            }

            destroy(root);
            root = copy(h.root);
            num_items = h.num_items;
            cmp = h.cmp;
            return *this;
        }

        LeftistHeap& operator=(LeftistHeap&& h)
        {
            swap(h);
            return *this;
        }

        void swap(LeftistHeap& h)
        {
            std::swap(root, h.root);
            std::swap(num_items, h.num_items);
            std::swap(cmp, h.cmp);
        }

        bool is_empty() const
        {
            return root == nullptr;
        }

        nat_t size() const
        {
            return num_items;
        }

        void clear()
        {
            destroy(root);
            root = nullptr;
            num_items = 0;
        }

        Node* insert(const Key& k)
        {
            Node* n = new Node(k);
            root = merge(root, n);
            ++num_items;
            return n;
        }

        Node* insert(Key&& k)
        {
            Node* n = new Node(std::forward<Key>(k));
            root = merge(root, n);
            ++num_items;
            return n;
        }

        const Key& get_min() const
        {
            if (is_empty())
            {
                throw std::underflow_error("LeftistHeap is empty");
            }

            return root->key;
        }

        Key extract_min()
        {
            if (is_empty())
            {
                throw std::underflow_error("LeftistHeap is empty");
            }

            Node* old_root = root;
            Key ret_val = std::move(old_root->key);

            root = merge(old_root->left, old_root->right);
            delete old_root;
            --num_items;

            return ret_val;
        }
    };

} // end namespace Designar
