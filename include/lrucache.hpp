/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file lrucache.hpp
    @brief LRUCache: a fixed-capacity key/value cache that evicts the
    least-recently-used entry once full, with O(1) get()/put() via a
    HashMap (key -> node) paired with an intrusive doubly-linked list
    tracking recency order.
    @ingroup DataStructures
*/

#pragma once

#include <map.hpp>

namespace Designar
{
    /** Every entry lives in exactly two structures at once: a HashMap
        (for O(1) lookup by key) and a doubly-linked list ordered by
        recency of use, most-recently-used at the head — both pointing at
        the *same* heap-allocated Node, so moving an entry to the front on
        access, or evicting the tail entry on overflow, is a handful of
        pointer updates plus one hash-map removal, never a scan. */
    template <typename Key, typename Value, class Cmp = std::equal_to<Key>>
    class LRUCache
    {
        struct Node
        {
            Key key;
            Value value;
            Node* prev = nullptr;
            Node* next = nullptr;

            /** A single forwarding constructor covers all four
                combinations of lvalue/rvalue Key/Value (matching every
                other container in this library's const&/&& overload
                ladder without writing out all four by hand) — nothing
                here needs to inspect `k`/`v`, only perfectly forward them
                into `key`/`value`, so `K`/`V` need only be deducible as
                `Key`/`Value` (or an rvalue thereof), not distinct types
                in their own right. */
            template <class K, class V>
            Node(K&& k, V&& v)
                : key(std::forward<K>(k)), value(std::forward<V>(v))
            {
                // empty
            }
        };

        nat_t cap;
        nat_t num_items;
        Node* head; // most recently used
        Node* tail; // least recently used
        HashMap<Key, Node*, Cmp> index;

        void unlink(Node* n)
        {
            if (n->prev != nullptr)
            {
                n->prev->next = n->next;
            }
            else
            {
                head = n->next;
            }

            if (n->next != nullptr)
            {
                n->next->prev = n->prev;
            }
            else
            {
                tail = n->prev;
            }

            n->prev = n->next = nullptr;
        }

        void push_front(Node* n)
        {
            n->prev = nullptr;
            n->next = head;

            if (head != nullptr)
            {
                head->prev = n;
            }

            head = n;

            if (tail == nullptr)
            {
                tail = n;
            }
        }

        void evict_tail()
        {
            Node* victim = tail;
            unlink(victim);
            index.remove(victim->key);
            delete victim;
            --num_items;
        }

        /** @see put()'s four overloads — `K`/`V` are always deduced as
            `Key`/`Value` (or an rvalue thereof), forwarded through to
            either Node's constructor (new entry) or a move/copy
            assignment (existing entry), whichever the caller's argument
            value category calls for. */
        template <class K, class V>
        void put_impl(K&& k, V&& v)
        {
            Node** np = index.search(k);

            if (np != nullptr)
            {
                Node* n = *np;
                n->value = std::forward<V>(v);
                unlink(n);
                push_front(n);
                return;
            }

            if (cap > 0 && num_items == cap)
            {
                evict_tail();
            }

            Node* n = new Node(std::forward<K>(k), std::forward<V>(v));
            push_front(n);
            index.insert(n->key, n);
            ++num_items;
        }

        void destroy_all()
        {
            Node* n = head;

            while (n != nullptr)
            {
                Node* next = n->next;
                delete n;
                n = next;
            }
        }

    public:
        /** `capacity == 0` means unbounded (put() never evicts) — mostly
            useful for testing the recency bookkeeping in isolation from
            eviction; a real cache should pick a real capacity. */
        LRUCache(nat_t capacity = 128)
            : cap(capacity), num_items(0), head(nullptr), tail(nullptr)
        {
            // empty
        }

        LRUCache(const LRUCache&) = delete;
        LRUCache& operator=(const LRUCache&) = delete;

        LRUCache(LRUCache&& other)
            : cap(other.cap),
              num_items(other.num_items),
              head(other.head),
              tail(other.tail),
              index(std::move(other.index))
        {
            other.head = other.tail = nullptr;
            other.num_items = 0;
        }

        LRUCache& operator=(LRUCache&& other)
        {
            if (this == &other)
            {
                return *this;
            }

            destroy_all();
            cap = other.cap;
            num_items = other.num_items;
            head = other.head;
            tail = other.tail;
            index = std::move(other.index);
            other.head = other.tail = nullptr;
            other.num_items = 0;
            return *this;
        }

        ~LRUCache()
        {
            destroy_all();
        }

        nat_t size() const
        {
            return num_items;
        }

        nat_t capacity() const
        {
            return cap;
        }

        bool is_empty() const
        {
            return num_items == 0;
        }

        bool contains(const Key& k) const
        {
            return index.search(k) != nullptr;
        }

        /** Returns a pointer to `k`'s value and marks it most-recently-used,
            or `nullptr` if `k` is not cached. */
        Value* get(const Key& k)
        {
            Node** np = index.search(k);

            if (np == nullptr)
            {
                return nullptr;
            }

            Node* n = *np;
            unlink(n);
            push_front(n);
            return &n->value;
        }

        const Value* get(const Key& k) const
        {
            return const_cast<LRUCache*>(this)->get(k);
        }

        /** Inserts or updates `k` -> `v`, marking it most-recently-used;
            evicts the least-recently-used entry first if the cache is at
            capacity and `k` is not already present. Four overloads (not
            just `const&`) so a move-only `Value` (e.g. `std::unique_ptr`,
            `std::future`) can be cached at all — the update-existing-key
            path move- or copy-assigns to match whichever overload was
            called. */
        void put(const Key& k, const Value& v)
        {
            put_impl(k, v);
        }

        void put(Key&& k, const Value& v)
        {
            put_impl(std::forward<Key>(k), v);
        }

        void put(const Key& k, Value&& v)
        {
            put_impl(k, std::forward<Value>(v));
        }

        void put(Key&& k, Value&& v)
        {
            put_impl(std::forward<Key>(k), std::forward<Value>(v));
        }

        void clear()
        {
            destroy_all();
            head = tail = nullptr;
            index.clear();
            num_items = 0;
        }
    };

} // end namespace Designar
