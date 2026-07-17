/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <quadtree.hpp>

using namespace Designar;

int main()
{
    QuadTree<Point2D> qt(QuadTree<Point2D>::Boundary(Point2D(0., 0.), 100.));

    qt.insert(Point2D(10., 10.));
    qt.insert(Point2D(-20., 30.));
    qt.insert(Point2D(50., -50.));
    qt.insert(Point2D(15., 12.));

    cout << "total points: " << qt.size() << endl;

    QuadTree<Point2D>::Boundary region(Point2D(12., 11.), 10.);
    auto found = qt.query_range(region);

    cout << "points within 10 units of (12, 11): ";

    for (const Point2D& p : found)
    {
        cout << "(" << p.get_x() << ", " << p.get_y() << ") ";
    }

    cout << endl;

    Point2D target(10., 10.);
    cout << "search(10, 10): " << (qt.search(target) ? "found" : "not found")
         << endl;

    qt.remove(target);
    cout << "after remove(10, 10), search(10, 10): "
         << (qt.search(target) ? "found" : "not found") << endl;
    cout << "total points: " << qt.size() << endl;

    return 0;
}
