/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file closestpair.hpp
    @brief ClosestPair: the classic O(n log^2 n) divide-and-conquer
    closest-pair-of-points algorithm.
    @ingroup Geometry
*/

#pragma once

#include <point2D.hpp>
#include <sort.hpp>

namespace Designar
{
    /** Finds the closest pair among `points` (which must contain at least
        two) via the classic O(n log^2 n) divide-and-conquer: sort by x,
        recursively solve each half, then check only the "strip" of
        points within the current best distance of the dividing line
        (itself sorted by y, so — thanks to a well-known packing argument —
        each point only needs to be compared against a small constant
        number of its neighbors in that sorted strip, not the whole
        strip). This is the textbook-simple variant that re-sorts the
        strip by y at every level of the recursion (O(n log^2 n) overall)
        rather than the fully optimized O(n lg n) version, which instead
        merges pre-sorted-by-y subarrays alongside the by-x recursion the
        same way merge sort merges its halves — a meaningful constant-factor
        simplification, not an asymptotic one, and considerably easier to
        verify correct. */
    template <class PointT>
    class ClosestPair
    {
    public:
        struct Result
        {
            PointT p1;
            PointT p2;
            real_t distance;
        };

    private:
        static Result brute_force(const DynArray<PointT>& pts, nat_t lo,
                                  nat_t hi)
        {
            Result best{pts[lo], pts[lo + 1],
                        pts[lo].distance_with(pts[lo + 1])};

            for (nat_t i = lo; i <= hi; ++i)
            {
                for (nat_t j = i + 1; j <= hi; ++j)
                {
                    real_t d = pts[i].distance_with(pts[j]);

                    if (d < best.distance)
                    {
                        best = Result{pts[i], pts[j], d};
                    }
                }
            }

            return best;
        }

        static Result solve(const DynArray<PointT>& by_x, nat_t lo, nat_t hi)
        {
            nat_t n = hi - lo + 1;

            if (n <= 3)
            {
                return brute_force(by_x, lo, hi);
            }

            nat_t mid = lo + (hi - lo) / 2;
            auto mid_x = by_x[mid].get_x();

            Result left = solve(by_x, lo, mid);
            Result right = solve(by_x, mid + 1, hi);
            Result best = left.distance <= right.distance ? left : right;

            DynArray<PointT> strip;

            for (nat_t i = lo; i <= hi; ++i)
            {
                if (std::abs(real_t(by_x[i].get_x()) - real_t(mid_x)) <
                    best.distance)
                {
                    strip.append(by_x[i]);
                }
            }

            inline_sort(strip, [](const PointT& p, const PointT& q)
                        { return p.get_y() < q.get_y(); });

            for (nat_t i = 0; i < strip.size(); ++i)
            {
                for (nat_t j = i + 1; j < strip.size() &&
                                      real_t(strip[j].get_y() -
                                             strip[i].get_y()) < best.distance;
                     ++j)
                {
                    real_t d = strip[i].distance_with(strip[j]);

                    if (d < best.distance)
                    {
                        best = Result{strip[i], strip[j], d};
                    }
                }
            }

            return best;
        }

    public:
        static Result compute(DynArray<PointT> points)
        {
            if (points.size() < 2)
            {
                throw std::domain_error(
                    "ClosestPair needs at least two points");
            }

            inline_sort(points, [](const PointT& p, const PointT& q)
                        { return p.get_x() < q.get_x(); });

            return solve(points, 0, points.size() - 1);
        }
    };

} // end namespace Designar
