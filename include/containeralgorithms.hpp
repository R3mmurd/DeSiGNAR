/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file containeralgorithms.hpp
    @brief CRTP mixin that gives any begin()/end()-providing container the
    library's generic iterator-range algorithms for free.
    @ingroup Algorithms
*/

#pragma once

#include <italgorithms.hpp>

namespace Designar
{
    /** A CRTP mixin: a container inherits from
        `ContainerAlgorithms<ContainerType, T>` (passing itself as
        `ContainerType`) to gain a whole suite of generic algorithms
        (traversal, search, transformation, comparison, etc.) without
        having to implement any of them itself. The private `me()` /
        `const_me()` helpers downcast `this` back to `ContainerType*` /
        `const ContainerType*`, which is what lets every member function
        below call `me().begin()` / `me().end()` (or the const
        equivalents) to obtain the container's own iterators and then
        simply forward them to the corresponding free `..._it` function
        in italgorithms.hpp.

        Because the real work lives in those free functions, this class
        contributes no per-container code of its own — DynArray, SLList,
        DLList, the various tree types, etc. all derive from it and
        thereby share one single implementation of `for_each`, `map`,
        `filter`, `find`, `exists`, `equal`, and the rest, instead of
        every container reimplementing them. */
    template <class ContainerType, typename T>
    class ContainerAlgorithms
    {
        ContainerType& me()
        {
            return *static_cast<ContainerType*>(this);
        }

        const ContainerType& const_me() const
        {
            return *static_cast<const ContainerType*>(this);
        }

    public:
        /** @brief Positional access to elements by index. */
        T* nth_ptr(nat_t i)
        {
            return nth_ptr_it<T>(me().begin(), me().end(), i);
        }

        T& nth(nat_t i)
        {
            return nth_it<T>(me().begin(), me().end(), i);
        }

        const T& nth(nat_t i) const
        {
            return nth_it(const_me().begin(), const_me().end(), i);
        }

        /** @brief Traversal: apply an operation to every element,
            optionally paired with its position. */
        template <class Op>
        void for_each(Op& op) const
        {
            for_each_it(const_me().begin(), const_me().end(), op);
        }

        template <class Op>
        void for_each(Op&& op = Op()) const
        {
            for_each_it(const_me().begin(), const_me().end(),
                        std::forward<Op>(op));
        }

        template <class Op>
        void enum_for_each(Op& op) const
        {
            enum_for_each_it(const_me().begin(), const_me().end(), op);
        }

        template <class Op>
        void enum_for_each(Op&& op = Op()) const
        {
            enum_for_each_it(const_me().begin(), const_me().end(),
                             std::forward<Op>(op));
        }

        /** @brief Transform: build a new container by selecting
            (filter), converting (map), or both (map_if) the elements,
            or by reducing them to a single value (fold). */
        template <class ContainerRet, class Pred>
        ContainerRet filter(Pred& pred) const
        {
            return filter_it<ContainerRet>(const_me().begin(), const_me().end(),
                                           pred);
        }

        template <class ContainerRet = SLList<T>, class Pred>
        ContainerRet filter(Pred&& pred = Pred()) const
        {
            return filter_it<ContainerRet>(const_me().begin(), const_me().end(),
                                           std::forward<Pred>(pred));
        }

        template <typename RetT = T, class ContainerRet = SLList<RetT>,
                  class Op>
        ContainerRet map(Op& op) const
        {
            return map_it<ContainerRet>(const_me().begin(), const_me().end(),
                                        op);
        }

        template <class RetT = T, class ContainerRet = SLList<RetT>, class Op>
        ContainerRet map(Op&& op = Op()) const
        {
            return map_it<ContainerRet>(const_me().begin(), const_me().end(),
                                        std::forward<Op>(op));
        }

        template <class RetT = T, class ContainerRet = SLList<RetT>, class Op,
                  class Pred>
        ContainerRet map_if(Op& op, Pred& pred) const
        {
            return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
                                           op, pred);
        }

