/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file italgorithms.hpp
    @brief Generic, iterator-range-based algorithms shared by every
    container in the library.
    @ingroup Algorithms
*/

#pragma once

#include <types.hpp>

namespace Designar
{
    template <typename T>
    class DynArray;
    template <typename T>
    class SLList;

    /** This is the library's analogue of the standard `<algorithm>`
        header: every function here is a free template expressed purely
        in terms of a `(begin, end)` iterator pair (`const It&, const
        It&`), with no dependency on any particular container. They know
        nothing about DynArray, SLList, trees, etc. — they only require
        that `It` support the usual forward-iteration interface
        (`operator*`, `operator++`, `operator==`/`!=`, and, where removal
        is involved, `del()`).

        Each `..._it` function typically comes in two overloads: one
        taking an lvalue functor/predicate (`Op&`/`Pred&`), and one
        taking an rvalue with a default-constructed default
        (`Op&& op = Op()`), so callers can pass either a named object or
        a temporary/stateless functor. The rvalue overload simply
        forwards to the lvalue one.

        `ContainerAlgorithms` (containeralgorithms.hpp) and
        `SetAlgorithms` (setalgorithms.hpp) are thin CRTP wrappers that
        adapt these free functions into member functions for any
        container/set type, so this file is effectively the single
        implementation shared by all of them. */

    /** @brief Positional access: fetch the pointer/reference to the
        element at a given offset within an iterator range. */
    template <typename RetT, class It>
    RetT* nth_ptr_it(const It&, const It&, nat_t);

    template <typename RetT, class It>
    RetT& nth_it(const It&, const It&, nat_t);

    /** @brief Traversal: apply an operation to every element (optionally
        paired with its position, or to every unordered pair of
        elements) in the range. */
    template <class It, class Op>
    void for_each_it(const It&, const It&, Op&);

    template <class It, class Op>
    void for_each_it(const It&, const It&, Op&& op = Op());

    template <class It, class Op>
    void enum_for_each_it(const It&, const It&, Op&);

    template <class It, class Op>
    void enum_for_each_it(const It&, const It&, Op&& op = Op());

    template <class It, class Op>
    void for_each_pair_it(const It&, const It&, Op&);

    template <class It, class Op>
    void for_each_pair_it(const It&, const It&, Op&& op = Op());

    /** @brief Filtering/transformation: build a new container from the
        elements of the range, optionally selecting (filter_it),
        converting (map_it), or both (map_if_it) them along the way. */
    template <class ContainerRet, class It, class Pred>
    ContainerRet filter_it(const It&, const It&, Pred&);

    template <class ContainerRet, class It, class Pred>
    ContainerRet filter_it(const It&, const It&, Pred&& pred = Pred());

    template <class ContainerRet, class It, class Op>
    ContainerRet map_it(const It&, const It&, Op&);

    template <class ContainerRet, class It, class Op>
    ContainerRet map_it(const It&, const It&, Op&& op = Op());

    template <class ContainerRet, class It, class Op, class Pred>
    ContainerRet map_if_it(const It&, const It&, Op&, Pred&);

    template <class ContainerRet, class It, class Op, class Pred>
    ContainerRet map_if_it(const It&, const It&, Op&, Pred&& pred = Pred());

    template <class ContainerRet, class It, class Op, class Pred>
    ContainerRet map_if_it(const It&, const It&, Op&&, Pred&);

    template <class ContainerRet, class It, class Op, class Pred>
    ContainerRet map_if_it(const It&, const It&, Op&& op = Op(),
                           Pred&& pred = Pred());

    /** @brief Search and quantification: locate an element satisfying a
        predicate, or check whether all/some/none of the elements in the
        range satisfy it. */
    template <typename RetT, class It, class Pred>
    RetT* search_ptr_it(const It&, const It&, Pred&);

    template <typename RetT, class It, class Pred>
    RetT* search_ptr_it(const It&, const It&, Pred&& pred = Pred());

    template <class It, class Pred>
    bool all_it(const It&, const It&, Pred&);

    template <class It, class Pred>
    bool all_it(const It&, const It&, Pred&& pred = Pred());

    template <class It, class Pred>
    bool exists_it(const It&, const It&, Pred&);

    template <class It, class Pred>
    bool exists_it(const It&, const It&, Pred&& pred = Pred());

    template <class It, class Pred>
    bool none_it(const It&, const It&, Pred&);

    template <class It, class Pred>
    bool none_it(const It&, const It&, Pred&& pred = Pred());

    /** @brief Reduction: accumulate the elements of the range into a
        single value via a binary combining operation. */
    template <typename RetT, class It, class Op>
    RetT fold_it(const It&, const It&, const RetT&, Op&);

    template <typename RetT, class It, class Op>
    RetT fold_it(const It&, const It&, const RetT&, Op&& op = Op());

