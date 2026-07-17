/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file voronoi.hpp
    @brief VoronoiDiagram: computes every site's Voronoi cell via
    half-plane intersection (Sutherland-Hodgman polygon clipping
    against every other site's perpendicular bisector) — O(n^2)
    overall rather than Fortune's O(n log n) sweep, the same
    teaching-clarity-over-asymptotic-optimality trade-off ClosestPair
    makes by using the textbook-simple O(n log^2 n) divide-and-conquer
    instead of the fully optimized O(n log n) merge variant: each
    site's cell falls out of a short, direct geometric argument
    ("intersect every half-plane closer to me than to some other
    site") with no sweep-line event queue or circle-event bookkeeping
    to get subtly wrong.
    @ingroup Geometry
*/

#pragma once

#include <point2D.hpp>
#include <array.hpp>

#include <stdexcept>

namespace Designar
{
    /** One site's Voronoi cell, as a convex polygon (vertices in order
        around the boundary) clipped to a finite square — the true
        Voronoi diagram's outer cells are mathematically unbounded
        regions, and this finite box is what turns them into an actual
        closed polygon a caller can draw or measure the area of, at the
        cost of only approximating the true (infinite) cell near that
        boundary. */
    template <class PointT>
    struct VoronoiCell
    {
        PointT site;
        DynArray<PointT> polygon;
    };

    template <class PointT>
    class VoronoiDiagram
    {
        using NumberType = typename PointT::NumberType;

        DynArray<PointT> sites;
        real_t bounds_half_size;

        static real_t sq_dist(const PointT& a, const PointT& b)
        {
            real_t dx = real_t(a.get_x()) - real_t(b.get_x());
            real_t dy = real_t(a.get_y()) - real_t(b.get_y());
            return dx * dx + dy * dy;
        }

        /** The point on segment (a, b) where dist2(p, site) == dist2(p,
            other) — i.e. where the segment crosses the perpendicular
            bisector of site/other. f(t) = dist2(lerp(a,b,t), site) -
            dist2(lerp(a,b,t), other) is *linear* in t (the quadratic |p|^2
            terms that appear in each squared-distance expansion are
            identical in both and cancel when subtracted), so its root is a
            single division away — no need for the general line-line
            intersection formula this could otherwise be phrased as. */
        static PointT intersect_bisector(const PointT& a, const PointT& b,
                                         const PointT& site,
                                         const PointT& other)
        {
            real_t fa = sq_dist(a, site) - sq_dist(a, other);
            real_t fb = sq_dist(b, site) - sq_dist(b, other);

            real_t t = fa / (fa - fb);

            real_t x =
                real_t(a.get_x()) + t * (real_t(b.get_x()) - real_t(a.get_x()));
            real_t y =
                real_t(a.get_y()) + t * (real_t(b.get_y()) - real_t(a.get_y()));

            return PointT(NumberType(x), NumberType(y));
        }

        /** Sutherland-Hodgman clipping of the convex polygon `poly` by the
            half-plane "closer to `site` than to `other`". `poly` is
            assumed convex (true of the initial bounding square, and every
            clip below preserves convexity, since intersecting a convex
            polygon with a half-plane is always convex), which is what
            makes a single "walk the edges, keep the inside portion" pass
            sufficient — the general polygon-clipping problem (clipping by
            an arbitrary, possibly non-convex, polygon) needs considerably
            more machinery than this. */
        static DynArray<PointT> clip_by_bisector(const DynArray<PointT>& poly,
                                                 const PointT& site,
                                                 const PointT& other)
        {
            DynArray<PointT> result;

            nat_t n = poly.size();

            if (n == 0)
            {
                return result;
            }

            auto inside = [&](const PointT& p)
            { return sq_dist(p, site) <= sq_dist(p, other); };

            for (nat_t i = 0; i < n; ++i)
            {
                const PointT& curr = poly[i];
                const PointT& prev = poly[(i + n - 1) % n];

                bool curr_in = inside(curr);
                bool prev_in = inside(prev);

                if (curr_in)
                {
                    if (!prev_in)
                    {
                        result.append(
                            intersect_bisector(prev, curr, site, other));
                    }

                    result.append(curr);
                }
                else if (prev_in)
                {
                    result.append(intersect_bisector(prev, curr, site, other));
                }
            }

            return result;
        }

    public:
        /** `_bounds_half_size` should be large enough relative to the
            sites' own spread that clipping against it does not visibly
            distort any cell's *interesting* (near-site) region — how large
            "enough" is depends entirely on the input, which is why this is
            a caller-supplied parameter rather than something computed
            automatically from `_sites` here. */
        VoronoiDiagram(const DynArray<PointT>& _sites, real_t _bounds_half_size)
            : sites(_sites), bounds_half_size(_bounds_half_size)
        {
            if (sites.size() < 2)
            {
                throw std::domain_error(
                    "VoronoiDiagram needs at least two sites");
            }
        }

        /** Computes and returns every site's cell, in the same order as
            the `sites` this diagram was constructed with. */
        DynArray<VoronoiCell<PointT>> compute() const
        {
            DynArray<VoronoiCell<PointT>> cells;

            for (nat_t i = 0; i < sites.size(); ++i)
            {
                const PointT& site = sites[i];

                real_t cx = real_t(site.get_x());
                real_t cy = real_t(site.get_y());
                real_t h = bounds_half_size;

                DynArray<PointT> poly = {
                    PointT(NumberType(cx - h), NumberType(cy - h)),
                    PointT(NumberType(cx + h), NumberType(cy - h)),
                    PointT(NumberType(cx + h), NumberType(cy + h)),
                    PointT(NumberType(cx - h), NumberType(cy + h))};

                for (nat_t j = 0; j < sites.size(); ++j)
                {
                    if (j == i)
                    {
                        continue;
                    }

                    poly = clip_by_bisector(poly, site, sites[j]);

                    if (poly.is_empty())
                    {
                        break;
                    }
                }

                cells.append(VoronoiCell<PointT>{site, std::move(poly)});
            }

            return cells;
        }
    };

} // end namespace Designar
