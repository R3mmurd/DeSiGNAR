/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file point3d.hpp
    @brief A minimal 3D point plus a plane (as a point and an outward
    normal), added only in support of point_in_polyhedron() — the rest
    of the geometry module (point2D.hpp/segment.hpp/polygon.hpp/
    triangle.hpp) is entirely 2D, and this file does not try to grow
    that into a full 3D geometry subsystem.
    @ingroup Geometry
*/

#pragma once

#include <array.hpp>
#include <math.hpp>

namespace Designar
{
    /** A point (or, interchangeably, a free vector) in R^3. */
    class Point3D
    {
        real_t x;
        real_t y;
        real_t z;

    public:
        Point3D() : x(0), y(0), z(0)
        {
            // empty
        }

        Point3D(real_t _x, real_t _y, real_t _z) : x(_x), y(_y), z(_z)
        {
            // empty
        }

        real_t get_x() const { return x; }
        real_t get_y() const { return y; }
        real_t get_z() const { return z; }

        Point3D operator-(const Point3D& p) const
        {
            return Point3D(x - p.x, y - p.y, z - p.z);
        }

        Point3D operator+(const Point3D& p) const
        {
            return Point3D(x + p.x, y + p.y, z + p.z);
        }

        real_t dot(const Point3D& p) const
        {
            return x * p.x + y * p.y + z * p.z;
        }

        Point3D cross(const Point3D& p) const
        {
            return Point3D(y * p.z - z * p.y, z * p.x - x * p.z,
                           x * p.y - y * p.x);
        }

        real_t norm() const
        {
            return std::sqrt(dot(*this));
        }
    };

    /** A plane given by a point on it plus an outward-pointing normal
        (not required to be unit length: only the *sign* of
        signed_distance() below is ever used). */
    class Plane
    {
        Point3D point;
        Point3D normal;

    public:
        Plane(const Point3D& _point, const Point3D& _normal)
            : point(_point), normal(_normal)
        {
            // empty
        }

        /** Builds the plane through three points, with the normal
            oriented by the right-hand rule from (p2 - p1) x (p3 - p1) —
            i.e. "outward" is whichever side that cross product points
            to, which callers must set up consistently (e.g. every face
            of a convex polyhedron listed with vertices ordered so this
            normal points away from the polyhedron's interior). */
        static Plane from_points(const Point3D& p1, const Point3D& p2,
                                 const Point3D& p3)
        {
            return Plane(p1, (p2 - p1).cross(p3 - p1));
        }

        const Point3D& get_point() const { return point; }
        const Point3D& get_normal() const { return normal; }

        /** Positive on the side the normal points to, negative on the
            other side, zero exactly on the plane. */
        real_t signed_distance(const Point3D& p) const
        {
            return normal.dot(p - point);
        }
    };

    /** Whether `p` lies inside (or on the boundary of) the convex
        polyhedron described as an intersection of half-spaces, one per
        `faces` entry, each built so its normal points outward — `p` is
        inside iff it is on the non-positive side of every one of them.
        Only meaningful for a convex polyhedron: a non-convex shape is
        not, in general, expressible as such an intersection at all. */
    inline bool point_in_polyhedron(const DynArray<Plane>& faces,
                                    const Point3D& p)
    {
        return faces.all(
            [&p](const Plane& plane) { return plane.signed_distance(p) <= EPSILON; });
    }

} // end namespace Designar
