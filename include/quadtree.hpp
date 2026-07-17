/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file quadtree.hpp
    @brief QuadTree: a region quadtree over 2D points — recursively
    subdivides its bounding square into four quadrants once a node
    holds more than a fixed capacity, giving fast range queries over
    spatially clustered data without the cost of a full grid (a plain
    grid's cell size has to be chosen up front and wastes memory over
    sparse regions; a quadtree instead subdivides only where points
    actually are).
    @ingroup Geometry
*/

#pragma once

#include <point2D.hpp>
#include <array.hpp>

namespace Designar
{
    /** An axis-aligned square region, described by its center and half
        its side length ("half-dimension") rather than by min/max corners
        — the parameterization that makes each child quadrant's boundary
        in QuadTree::subdivide() a one-line offset of +-half_dimension/2
        along each axis from the parent's own center. */
    template <class PointT>
    struct QuadBoundary
    {
        using NumberType = typename PointT::NumberType;

        PointT center;
        real_t half_dimension;

        QuadBoundary() : center(), half_dimension(0)
        {
            // empty
        }

        QuadBoundary(const PointT& _center, real_t _half_dimension)
            : center(_center), half_dimension(_half_dimension)
        {
            // empty
        }

        bool contains(const PointT& p) const
        {
            real_t cx = real_t(center.get_x());
            real_t cy = real_t(center.get_y());

            return real_t(p.get_x()) >= cx - half_dimension &&
                   real_t(p.get_x()) <= cx + half_dimension &&
                   real_t(p.get_y()) >= cy - half_dimension &&
                   real_t(p.get_y()) <= cy + half_dimension;
        }

        /** Whether this square and `other` overlap at all — used to prune
            query_range()'s recursion: a node whose whole boundary misses
            the query range cannot contain any point inside it. */
        bool intersects(const QuadBoundary& other) const
        {
            real_t dx =
                std::abs(real_t(center.get_x()) - real_t(other.center.get_x()));
            real_t dy =
                std::abs(real_t(center.get_y()) - real_t(other.center.get_y()));

            return dx <= (half_dimension + other.half_dimension) &&
                   dy <= (half_dimension + other.half_dimension);
        }
    };

    /** `NODE_CAPACITY` points fit in a node before it subdivides; every
        insert()/query_range() call is `O(NODE_CAPACITY * depth)`, and
        depth stays `O(log n)` on average for spatially well-distributed
        points — the same way an unbalanced BST stays shallow on average
        for random insertion order despite having no explicit balancing.
        A deliberately adversarial input (e.g. many points at/near the
        exact same coordinate) can still force arbitrarily deep
        subdivision, since — unlike a k-d tree built from a known point
        set — this quadtree subdivides purely spatially and has no notion
        of "give up and store extras anyway" other than that geometric
        split; `MAX_DEPTH` below exists specifically to cap that
        pathological case. */
    template <class PointT>
    class QuadTree
    {
    public:
        using Boundary = QuadBoundary<PointT>;

    private:
        static constexpr nat_t NODE_CAPACITY = 4;
        static constexpr nat_t MAX_DEPTH = 32;

        Boundary boundary;
        DynArray<PointT> points;
        nat_t depth;

        QuadTree* northwest;
        QuadTree* northeast;
        QuadTree* southwest;
        QuadTree* southeast;

        bool is_leaf() const
        {
            return northwest == nullptr;
        }

        void subdivide()
        {
            real_t cx = real_t(boundary.center.get_x());
            real_t cy = real_t(boundary.center.get_y());
            real_t h = boundary.half_dimension / 2;

            using NumberType = typename PointT::NumberType;

            northwest = new QuadTree(
                Boundary(PointT(NumberType(cx - h), NumberType(cy + h)), h),
                depth + 1);
            northeast = new QuadTree(
                Boundary(PointT(NumberType(cx + h), NumberType(cy + h)), h),
                depth + 1);
            southwest = new QuadTree(
                Boundary(PointT(NumberType(cx - h), NumberType(cy - h)), h),
                depth + 1);
            southeast = new QuadTree(
                Boundary(PointT(NumberType(cx + h), NumberType(cy - h)), h),
                depth + 1);
        }

        void destroy()
        {
            delete northwest;
            delete northeast;
            delete southwest;
            delete southeast;
            northwest = northeast = southwest = southeast = nullptr;
        }

        void copy_from(const QuadTree& t)
        {
            boundary = t.boundary;
            points = t.points;
            depth = t.depth;

            if (t.is_leaf())
            {
                northwest = northeast = southwest = southeast = nullptr;
            }
            else
            {
                northwest = new QuadTree(*t.northwest);
                northeast = new QuadTree(*t.northeast);
                southwest = new QuadTree(*t.southwest);
                southeast = new QuadTree(*t.southeast);
            }
        }

        explicit QuadTree(const Boundary& _boundary, nat_t _depth)
            : boundary(_boundary),
              depth(_depth),
              northwest(nullptr),
              northeast(nullptr),
              southwest(nullptr),
              southeast(nullptr)
        {
            // empty
        }

    public:
        explicit QuadTree(const Boundary& _boundary) : QuadTree(_boundary, 0)
        {
            // empty
        }

        QuadTree(const QuadTree& t)
            : boundary(t.boundary),
              points(t.points),
              depth(t.depth),
              northwest(nullptr),
              northeast(nullptr),
              southwest(nullptr),
              southeast(nullptr)
        {
            copy_from(t);
        }

        QuadTree(QuadTree&& t) : QuadTree(t.boundary, t.depth)
        {
            swap(t);
        }

        ~QuadTree()
        {
            destroy();
        }

        QuadTree& operator=(const QuadTree& t)
        {
            if (this == &t)
            {
                return *this;
            }

            destroy();
            copy_from(t);

            return *this;
        }