    template <typename RetT, class It, class Op>
    RetT fold_it(const It&, const It&, RetT&&, Op&);

    template <typename RetT, class It, class Op>
    RetT fold_it(const It&, const It&, RetT&&, Op&& op = Op());

    /** @brief Removal: erase the first element (remove_first_if_it) or
        every element (remove_if_it) matching a predicate, via the
        iterator's own `del()`. */
    template <class It, class Pred>
    bool remove_first_if_it(const It&, const It&, Pred&);

    template <class It, class Pred>
    bool remove_first_if_it(const It&, const It&, Pred&& pred = Pred());

    template <class It, class Pred>
    void remove_if_it(const It&, const It&, Pred&);

    template <class It, class Pred>
    void remove_if_it(const It&, const It&, Pred&& pred = Pred());

    /** @brief Comparison: check element-wise equality between two
        ranges, or whether a single range is sorted according to a
        comparator. */
    template <class It1, class It2, class Eq>
    bool equal_it(const It1&, const It1&, const It2&, const It2&, Eq&);

    template <class It1, class It2, class Eq>
    bool equal_it(const It1&, const It1&, const It2&, const It2&,
                  Eq&& eq = Eq());

    template <class It, class Cmp>
    bool is_sorted_it(const It&, const It&, Cmp&);

    template <class It, class Cmp>
    bool is_sorted_it(const It&, const It&, Cmp&& cmp = Cmp());

    /** @brief Pairing: combine two ranges element-wise into a list of
        pairs, stopping at the shorter range (zip_it), requiring equal
        lengths (zip_eq_it), or cycling the shorter range to match the
        longer one (zip_left_it/zip_right_it). */
    template <typename T1, typename T2, class It1, class It2>
    SLList<std::pair<T1, T2>> zip_it(const It1&, const It1&, const It2&,
                                     const It2&);

    template <typename T1, typename T2, class It1, class It2>
    SLList<std::pair<T1, T2>> zip_eq_it(const It1&, const It1&, const It2&,
                                        const It2&);

    template <typename T1, typename T2, class It1, class It2>
    SLList<std::pair<T1, T2>> zip_left_it(const It1&, const It1&, const It2&,
                                          const It2&);

    template <typename T1, typename T2, class It1, class It2>
    SLList<std::pair<T1, T2>> zip_right_it(const It1&, const It1&, const It2&,
                                           const It2&);

    /** @brief Materialization: drain a range into a fresh container of a
        given type (to_container), or specifically into an SLList
        (to_list_it) or a DynArray (to_array_it). */
    template <class ContainerType, class It>
    ContainerType to_container(const It&, const It&);

    template <typename T, class It>
    SLList<T> to_list_it(const It&, const It&);

    template <typename T, class It>
    DynArray<T> to_array_it(const It&, const It&);

    template <typename RetT, class It>
    RetT* nth_ptr_it(const It& a, const It& b, nat_t pos)
    {
        for (It i = a; i != b; ++i)
        {
            if (pos == 0)
            {
                return &*i;
            }
            --pos;
        }

        return nullptr;
    }

    template <typename RetT, class It>
    RetT& nth_it(const It& a, const It& b, nat_t pos)
    {
        for (It i = a; i != b; ++i)
        {
            if (pos == 0)
            {
                return *i;
            }
            --pos;
        }

        throw std::overflow_error("Index is to large");
    }

    template <class It, class Op>
    void for_each_it(const It& a, const It& b, Op& op)
    {
        for (It i = a; i != b; ++i)
        {
            op(*i);
        }
    }

    template <class It, class Op>
    void for_each_it(const It& a, const It& b, Op&& op)
    {
        for_each_it<It, Op>(a, b, op);
    }

    template <class It, class Op>
    void enum_for_each_it(const It& a, const It& b, Op& op)
    {
        nat_t p = 0;
        for (It i = a; i != b; ++i, ++p)
        {
            op(p, *i);
        }
    }

    template <class It, class Op>
    void enum_for_each_it(const It& a, const It& b, Op&& op)
    {
        enum_for_each_it<It, Op>(a, b, op);
    }

    template <class It, class Op>
    void for_each_pair_it(const It& a, const It& b, Op& op)
    {
        It last_i = prev_it(b);
        for (It i = a; i != last_i; ++i)
        {
            for (It j = next_it(i); j != b; ++j)
            {
                op(*i, *j);
            }
        }
    }

    template <class It, class Op>
    void for_each_pair_it(const It& a, const It& b, Op&& op)
    {
        for_each_pair_it<It, Op>(a, b, op);
    }

    template <class ContainerRet, class It, class Pred>
    ContainerRet filter_it(const It& a, const It& b, Pred& pred)
    {
        ContainerRet ret;

        for (It i = a; i != b; ++i)
        {
            if (pred(*i))
            {
                ret.append(*i);
            }
        }

        return ret;
    }

