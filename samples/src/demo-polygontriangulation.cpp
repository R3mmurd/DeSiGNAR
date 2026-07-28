/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <polygontriangulation.hpp>

using namespace Designar;

namespace
{
    void print_triangles(const DynArray<PolygonTriangle>& tris)
    {
        for (const PolygonTriangle& t : tris)
        {
            cout << "  (" << t.a << ", " << t.b << ", " << t.c << ")\n";
        }
    }
} // end anonymous namespace

int main()
{
    // A polygon with a reflex ("notch") vertex, so both triangulation
    // techniques have something nontrivial to do.
    DynArray<Point2D> comb = {Point2D(0, 0), Point2D(4, 0), Point2D(4, 4),
                              Point2D(3, 4), Point2D(2, 1), Point2D(1, 4),
                              Point2D(0, 4)};

    cout << "Ear-clipping triangulation:\n";
    print_triangles(ear_clipping_triangulation(comb));

    cout << "\nMonotone-decomposition triangulation:\n";
    print_triangles(monotone_triangulation(comb));

    cout << "\nMonotone pieces:\n";

    for (const DynArray<nat_t>& piece : monotone_partition(comb))
    {
        cout << "  {";

        for (nat_t i = 0; i < piece.size(); ++i)
        {
            cout << piece[i] << (i + 1 < piece.size() ? ", " : "");
        }

        cout << "}\n";
    }

    DynArray<nat_t> guards = art_gallery_guards(comb);
    cout << "\nArt gallery guards (" << guards.size() << " of "
         << comb.size() << " vertices): {";

    for (nat_t i = 0; i < guards.size(); ++i)
    {
        cout << guards[i] << (i + 1 < guards.size() ? ", " : "");
    }

    cout << "}\n";

    return 0;
}
