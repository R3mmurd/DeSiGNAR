/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#pragma once

#include <type_traits>

namespace Designar
{

  template <typename To, typename FromHead, typename... FromTail>
  struct AllAreConvertible
  {
    static constexpr bool value =
        std::is_convertible<FromHead, To>::value and
        AllAreConvertible<To, FromTail...>::value;
  };

  template <typename To, typename From>
  struct AllAreConvertible<To, From>
  {
    static constexpr bool value = std::is_convertible<From, To>::value;
  };

  /** Helper base that owns a default-constructed `Cmp` instance.

      Several containers (GenArraySet, CmpWrapper, RankedTreap, the
      heaps, LHashTable) intentionally store their comparator as a
      reference (`Cmp &cmp`) rather than by value, so that a caller who
      wants a *stateful* comparator can pass one in by name and have the
      container observe (and be observed by) mutations to that same
      object for as long as both are alive. That design only works when
      the caller actually supplies such an lvalue; when no comparator is
      given at all, the container still needs *something* to bind `cmp`
      to. Naively doing `Cmp &&_cmp = Cmp()` and binding the reference
      member to that default argument is a dangling-reference bug: the
      temporary `Cmp()` is destroyed at the end of the constructor's
      full-expression, so `cmp` ends up referring to destroyed storage
      for the entire lifetime of the container.

      Deriving from `DefaultCmpHolder<Cmp>` *before* any base/member that
      needs to bind a reference to `default_cmp` guarantees, via the
      standard's base-then-member construction order (bases in
      declaration order, left to right), that `default_cmp` is already a
      live object by the time those later bases/members are constructed
      and can safely take its address. `default_cmp` is owned by (and
      shares the lifetime of) the container instance, so the reference
      it hands out never dangles. */
  template <class Cmp>
  struct DefaultCmpHolder
  {
    Cmp default_cmp;

    DefaultCmpHolder()
        : default_cmp()
    {
      // empty
    }
  };

} // end namespace Designar
