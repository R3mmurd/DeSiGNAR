/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file pairingheap.hpp
    @brief PairingHeap: a self-adjusting mergeable min-heap (Fredman,
    Sedgewick, Sleator & Tarjan, 1986) supporting O(1) insert/get_min
    and O(lg n) amortized decrease_key/extract_min — the same
    asymptotic promises as a Fibonacci heap, with a dramatically simpler
    implementation and, in practice, better constant factors.
    @ingroup DataStructures
*/

#pragma once

#include <array.hpp>
#include <typetraits.hpp>

namespace Designar
{
    /** A pairing heap represents its forest as a single multi-way tree
        using the classic "leftmost-child, next-sibling" encoding (each
        node points at its first child and its next sibling, rather than
        an array/list of children), which is what makes meld() — the
        operation every other operation (insert, extract_min,
        decrease_key) is built from — a handful of pointer updates: to
        combine two heaps, compare their roots and hang the
        larger-rooted tree off the smaller-rooted one as its new leftmost
        child.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class PairingHeap : private DefaultCmpHolder<Cmp>
    {
    public:
        class Node
        {
            friend class PairingHeap;

            Key key;
            Node* child;
            Node* sibling;

            Node(const Key& k) : key(k), child(nullptr), sibling(nullptr)
            {
                // empty
            }

            Node(Key&& k)
                : key(std::forward<Key>(k)), child(nullptr), sibling(nullptr)
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

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(PairingHeap& self, const PairingHeap& h)
        {
            if (&h.cmp == &h.default_cmp)
            {
                self.default_cmp = h.default_cmp;
                return self.default_cmp;
            }

            return h.cmp;
        }

        /** Combines two heaps' root trees into one, in O(1): whichever
            root's key loses the comparison becomes the new leftmost child
            of the winner. This one operation is the basis for insert()
            (meld with a new singleton), extract_min() (meld all of the
            removed root's children back together), and decrease_key()
            (cut a node out and meld it back in at the top level). */
        Node* meld(Node* a, Node* b)
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

            b->sibling = a->child;
            a->child = b;
            return a;
        }

        /** extract_min()'s classic two-pass pairing: first pair up
            consecutive siblings left to right (meld each pair into one
            tree), then combine the resulting (about half as many) trees
            right to left into a single tree. This specific two-pass
            pattern (rather than e.g. melding strictly left to right) is
            what gives the amortized O(lg n) bound; melding naively in one
            left-to-right pass can degrade to O(n) per extract_min on
            certain input sequences. */
        Node* merge_pairs(Node* first)
        {
            if (first == nullptr || first->sibling == nullptr)
            {
                return first;
            }

            Node* a = first;
            Node* b = first->sibling;
            Node* rest = b->sibling;

            a->sibling = b->sibling = nullptr;

            return meld(meld(a, b), merge_pairs(rest));
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
            c->child = copy(n->child);
            c->sibling = copy(n->sibling);
            return c;
        }

    public:
        PairingHeap(Cmp& _cmp) : root(nullptr), num_items(0), cmp(_cmp)
        {
            // empty
        }

        /** @see RankedTreap's matching constructor for why this cannot
            simply delegate to the Cmp& overload above, and why
            copy-assignment (not move) is used for `default_cmp`. */
        PairingHeap(Cmp&& _cmp = Cmp())
            : root(nullptr), num_items(0), cmp(this->default_cmp)
        {
            this->default_cmp = _cmp;
        }

        PairingHeap(const PairingHeap& h)
            : root(copy(h.root)),
              num_items(h.num_items),
              cmp(cmp_for_copy(*this, h))
        {
            // empty
        }

        PairingHeap(PairingHeap&& h) : PairingHeap()
        {
            swap(h);
        }

        ~PairingHeap()
        {
            clear();
        }

        PairingHeap& operator=(const PairingHeap& h)
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

        PairingHeap& operator=(PairingHeap&& h)
        {
            swap(h);
            return *this;
        }

        void swap(PairingHeap& h)
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

        /** Returns a handle to the newly-inserted node, which
            decrease_key()/remove() need to identify *which* node to act on
            — there is no search() (a pairing heap's tree shape carries no
            useful ordering below the root for locating an arbitrary key
            quickly; callers that need to find a node again should keep the
            handle this returns). */
        Node* insert(const Key& k)
        {
            Node* n = new Node(k);
            root = meld(root, n);
            ++num_items;
            return n;
        }

        Node* insert(Key&& k)
        {
            Node* n = new Node(std::forward<Key>(k));
            root = meld(root, n);
            ++num_items;
            return n;
        }

        const Key& get_min() const
        {
            if (is_empty())
            {
                throw std::underflow_error("PairingHeap is empty");
            }

            return root->key;
        }

        Key extract_min()
        {
            if (is_empty())
            {
                throw std::underflow_error("PairingHeap is empty");
            }

            Node* old_root = root;
            Key ret_val = std::move(old_root->key);

            root = merge_pairs(old_root->child);
            delete old_root;
            --num_items;

            return ret_val;
        }

        /** Lowers `n`'s key to `new_key` (which must not compare greater
            than its current key — this is a min-heap, so a "decrease") and
            restores heap order in O(lg n) amortized time by cutting `n`
            (with its whole subtree) away from its parent and melding it
            back in as a top-level tree, rather than sifting it up in
            place — a pairing heap's nodes don't keep a parent pointer to
            sift up with, only sibling/child, so cut-and-reinsert is the
            mechanism available. */
        void decrease_key(Node* n, const Key& new_key)
        {
            if (cmp(n->key, new_key))
            {
                throw std::domain_error("new_key is greater than current key");
            }

            n->key = new_key;

            if (n == root)
            {
                return;
            }

            // Detach n from wherever it sits in its parent's child list.
            // Pairing heaps keep no parent pointer, so unlinking n means
            // walking the tree to find whichever node's child list currently
            // contains it — the same trade-off (no parent pointer, O(1)
            // meld) as the rest of the structure.
            unlink_from_parent(root, n);
            root = meld(root, n);
        }

    private:
        /** Searches `subtree` for whichever node has `target` in its child
            list and splices `target` out of that list (leaving `target`'s
            own child/sibling pointers as they were, except `target->sibling`
            is cleared — it is about to be re-melded as a standalone tree by
            decrease_key(), so it must not still point at its former
            siblings). Returns whether `target` was found and unlinked. */
        static bool unlink_from_parent(Node* subtree, Node* target)
        {
            if (subtree == nullptr)
            {
                return false;
            }

            Node* prev = nullptr;

            for (Node* c = subtree->child; c != nullptr;
                 prev = c, c = c->sibling)
            {
                if (c == target)
                {
                    if (prev == nullptr)
                    {
                        subtree->child = target->sibling;
                    }
                    else
                    {
                        prev->sibling = target->sibling;
                    }

                    target->sibling = nullptr;
                    return true;
                }

                if (unlink_from_parent(c, target))
                {
                    return true;
                }
            }

            return false;
        }
    };

} // end namespace Designar