    template <class ContainerRet, class It, class Pred>
    ContainerRet filter_it(const It& a, const It& b, Pred&& pred)
    {
        return filter_it<ContainerRet, It, Pred>(a, b, pred);
    }

    template <class ContainerRet, class It, class Op>
    ContainerRet map_it(const It& a, const It& b, Op& op)
    {
        ContainerRet ret;

        for (It i = a; i != b; ++i)
        {
            ret.append(op(*i));
        }

        return ret;
    }

    template <class ContainerRet, class It, class Op>
    ContainerRet map_it(const It& a, const It& b, Op&& op)
    {
        return map_it<ContainerRet, It, Op>(a, b, op);
    }

    template <class ContainerRet, class It, class Op, class Pred>
    ContainerRet map_if_it(const It& a, const It& b, Op& op, Pred& pred)
    {
        ContainerRet ret;

        for (It i = a; i != b; ++i)
        {
            if (pred(*i))
            {
                ret.append(op(*i));
            }
        }

        return ret;
    }

    template <class ContainerRet, class It, class Op, class Pred>
    ContainerRet map_if_it(const It& a, const It& b, Op& op, Pred&& pred)
    {
        return map_if_it<ContainerRet, It, Op, Pred>(a, b, op, pred);
    }

    template <class ContainerRet, class It, class Op, class Pred>
    ContainerRet map_if_it(const It& a, const It& b, Op&& op, Pred& pred)
    {
        return map_if_it<ContainerRet, It, Op, Pred>(a, b, op, pred);
    }

    template <class ContainerRet, class It, class Op, class Pred>
    ContainerRet map_if_it(const It& a, const It& b, Op&& op, Pred&& pred)
    {
        return map_if_it<ContainerRet, It, Op, Pred>(a, b, op, pred);
    }

    template <typename RetT, class It, class Op>
    RetT fold_it(const It& a, const It& b, const RetT& init_value, Op& op)
    {
        RetT ret_val = init_value;

        for (It i = a; i != b; ++i)
        {
            ret_val = op(*i, ret_val);
        }

        return ret_val;
    }

    template <typename RetT, class It, class Op>
    RetT fold_it(const It& a, const It& b, const RetT& init_value, Op&& op)
    {
        return fold_it<RetT, It, Op>(a, b, init_value, op);
    }

    template <typename RetT, class It, class Op>
    RetT fold_it(const It& a, const It& b, RetT&& init_value, Op& op)
    {
        RetT ret_val = std::move(init_value);

        for (It i = a; i != b; ++i)
        {
            ret_val = op(*i, ret_val);
        }

        return ret_val;
    }

    template <typename RetT, class It, class Op>
    RetT fold_it(const It& a, const It& b, RetT&& init_value, Op&& op)
    {
        return fold_it<RetT, It, Op>(a, b, std::forward<RetT>(init_value), op);
    }

    template <class It, class Pred>
    bool all_it(const It& a, const It& b, Pred& pred)
    {
        for (It i = a; i != b; ++i)
        {
            if (!pred(*i))
            {
                return false;
            }
        }
        return true;
    }

    template <class It, class Pred>
    bool all_it(const It& a, const It& b, Pred&& pred)
    {
        return all_it<It, Pred>(a, b, pred);
    }

    template <class It, class Pred>
    bool exists_it(const It& a, const It& b, Pred& pred)
    {
        for (It i = a; i != b; ++i)
        {
            if (pred(*i))
            {
                return true;
            }
        }
        return false;
    }

    template <class It, class Pred>
    bool exists_it(const It& a, const It& b, Pred&& pred)
    {
        return exists_it<It, Pred>(a, b, pred);
    }

    template <class It, class Pred>
    bool none_it(const It& a, const It& b, Pred& pred)
    {
        for (It i = a; i != b; ++i)
        {
            if (pred(*i))
            {
                return false;
            }
        }
        return true;
    }

    template <class It, class Pred>
    bool none_it(const It& a, const It& b, Pred&& pred)
    {
        return none_it<It, Pred>(a, b, pred);
    }

    template <typename RetT, class It, class Pred>
    RetT* search_ptr_it(const It& a, const It& b, Pred& pred)
    {
        for (It i = a; i != b; ++i)
        {
            if (pred(*i))
            {
                return &*i;
            }
        }

        return nullptr;
    }

    template <typename RetT, class It, class Pred>
    RetT* search_ptr_it(const It& a, const It& b, Pred&& pred)
    {
        return search_ptr_it<RetT, It, Pred>(a, b, pred);
    }

