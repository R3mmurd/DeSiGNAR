/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file types.hpp
    @brief The library's fundamental type aliases and a placeholder
    "empty" type, included by nearly every other header.

    Defines the numeric aliases used consistently in place of raw
    built-in types throughout the codebase: nat_t (unsigned, for sizes,
    counts, and indices), int_t (signed integer arithmetic), and real_t
    (floating point arithmetic), along with the random-number engine,
    clock, and time-related aliases layered on top of them, plus
    EmptyClass, a no-op tag type used as a default template argument
    where a piece of user data (e.g. a node's payload) is optional.
    @ingroup utils
*/

#pragma once

#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>
#include <tuple>
#include <stdexcept>
#include <limits>
#include <chrono>
#include <random>
#include <functional>
#include <regex>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <typetraits.hpp>

namespace Designar
{
    /** @name Fundamental type aliases
        The signed/unsigned integer and floating point types used
        throughout the library instead of raw built-ins, plus the
        random-number engine, its seed type, and clock-related aliases
        built on top of them.
        @{ */
    using int_t = int64_t;
    using nat_t = uint64_t;
    using real_t = double;
    using rng_t = std::mt19937_64;
    using rng_seed_t = rng_t::result_type;
    using clock_t = std::chrono::high_resolution_clock;
    using time_point_t = clock_t::time_point;
    using duration_t = clock_t::duration;
    /** @} */

    constexpr int_t QuicksortThreshold = 40;

    /** @brief A no-op placeholder type: default-constructible,
        copyable, comparable (all instances compare equal), and
        stream-insertable/extractable as a no-op. Used as the default
        template argument for optional payload types (e.g. ArcInfo and
        GraphInfo in Graph) so that a type parameter can be left
        unspecified without special-casing "no data" everywhere it is
        used. */
    class EmptyClass
    {
    public:
        EmptyClass()
        { /* empty */
        }

        EmptyClass(const EmptyClass&)
        { /* empty */
        }

        EmptyClass(EmptyClass&&)
        { /* empty */
        }

        ~EmptyClass()
        { /* empty */
        }

        EmptyClass& operator=(const EmptyClass&)
        {
            return *this;
        }

        EmptyClass& operator=(EmptyClass&&)
        {
            return *this;
        }

        bool operator==(const EmptyClass&) const
        {
            return true;
        }

        bool operator!=(const EmptyClass&) const
        {
            return false;
        }

        bool operator<(const EmptyClass&) const
        {
            return false;
        }

        bool operator<=(const EmptyClass&) const
        {
            return true;
        }

        bool operator>(const EmptyClass&) const
        {
            return false;
        }

        bool operator>=(const EmptyClass&) const
        {
            return true;
        }

        friend std::ostream& operator<<(std::ostream& out, const EmptyClass&)
        {
            return out;
        }

        friend std::istream& operator>>(std::istream& in, EmptyClass&)
        {
            return in;
        }
    };

} // end namespace Designar