        QuadTree& operator=(QuadTree&& t)
        {
            swap(t);
            return *this;
        }

        void swap(QuadTree& t)
        {
            std::swap(boundary, t.boundary);
            points.swap(t.points);
            std::swap(depth, t.depth);
            std::swap(northwest, t.northwest);
            std::swap(northeast, t.northeast);
            std::swap(southwest, t.southwest);
            std::swap(southeast, t.southeast);
        }

        const Boundary& get_boundary() const
        {
            return boundary;
        }

        /** Inserts `p`, returning false without modifying anything if `p`
            falls outside this node's boundary (the top-level caller is
            expected to have picked a boundary covering every point it
            cares about; a false here at the root usually means exactly
            that assumption was violated). */
        bool insert(const PointT& p)
        {
            if (!boundary.contains(p))
            {
                return false;
            }

            if (is_leaf() &&
                (points.size() < NODE_CAPACITY || depth >= MAX_DEPTH))
            {
                points.append(p);
                return true;
            }

            if (is_leaf())
            {
                subdivide();
            }

            return northwest->insert(p) || northeast->insert(p) ||
                   southwest->insert(p) || southeast->insert(p);
        }

        /** Appends every stored point that falls within `range` to `found`
            — prunes whole subtrees via Boundary::intersects() rather than
            checking every point individually, which is the entire reason
            to use a quadtree over a flat array for this query. */
        void query_range(const Boundary& range, DynArray<PointT>& found) const
        {
            if (!boundary.intersects(range))
            {
                return;
            }

            for (nat_t i = 0; i < points.size(); ++i)
            {
                if (range.contains(points[i]))
                {
                    found.append(points[i]);
                }
            }

            if (!is_leaf())
            {
                northwest->query_range(range, found);
                northeast->query_range(range, found);
                southwest->query_range(range, found);
                southeast->query_range(range, found);
            }
        }

        DynArray<PointT> query_range(const Boundary& range) const
        {
            DynArray<PointT> found;
            query_range(range, found);
            return found;
        }

        /** Whether `p` (compared via PointT::operator==, not merely
            boundary containment) is stored anywhere in this node or its
            descendants. A subdivided node's own `points` array can still
            hold points inserted before it subdivided — subdivide() does
            not redistribute them into the new children — so, exactly
            like query_range(), this checks `points` unconditionally
            (leaf or not) rather than only inside the is_leaf() branch. */
        bool search(const PointT& p) const
        {
            if (!boundary.contains(p))
            {
                return false;
            }

            for (nat_t i = 0; i < points.size(); ++i)
            {
                if (points[i] == p)
                {
                    return true;
                }
            }

            if (is_leaf())
            {
                return false;
            }

            return northwest->search(p) || northeast->search(p) ||
                   southwest->search(p) || southeast->search(p);
        }

        /** Removes `p` if present, returning whether it was found. Checks
            this node's own `points` first (see search()'s comment on why
            a non-leaf node can still hold points directly), then
            recurses into the children. After a successful removal from a
            child subtree, this node tries to collapse back into a leaf
            (try_merge()) if all four children are themselves leaves
            whose combined points, together with this node's own, would
            still fit in a single node — the same shape a fresh
            insert()-only tree over the surviving points would have
            settled into, so a tree that has had points removed does not
            carry indefinitely many needlessly-subdivided
            empty/near-empty nodes. */
        bool remove(const PointT& p)
        {
            if (!boundary.contains(p))
            {
                return false;
            }

            for (nat_t i = 0; i < points.size(); ++i)
            {
                if (points[i] == p)
                {
                    points.remove_pos(i);
                    return true;
                }
            }

            if (is_leaf())
            {
                return false;
            }

            bool removed = northwest->remove(p) || northeast->remove(p) ||
                           southwest->remove(p) || southeast->remove(p);

            if (removed)
            {
                try_merge();
            }

            return removed;
        }

        /** Total number of points stored anywhere in this node or its
            descendants (an O(number of nodes) traversal, not O(1) — this
            quadtree does not cache a running count, so callers that need
            the count on a hot path should track it themselves at
            insert()/remove() time). */
        nat_t size() const
        {
            nat_t total = points.size();

            if (!is_leaf())
            {
                total += northwest->size();
                total += northeast->size();
                total += southwest->size();
                total += southeast->size();
            }

            return total;
        }

        bool is_empty() const
        {
            return is_leaf() && points.is_empty();
        }

    private:
        /** Collapses this node's four children back into it (becoming a
            leaf again) if every child is itself already a leaf and their
            combined point count, together with whatever this node
            already holds directly in its own `points` (see search()'s
            comment on why a non-leaf node can hold points of its own),
            still fits within NODE_CAPACITY — mirrors subdivide()'s own
            condition in reverse, so a tree that has had points removed
            converges to the same shape a fresh insert()-only build over
            the surviving points would have. Only ever called right after
            a successful remove() beneath this node, so this node itself
            is never a leaf here. */
        void try_merge()
        {
            if (!northwest->is_leaf() || !northeast->is_leaf() ||
                !southwest->is_leaf() || !southeast->is_leaf())
            {
                return;
            }

            nat_t total = points.size() + northwest->points.size() +
                          northeast->points.size() + southwest->points.size() +
                          southeast->points.size();

            if (total > NODE_CAPACITY)
            {
                return;
            }

            QuadTree* children[4] = {northwest, northeast, southwest,
                                     southeast};

            for (nat_t c = 0; c < 4; ++c)
            {
                for (nat_t i = 0; i < children[c]->points.size(); ++i)
                {
                    points.append(children[c]->points[i]);
                }
            }

            destroy();
        }
    };

} // end namespace Designar
