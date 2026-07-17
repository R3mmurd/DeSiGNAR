/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file skiplist.hpp
    @brief SkipList: a probabilistically-balanced ordered set, built from
    several stacked linked lists where each level is a random subset of
    the level below it — giving expected O(lg n) search/insert/remove
    without any rotations at all (William Pugh, 1990).
    @ingroup DataStructures
*/

#pragma once

#include <optional>

#include <array.hpp>
#include <containeralgorithms.hpp>
#include <setalgorithms.hpp>
#include <iterator.hpp>
#include <typetraits.hpp>
#include <random.hpp>

namespace Designar
{
    /** Every node has a "tower" of forward pointers, one per level it
        participates in; a search walks each level's list as far as
        possible before dropping down one level, so faster (higher) levels
        let it skip over large stretches of the lower, denser levels.
        Whether a newly-inserted node's tower grows another level is
        decided by an independent coin flip per level (probability `P`),
        which is what gives a skip list its balance guarantee in
        expectation — much like Treap's random priorities, but applied to
        how tall each node's tower grows rather than to a total order used
        for rotations.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base. */
    template <typename Key, class Cmp = std::less<Key>>
    class SkipList : private DefaultCmpHolder<Cmp>,
                     public ContainerAlgorithms<SkipList<Key, Cmp>, Key>,
                     public SetAlgorithms<SkipList<Key, Cmp>, Key>
    {
        static constexpr nat_t MAX_LEVEL = 32;
        static constexpr real_t P = 0.5;

        /** `key` is a `std::optional<Key>` rather than a plain `Key key;`
            — `header` (the one node built via the level-only
            constructor) never actually has its key read anywhere in this
            file, so requiring `Key` to be default-constructible purely
            to give the header node *some* key value was unnecessary (the
            same class of needless requirement DynArray used to impose on
            its own element type). */
        struct Node
        {
            std::optional<Key> key;
            DynArray<Node*> forward;

            Node(const Key& k, nat_t lvl) : key(k), forward(lvl, nullptr)
            {
                // empty
            }

            Node(Key&& k, nat_t lvl)
                : key(std::forward<Key>(k)), forward(lvl, nullptr)
            {
                // empty
            }

            Node(nat_t lvl) : key(), forward(lvl, nullptr)
            {
                // empty
            }
        };

        Node* header;
        nat_t level;
        nat_t num_items;
        Cmp& cmp;
        rng_t rng;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(SkipList& self, const SkipList& t)
        {
            if (&t.cmp == &t.default_cmp)
            {
                self.default_cmp = t.default_cmp;
                return self.default_cmp;
            }

            return t.cmp;
        }

        nat_t random_level()
        {
            nat_t lvl = 1;

            while (lvl < MAX_LEVEL && random_Bernoulli(rng, P))
            {
                ++lvl;
            }

            return lvl;
        }

        void destroy()
        {
            Node* n = header->forward[0];

            while (n != nullptr)
            {
                Node* next = n->forward[0];
                delete n;
                n = next;
            }
        }

        /** Finds, for every level from the top down to 0, the last node
            whose key is < `k` — the classic "update" array a skip list
            needs both to insert (splice the new node in after each of
            these) and to remove (splice the found node out from after each
            of these). */
        DynArray<Node*> locate_update(const Key& k) const
        {
            DynArray<Node*> update(MAX_LEVEL, header);

            Node* curr = header;

            for (int_t i = int_t(level) - 1; i >= 0; --i)
            {
                while (curr->forward[i] != nullptr &&
                       cmp(*curr->forward[i]->key, k))
                {
                    curr = curr->forward[i];
                }

                update[i] = curr;
            }

            return update;
        }

    public:
        using ItemType = Key;
        using KeyType = Key;
        using DataType = Key;
        using ValueType = Key;
        using SizeType = nat_t;
        using CmpType = Cmp;

        SkipList(Cmp& _cmp)
            : header(new Node(MAX_LEVEL)),
              level(1),
              num_items(0),
              cmp(_cmp),
              rng(time(nullptr))
        {
            // empty
        }

        /** @see RankedTreap's matching constructor for why this cannot
            simply delegate to the Cmp& overload above, and why
            copy-assignment (not move) is used for `default_cmp`. */
        SkipList(Cmp&& _cmp = Cmp())
            : header(new Node(MAX_LEVEL)),
              level(1),
              num_items(0),
              cmp(this->default_cmp),
              rng(time(nullptr))
        {
            this->default_cmp = _cmp;
        }

        SkipList(const SkipList& t)
            : header(new Node(MAX_LEVEL)),
              level(1),
              num_items(0),
              cmp(cmp_for_copy(*this, t)),
              rng(time(nullptr))
        {
            for (const Key& k : t)
            {
                insert(k);
            }
        }

        SkipList(SkipList&& t) : SkipList()
        {
            swap(t);
        }

        SkipList(const std::initializer_list<Key>&);

        ~SkipList()
        {
            destroy();
            delete header;
        }

        SkipList& operator=(const SkipList& t)
        {
            if (this == &t)
            {
                return *this;
            }

            destroy();
            level = 1;
            num_items = 0;

            for (nat_t i = 0; i < MAX_LEVEL; ++i)
            {
                header->forward[i] = nullptr;
            }

            cmp = t.cmp;

            for (const Key& k : t)
            {
                insert(k);
            }

            return *this;
        }

        SkipList& operator=(SkipList&& t)
        {
            swap(t);
            return *this;
        }

        void swap(SkipList& t)
        {
            std::swap(header, t.header);
            std::swap(level, t.level);
            std::swap(num_items, t.num_items);
            std::swap(cmp, t.cmp);
        }

        bool is_empty() const
        {
            return num_items == 0;
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
            destroy();
            level = 1;
            num_items = 0;

            for (nat_t i = 0; i < MAX_LEVEL; ++i)
            {
                header->forward[i] = nullptr;
            }
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
            DynArray<Node*> update = locate_update(k);
            Node* curr = update[0]->forward[0];

            if (curr != nullptr && !cmp(k, *curr->key) && !cmp(*curr->key, k))
            {
                return nullptr; // duplicate
            }

            nat_t new_level = random_level();

            if (new_level > level)
            {
                for (nat_t i = level; i < new_level; ++i)
                {
                    update[i] = header;
                }

                level = new_level;
            }

            Node* n = new Node(k, new_level);

            for (nat_t i = 0; i < new_level; ++i)
            {
                n->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = n;
            }

            ++num_items;
            return &*n->key;
        }

        Key* insert(Key&& k)
        {
            DynArray<Node*> update = locate_update(k);
            Node* curr = update[0]->forward[0];

            if (curr != nullptr && !cmp(k, *curr->key) && !cmp(*curr->key, k))
            {
                return nullptr; // duplicate
            }

            nat_t new_level = random_level();

            if (new_level > level)
            {
                for (nat_t i = level; i < new_level; ++i)
                {
                    update[i] = header;
                }

                level = new_level;
            }

            Node* n = new Node(std::forward<Key>(k), new_level);

            for (nat_t i = 0; i < new_level; ++i)
            {
                n->forward[i] = update[i]->forward[i];
                update[i]->forward[i] = n;
            }

            ++num_items;
            return &*n->key;
        }

        Key* append(const Key& k)
        {
            return insert(k);
        }

        Key* append(Key&& k)
        {
            return insert(std::forward<Key>(k));
        }

        Key* search(const Key& k)
        {
            Node* curr = header;

            for (int_t i = int_t(level) - 1; i >= 0; --i)
            {
                while (curr->forward[i] != nullptr &&
                       cmp(*curr->forward[i]->key, k))
                {
                    curr = curr->forward[i];
                }
            }

            curr = curr->forward[0];

            if (curr != nullptr && !cmp(k, *curr->key) && !cmp(*curr->key, k))
            {
                return &*curr->key;
            }

            return nullptr;
        }

        const Key* search(const Key& k) const
        {
            return const_cast<SkipList*>(this)->search(k);
        }

        Key* search_or_insert(const Key& k)
        {
            Key* found = search(k);
            return found != nullptr ? found : insert(k);
        }

        Key* search_or_insert(Key&& k)
        {
            Key* found = search(k);
            return found != nullptr ? found : insert(std::forward<Key>(k));
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
            DynArray<Node*> update = locate_update(k);
            Node* curr = update[0]->forward[0];

            if (curr == nullptr || cmp(k, *curr->key) || cmp(*curr->key, k))
            {
                return false;
            }

            for (nat_t i = 0; i < level; ++i)
            {
                if (update[i]->forward[i] != curr)
                {
                    break;
                }

                update[i]->forward[i] = curr->forward[i];
            }

            delete curr;

            while (level > 1 && header->forward[level - 1] == nullptr)
            {
                --level;
            }

            --num_items;
            return true;
        }

        const Key& min() const
        {
            if (is_empty())
            {
                throw std::underflow_error("SkipList is empty");
            }

            return *header->forward[0]->key;
        }

        const Key& max() const
        {
            if (is_empty())
            {
                throw std::underflow_error("SkipList is empty");
            }

            Node* curr = header;

            for (int_t i = int_t(level) - 1; i >= 0; --i)
            {
                while (curr->forward[i] != nullptr)
                {
                    curr = curr->forward[i];
                }
            }

            return *curr->key;
        }

        class Iterator : public ForwardIterator<Iterator, Key>
        {
            friend class BasicIterator<Iterator, Key>;
            friend class SkipList;

            Node* curr;

        protected:
            Node* get_location() const
            {
                return curr;
            }

        public:
            Iterator() : curr(nullptr)
            {
                // empty
            }

            Iterator(Node* n) : curr(n)
            {
                // empty
            }

            bool has_current() const
            {
                return curr != nullptr;
            }

            Key& get_current()
            {
                if (!has_current())
                {
                    throw std::overflow_error("There is not current element");
                }

                return *curr->key;
            }

            const Key& get_current() const
            {
                if (!has_current())
                {
                    throw std::overflow_error("There is not current element");
                }

                return *curr->key;
            }

            void next()
            {
                if (!has_current())
                {
                    return;
                }

                curr = curr->forward[0];
            }
        };

        Iterator begin()
        {
            return Iterator(header->forward[0]);
        }

        Iterator begin() const
        {
            return Iterator(header->forward[0]);
        }

        Iterator end()
        {
            return Iterator(nullptr);
        }

        Iterator end() const
        {
            return Iterator(nullptr);
        }
    };

    template <typename Key, class Cmp>
    SkipList<Key, Cmp>::SkipList(const std::initializer_list<Key>& l)
        : SkipList()
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

} // end namespace Designar
