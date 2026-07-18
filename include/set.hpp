/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <array.hpp>
#include <tree.hpp>
#include <rankedtree.hpp>
#include <treap.hpp>
#include <avltree.hpp>
#include <rbtree.hpp>
#include <randomizedtree.hpp>
#include <splaytree.hpp>
#include <hash.hpp>
#include <openhash.hpp>
#include <sort.hpp>
#include <typetraits.hpp>

namespace Designar
{
    /** @see DefaultCmpHolder for why `cmp` is a reference and why a
        default-constructed instance must be owned (via
        DefaultCmpHolder<Cmp>) rather than bound to a temporary. */
    template <typename Key, class Cmp = std::less<Key>>
    struct NotEqualKey : private DefaultCmpHolder<Cmp>
    {
        Cmp& cmp;

        NotEqualKey(Cmp& c) : cmp(c)
        {
            // empty
        }

        NotEqualKey() : cmp(this->default_cmp)
        {
            // empty
        }

        bool operator()(const Key& a, const Key& b) const
        {
            return cmp(a, b) || cmp(b, a);
        }
    };

    template <typename Key, class Cmp = std::less<Key>>
    struct EqualKey : private DefaultCmpHolder<Cmp>
    {
        Cmp& cmp;

        EqualKey(Cmp& c) : cmp(c)
        {
            // empty
        }

        EqualKey() : cmp(this->default_cmp)
        {
            // empty
        }

        bool operator()(const Key& a, const Key& b) const
        {
            return !NotEqualKey<Key, Cmp>(cmp)(a, b);
        }
    };

    template <typename Key, class Cmp>
    class SortedArraySetOp
    {
        DynArray<Key>& array;
        Cmp& cmp;

    protected:
        NotEqualKey<Key, Cmp> not_equal_key;
        EqualKey<Key, Cmp> equal_key;

        int_t search(const Key& k, int_t l, int_t r) const
        {
            return binary_search(array, k, l, r, cmp);
        }

    public:
        SortedArraySetOp(DynArray<Key>& a, Cmp& c)
            : array(a), cmp(c), not_equal_key(cmp), equal_key(cmp)
        {
            // empty
        }

        bool is_sorted() const
        {
            return true;
        }

        Key* insert(const Key& item)
        {
            int_t pos = search(item, 0, array.size() - 1);

            if (pos == array.size())
            {
                return &array.append(item);
            }

            if (equal_key(item, array.at(pos)))
            {
                return nullptr;
            }

            return &array.insert(pos, item);
        }

        Key* insert(Key&& item)
        {
            int_t pos = search(item, 0, array.size() - 1);

            if (pos == array.size())
            {
                return &array.append(std::forward<Key>(item));
            }

            if (equal_key(item, array.at(pos)))
            {
                return nullptr;
            }

            return &array.insert(pos, std::forward<Key>(item));
        }

        Key* insert_dup(const Key& item)
        {
            int_t pos = search(item, 0, array.size() - 1);

            if (pos == array.size())
            {
                return &array.append(item);
            }

            return &array.insert(pos, item);
        }

        Key* insert_dup(Key&& item)
        {
            int_t pos = search(item, 0, array.size() - 1);

            if (pos == array.size())
            {
                return &array.append(std::forward<Key>(item));
            }

            return &array.insert(pos, std::forward<Key>(item));
        }

        Key* search_or_insert(const Key& item)
        {
            int_t pos = search(item, 0, array.size() - 1);

            if (pos == array.size())
            {
                return &array.append(item);
            }

            if (equal_key(item, array.at(pos)))
            {
                return &array[pos];
            }

            return &array.insert(pos, item);
        }

        Key* search_or_insert(Key&& item)
        {
            int_t pos = search(item, 0, array.size() - 1);

            if (pos == array.size())
            {
                return &array.append(std::forward<Key>(item));
            }

            if (equal_key(item, array.at(pos)))
            {
                return &array[pos];
            }

            return &array.insert(pos, std::forward<Key>(item));
        }

        Key remove_pos(nat_t pos)
        {
            return array.remove_pos_closing_breach(pos);
        }

        const Key& select(nat_t i)
        {
            return array.at(i);
        }

        nat_t position(const Key& item)
        {
            return search(item, 0, array.size() - 1);
        }

