/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file concurrentmap.hpp
    @brief ConcurrentMap / ConcurrentSet: mutex-guarded facades around
    HashMap/HashSet, following the same design ConcurrentQueue
    (queue.hpp) already established for this library's single-threaded
    containers.
    @ingroup Concurrency
*/

#pragma once

#include <map.hpp>
#include <mutex>
#include <optional>

namespace Designar
{
  /** @see ConcurrentQueue::put() for why every operation here returns a
      *value* (a copy, a bool, an `optional`) rather than a reference or
      pointer into the map's internal storage: any such reference would
      only be valid while the lock is held, and the lock is always
      released before the caller gets control back. get() in particular
      returns `std::optional<Value>` — a real copy of the value, taken
      while the lock is held — rather than `Value*`, for exactly that
      reason. */
  template <typename Key, typename Value, class Cmp = std::equal_to<Key>>
  class ConcurrentMap
  {
    mutable std::mutex mtx;
    HashMap<Key, Value, Cmp> map;

  public:
    ConcurrentMap() = default;

    ConcurrentMap(const ConcurrentMap&) = delete;
    ConcurrentMap& operator=(const ConcurrentMap&) = delete;

    bool insert(const Key& k, const Value& v)
    {
      std::lock_guard<std::mutex> lck(mtx);
      return map.insert(k, v) != nullptr;
    }

    /** Insert-or-update: unlike insert(), overwrites `k`'s value if it
        is already present. */
    void put(const Key& k, const Value& v)
    {
      std::lock_guard<std::mutex> lck(mtx);
      map[k] = v;
    }

    bool contains(const Key& k) const
    {
      std::lock_guard<std::mutex> lck(mtx);
      return map.search(k) != nullptr;
    }

    std::optional<Value> get(const Key& k) const
    {
      std::lock_guard<std::mutex> lck(mtx);
      const Value* v = map.search(k);

      if (v == nullptr)
      {
        return std::nullopt;
      }

      return *v;
    }

    bool remove(const Key& k)
    {
      std::lock_guard<std::mutex> lck(mtx);
      return map.remove(k);
    }

    nat_t size() const
    {
      std::lock_guard<std::mutex> lck(mtx);
      return map.size();
    }

    bool is_empty() const
    {
      std::lock_guard<std::mutex> lck(mtx);
      return map.is_empty();
    }

    void clear()
    {
      std::lock_guard<std::mutex> lck(mtx);
      map.clear();
    }

    /** Runs `fn(underlying_map)` while holding the lock — the escape
        hatch for atomic read-modify-write sequences (e.g.
        "increment-if-present") that no single method above expresses,
        without exposing the underlying HashMap (and its non-thread-safe
        references/iterators) outside of a locked context. `fn` must not
        itself try to lock this same ConcurrentMap (directly or by
        calling back into one of the methods above) — that would
        deadlock, `std::mutex` not being recursive. */
    template <class Fn>
    decltype(auto) with_lock(Fn&& fn)
    {
      std::lock_guard<std::mutex> lck(mtx);
      return fn(map);
    }
  };

  /** @see ConcurrentMap above — same design, wrapping HashSet instead of
      HashMap. */
  template <typename Key, class Cmp = std::equal_to<Key>>
  class ConcurrentSet
  {
    mutable std::mutex mtx;
    HashSet<Key, Cmp> set;

  public:
    ConcurrentSet() = default;

    ConcurrentSet(const ConcurrentSet&) = delete;
    ConcurrentSet& operator=(const ConcurrentSet&) = delete;

    bool insert(const Key& k)
    {
      std::lock_guard<std::mutex> lck(mtx);
      return set.insert(k) != nullptr;
    }

    bool contains(const Key& k) const
    {
      std::lock_guard<std::mutex> lck(mtx);
      return set.search(k) != nullptr;
    }

    bool remove(const Key& k)
    {
      std::lock_guard<std::mutex> lck(mtx);
      return set.remove(k);
    }

    nat_t size() const
    {
      std::lock_guard<std::mutex> lck(mtx);
      return set.size();
    }

    bool is_empty() const
    {
      std::lock_guard<std::mutex> lck(mtx);
      return set.is_empty();
    }

    void clear()
    {
      std::lock_guard<std::mutex> lck(mtx);
      set.clear();
    }

    /** @see ConcurrentMap::with_lock() — same escape hatch, same
        re-entrancy caveat. */
    template <class Fn>
    decltype(auto) with_lock(Fn&& fn)
    {
      std::lock_guard<std::mutex> lck(mtx);
      return fn(set);
    }
  };

} // end namespace Designar
