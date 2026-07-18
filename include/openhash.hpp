/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file openhash.hpp
    @brief Open-addressing hash tables: a single generic implementation
    parameterized by pluggable probe-sequence policies (linear probing,
    quadratic probing, double hashing), as an alternative collision
    strategy to chainedhash.hpp's SeparateChainingHashTable.
    @ingroup Hashing
*/

#pragma once

#include <optional>

#include <array.hpp>
#include <containeralgorithms.hpp>
#include <setalgorithms.hpp>
#include <iterator.hpp>
#include <typetraits.hpp>
#include <hash.hpp> // for super_fast_hash

namespace Designar
{
    enum class OAState : unsigned char
    {
        EMPTY,
        OCCUPIED,
        DELETED
    };

    /** `key` is a `std::optional<Key>` rather than a plain `Key key{};` —
        an EMPTY slot never actually needs a live `Key` at all, so
        requiring `Key` to be default-constructible purely to have *some*
        value sitting in every never-yet-used slot is unnecessary (the
        same class of needless requirement DynArray used to impose on its
        own element type). `std::optional<Key>` default-constructs to
        "no value" regardless of whether `Key` itself is
        default-constructible, which is exactly what an EMPTY/DELETED
        slot means here. */
    template <typename Key>
    struct OAEntry
    {
        OAState state = OAState::EMPTY;
        std::optional<Key> key;
    };

    /** probe(h1, h2, i, cap) returns the slot to examine on the i-th probe
        (i starting at 0) of a key whose two independent hash values are
        `h1` and `h2`, in a table of `cap` slots. `h2` is unused by linear
        and quadratic probing — it only matters for DoubleHashing — but is
        always computed and passed uniformly so all three policies share
        one calling convention. */
    struct LinearProbing
    {
        static nat_t probe(nat_t h1, nat_t /*h2*/, nat_t i, nat_t cap)
        {
            return (h1 + i) & (cap - 1); // cap is always a power of two
        }
    };

    /** Triangular-number ("safe") quadratic probing: probing offsets are
        0, 1, 3, 6, 10, ... (i.e. `i*(i+1)/2`). This specific sequence is
        known to visit every slot of a power-of-two-sized table exactly
        once before repeating — the naive `i*i` offset does not have that
        guarantee and can loop forever revisiting a handful of slots while
        the table still has empty ones. */
    struct QuadraticProbing
    {
        static nat_t probe(nat_t h1, nat_t /*h2*/, nat_t i, nat_t cap)
        {
            return (h1 + ((i * (i + 1)) >> 1)) & (cap - 1);
        }
    };

    /** Double hashing: the probe offset itself is a multiple of a second,
        independent hash of the key (`h1 + i*h2`), so different keys that
        collide on `h1` almost certainly follow different probe sequences
        — unlike linear/quadratic probing, where every key colliding at
        the same slot follows the exact same subsequent sequence
        ("clustering"). Visiting every slot requires `h2` to be coprime
        with `cap`; since `cap` is always a power of two here, forcing
        `h2` to be odd (see OpenAddressingHashTable::secondary_hash) is
        sufficient to guarantee that. */
    struct DoubleHashing
    {
        static nat_t probe(nat_t h1, nat_t h2, nat_t i, nat_t cap)
        {
            return (h1 + i * h2) & (cap - 1);
        }
    };

