/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file intutilities.hpp
    @brief Free functions for integer arithmetic: ranged products,
    factorial, permutations, and combinations.
    @ingroup utils
*/

#pragma once

#include <types.hpp>

namespace Designar
{
    /** @brief Computes the product of all integers from `a` up to `b`
        inclusive, ascending (a * (a+1) * ... * b). */
    template <typename T>
    T forward_prod(T, T);

    /** @brief Computes the product of all integers from `a` down to `b`
        inclusive, descending (a * (a-1) * ... * b). */
    template <typename T>
    T backward_prod(T, T);

    /** @brief Computes n! via forward_prod. Throws std::domain_error if
        n is negative. */
    template <typename T>
    T factorial(T);

    /** @brief Computes the number of permutations of r elements taken
        from a set of n elements (nPr = n! / (n - r)!). */
    template <typename T>
    T count_permutations(T, T);

    /** @brief Computes the number of combinations of r elements taken
        from a set of n elements (nCr = n! / (r! * (n - r)!)). */
    template <typename T>
    T count_combinations(T, T);

    template <typename T>
    T forward_prod(T a, T b)
    {
        static_assert(std::is_integral<T>::value,
                      "Argument must be an integral type");

        T ret_val = 1;

        while (a <= b)
        {
            ret_val *= a++;
        }

        return ret_val;
    }

    template <typename T>
    T backward_prod(T a, T b)
    {
        static_assert(std::is_integral<T>::value,
                      "Argument must be an integral type");

        T ret_val = 1;

        while (a >= b)
        {
            ret_val *= a--;
        }

        return ret_val;
    }

    template <typename T>
    T factorial(T n)
    {
        if (n < 0)
        {
            throw std::domain_error("Argument must be positive");
        }

        return forward_prod(T(1), n);
    }

    template <typename T>
    T count_permutations(T n, T r)
    {
        if (n < 0 || r < 0)
        {
            throw std::domain_error("Arguments must be positive numbers");
        }

        if (r > n)
        {
            throw std::logic_error("r > n");
        }

        return backward_prod(n, n - r + T(1));
    }

    template <typename T>
    T count_combinations(T n, T r)
    {
        if (n < 0 || r < 0)
        {
            throw std::domain_error("Arguments must be positive numbers");
        }

        if (r > n)
        {
            throw std::logic_error("r > n");
        }

        return backward_prod(n, n - r + T(1)) / factorial(r);
    }

} // end namespace Designar
