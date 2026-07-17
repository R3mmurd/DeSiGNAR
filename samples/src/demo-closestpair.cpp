/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <closestpair.hpp>

using namespace Designar;

int main()
{
    DynArray<Point2D> points = {Point2D(0., 0.), Point2D(10., 10.),
                                Point2D(0.1, 0.1), Point2D(5., 5.)};

    auto result = ClosestPair<Point2D>::compute(points);

    cout << "closest pair: (" << result.p1.get_x() << ", " << result.p1.get_y()
         << ") and (" << result.p2.get_x() << ", " << result.p2.get_y() << ")"
         << endl;
    cout << "distance: " << result.distance << endl;

    return 0;
}
