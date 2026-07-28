/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file fibonacciheap.hpp
    @brief FibonacciHeap: a mergeable min-heap with amortized O(1)
    insert/decrease_key (the whole reason Dijkstra/Prim are stated with
    an O(E + V lg V) bound in the literature — that bound assumes a
    Fibonacci heap specifically) and O(lg n) amortized extract_min. Root
    list and every child list are circular doubly-linked rings; the
    "lazy" part is that insert() and the children of a just-removed
    minimum are simply spliced into the root list with no rebalancing
    at all — the O(lg n) bound only gets paid for, in one lump, the next
    time extract_min() has to consolidate the root list back down to at
    most one tree per degree.
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
    class FibonacciHeap : private DefaultCmpHolder<Cmp>
    {
    public:
        class Node
        {
            friend class FibonacciHeap;

            Key key;
            Node* parent;
            Node* child;
            Node* left;
            Node* right;
            nat_t degree;
            bool mark;

            Node(const Key& k)
                : key(k),
                  parent(nullptr),
                  child(nullptr),
                  left(this),
                  right(this),
                  degree(0),
                  mark(false)
            {
                // empty
            }

            Node(Key&& k)
                : key(std::forward<Key>(k)),
                  parent(nullptr),
                  child(nullptr),
                  left(this),
                  right(this),
                  degree(0),
                  mark(false)
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
        Node* min_node;
        nat_t num_items;
        Cmp& cmp;

        static Cmp& cmp_for_copy(FibonacciHeap& self, const FibonacciHeap& h)
        {
            if (&h.cmp == &h.default_cmp)
            {
                self.default_cmp = h.default_cmp;
                return self.default_cmp;
            }

            return h.cmp;
        }

        /** Splices circular ring `b` into circular ring `a` — both must
            be valid non-empty rings (a lone node's ring is itself, via
            the constructor's `left = right = this`). This is the one
            primitive every ring-mutating operation below is built from:
            merging two root lists, and attaching a new child into an
            existing (non-empty) child list, are the exact same
            operation on two circular doubly-linked lists. */
        static void splice_rings(Node* a, Node* b)
        {
            Node* a_right = a->right;
            Node* b_left = b->left;
            a->right = b;
            b->left = a;
            a_right->left = b_left;
            b_left->right = a_right;
        }

        /** Removes `n` from whatever ring it currently sits in, leaving
            `n` as a singleton ring of its own (`left == right == n`) —
            the state every node must be in before it can be spliced
            into a different ring via splice_rings(). */
        static void isolate(Node* n)
        {
            n->left->right = n->right;
            n->right->left = n->left;
            n->left = n->right = n;
        }

        /** `n` must already be a singleton ring (freshly constructed, or
            just isolate()d). */
        void add_to_root_list(Node* n)
        {
            if (min_node == nullptr)
            {
                min_node = n;
            }
            else
            {
                splice_rings(min_node, n);

                if (cmp(n->key, min_node->key))
                {
                    min_node = n;
                }
            }
        }

        /** Makes `y` a child of `x` — both are assumed equal-degree root-
            list trees (the one case consolidate() ever calls this for),
            so `x`'s own degree simply increments. `y` is isolated from
            the root list first, since it is about to move into `x`'s
            child ring instead. */
        void link(Node* y, Node* x)
        {
            isolate(y);
            y->parent = x;
            y->mark = false;

            if (x->child == nullptr)
            {
                x->child = y;
            }
            else
            {
                splice_rings(x->child, y);
            }

            ++x->degree;
        }

        /** Collapses the root list down to at most one tree per degree
            — the "pay for the laziness" step every extract_min() ends
            with. Snapshotting every current root into `roots` first is
            necessary: the loop below relinks roots into each other's
            child lists as it goes, which would corrupt an in-progress
            walk of the *same* ring being mutated.

            The degree table is sized `num_items + 1` — a deliberately
            loose, always-safe upper bound (a node's degree can never
            reach the total item count) rather than the tighter
            log-golden-ratio bound the literature derives; simplicity
            over shaving a constant factor off an array this library
            never needs to be performance-critical about. */
        void consolidate()
        {
            if (min_node == nullptr)
            {
                return;
            }

            DynArray<Node*> roots;
            Node* start = min_node;
            Node* curr = start;

            do
            {
                roots.append(curr);
                curr = curr->right;
            } while (curr != start);

            DynArray<Node*> degree_table(num_items + 1, static_cast<Node*>(nullptr));

            for (Node* w : roots)
            {
                Node* x = w;
                nat_t d = x->degree;

                while (degree_table[d] != nullptr)
                {
                    Node* y = degree_table[d];

                    if (cmp(y->key, x->key))
                    {
                        std::swap(x, y);
                    }

                    link(y, x);
                    degree_table[d] = nullptr;
                    ++d;
                }

                degree_table[d] = x;
            }

            min_node = nullptr;

            for (Node* n : degree_table)
            {
                if (n != nullptr)
                {
                    n->left = n->right = n;
                    add_to_root_list(n);
                }
            }
        }

        /** Detaches `n` from parent `p`'s child list and reinserts it as
            a top-level tree — the mechanism decrease_key() uses when a
            child's key drops below its parent's. `next_sibling` must be
            captured *before* isolate() runs, since isolate() resets
            `n`'s own left/right to point at itself. */
        void cut(Node* n, Node* p)
        {
            Node* next_sibling = (n->right == n) ? nullptr : n->right;
            isolate(n);

            if (p->child == n)
            {
                p->child = next_sibling;
            }

            --p->degree;
            n->parent = nullptr;
            n->mark = false;
            add_to_root_list(n);
        }

        /** The amortized-O(1) half of decrease_key(): a node that has
            already lost one child since it last became a child itself
            (marked `true`) loses its place in the tree the *next* time
            it loses another — cascading up as far as this keeps being
            true, which is exactly what keeps any single tree from
            drifting toward an unbalanced shape after many
            decrease_key() calls. */
        void cascading_cut(Node* p)
        {
            Node* z = p->parent;

            if (z == nullptr)
            {
                return;
            }

            if (!p->mark)
            {
                p->mark = true;
            }
            else
            {
                cut(p, z);
                cascading_cut(z);
            }
        }

        static void destroy_ring(Node* head)
        {
            if (head == nullptr)
            {
                return;
            }

            DynArray<Node*> nodes;
            Node* curr = head;

            do
            {
                nodes.append(curr);
                curr = curr->right;
            } while (curr != head);

            for (Node* n : nodes)
            {
                destroy_ring(n->child);
                delete n;
            }
        }

        /** Copies a ring node-by-node via plain re-insertion of keys
            rather than reconstructing the exact tree shape/mark bits —
            simpler to get right than mirroring the circular-ring/parent
            graph directly, at the cost of the copy's internal structure
            not matching the original's (irrelevant: only the heap
            property and the key multiset are ever part of this class's
            contract). */
        static void copy_by_reinsertion(FibonacciHeap& dst, Node* src_ring)
        {
            if (src_ring == nullptr)
            {
                return;
            }

            Node* curr = src_ring;

            do
            {
                dst.insert(curr->key);
                copy_by_reinsertion(dst, curr->child);
                curr = curr->right;
            } while (curr != src_ring);
        }

    public:
        FibonacciHeap(Cmp& _cmp)
            : min_node(nullptr), num_items(0), cmp(_cmp)
        {
            // empty
        }

        FibonacciHeap(Cmp&& _cmp = Cmp())
            : min_node(nullptr), num_items(0), cmp(this->default_cmp)
        {
            this->default_cmp = _cmp;
        }

        FibonacciHeap(const FibonacciHeap& h)
            : min_node(nullptr), num_items(0), cmp(cmp_for_copy(*this, h))
        {
            copy_by_reinsertion(*this, h.min_node);
        }

        FibonacciHeap(FibonacciHeap&& h) : FibonacciHeap()
        {
            swap(h);
        }

        ~FibonacciHeap()
        {
            clear();
        }

        FibonacciHeap& operator=(const FibonacciHeap& h)
        {
            if (this == &h)
            {
                return *this;
            }

            FibonacciHeap tmp(h);
            swap(tmp);
            return *this;
        }

        FibonacciHeap& operator=(FibonacciHeap&& h)
        {
            swap(h);
            return *this;
        }

        void swap(FibonacciHeap& h)
        {
            std::swap(min_node, h.min_node);
            std::swap(num_items, h.num_items);
            std::swap(cmp, h.cmp);
        }

        bool is_empty() const
        {
            return min_node == nullptr;
        }

        nat_t size() const
        {
            return num_items;
        }

        void clear()
        {
            destroy_ring(min_node);
            min_node = nullptr;
            num_items = 0;
        }

        Node* insert(const Key& k)
        {
            Node* n = new Node(k);
            add_to_root_list(n);
            ++num_items;
            return n;
        }

        Node* insert(Key&& k)
        {
            Node* n = new Node(std::forward<Key>(k));
            add_to_root_list(n);
            ++num_items;
            return n;
        }

        const Key& get_min() const
        {
            if (is_empty())
            {
                throw std::underflow_error("FibonacciHeap is empty");
            }

            return min_node->key;
        }

        Key extract_min()
        {
            if (is_empty())
            {
                throw std::underflow_error("FibonacciHeap is empty");
            }

            Node* z = min_node;

            if (z->child != nullptr)
            {
                DynArray<Node*> kids;
                Node* c = z->child;

                do
                {
                    kids.append(c);
                    c = c->right;
                } while (c != z->child);

                for (Node* k : kids)
                {
                    isolate(k);
                    k->parent = nullptr;
                    add_to_root_list(k);
                }

                z->child = nullptr;
            }

            Node* next_candidate = (z->right == z) ? nullptr : z->right;
            isolate(z);
            min_node = next_candidate;

            Key ret_val = std::move(z->key);
            delete z;
            --num_items;

            if (min_node != nullptr)
            {
                consolidate();
            }

            return ret_val;
        }

        /** The textbook amortized-O(1) decrease_key: if `n`'s new key no
            longer respects heap order under its parent, cut it out to
            the root list and propagate cascading_cut() upward; O(1)
            work except for the (amortized, not worst-case) cascade. */
        void decrease_key(Node* n, const Key& new_key)
        {
            if (cmp(n->key, new_key))
            {
                throw std::domain_error("new_key is greater than current key");
            }

            n->key = new_key;
            Node* p = n->parent;

            if (p != nullptr && cmp(n->key, p->key))
            {
                cut(n, p);
                cascading_cut(p);
            }

            if (cmp(n->key, min_node->key))
            {
                min_node = n;
            }
        }
    };

} // end namespace Designar