        template <class RetT = T, class ContainerRet = SLList<RetT>, class Op,
                  class Pred>
        ContainerRet map_if(Op& op, Pred&& pred = Pred()) const
        {
            return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
                                           op, std::forward<Pred>(pred));
        }

        template <class RetT = T, class ContainerRet = SLList<RetT>, class Op,
                  class Pred>
        ContainerRet map_if(Op&& op, Pred& pred) const
        {
            return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
                                           std::forward<Op>(op), pred);
        }

        template <typename RetT = T, class ContainerRet = SLList<RetT>,
                  class Op, class Pred>
        ContainerRet map_if(Op&& op = Op(), Pred&& pred = Pred()) const
        {
            return map_if_it<ContainerRet>(const_me().begin(), const_me().end(),
                                           std::forward<Op>(op),
                                           std::forward<Pred>(pred));
        }

        template <typename RetT = T, class Op>
        RetT fold(const RetT& init_val, Op& op) const
        {
            return fold_it<RetT>(const_me().begin(), const_me().end(), init_val,
                                 op);
        }

        template <typename RetT = T, class Op>
        RetT fold(const RetT& init_val, Op&& op = Op()) const
        {
            return fold_it<RetT>(const_me().begin(), const_me().end(), init_val,
                                 std::forward<Op>(op));
        }

        template <typename RetT = T, class Op>
        RetT fold(RetT&& init_val, Op& op) const
        {
            return fold_it<RetT>(const_me().begin(), const_me().end(),
                                 std::forward<RetT>(init_val), op);
        }

        template <typename RetT = T, class Op>
        RetT fold(RetT&& init_val, Op&& op = Op()) const
        {
            return fold_it<RetT>(const_me().begin(), const_me().end(),
                                 std::forward<RetT>(init_val),
                                 std::forward<Op>(op));
        }

        /** @brief Search/predicate: check whether all/some/none of the
            elements satisfy a predicate, find one satisfying it, or
            remove the ones that do. */
        template <class Pred>
        bool all(Pred& pred) const
        {
            return all_it(const_me().begin(), const_me().end(), pred);
        }

        template <class Pred>
        bool all(Pred&& pred = Pred()) const
        {
            return all_it(const_me().begin(), const_me().end(),
                          std::forward<Pred>(pred));
        }

        template <class Pred>
        bool exists(Pred& pred) const
        {
            return exists_it(const_me().begin(), const_me().end(), pred);
        }

        template <class Pred>
        bool exists(Pred&& pred = Pred()) const
        {
            return exists_it(const_me().begin(), const_me().end(),
                             std::forward<Pred>(pred));
        }

        template <class Pred>
        bool none(Pred& pred) const
        {
            return none_it(const_me().begin(), const_me().end(), pred);
        }

        template <class Pred>
        bool none(Pred&& pred = Pred()) const
        {
            return none_it(const_me().begin(), const_me().end(),
                           std::forward<Pred>(pred));
        }

        template <class Pred>
        T* search_ptr(Pred& pred) const
        {
            return search_ptr_it<T>(const_me().begin(), const_me().end(), pred);
        }

        template <class Pred>
        T* search_ptr(Pred&& pred = Pred()) const
        {
            return search_ptr_it<T>(const_me().begin(), const_me().end(),
                                    std::forward<Pred>(pred));
        }

        template <class Pred>
        bool remove_first_if(Pred& pred)
        {
            return remove_first_if_it(me().begin(), me().end(), pred);
        }

        template <class Pred>
        bool remove_first_if(Pred&& pred = Pred())
        {
            return remove_first_if_it(me().begin(), me().end(),
                                      std::forward<Pred>(pred));
        }

        template <class Pred>
        void remove_if(Pred& pred)
        {
            remove_if_it(me().begin(), me().end(), pred);
        }

        template <class Pred>
        void remove_if(Pred&& pred = Pred())
        {
            remove_if_it(me().begin(), me().end(), std::forward<Pred>(pred));
        }

        /** @brief Comparison and pairing: check element-wise equality
            with another container, whether this container is sorted, or
            zip its elements together with another container's. */
        template <class ContainerType2 = ContainerType, class Eq>
        bool equal(const ContainerType2& c, Eq& eq) const
        {
            return equal_it(const_me().begin(), const_me().end(), c.begin(),
                            c.end(), eq);
        }

        template <class ContainerType2 = ContainerType,
                  class Eq = std::equal_to<T>>
        bool equal(const ContainerType2& c, Eq&& eq = Eq()) const
        {
            return equal_it(const_me().begin(), const_me().end(), c.begin(),
                            c.end(), std::forward<Eq>(eq));
        }

        template <class Cmp>
        bool is_sorted(Cmp& cmp) const
        {
            return is_sorted_it(const_me().begin(), const_me().end(), cmp);
        }

        template <class Cmp = std::less<T>>
        bool is_sorted(Cmp&& cmp = Cmp()) const
        {
            return is_sorted_it(const_me().begin(), const_me().end(),
                                std::forward<Cmp>(cmp));
        }

        template <class ContainerType2 = ContainerType>
        SLList<std::pair<T, typename ContainerType2::KeyType>>
        zip(const ContainerType2& c) const
        {
            using T2 = typename ContainerType2::KeyType;

            return zip_it<T, T2>(const_me().begin(), const_me().end(),
                                 c.begin(), c.end());
        }

        template <class ContainerType2 = ContainerType>
        SLList<std::pair<T, typename ContainerType2::KeyType>>
        zip_eq(const ContainerType2& c) const
        {
            using T2 = typename ContainerType2::KeyType;

            return zip_eq_it<T, T2>(const_me().begin(), const_me().end(),
                                    c.begin(), c.end());
        }

        template <class ContainerType2 = ContainerType>
        SLList<std::pair<T, typename ContainerType2::KeyType>>
        zip_left(const ContainerType2& c) const
        {
            using T2 = typename ContainerType2::KeyType;

            return zip_left_it<T, T2>(const_me().begin(), const_me().end(),
                                      c.begin(), c.end());
        }

        template <class ContainerType2 = ContainerType>
        SLList<std::pair<T, typename ContainerType2::KeyType>>
        zip_right(const ContainerType2& c) const
        {
            using T2 = typename ContainerType2::KeyType;

            return zip_right_it<T, T2>(const_me().begin(), const_me().end(),
                                       c.begin(), c.end());
        }

        DynArray<T> to_array() const
        {
            return to_array_it<T>(const_me().begin(), const_me().end());
        }

        SLList<T> to_list() const
        {
            return to_list_it<T>(const_me().begin(), const_me().end());
        }
    };

} // end namespace Designar