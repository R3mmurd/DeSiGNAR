/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file math.hpp
    @brief Common mathematical constants and free functions: sign
    testing, tolerance-aware equality, absolute value, and exponentiation.
    @ingroup utils
*/

#pragma once

#include <types.hpp>

namespace Designar
{

    /** @name Mathematical constants
        PI and its common fractions/multiples, a default tolerance
        (EPSILON) used by real_equal() to compare floating point values,
        and INF, the floating point representation of infinity.
        @{ */
    constexpr real_t PI = 3.1415926535897932384626433832795028841971693993751;
    constexpr real_t PI_2 = PI / 2.;
    constexpr real_t PI_4 = PI / 4.;
    constexpr real_t PI3_2 = 3. * PI / 2.;
    constexpr real_t EPSILON = 1e-6;
    constexpr real_t INF = std::numeric_limits<real_t>::infinity();
    /** @} */

    /** @brief The sign of a number, as returned by sign(): NEGATIVE for
        values below zero, POSITIVE for values above zero, and ZERO for
        values equal to zero. */
    enum class Sign
    {
        NEGATIVE,
        POSITIVE,
        ZERO
    };

    /** @brief Returns the Sign of `n` (NEGATIVE, POSITIVE, or ZERO). */
    template <typename T>
    Sign sign(T);

    /** @brief Returns true if `n` is strictly positive. */
    template <typename T>
    bool is_positive(T);

    /** @brief Returns true if `n` is strictly negative. */
    template <typename T>
    bool is_negative(T);

    /** @brief Absolute value for floating point types. */
    template <typename T>
    T abs(T);

    /** @brief Compares two floating point values for equality within
        EPSILON tolerance, avoiding the pitfalls of exact `==` on
        floating point numbers. */
    template <typename T>
    bool real_equal(T, T);

    /** @brief Equality test for integral types (plain `==`); overloaded
        below for float, double, and long double to defer to
        real_equal(). */
    template <typename T>
    bool num_equal(T, T);

    template <>
    bool num_equal<float>(float, float);

    template <>
    bool num_equal<double>(double, double);

    template <>
    bool num_equal<long double>(long double, long double);

    /** @brief Forward declaration of the 2D point template, needed only
        to declare area_of_parallelogram() below. */
    template <class NumberType>
    class GenPoint2D;

    /** @brief Computes (twice) the signed area of the parallelogram
        spanned by vectors ab and ac, i.e. the 2D cross product
        (b - a) x (c - a); its sign indicates the orientation of the
        triple (a, b, c). */
    template <typename NumberType>
    real_t area_of_parallelogram(const GenPoint2D<NumberType>&,
                                 const GenPoint2D<NumberType>&,
                                 const GenPoint2D<NumberType>&);

    /** @brief Computes base^exp for integral base and unsigned integral
        exponent via exponentiation by squaring, in O(log exp) time
        without floating point conversion. */
    template <typename BT, typename ET>
    BT fast_integral_pow(BT, ET);

    /** @brief Generic exponentiation entry point: dispatches at compile
        time to FastIntegralPow when base and exponent are both integral
        (and the exponent unsigned), or to StdPow otherwise. */
    template <typename BT, typename ET>
    BT pow(BT, ET);

    template <typename T>
    T abs(T x)
    {
        static_assert(std::is_floating_point<T>::value,
                      "Template argument must be a floating point type");
        return (x < T(0) ? -x : x);
    }

    template <typename T>
    bool real_equal(T a, T b)
    {
        static_assert(std::is_floating_point<T>::value,
                      "Template argument must be a floating point type");
        return abs(a - b) <= EPSILON;
    }

    template <typename T>
    bool num_equal(T a, T b)
    {
        static_assert(std::is_integral<T>::value,
                      "Template argument must be an integral type");
        return a == b;
    }

    template <typename T>
    Sign sign(T n)
    {
        if (n < T(0))
        {
            return Sign::NEGATIVE;
        }
        else if (n > T(0))
        {
            return Sign::POSITIVE;
        }
        return Sign::ZERO;
    }

    template <typename T>
    bool is_positive(T n)
    {
        return sign(n) == Sign::POSITIVE;
    }

    template <typename T>
    bool is_negative(T n)
    {
        return sign(n) == Sign::NEGATIVE;
    }

    template <typename NumberType>
    real_t area_of_parallelogram(const GenPoint2D<NumberType>& a,
                                 const GenPoint2D<NumberType>& b,
                                 const GenPoint2D<NumberType>& c)
    {
        return (b.get_x() - a.get_x()) * (c.get_y() - a.get_y()) -
               (c.get_x() - a.get_x()) * (b.get_y() - a.get_y());
    }

    template <typename BT, typename ET>
    BT fast_integral_pow(BT base, ET exp)
    {
        static_assert(std::is_integral<BT>::value &&
                          std::is_integral<ET>::value,
                      "Arguments must be integral types");
        static_assert(std::is_unsigned<ET>::value,
                      "Exponent must be an unsigned type");

        BT ret_val = BT(1);

        while (exp > ET(0))
        {
            if (exp & ET(1))
            {
                ret_val *= base;
            }

            exp >>= 1;
            base *= base;
        }

        return ret_val;
    }

    /** @brief Function-object wrapper around fast_integral_pow(): the
        strategy pow() selects for integral base/exponent pairs, computing
        the result exactly via exponentiation by squaring. */
    template <typename BT, typename ET>
    struct FastIntegralPow
    {
        BT operator()(BT base, ET exp)
        {
            return fast_integral_pow(base, exp);
        }
    };

    /** @brief Function-object wrapper around std::pow(): the strategy
        pow() selects when integral exponentiation by squaring does not
        apply (e.g. floating point base/exponent or signed exponent). */
    template <typename BT, typename ET>
    struct StdPow
    {
        BT operator()(BT base, ET exp)
        {
            return std::pow(base, exp);
        }
    };

    template <typename BT, typename ET>
    BT pow(BT base, ET exp)
    {
        auto fct = typename std::conditional < std::is_integral<BT>::value and
                   std::is_integral<ET>::value and std::is_unsigned<ET>::value,
             FastIntegralPow<BT, ET>, StdPow < BT, ET >> ::type();
        return fct(base, exp);
    }

} // end namespace Designar