        /** Heterogeneous lookup: `k` (of any type `K`, not necessarily
            `Key` itself) is compared directly against each stored `Key`
            via `cmp` — `cmp` need only overload `operator()` for the
            `(K, Key)`/`(Key, K)` combinations actually used here, it does
            not need `K` and `Key` to be the same type. This is what lets
            GenMap (map.hpp) look an entry up by a bare `Key` without ever
            constructing a full `MapKey<Key, Value>` probe pair, which
            used to require `Value` to be default-constructible purely to
            have *something* in the probe's unused second field. */
        template <typename K>
        Key* search_by(const K& k)
        {
            int_t pos = binary_search_by(array, k, 0, array.size() - 1, cmp);

            if (pos == array.size())
            {
                return nullptr;
            }

            const Key& candidate = array.at(pos);

            if (cmp(k, candidate) || cmp(candidate, k))
            {
                return nullptr;
            }

            return &array[pos];
        }
    };

    template <typename Key, class Cmp>
    class UnsortedArraySetOp
    {
        DynArray<Key>& array;
        Cmp& cmp;

    protected:
        NotEqualKey<Key, Cmp> not_equal_key;
        EqualKey<Key, Cmp> equal_key;

    protected:
        int_t search(const Key& k, int_t l, int_t r) const
        {
            return sequential_search(array, k, l, r, equal_key);
        }

    public:
        UnsortedArraySetOp(DynArray<Key>& a, Cmp& c)
            : array(a), cmp(c), not_equal_key(cmp), equal_key(cmp)
        {
            // empty
        }

        bool is_sorted() const
        {
            return array.template is_sorted<Cmp>(cmp);
        }

        Key* insert(const Key& item)
        {
            int_t pos = search(item, 0, int_t(array.size()) - 1);

            if (pos < array.size())
            {
                return nullptr;
            }

            return &array.append(item);
        }

        Key* insert(Key&& item)
        {
            int_t pos = search(item, 0, int_t(array.size()) - 1);

            if (pos < array.size())
            {
                return nullptr;
            }

            return &array.append(std::forward<Key>(item));
        }

        Key* insert_dup(const Key& item)
        {
            return &array.append(item);
        }

        Key* insert_dup(Key&& item)
        {
            return &array.append(std::forward<Key>(item));
        }

        Key* search_or_insert(const Key& item)
        {
            int_t pos = search(item, 0, int_t(array.size()) - 1);

            if (pos < array.size())
            {
                return &array[pos];
            }

            return &array.append(item);
        }

        Key* search_or_insert(Key&& item)
        {
            int_t pos = search(item, 0, int_t(array.size()) - 1);

            if (pos < array.size())
            {
                return &array[pos];
            }

            return &array.append(std::forward<Key>(item));
        }

        Key remove_pos(nat_t pos)
        {
            return array.remove_pos(pos);
        }

        const Key& select(nat_t i)
        {
            quicksort(array, 0, array.size() - 1, cmp);
            return array.at(i);
        }

        nat_t position(const Key& item)
        {
            quicksort(array, 0, array.size() - 1, cmp);
            return search(item, 0, array.size() - 1);
        }

        /** @see SortedArraySetOp::search_by — same heterogeneous-lookup
            purpose, via a linear scan (matching this class's own
            unsorted search()) rather than binary search. */
        template <typename K>
        Key* search_by(const K& k)
        {
            for (nat_t i = 0; i < array.size(); ++i)
            {
                if (!cmp(k, array[i]) && !cmp(array[i], k))
                {
                    return &array[i];
                }
            }

            return nullptr;
        }
    };

