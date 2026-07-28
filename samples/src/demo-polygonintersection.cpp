/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <polygonintersection.hpp>

using namespace Designar;

namespace
{
    void print_polygon(const DynArray<Point2D>& p)
    {
        cout << "  {";

        for (nat_t i = 0; i < p.size(); ++i)
        {
            cout << p[i].to_string() << (i + 1 < p.size() ? ", " : "");
        }

        cout << "}\n";
    }
} // end anonymous namespace

int main()
{
    DynArray<Point2D> s1 = {Point2D(0, 0), Point2D(2, 0), Point2D(2, 2),
                            Point2D(0, 2)};
    DynArray<Point2D> s2 = {Point2D(1, 1), Point2D(3, 1), Point2D(3, 3),
                            Point2D(1, 3)};

    cout << "Convex/convex intersection of two overlapping squares:\n";
    print_polygon(convex_polygon_intersection(s1, s2));

    DynArray<Point2D> l_shape = {Point2D(0, 0), Point2D(3, 0), Point2D(3, 1),
                                Point2D(1, 1), Point2D(1, 3), Point2D(0, 3)};
    DynArray<Point2D> square = {Point2D(0, 0), Point2D(2, 0), Point2D(2, 2),
                                Point2D(0, 2)};

    cout << "\nNon-convex L-shape intersected with a square (as convex "
            "pieces):\n";

    for (const DynArray<Point2D>& piece : polygon_intersection(l_shape, square))
    {
        print_polygon(piece);
    }

    return 0;
}
