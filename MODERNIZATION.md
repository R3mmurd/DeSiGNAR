# Possible modernizations with stable C++ standards

DeSiGNAR currently targets C++17. This is a survey of what adopting a newer
*stable* standard could buy the library — C++20 first (shipped in GCC,
Clang, and MSVC for several years now, genuinely stable), then a shorter
C++23 section (newer, compiler support still catching up, but worth
knowing about). None of this is implemented — it's a menu to choose from,
each with a rough cost/benefit note.

## C++20

### Concepts, instead of implicit template requirements

Right now, a type parameter like `Cmp`, `Distance`, or the `GT` graph-type
parameter used throughout `graphalgorithms.hpp` is constrained only by
convention and a compile error deep in a template instantiation if you get
it wrong. A `concept` turns that into a single, readable, checked
declaration:

```cpp
template <class GT>
concept GraphType = requires(GT g, Node<GT>* n) {
  { g.get_num_nodes() } -> std::same_as<nat_t>;
  { g.for_each_node([](Node<GT>*) {}) };
  // ...
};

template <GraphType GT, class Distance = DefaultDistance<GT>>
class Dijkstra { ... };
```

Passing the wrong type now fails at the call site with "GT does not
satisfy GraphType: missing get_num_nodes()" instead of a page of nested
template-instantiation errors from deep inside `Dijkstra`. This is also
the most *teachable* item on this list — concepts make the implicit
contract every template in this library already has explicit and
readable, which fits a teaching-oriented library especially well.

### `operator<=>` for the comparison-heavy iterator/node code

`RandomAccessIterator` (iterator.hpp) hand-writes `<`, `<=`, `>`, `>=` from
a single `get_location()` comparison; several tree/heap node comparisons
do the same for keys. C++20's defaulted three-way comparison collapses
that boilerplate:

```cpp
auto operator<=>(const Iterator& it) const
{
  return const_me().get_location() <=> it.get_location();
}
```

One function generates all four (plus `==`/`!=`) instead of four
hand-written ones that have to stay in sync.

### Ranges

`for_each_node()`/`for_each_arc()` (graph.hpp) and the various
`for_each_it()` free functions (italgorithms.hpp) predate `<ranges>` and
use a callback-passing style instead. Once every container's iterators
satisfy `std::ranges::forward_range` (they mostly already do, given the
work done to make them `std::` algorithm-compatible), the containers
become directly usable with range adaptors:

```cpp
for (auto& node : graph.nodes() | std::views::filter(is_active))
  ...
```

