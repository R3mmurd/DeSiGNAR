/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <arrangement.hpp>

using namespace Designar;

int main()
{
    LineArrangement arr(10.0);
    arr.insert(Point2D(-5, 0), Point2D(5, 0));
    arr.insert(Point2D(0, -5), Point2D(0, 5));
    arr.insert(Point2D(-5, -3), Point2D(5, 3));

    cout << "Lines inserted: " << arr.num_lines() << "\n";
    cout << "Vertices: " << arr.vertices().size() << "\n";

    for (const Point2D& v : arr.vertices())
    {
        cout << "  " << v.to_string() << "\n";
    }

    cout << "Edges: " << arr.edges().size() << "\n";
    cout << "Faces (Euler's-formula upper bound): "
         << arr.num_faces_upper_bound() << "\n";

    return 0;
}