    /** @see DefaultCmpHolder for why this class privately derives from
        `DefaultCmpHolder<Cmp>` (as its first base, so `default_cmp` is
        already alive when `ArraySetOp`'s constructor runs) instead of
        binding `cmp` to a default-constructed temporary. */
    template <typename Key, class Cmp, class ArraySetOp>
    class GenArraySet
        : private DefaultCmpHolder<Cmp>,
          public ArraySetOp,
          public ContainerAlgorithms<GenArraySet<Key, Cmp, ArraySetOp>, Key>,
          public SetAlgorithms<GenArraySet<Key, Cmp, ArraySetOp>, Key>
    {
    public:
        using ItemType = Key;
        using KeyType = Key;
        using DataType = Key;
        using ValueType = Key;
        using SizeType = nat_t;

        DynArray<Key> array;
        Cmp& cmp;

    private:
        /** A copy must preserve *ownership* semantics, not just reference
            identity: if `a` uses its own owned `default_cmp` (no external
            comparator was ever supplied to it), the copy must own an
            independent comparator too — aliasing `a.default_cmp` would leave
            the copy's `cmp` dangling the moment `a` is destroyed. If `a`
            instead references a caller-supplied external comparator, the
            copy intentionally shares that same external object, matching
            the reference-based design (see DefaultCmpHolder). */
        static Cmp& cmp_for_copy(GenArraySet& self, const GenArraySet& a)
        {
            if (&a.cmp == &a.default_cmp)
            {
                self.default_cmp = a.default_cmp;
                return self.default_cmp;
            }

            return a.cmp;
        }

    public:
        GenArraySet(nat_t cap, Cmp& _cmp)
            : ArraySetOp(array, _cmp), array(cap), cmp(_cmp)
        {
            // empty
        }

        /** Accepts an explicit comparator by value (or none, via the default
            argument) and copies its value into the owned `default_cmp`
            slot, binding `cmp` to that owned storage rather than to the
            constructor parameter itself. `_cmp` is a temporary (or the
            materialized `Cmp()` default argument) that is destroyed at the
            end of this constructor call; a reference member bound directly
            to it would dangle immediately. This is also what makes it safe
            for GenMap-derived classes (ArrayMap, TreeMap, HashMap) to build
            a `CmpWrapper` adapter as a temporary and hand it here — a real,
            load-bearing use of this overload, not just sugar for "no
            comparator given".

            A copy-assignment (`default_cmp = _cmp`) is used deliberately
            instead of a move: when `_cmp` is itself a CmpWrapper whose own
            `cmp` references someone else's long-lived comparator (exactly
            the case when ArrayMap/TreeMap/HashMap forward a
            reference-holding CmpWrapper here), CmpWrapper's move-assignment
            swaps the *values* the two wrappers' references point to — which
            would reach through and stomp the caller's external comparator
            with a freshly-default-constructed one as a side effect of
            merely constructing this set. Copy-assignment only ever writes
            into `default_cmp`'s own storage, leaving whatever `_cmp`
            referenced untouched. Comparators are expected to be small
            (usually stateless), so the extra copy is not a real cost. */
        GenArraySet(Cmp&& _cmp = Cmp())
            : ArraySetOp(array, this->default_cmp),
              array(),
              cmp(this->default_cmp)
        {
            this->default_cmp = _cmp;
        }

        GenArraySet(nat_t cap, Cmp&& _cmp = Cmp())
            : ArraySetOp(array, this->default_cmp),
              array(cap),
              cmp(this->default_cmp)
        {
            this->default_cmp = _cmp;
        }

        GenArraySet(const GenArraySet& a)
            : ArraySetOp(array, cmp_for_copy(*this, a)),
              array(a.array),
              cmp(cmp_for_copy(*this, a))
        {
            // empty
        }

        GenArraySet(GenArraySet&& a) : GenArraySet()
        {
            swap(a);
        }

        GenArraySet(const std::initializer_list<Key>&);

        GenArraySet& operator=(const GenArraySet& a)
        {
            if (&a == this)
            {
                return *this;
            }

            array = a.array;
            cmp = a.cmp;

            return *this;
        }

        GenArraySet& operator=(GenArraySet&& a)
        {
            swap(a);
            return *this;
        }

        void swap(GenArraySet& a)
        {
            array.swap(a.array);
            std::swap(cmp, a.cmp);
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
            return array.is_empty();
        }

        nat_t size() const
        {
            return array.size();
        }

        void clear()
        {
            array.clear();
        }

        Key* append(const Key& k)
        {
            return ArraySetOp::insert(k);
        }

        Key* append(Key&& k)
        {
            return ArraySetOp::insert(std::forward<Key>(k));
        }

        Key* append_dup(const Key& k)
        {
            return ArraySetOp::insert_dup(k);
        }

        Key* append_dup(Key&& k)
        {
            return ArraySetOp::insert_dup(std::forward<Key>(k));
        }

        Key* search(const Key& item)
        {
            int_t pos = ArraySetOp::search(item, 0, int_t(this->size()) - 1);

            if (pos >= this->size() ||
                ArraySetOp::not_equal_key(item, array.at(pos)))
            {
                return nullptr;
            }

            return &array.at(pos);
        }

        const Key* search(const Key& item) const
        {
            int_t pos = ArraySetOp::search(item, 0, int_t(this->size()) - 1);

            if (pos >= this->size() ||
                ArraySetOp::not_equal_key(item, array.at(pos)))
            {
                return nullptr;
            }

            return &array.at(pos);
        }

        Key& find(const Key& item)
        {
            Key* result = search(item);
            if (result == nullptr)
            {
                throw std::domain_error("Item does not exist");
            }
            return *result;
        }

        const Key& find(const Key& item) const
        {
            const Key* result = search(item);
            if (result == nullptr)
            {
                throw std::domain_error("Item does not exist");
            }
            return *result;
        }

        bool remove(const Key& item)
        {
            int_t pos = ArraySetOp::search(item, 0, int_t(this->size()) - 1);

            if (pos >= this->size() ||
                ArraySetOp::not_equal_key(array.at(pos), item))
            {
                return false;
            }

            ArraySetOp::remove_pos(pos);
            return true;
        }

        Key& operator[](nat_t i)
        {
            return array[i];
        }

        const Key& operator[](nat_t i) const
        {
            return array[i];
        }

        class Iterator : public DynArray<Key>::Iterator
        {
            using Base = typename DynArray<Key>::Iterator;

        public:
            Iterator() : Base()
            {
                // empty
            }

            Iterator(const GenArraySet& a) : Base(a.array)
            {
                // empty
            }

            Iterator(const GenArraySet& a, nat_t c) : Base(a.array, c)
            {
                // empty
            }

            Iterator(const Iterator& itor) : Base(itor)
            {
                // empty
            }

            Iterator(Iterator&& itor) : Iterator()
            {
                Base::swap(itor);
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
            return Iterator(*this, size());
        }

        Iterator end() const
        {
            return Iterator(*this, size());
        }
    };

    template <typename Key, class Cmp, class ArraySetOp>
    GenArraySet<Key, Cmp, ArraySetOp>::GenArraySet(
        const std::initializer_list<Key>& l)
        : GenArraySet(l.size())
    {
        for (const Key& item : l)
        {
            append(item);
        }
    }

    template <typename Key, class Cmp = std::less<Key>>
    class UnsortedArraySet
        : public GenArraySet<Key, Cmp, UnsortedArraySetOp<Key, Cmp>>
    {
        using Base = GenArraySet<Key, Cmp, UnsortedArraySetOp<Key, Cmp>>;
        using Base::Base;
    };

    template <typename Key, class Cmp = std::less<Key>>
    class SortedArraySet
        : public GenArraySet<Key, Cmp, SortedArraySetOp<Key, Cmp>>
    {
        using Base = GenArraySet<Key, Cmp, SortedArraySetOp<Key, Cmp>>;
        using Base::Base;
    };

    template <typename Key, class Cmp = std::less<Key>,
              template <typename, class> class ArrayType = UnsortedArraySet>
    class ArraySet : public ArrayType<Key, Cmp>
    {
        using Base = ArrayType<Key, Cmp>;
        using Base::Base;

    public:
        using ContainerType = ArrayType<Key, Cmp>;
    };

    template <typename Key, class Cmp = std::less<Key>,
              template <typename, class> class TreeType = RankedTreap>
    class TreeSet : public TreeType<Key, Cmp>
    {
        using Base = TreeType<Key, Cmp>;
        using Base::Base;

    public:
        using ContainerType = TreeType<Key, Cmp>;
    };

    template <typename Key, class Cmp = std::equal_to<Key>,
              template <typename, class> class HashTableType = LHashTable>
    class HashSet : public HashTableType<Key, Cmp>
    {
        using Base = HashTableType<Key, Cmp>;
        using Base::Base;

    public:
        using ContainerType = HashTableType<Key, Cmp>;
    };

} // namespace Designar
