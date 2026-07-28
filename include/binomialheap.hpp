/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file binomialheap.hpp
    @brief BinomialHeap: a mergeable min-heap kept as a forest of
    binomial trees, at most one of each order (the same structure as
    the binary representation of the item count — a binomial heap with
    `n` items has exactly the binomial trees corresponding to `n`'s set
    bits), giving O(lg n) insert/extract_min/decrease_key and, unlike a
    binary heap, O(lg n) meld of two whole heaps.
    @ingroup DataStructures
*/

#pragma once

#include <array.hpp>
#include <typetraits.hpp>

namespace Designar
{
    /** @see PairingHeap (pairingheap.hpp) for why this privately derives
        from `DefaultCmpHolder<Cmp>` and why that must be the first
        base — same reasoning applies verbatim.

        @warning decrease_key() follows the standard textbook binomial-
        heap technique of swapping *keys* up the parent chain rather
        than relinking pointers — exactly what a binary heap's sift-up
        does. That means the `Node*` handle passed to decrease_key() may
        end up holding a *different* key than the one just lowered (the
        lowered value can end up stored in an ancestor node instead,
        with some other key swapped down into the handle passed in): a
        handle should be treated as "the key I originally inserted, or
        wherever that value bubbled to" only if you keep re-reading
        `get_key()` immediately after each decrease_key() call, not as a
        stable identity across multiple decreases. */
    template <typename Key, class Cmp = std::less<Key>>
    class BinomialHeap : private DefaultCmpHolder<Cmp>
    {
    public:
        class Node
        {
            friend class BinomialHeap;

            Key key;
            Node* parent;
            Node* child;
            Node* sibling;
            nat_t degree;

            Node(const Key& k)
                : key(k),
                  parent(nullptr),
                  child(nullptr),
                  sibling(nullptr),
                  degree(0)
            {
                // empty
            }

            Node(Key&& k)
                : key(std::forward<Key>(k)),
                  parent(nullptr),
                  child(nullptr),
                  sibling(nullptr),
                  degree(0)
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
        Node* head; // root list, sorted by strictly increasing degree
        nat_t num_items;
        Cmp& cmp;

        static Cmp& cmp_for_copy(BinomialHeap& self, const BinomialHeap& h)
        {
            if (&h.cmp == &h.default_cmp)
            {
                self.default_cmp = h.default_cmp;
                return self.default_cmp;
            }

            return h.cmp;
        }

        /** Makes `y` the new leftmost child of `x` — only ever called
            with `y`/`x` of equal degree (the one case binomial-tree
            linking is defined for), so `x`'s degree simply increments
            to become the next binomial tree order up. */
        static void link(Node* y, Node* x)
        {
            y->parent = x;
            y->sibling = x->child;
            x->child = y;
            ++x->degree;
        }

        /** Merges two root lists (each already sorted by degree) into
            one sorted-by-degree list, without combining any equal-
            degree pair yet — that is union_heaps()'s job just below.
            Plain linked-list merge, the same shape as merge sort's
            merge step. */
        static Node* merge_root_lists(Node* h1, Node* h2)
        {
            if (h1 == nullptr)
            {
                return h2;
            }

            if (h2 == nullptr)
            {
                return h1;
            }

            Node* new_head;
            Node** tail = &new_head;

            while (h1 != nullptr && h2 != nullptr)
            {
                if (h1->degree <= h2->degree)
                {
                    *tail = h1;
                    h1 = h1->sibling;
                }
                else
                {
                    *tail = h2;
                    h2 = h2->sibling;
                }

                tail = &(*tail)->sibling;
            }

            *tail = h1 != nullptr ? h1 : h2;
            return new_head;
        }

        /** The classic binary-addition-with-carry walk over the merged,
            degree-sorted root list: three same-degree roots in a row
            can appear right after merge_root_lists() (one list already
            had a tree of that degree, the other contributed two —
            impossible on its own, but the *merged* list can still have
            a duplicate-degree run of length up to 3 momentarily), so
            the lookahead at `next->sibling` decides whether to link now
            or defer exactly the way you'd decide whether to carry when
            adding two binary numbers digit by digit. */
        Node* union_heaps(Node* h1, Node* h2)
        {
            Node* h = merge_root_lists(h1, h2);

            if (h == nullptr)
            {
                return nullptr;
            }

            Node* prev = nullptr;
            Node* curr = h;
            Node* next = curr->sibling;

            while (next != nullptr)
            {
                bool same_degree = curr->degree == next->degree;
                bool triple_run =
                    next->sibling != nullptr &&
                    next->sibling->degree == curr->degree;

                if (!same_degree || triple_run)
                {
                    prev = curr;
                    curr = next;
                }
                else if (!cmp(next->key, curr->key))
                {
                    // curr->key <= next->key: next becomes curr's child.
                    curr->sibling = next->sibling;
                    link(next, curr);
                }
                else
                {
                    // next->key < curr->key: curr becomes next's child.
                    if (prev == nullptr)
                    {
                        h = next;
                    }
                    else
                    {
                        prev->sibling = next;
                    }

                    link(curr, next);
                    curr = next;
                }

                next = curr->sibling;
            }

            return h;
        }

