/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

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

    DynArray<Point3D> samples = {Point3D(0.5, 0.5, 0.5), Point3D(0, 0, 0),
                                 Point3D(1.5, 0.5, 0.5),
                                 Point3D(-0.1, 0.5, 0.5)};

    for (const Point3D& p : samples)
    {
        cout << "(" << p.get_x() << ", " << p.get_y() << ", " << p.get_z()
             << ") is "
             << (point_in_polyhedron(cube, p) ? "inside" : "outside")
             << " the unit cube\n";
    }

    return 0;
}
