/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file stringutilities.hpp
    @brief Free functions for common string operations: quoting and
    splitting/joining strings on a separator.
    @ingroup utils
*/

#pragma once

#include <types.hpp>

namespace Designar
{
    template <typename T>
    class DynArray;

    /** @brief Wraps `s` between two copies of `b` (e.g. q("x", "*")
        yields "*x*"). Used by sq() and dq() to implement quoting. */
    std::string q(const std::string& s, const std::string& b)
    {
        return b + s + b;
    }

    /** @brief Wraps `s` in single quotes. */
    std::string sq(const std::string& s)
    {
        return q(s, "'");
    }

    /** @brief Wraps `s` in double quotes. */
    std::string dq(const std::string& s)
    {
        return q(s, "\"");
    }

    /** @brief Splits `str` on every occurrence of the literal separator
        `sep`, returning the pieces in a container of type
        ContainerType (DynArray<std::string> by default). */
    template <typename ContainerType = DynArray<std::string>>
    ContainerType split_string(const std::string&, const std::string&);

    /** @brief Splits `str` on every match of the regular expression
        `pattern`, returning the pieces in a container of type
        ContainerType (DynArray<std::string> by default). */
    template <typename ContainerType = DynArray<std::string>>
    ContainerType split_string_re(const std::string&, const std::regex&);

    /** @brief Joins the elements of `ss` into a single string, inserting
        `sep` between consecutive elements. */
    template <typename ContainerType = DynArray<std::string>>
    std::string join_string(const std::string&, const ContainerType&);

    template <typename ContainerType>
    ContainerType split_string(const std::string& str, const std::string& sep)
    {
        ContainerType ss;

        if (str.empty())
        {
            return ss;
        }

        size_t sep_sz = sep.size();

        size_t beg = 0;
        size_t end = str.find(sep);

        while (end != std::string::npos)
        {
            ss.append(str.substr(beg, end - beg));
            beg = end + sep_sz;
            end = str.find(sep, beg);
        }

        ss.append(str.substr(beg, str.size() - beg));

        return ss;
    }

    template <typename ContainerType>
    ContainerType split_string_re(const std::string& str,
                                  const std::regex& pattern)
    {
        ContainerType ss;

        if (str.empty())
        {
            return ss;
        }

        std::string s = str;
        std::smatch m;

        while (regex_search(s, m, pattern))
        {
            ss.append(m.prefix().str());
            s = m.suffix().str();
        }

        ss.append(s);

        return ss;
    }

    template <typename ContainerType>
    std::string join_string(const std::string& sep, const ContainerType& ss)
    {
        std::string ret_val = "";

        if (ss.is_empty())
        {
            return ret_val;
        }

        auto it = ss.begin();
        ret_val = it.get_current();
        it.next();

        for (; it != ss.end(); ++it)
        {
            ret_val += sep + *it;
        }

        return ret_val;
    }

} // end namespace Designar