        Node* find_min_root() const
        {
            Node* best = head;

            for (Node* n = head; n != nullptr; n = n->sibling)
            {
                if (cmp(n->key, best->key))
                {
                    best = n;
                }
            }

            return best;
        }

        /** Reverses a child list into a standalone root list (used by
            extract_min() on the removed minimum's children), clearing
            each node's parent pointer along the way since it is about
            to become a top-level tree again. */
        static Node* reverse_and_orphan(Node* n)
        {
            Node* prev = nullptr;

            while (n != nullptr)
            {
                Node* next = n->sibling;
                n->sibling = prev;
                n->parent = nullptr;
                prev = n;
                n = next;
            }

            return prev;
        }

        static void destroy(Node* n)
        {
            while (n != nullptr)
            {
                Node* next = n->sibling;
                destroy(n->child);
                delete n;
                n = next;
            }
        }

        static Node* copy(Node* n)
        {
            if (n == nullptr)
            {
                return nullptr;
            }

            Node* c = new Node(n->key);
            c->degree = n->degree;
            c->child = copy(n->child);

            for (Node* cc = c->child; cc != nullptr; cc = cc->sibling)
            {
                cc->parent = c;
            }

            c->sibling = copy(n->sibling);
            return c;
        }

    public:
        BinomialHeap(Cmp& _cmp) : head(nullptr), num_items(0), cmp(_cmp)
        {
            // empty
        }

        BinomialHeap(Cmp&& _cmp = Cmp())
            : head(nullptr), num_items(0), cmp(this->default_cmp)
        {
            this->default_cmp = _cmp;
        }

        BinomialHeap(const BinomialHeap& h)
            : head(copy(h.head)),
              num_items(h.num_items),
              cmp(cmp_for_copy(*this, h))
        {
            // empty
        }

        BinomialHeap(BinomialHeap&& h) : BinomialHeap()
        {
            swap(h);
        }

        ~BinomialHeap()
        {
            clear();
        }

        BinomialHeap& operator=(const BinomialHeap& h)
        {
            if (this == &h)
            {
                return *this;
            }

            destroy(head);
            head = copy(h.head);
            num_items = h.num_items;
            cmp = h.cmp;
            return *this;
        }

        BinomialHeap& operator=(BinomialHeap&& h)
        {
            swap(h);
            return *this;
        }

        void swap(BinomialHeap& h)
        {
            std::swap(head, h.head);
            std::swap(num_items, h.num_items);
            std::swap(cmp, h.cmp);
        }

        bool is_empty() const
        {
            return head == nullptr;
        }

        nat_t size() const
        {
            return num_items;
        }

        void clear()
        {
            destroy(head);
            head = nullptr;
            num_items = 0;
        }

        Node* insert(const Key& k)
        {
            Node* n = new Node(k);
            head = union_heaps(head, n);
            ++num_items;
            return n;
        }

        Node* insert(Key&& k)
        {
            Node* n = new Node(std::forward<Key>(k));
            head = union_heaps(head, n);
            ++num_items;
            return n;
        }

        const Key& get_min() const
        {
            if (is_empty())
            {
                throw std::underflow_error("BinomialHeap is empty");
            }

            return find_min_root()->key;
        }

        Key extract_min()
        {
            if (is_empty())
            {
                throw std::underflow_error("BinomialHeap is empty");
            }

            Node* min_node = find_min_root();

            Node* remaining;
            Node** tail = &remaining;

            for (Node* n = head; n != nullptr; n = n->sibling)
            {
                if (n != min_node)
                {
                    *tail = n;
                    tail = &(*tail)->sibling;
                }
            }

            *tail = nullptr;

            Node* orphaned_children = reverse_and_orphan(min_node->child);
            head = union_heaps(remaining, orphaned_children);

            Key ret_val = std::move(min_node->key);
            delete min_node;
            --num_items;

            return ret_val;
        }

        /** @warning see the class doc comment — bubbles the new key up
            by swapping key values with ancestors, not by relinking
            `n` itself, so `n` may end up holding a different key than
            the one just lowered. */
        void decrease_key(Node* n, const Key& new_key)
        {
            if (cmp(n->key, new_key))
            {
                throw std::domain_error("new_key is greater than current key");
            }

            n->key = new_key;
            Node* p = n->parent;

            while (p != nullptr && cmp(n->key, p->key))
            {
                std::swap(n->key, p->key);
                n = p;
                p = n->parent;
            }
        }
    };

} // end namespace Designar
