/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>

#include <point3d.hpp>

using namespace Designar;

int main()
{
    // A unit cube [0,1]^3 as 6 outward-facing half-spaces.
    DynArray<Plane> cube = {
        Plane(Point3D(0, 0, 0), Point3D(-1, 0, 0)),
        Plane(Point3D(1, 0, 0), Point3D(1, 0, 0)),
        Plane(Point3D(0, 0, 0), Point3D(0, -1, 0)),
        Plane(Point3D(0, 1, 0), Point3D(0, 1, 0)),
        Plane(Point3D(0, 0, 0), Point3D(0, 0, -1)),
        Plane(Point3D(0, 0, 1), Point3D(0, 0, 1))};

    assert(point_in_polyhedron(cube, Point3D(0.5, 0.5, 0.5)));
    assert(point_in_polyhedron(cube, Point3D(0, 0, 0))); // on the boundary
    assert(!point_in_polyhedron(cube, Point3D(1.5, 0.5, 0.5)));
    assert(!point_in_polyhedron(cube, Point3D(-0.1, 0.5, 0.5)));

    // Plane::from_points() must orient the normal via the right-hand
    // rule: (p2-p1) x (p3-p1).
    Plane p = Plane::from_points(Point3D(0, 0, 0), Point3D(1, 0, 0),
                                 Point3D(0, 1, 0));
    assert(std::abs(p.signed_distance(Point3D(0, 0, 1)) - 1.0) < 1e-9);
    assert(std::abs(p.signed_distance(Point3D(0, 0, -1)) + 1.0) < 1e-9);

    // Point3D arithmetic sanity checks.
    Point3D a(1, 0, 0);
    Point3D b(0, 1, 0);
    Point3D c = a.cross(b);
    assert(std::abs(c.get_z() - 1.0) < 1e-9);
    assert(std::abs(a.dot(b)) < 1e-9);
    assert(std::abs(a.norm() - 1.0) < 1e-9);

    return 0;
}