This is additive, not a replacement — the existing callback-based
`for_each_*` methods stay (they're what makes DFS/BFS-style traversal
early-exit-able, which a plain range can't express as naturally), but
exposing a `std::ranges`-compatible view alongside them would let this
library's containers compose with the standard range adaptor pipeline
directly, without a caller having to write `std::begin(x)`/`std::end(x)`
adapters by hand.

### `std::jthread` + `std::stop_token` for the concurrency containers

`ThreadPool` (threadpool.hpp) currently shuts down via a manual
"poison pill" — one sentinel task pushed per worker so each one wakes up,
recognizes the empty `std::function`, and exits, then the destructor
`join()`s every `std::thread` by hand. `std::jthread` was designed for
exactly this: it joins automatically in its destructor and carries a
`std::stop_token` a running task can poll to know it should wind down
cooperatively, without needing the sentinel-task trick at all:

```cpp
std::vector<std::jthread> workers;
// ...
workers.emplace_back([this](std::stop_token stoken)
{
  while (!stoken.stop_requested())
  {
    // still need ConcurrentQueue::get() to itself become
    // interruptible on a stop request to fully benefit
  }
});
```

The one wrinkle: `ConcurrentQueue::get()` blocks unconditionally today, so
getting the full benefit also means teaching it to wake up on a stop
request (e.g. `condition_variable_any::wait(lock, stoken, pred)`), not
just swapping `std::thread` for `std::jthread`.

### `std::span` for read-only bulk access

Several places (e.g. `FixedArray`/`DynArray`'s bulk constructors, or a
hypothetical "give me a contiguous view of this array's storage without
copying" use case) currently either copy into a `DynArray` or require the
caller to already have one. `std::span<const T>` is the standard
non-owning contiguous-range view and would let e.g.
`DynArray(std::span<const T>)` accept a plain C array, a
`std::vector`, or another `DynArray`'s data without an intermediate copy
or an initializer-list-only constructor.

### `constexpr`/`consteval` for the pure numeric helpers

`mix64()` (bloomfilter.hpp), the hash-combining helpers in hash.hpp, and
`math.hpp`'s numeric-comparison helpers are all pure functions with no
I/O or allocation — good candidates for `constexpr`, letting a caller
that already knows its inputs at compile time (e.g. a fixed `BloomFilter`
sizing computed from a compile-time constant `expected_items`) get that
computation done at compile time instead of at every program run.

### `std::format` for the graph I/O helpers

`OutputGraph`/`DotGraph` (graphutilities.hpp) build output via chained
`operator<<` on `std::ostream`. `std::format` (and `std::print` once
broadly available) reads more directly for the "template string with
placeholders" shape most of this output actually has, and — unlike
`iostream` manipulator state (`std::hex`, `std::setw`, etc.) — doesn't
leak formatting state across calls that forget to reset it.

## C++23 (newer — check your toolchain's support before relying on these)

### Deducing `this` — could remove the CRTP `me()`/`const_me()` boilerplate entirely

This is the big one for this codebase specifically. Every iterator base
class (`BasicIterator`, `ForwardIterator`, ...) uses the classic CRTP
"cast `this` down to the derived type" pattern:

```cpp
protected:
  Iterator& me() { return *static_cast<Iterator*>(this); }
  const Iterator& const_me() const { return *static_cast<const Iterator*>(this); }
```

C++23's explicit object parameter ("deducing this") replaces the whole
CRTP-base-class-plus-cast idiom with an ordinary template parameter on the
member function itself:

```cpp
template <class Self>
auto&& get_curr(this Self&& self)
{
  return self.get_current();
}
```

No `static_cast`, no separate const/non-const overload pair, no CRTP
template parameter threaded through every base class in the hierarchy —
the exact three-level CRTP-chain fragility this project's own `std::`
compatibility work just had to fix by hand in `TArrayIterator` (adding an
explicit `Derived` parameter so `operator+`/`operator-` return the true
leaf type) would not have been possible to get wrong in the first place,
since there would be no intermediate CRTP type to accidentally return.
This would be a genuinely invasive rewrite of `iterator.hpp` and every
`*Node`/`*Iterator` class built on it — worth doing deliberately as its
own project, not as an incidental change.

### `std::expected<T, E>` for the exception-vs-error-code question

Several places in this codebase throw (`std::domain_error`,
`std::overflow_error`, `std::length_error`) for conditions a caller might
want to check without paying exception-handling overhead or writing a
`try`/`catch` (e.g. `ClosestPair::compute()` on fewer than two points,
`BloomFilter`'s constructor on a zero size). `std::expected<T, E>` gives
those call sites a way to return "a `T`, or a documented reason it
couldn't produce one" without exceptions being the only option — useful
particularly for the algorithm classes (this library's own
`FloydWarshall`, `EdmondsKarp`, etc.) where a caller doing a hot-path
computation might reasonably want to check-not-throw.

### Multidimensional `operator[]`

`MultiDimArray::Slice::operator()` (array.hpp) exists specifically because
C++ before C++23 only allows `operator[]` to take exactly one argument —
`operator()` was the workaround for "I want `matrix[i, j]`-style access."
C++23 allows `operator[](i, j)` directly:

```cpp
T& operator[](nat_t i, nat_t j) { return data[i * cols + j]; }
```

This is a straightforward drop-in once the minimum supported compiler
version is raised — `operator()` can stay as a deprecated alias for
compatibility, or be dropped in a major version bump.

### `std::flat_map`/`std::flat_set`

Contiguous, sorted-vector-backed associative containers — a genuinely
different trade-off than every associative structure this library already
has (hash tables, and the six balanced-tree flavors), optimized for
mostly-read-after-built workloads where cache locality beats O(log n)
tree-pointer-chasing. Worth a mention as a *teaching* opportunity as much
as an implementation one: it's a good example of "same interface,
different big-O/cache trade-off" to place alongside the tree/hash variants
already in this library.
