/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <voronoi.hpp>

using namespace Designar;

int main()
{
    DynArray<Point2D> sites = {Point2D(-10., 0.), Point2D(10., 0.),
                               Point2D(0., 15.)};

    VoronoiDiagram<Point2D> diagram(sites, 50.);
    auto cells = diagram.compute();

    for (const auto& cell : cells)
    {
        cout << "site (" << cell.site.get_x() << ", " << cell.site.get_y()
             << ") cell has " << cell.polygon.size() << " vertices" << endl;
    }

    return 0;
}
