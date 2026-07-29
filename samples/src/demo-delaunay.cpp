/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <delaunay.hpp>
#include <random.hpp>

using namespace Designar;

int main()
{
    rng_t rng(get_random_seed());
    DynArray<Point2D> points;

    for (nat_t i = 0; i < 10; ++i)
    {
        points.append(
            Point2D(random_uniform(rng, 100.0), random_uniform(rng, 100.0)));
    }

    DynArray<DelaunayTriangle> tris = delaunay_triangulation(points);

    cout << "Delaunay triangulation of " << points.size()
         << " random points: " << tris.size() << " triangles\n";

    for (const DelaunayTriangle& t : tris)
    {
        cout << "  (" << t.a << ", " << t.b << ", " << t.c << ")\n";
    }

    return 0;
}