    template <class It, class Pred>
    bool remove_first_if_it(const It& a, const It& b, Pred& pred)
    {
        for (It i = a; i != b; ++i)
        {
            if (pred(*i))
            {
                i.del();
                return true;
            }
        }

        return false;
    }

    template <class It, class Pred>
    bool remove_first_if_it(const It& a, const It& b, Pred&& pred)
    {
        return remove_first_if_it<It, Pred>(a, b, pred);
    }

    template <class It, class Pred>
    void remove_if_it(const It& a, const It& b, Pred& pred)
    {
        for (It i = a; i != b;)
        {
            if (pred(*i))
            {
                i.del();
            }
            else
            {
                ++i;
            }
        }
    }

    template <class It, class Pred>
    void remove_if_it(const It& a, const It& b, Pred&& pred)
    {
        remove_if_it<It, Pred>(a, b, pred);
    }

    template <class It1, class It2, class Eq>
    bool equal_it(const It1& a1, const It1& b1, const It2& a2, const It2& b2,
                  Eq& eq)
    {
        It1 i1 = a1;
        It2 i2 = a2;

        for (; i1 != b1 && i2 != b2; ++i1, ++i2)
        {
            if (!eq(*i1, *i2))
            {
                return false;
            }
        }

        return (i1 == b1 && i2 == b2);
    }

    template <class It1, class It2, class Eq>
    bool equal_it(const It1& a1, const It1& b1, const It2& a2, const It2& b2,
                  Eq&& eq)
    {
        return equal_it<It1, It2, Eq>(a1, b1, a2, b2, eq);
    }

    template <class It, class Cmp>
    bool is_sorted_it(const It& a, const It& b, Cmp& cmp)
    {
        if (a == b)
        {
            return true;
        }

        It i = a;
        It j = a;
        ++j;

        for (; j != b; ++i, ++j)
        {
            if (cmp(*j, *i))
            {
                return false;
            }
        }

        return true;
    }

    template <class It, class Cmp>
    bool is_sorted_it(const It& a, const It& b, Cmp&& cmp)
    {
        return is_sorted_it<It, Cmp>(a, b, cmp);
    }

    template <typename T1, typename T2, class It1, class It2>
    SLList<std::pair<T1, T2>> zip_it(const It1& a1, const It1& b1,
                                     const It2& a2, const It2& b2)
    {
        SLList<std::pair<T1, T2>> ret;

        It1 i1 = a1;
        It2 i2 = a2;

        for (; i1 != b1 && i2 != b2; ++i1, ++i2)
        {
            ret.append(std::make_pair(*i1, *i2));
        }

        return ret;
    }

    template <typename T1, typename T2, class It1, class It2>
    SLList<std::pair<T1, T2>> zip_eq_it(const It1& a1, const It1& b1,
                                        const It2& a2, const It2& b2)
    {
        SLList<std::pair<T1, T2>> ret;

        It1 i1 = a1;
        It2 i2 = a2;

        for (; i1 != b1 && i2 != b2; ++i1, ++i2)
        {
            ret.append(std::make_pair(*i1, *i2));
        }

        if (i1 != b1 || i2 != b2)
        {
            throw std::length_error("Container sizes mismatch");
        }

        return ret;
    }

    template <typename T1, typename T2, class It1, class It2>
    SLList<std::pair<T1, T2>> zip_left_it(const It1& a1, const It1& b1,
                                          const It2& a2, const It2& b2)
    {
        SLList<std::pair<T1, T2>> ret;

        It1 i1 = a1;
        It2 i2 = a2;

        for (; i1 != b1; ++i1)
        {
            ret.append(std::make_pair(*i1, *i2));

            ++i2;

            if (i2 == b2)
            {
                i2 = a2;
            }
        }

        return ret;
    }

    template <typename T1, typename T2, class It1, class It2>
    SLList<std::pair<T1, T2>> zip_right_it(const It1& a1, const It1& b1,
                                           const It2& a2, const It2& b2)
    {
        SLList<std::pair<T1, T2>> ret;

        It1 i1 = a1;
        It2 i2 = a2;

        for (; i2 != b2; ++i2)
        {
            ret.append(std::make_pair(*i1, *i2));

            ++i1;

            if (i1 == b1)
            {
                i1 = a1;
            }
        }

        return ret;
    }

    template <class ContainerType, class It>
    ContainerType to_container(const It& a, const It& b)
    {
        ContainerType ret;

        for (It i = a; i != b; ++i)
        {
            ret.append(*i);
        }

        return ret;
    }

    template <typename T, class It>
    SLList<T> to_list_it(const It& a, const It& b)
    {
        return to_container<SLList<T>, It>(a, b);
    }

    template <typename T, class It>
    DynArray<T> to_array_it(const It& a, const It& b)
    {
        return to_container<DynArray<T>, It>(a, b);
    }

} // end namespace Designar
