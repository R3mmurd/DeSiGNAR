/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <polygonpartition.hpp>
#include <polygontriangulation.hpp>
#include <random.hpp>

using namespace Designar;

int main()
{
    DynArray<Point2D> plus = {
        Point2D(1, 0), Point2D(2, 0), Point2D(2, 1), Point2D(3, 1),
        Point2D(3, 2), Point2D(2, 2), Point2D(2, 3), Point2D(1, 3),
        Point2D(1, 2), Point2D(0, 2), Point2D(0, 1), Point2D(1, 1)};

    cout << "Monotone-partition diagonals of a plus-shaped polygon:\n";

    for (const auto& d : monotone_partition_diagonals(plus))
    {
        cout << "  " << d.first << " -- " << d.second << "\n";
    }

    cout << "\nConvex partition (via merged ear-clipping triangles):\n";

    DynArray<PolygonTriangle> tris = ear_clipping_triangulation(plus);
    DynArray<DynArray<nat_t>> pieces =
        convex_partition_from_triangulation(plus, tris);

    for (const DynArray<nat_t>& piece : pieces)
    {
        cout << "  {";

        for (nat_t i = 0; i < piece.size(); ++i)
        {
            cout << piece[i] << (i + 1 < piece.size() ? ", " : "");
        }

        cout << "}\n";
    }

    cout << "\nTrapezoidalization:\n";

    rng_t rng(get_random_seed());
    DynArray<Trapezoid> traps = trapezoidalize(plus, rng);

    for (const Trapezoid& t : traps)
    {
        cout << "  at x = " << t.left_x << ": top "
             << t.top.get_src_point().to_string() << "-"
             << t.top.get_tgt_point().to_string() << " / bottom "
             << t.bottom.get_src_point().to_string() << "-"
             << t.bottom.get_tgt_point().to_string() << "\n";
    }

    return 0;
}
