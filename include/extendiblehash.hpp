/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file extendiblehash.hpp
    @brief ExtendibleHashTable: the classic extendible hashing scheme
    (Fagin, Nievergelt, Pippenger, Strehlo, 1979) — a directory of
    `2^global_depth` pointers to fixed-capacity buckets, each bucket
    tagged with its own `local_depth <= global_depth` (multiple
    directory slots can point to the same bucket when its local depth
    is smaller than the directory's global depth). Inserting into a full
    bucket splits it (doubling the directory first if the bucket's
    local depth has caught up to the global depth) rather than chaining
    or open-addressing further — the "grows by splitting/doubling"
    property this is taught for, as opposed to openhash.hpp's
    OpenAddressingHashTable or chainedhash.hpp's
    SeparateChainingHashTable, which grow by a single wholesale rehash.
    @ingroup Hashing
*/

#pragma once

#include <stdexcept>
#include <utility>

#include <array.hpp>
#include <hash.hpp>

namespace Designar
{
    /** Deliberately small (4), not a performance choice — a bigger
        capacity would make splits rare enough that exercising the
        directory-doubling path in a test/demo would need many more
        keys than is convenient to read. */
    template <typename Key, class Cmp = std::equal_to<Key>>
    class ExtendibleHashTable
    {
        struct Bucket
        {
            nat_t local_depth;
            DynArray<Key> keys;

            explicit Bucket(nat_t ld) : local_depth(ld)
            {
                // empty
            }
        };

        static constexpr nat_t BUCKET_CAPACITY = 4;

        nat_t global_depth;
        DynArray<Bucket*> directory;

        /** Owns every distinct bucket exactly once — `directory` only
            ever holds *references* to buckets, and the same bucket is
            deliberately pointed to by more than one directory slot
            whenever its local depth is below the global depth, so
            deleting through `directory` would double-free. */
        DynArray<Bucket*> all_buckets;

        Cmp cmp;

        static nat_t mask(nat_t depth)
        {
            return depth == 0 ? nat_t(0) : ((nat_t(1) << depth) - 1);
        }

        Bucket* find_bucket(nat_t h) const
        {
            return directory[h & mask(global_depth)];
        }

        /** Copies `directory[i]` into a local before calling append() —
            `directory.append(directory[i])` would pass a reference into
            `directory` itself as append()'s argument, and append() can
            reallocate (moving/freeing the old backing storage) before
            reading that argument, turning the reference into a
            dangling one read right after (the same self-referential-
            append pitfall `std::vector::push_back(v[i])` has). */
        void double_directory()
        {
            nat_t old_size = directory.size();

            for (nat_t i = 0; i < old_size; ++i)
            {
                Bucket* b = directory[i];
                directory.append(b);
            }

            ++global_depth;
        }

        /** Splits an overflowing `bucket` (found at directory slot
            `idx`, only needed to know the hash prefix it was reached
            through — not strictly, since we recompute which directory
            slots point to it by scanning, but kept for clarity/future
            use). Doubles the directory first if `bucket` has no spare
            bit left to split on. */
        void split_bucket(Bucket* bucket)
        {
            if (bucket->local_depth == global_depth)
            {
                double_directory();
            }

            Bucket* new_bucket = new Bucket(bucket->local_depth + 1);
            all_buckets.append(new_bucket);
            bucket->local_depth = new_bucket->local_depth;

            nat_t new_bit_pos = bucket->local_depth - 1;

            for (nat_t i = 0; i < directory.size(); ++i)
            {
                if (directory[i] == bucket &&
                    ((i >> new_bit_pos) & nat_t(1)) == 1)
                {
                    directory[i] = new_bucket;
                }
            }

            DynArray<Key> old_keys = std::move(bucket->keys);
            bucket->keys.clear();

            for (Key& k : old_keys)
            {
                nat_t h = super_fast_hash(k);

                if (((h >> new_bit_pos) & nat_t(1)) == 1)
                {
                    new_bucket->keys.append(std::move(k));
                }
                else
                {
                    bucket->keys.append(std::move(k));
                }
            }
        }

        void swap(ExtendibleHashTable& t)
        {
            std::swap(global_depth, t.global_depth);
            directory.swap(t.directory);
            all_buckets.swap(t.all_buckets);
            std::swap(cmp, t.cmp);
        }

        void destroy_buckets()
        {
            for (Bucket* b : all_buckets)
            {
                delete b;
            }
        }

    public:
        using ItemType = Key;
        using KeyType = Key;
        using DataType = Key;
        using ValueType = Key;
        using SizeType = nat_t;
        using CmpType = Cmp;

        ExtendibleHashTable() : global_depth(0)
        {
            Bucket* b = new Bucket(0);
            all_buckets.append(b);
            directory.append(b);
        }

        /** Rebuilt by re-inserting every key from `other` rather than
            deep-copying the directory/bucket graph directly — much
            simpler to get right (no risk of miscounting shared
            directory-slot references) at the cost of re-hashing every
            key, which is the same trade a from-scratch rebuild always
            makes and is not a concern for a teaching-sized table. */
        ExtendibleHashTable(const ExtendibleHashTable& t) : global_depth(0)
        {
            Bucket* b = new Bucket(0);
            all_buckets.append(b);
            directory.append(b);

            for (Bucket* other_bucket : t.all_buckets)
            {
                for (const Key& k : other_bucket->keys)
                {
                    insert(k);
                }
            }
        }

        ExtendibleHashTable(ExtendibleHashTable&& t) : global_depth(0)
        {
            Bucket* b = new Bucket(0);
            all_buckets.append(b);
            directory.append(b);
            swap(t);
        }

        ~ExtendibleHashTable()
        {
            destroy_buckets();
        }

        ExtendibleHashTable& operator=(const ExtendibleHashTable& t)
        {
            if (this == &t)
            {
                return *this;
            }

            ExtendibleHashTable tmp(t);
            destroy_buckets();
            swap(tmp);
            return *this;
        }

        ExtendibleHashTable& operator=(ExtendibleHashTable&& t)
        {
            swap(t);
            return *this;
        }

        nat_t get_global_depth() const
        {
            return global_depth;
        }

        nat_t get_num_buckets() const
        {
            return all_buckets.size();
        }

        nat_t size() const
        {
            nat_t total = 0;

            for (Bucket* b : all_buckets)
            {
                total += b->keys.size();
            }

            return total;
        }

        bool is_empty() const
        {
            return size() == 0;
        }

        Key* search(const Key& key)
        {
            nat_t h = super_fast_hash(key);
            Bucket* b = find_bucket(h);

            for (nat_t i = 0; i < b->keys.size(); ++i)
            {
                if (cmp(b->keys[i], key))
                {
                    return &b->keys[i];
                }
            }

            return nullptr;
        }

        const Key* search(const Key& key) const
        {
            return const_cast<ExtendibleHashTable*>(this)->search(key);
        }

        bool has(const Key& key) const
        {
            return search(key) != nullptr;
        }

        /** Returns `nullptr` if `key` is already present (same
            duplicate-rejection convention as
            SeparateChainingHashTable::insert()/
            OpenAddressingHashTable::insert()), otherwise a pointer to
            the newly stored key — splitting (and, if needed, doubling
            the directory) as many times as it takes for the target
            bucket to have room, which terminates quickly in practice
            since `super_fast_hash` spreads keys over the full bit
            width. */
        Key* insert(const Key& key)
        {
            if (search(key) != nullptr)
            {
                return nullptr;
            }

            nat_t h = super_fast_hash(key);

            while (true)
            {
                Bucket* b = find_bucket(h);

                if (b->keys.size() < BUCKET_CAPACITY)
                {
                    b->keys.append(key);
                    return &b->keys[b->keys.size() - 1];
                }

                split_bucket(b);
            }
        }

        /** Removes `key` if present, returning whether it was found.
            Deliberately does not merge underflowing buckets back
            together or shrink the directory — the teaching point of
            extendible hashing is the split/double growth path; bucket
            merging is a standard, independent extension left out here
            the same way this library elsewhere favors the simpler
            correct approach over exhaustive symmetry (e.g.
            randomized_select's plain Lomuto partition over a fancier
            three-way one). */
        bool remove(const Key& key)
        {
            nat_t h = super_fast_hash(key);
            Bucket* b = find_bucket(h);

            for (nat_t i = 0; i < b->keys.size(); ++i)
            {
                if (cmp(b->keys[i], key))
                {
                    b->keys.remove_pos_closing_breach(i);
                    return true;
                }
            }

            return false;
        }

        void clear()
        {
            destroy_buckets();
            directory.clear();
            all_buckets.clear();
            global_depth = 0;

            Bucket* b = new Bucket(0);
            all_buckets.append(b);
            directory.append(b);
        }
    };

} // end namespace Designar