    /** A hash table using open addressing: unlike chainedhash.hpp's
        SeparateChainingHashTable (which resolves collisions by chaining
        a linked list per bucket),
        every key here lives directly in the backing array itself, and a
        collision is resolved by probing a sequence of alternative slots
        (determined by `Probing`) until an empty one is found. Deletions
        leave a DELETED tombstone behind (a slot search must keep probing
        past it, since stopping there could hide a key inserted later that
        probed past this same slot) rather than truly emptying the slot;
        tombstones count against the load factor and are cleared out
        whenever the table grows.

        @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` and why that must be its first base.
        @see SeparateChainingHashTable (chainedhash.hpp) for the
        separate-chaining alternative.
    */
    template <typename Key, class Cmp = std::equal_to<Key>,
              class Probing = LinearProbing>
    class OpenAddressingHashTable
        : private DefaultCmpHolder<Cmp>,
          public ContainerAlgorithms<OpenAddressingHashTable<Key, Cmp, Probing>,
                                     Key>,
          public SetAlgorithms<OpenAddressingHashTable<Key, Cmp, Probing>, Key>
    {
    public:
        using Entry = OAEntry<Key>;
        using ItemType = Key;
        using KeyType = Key;
        using DataType = Key;
        using ValueType = Key;
        using SizeType = nat_t;
        using CmpType = Cmp;
        using HashFctPtr = nat_t (*)(const Key&);
        using HashFctType = std::function<nat_t(const Key&)>;

        static constexpr nat_t DFT_SIZE = 32; // must stay a power of two
        static constexpr real_t DFT_UPPER_ALPHA = 0.5;

    private:
        DynArray<Entry> table;
        nat_t num_items;
        nat_t num_tombstones;
        Cmp& cmp;
        HashFctType hash_fct;
        real_t upper_alpha;

        /** @see GenArraySet::cmp_for_copy — same ownership-preserving copy
            logic and the same reason it is needed. */
        static Cmp& cmp_for_copy(OpenAddressingHashTable& self,
                                 const OpenAddressingHashTable& h)
        {
            if (&h.cmp == &h.default_cmp)
            {
                self.default_cmp = h.default_cmp;
                return self.default_cmp;
            }

            return h.cmp;
        }

        static nat_t next_pow2(nat_t n)
        {
            nat_t p = 1;

            while (p < n)
            {
                p <<= 1;
            }

            return p;
        }

        /** `cap` independently-default-constructed empty entries, built via
            repeated append() rather than `DynArray<Entry>(cap, Entry())`
            (which copy-*constructs* every slot from one shared `Entry()`)
            — `Entry` is only ever default-constructible when `Key` is
            move-only (its `std::optional<Key>` member has no copy
            constructor then), so copying a single instance `cap` times
            would needlessly require `Key` to be copyable just to build an
            empty table. */
        static DynArray<Entry> make_empty_table(nat_t cap)
        {
            DynArray<Entry> t(cap);

            for (nat_t i = 0; i < cap; ++i)
            {
                t.append(Entry());
            }

            return t;
        }

        /** A second, independent hash used only by DoubleHashing (see its
            comment); forced odd so it is always coprime with `cap` (a power
            of two), which is what guarantees every slot is eventually
            probed instead of only a fraction of them. Computed unconditionally,
            even for policies that ignore it, to keep one calling convention
            for `Probing::probe` across all three policies. */
        nat_t secondary_hash(const Key& k) const
        {
            nat_t cap = table.size();
            return (2 * ((hash_fct(k) / cap) % (cap / 2))) + 1;
        }

        real_t alpha() const
        {
            return real_t(num_items + num_tombstones) / real_t(table.size());
        }

        /** Rebuilds the table at (at least) `new_cap` slots, re-inserting
            every live key with fresh probe sequences and dropping every
            tombstone — the only way tombstones are ever reclaimed. */
        void rehash(nat_t new_cap)
        {
            new_cap = next_pow2(new_cap);

            DynArray<Entry> new_table(make_empty_table(new_cap));

            DynArray<Entry> old_table;
            std::swap(table, old_table);

            table = std::move(new_table);
            nat_t old_num_items = num_items;
            num_items = 0;
            num_tombstones = 0;

            for (nat_t i = 0; i < old_table.size(); ++i)
            {
                if (old_table[i].state == OAState::OCCUPIED)
                {
                    raw_insert(std::move(*old_table[i].key));
                }
            }

            assert(num_items == old_num_items);
            (void)old_num_items;
        }

        void grow_if_needed()
        {
            if (alpha() < upper_alpha)
            {
                return;
            }

            rehash(table.size() * 2);
        }

        /** Inserts `k`, assuming it is not already present and the table has
            room — used internally by rehash() (where both are already
            guaranteed) and by the public insert() after it has done the
            duplicate check and possible grow_if_needed() itself.
            find_target_slot() does everything except actually writing `k`
            in, so the const&/&& overloads below only differ in that one
            assignment (copy vs move). */
        nat_t find_target_slot(const Key& k)
        {
            nat_t cap = table.size();
            nat_t h1 = hash_fct(k) & (cap - 1);
            nat_t h2 = secondary_hash(k);
            nat_t first_tombstone = cap; // cap is never a valid slot index

            for (nat_t i = 0; i < cap; ++i)
            {
                nat_t idx = Probing::probe(h1, h2, i, cap);
                Entry& e = table[idx];

                if (e.state == OAState::EMPTY)
                {
                    nat_t target =
                        first_tombstone < cap ? first_tombstone : idx;

                    if (target != idx)
                    {
                        --num_tombstones;
                    }

                    table[target].state = OAState::OCCUPIED;
                    ++num_items;
                    return target;
                }

                if (e.state == OAState::DELETED)
                {
                    if (first_tombstone == cap)
                    {
                        first_tombstone = idx;
                    }

                    continue;
                }
            }

            throw std::logic_error(
                "OpenAddressingHashTable: no empty slot found "
                "(load factor invariant violated)");
        }

        Key* raw_insert(const Key& k)
        {
            nat_t target = find_target_slot(k);
            table[target].key = k;
            return &*table[target].key;
        }

        Key* raw_insert(Key&& k)
        {
            nat_t target = find_target_slot(k);
            table[target].key = std::move(k);
            return &*table[target].key;
        }

    public:
        bool verify() const
        {
            nat_t occupied = 0, tombstones = 0;

            for (nat_t i = 0; i < table.size(); ++i)
            {
                if (table[i].state == OAState::OCCUPIED)
                {
                    ++occupied;
                }
                else if (table[i].state == OAState::DELETED)
                {
                    ++tombstones;
                }
            }

            return occupied == num_items && tombstones == num_tombstones;
        }

        OpenAddressingHashTable(nat_t size, Cmp& _cmp, HashFctType fct,
                                real_t _upper_alpha)
            : table(make_empty_table(next_pow2(size))),
              num_items(0),
              num_tombstones(0),
              cmp(_cmp),
              hash_fct(fct),
              upper_alpha(_upper_alpha)
        {
            // empty
        }

        OpenAddressingHashTable(nat_t size, Cmp& _cmp,
                                HashFctPtr fct = &super_fast_hash)
            : OpenAddressingHashTable(size, _cmp, fct, DFT_UPPER_ALPHA)
        {
            // empty
        }

        /** @see RankedTreap's matching constructor for why this cannot
            simply delegate to the Cmp& overload above, and why
            copy-assignment (not move) is used for `default_cmp`. */
        OpenAddressingHashTable(nat_t size, Cmp&& _cmp = Cmp(),
                                HashFctPtr fct = &super_fast_hash)
            : table(make_empty_table(next_pow2(size))),
              num_items(0),
              num_tombstones(0),
              cmp(this->default_cmp),
              hash_fct(fct),
              upper_alpha(DFT_UPPER_ALPHA)
        {
            this->default_cmp = _cmp;
        }

        OpenAddressingHashTable(Cmp& _cmp, HashFctPtr fct = &super_fast_hash)
            : OpenAddressingHashTable(DFT_SIZE, _cmp, fct)
        {
            // empty
        }

        OpenAddressingHashTable(Cmp&& _cmp = Cmp(),
                                HashFctPtr fct = &super_fast_hash)
            : OpenAddressingHashTable(DFT_SIZE, std::forward<Cmp>(_cmp), fct)
        {
            // empty
        }

        OpenAddressingHashTable(const OpenAddressingHashTable& h)
            : table(h.table),
              num_items(h.num_items),
              num_tombstones(h.num_tombstones),
              cmp(cmp_for_copy(*this, h)),
              hash_fct(h.hash_fct),
              upper_alpha(h.upper_alpha)
        {
            // empty
        }

        OpenAddressingHashTable(OpenAddressingHashTable&& h)
            : OpenAddressingHashTable()
        {
            swap(h);
        }

        OpenAddressingHashTable(const std::initializer_list<Key>&);

        OpenAddressingHashTable& operator=(const OpenAddressingHashTable& h)
        {
            if (this == &h)
            {
                return *this;
            }

            table = h.table;
            num_items = h.num_items;
            num_tombstones = h.num_tombstones;
            cmp = h.cmp;
            hash_fct = h.hash_fct;
            upper_alpha = h.upper_alpha;
            return *this;
        }

        OpenAddressingHashTable& operator=(OpenAddressingHashTable&& h)
        {
            swap(h);
            return *this;
        }

        void swap(OpenAddressingHashTable& h)
        {
            table.swap(h.table);
            std::swap(num_items, h.num_items);
            std::swap(num_tombstones, h.num_tombstones);
            std::swap(cmp, h.cmp);
            std::swap(hash_fct, h.hash_fct);
            std::swap(upper_alpha, h.upper_alpha);
        }

        Cmp& get_cmp()
        {
            return cmp;
        }

        const Cmp& get_cmp() const
        {
            return cmp;
        }

        bool is_empty() const
        {
            return num_items == 0;
        }

        nat_t size() const
        {
            return num_items;
        }

        nat_t capacity() const
        {
            return table.size();
        }

        void clear()
        {
            for (nat_t i = 0; i < table.size(); ++i)
            {
                table[i].state = OAState::EMPTY;
            }

            num_items = num_tombstones = 0;
        }

        /** Returns the slot index containing `k`, or `table.size()` (never a
            valid index) if `k` is not present. */
        nat_t find_slot(const Key& k) const
        {
            return find_slot_by(k, hash_fct);
        }

        /** Heterogeneous counterpart of find_slot(): `k` (of any type `K`)
            is compared to stored keys via `cmp` and hashed via the
            caller-supplied `key_hash_fct` rather than this table's own
            `hash_fct` (which is fixed to hash a full `Key`) — see
            GenMap::search() (map.hpp), which supplies HashMap's own raw
            per-mapped-Key hash function here, the exact one `hash_fct`
            was itself built from. */
        template <typename K, class KeyHashFct>
        nat_t find_slot_by(const K& k, KeyHashFct&& key_hash_fct) const
        {
            nat_t cap = table.size();
            nat_t raw_hash = key_hash_fct(k);
            nat_t h1 = raw_hash & (cap - 1);
            nat_t h2 =
                (2 * ((raw_hash / cap) % (cap / 2))) + 1; // secondary_hash(k)

            for (nat_t i = 0; i < cap; ++i)
            {
                nat_t idx = Probing::probe(h1, h2, i, cap);
                const Entry& e = table[idx];

                if (e.state == OAState::EMPTY)
                {
                    return cap;
                }

                if (e.state == OAState::OCCUPIED && cmp(*e.key, k))
                {
                    return idx;
                }
            }

            return cap;
        }

        template <typename K, class KeyHashFct>
        Key* search_by(const K& k, KeyHashFct&& key_hash_fct)
        {
            nat_t idx = find_slot_by(k, key_hash_fct);
            return idx == table.size() ? nullptr : &*table[idx].key;
        }

        template <typename K, class KeyHashFct>
        const Key* search_by(const K& k, KeyHashFct&& key_hash_fct) const
        {
            nat_t idx = find_slot_by(k, key_hash_fct);
            return idx == table.size() ? nullptr : &*table[idx].key;
        }

        Key* insert(const Key& k)
        {
            if (find_slot(k) != table.size())
            {
                return nullptr;
            }

            grow_if_needed();
            return raw_insert(k);
        }

        Key* insert(Key&& k)
        {
            if (find_slot(k) != table.size())
            {
                return nullptr;
            }

            grow_if_needed();
            return raw_insert(std::forward<Key>(k));
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
            nat_t idx = find_slot(k);
            return idx == table.size() ? nullptr : &*table[idx].key;
        }

        const Key* search(const Key& k) const
        {
            nat_t idx = find_slot(k);
            return idx == table.size() ? nullptr : &*table[idx].key;
        }

        Key* search_or_insert(const Key& k)
        {
            Key* found = search(k);

            if (found != nullptr)
            {
                return found;
            }

            grow_if_needed();
            return raw_insert(k);
        }

        Key* search_or_insert(Key&& k)
        {
            Key* found = search(k);

            if (found != nullptr)
            {
                return found;
            }

            grow_if_needed();
            return raw_insert(std::forward<Key>(k));
        }

        Key& find(const Key& k)
        {
            Key* result = search(k);

            if (result == nullptr)
            {
                throw std::domain_error("Key does not exist");
            }

            return *result;
        }

        const Key& find(const Key& k) const
        {
            const Key* result = search(k);

            if (result == nullptr)
            {
                throw std::domain_error("Key does not exist");
            }

            return *result;
        }

        bool remove(const Key& k)
        {
            nat_t idx = find_slot(k);

            if (idx == table.size())
            {
                return false;
            }

            table[idx].state = OAState::DELETED;
            table[idx].key.reset();
            --num_items;
            ++num_tombstones;
            return true;
        }

        class Iterator : public BidirectionalIterator<Iterator, Key>
        {
            friend class BasicIterator<Iterator, Key>;

            OpenAddressingHashTable* set_ptr;
            nat_t idx;

            void skip_forward()
            {
                while (idx < set_ptr->table.size() &&
                       set_ptr->table[idx].state != OAState::OCCUPIED)
                {
                    ++idx;
                }
            }

            void skip_backward()
            {
                while (idx > 0 &&
                       set_ptr->table[idx - 1].state != OAState::OCCUPIED)
                {
                    --idx;
                }
            }

        protected:
            nat_t get_location() const
            {
                return idx;
            }

        public:
            Iterator() : set_ptr(nullptr), idx(0)
            {
                // empty
            }

            Iterator(const OpenAddressingHashTable& h)
                : set_ptr(const_cast<OpenAddressingHashTable*>(&h)), idx(0)
            {
                skip_forward();
            }

            Iterator(const OpenAddressingHashTable& h, nat_t i)
                : set_ptr(const_cast<OpenAddressingHashTable*>(&h)), idx(i)
            {
                // empty
            }

            bool has_current() const
            {
                return idx < set_ptr->table.size();
            }

            Key& get_current()
            {
                if (!has_current())
                {
                    throw std::overflow_error("There is not current element");
                }

                return *set_ptr->table[idx].key;
            }

            const Key& get_current() const
            {
                if (!has_current())
                {
                    throw std::overflow_error("There is not current element");
                }

                return *set_ptr->table[idx].key;
            }

            void next()
            {
                if (!has_current())
                {
                    return;
                }

                ++idx;
                skip_forward();
            }

            void prev()
            {
                skip_backward();
            }

            void reset_first()
            {
                idx = 0;
                skip_forward();
            }

            void reset_last()
            {
                idx = set_ptr->table.size();
                skip_backward();

                if (idx > 0)
                {
                    --idx;
                }
            }
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
            return Iterator(*this, table.size());
        }

        Iterator end() const
        {
            return Iterator(*this, table.size());
        }
    };

    template <typename Key, class Cmp, class Probing>
    OpenAddressingHashTable<Key, Cmp, Probing>::OpenAddressingHashTable(
        const std::initializer_list<Key>& l)
        : OpenAddressingHashTable(next_pow2(l.size() * 2))
    {
        for (const auto& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp = std::equal_to<Key>>
    using LinearHashTable = OpenAddressingHashTable<Key, Cmp, LinearProbing>;

    template <typename Key, class Cmp = std::equal_to<Key>>
    using QuadraticHashTable =
        OpenAddressingHashTable<Key, Cmp, QuadraticProbing>;

    template <typename Key, class Cmp = std::equal_to<Key>>
    using DoubleHashingTable = OpenAddressingHashTable<Key, Cmp, DoubleHashing>;

} // end namespace Designar
