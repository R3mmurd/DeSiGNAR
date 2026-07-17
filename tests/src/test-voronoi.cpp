/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <voronoi.hpp>
#include <random.hpp>

using namespace std;
using namespace Designar;

namespace
{
  real_t sq_dist(const Point2D& a, const Point2D& b)
  {
    real_t dx = a.get_x() - b.get_x();
    real_t dy = a.get_y() - b.get_y();
    return dx * dx + dy * dy;
  }
} // end anonymous namespace

int main()
{
  // Two sites: the cell boundary must be exactly the perpendicular
  // bisector, so every vertex of each cell should be (within floating
  // point tolerance) equidistant from both sites or strictly closer to
  // its own — never strictly closer to the other one.
  {
    DynArray<Point2D> sites = {Point2D(-10., 0.), Point2D(10., 0.)};

    VoronoiDiagram<Point2D> diagram(sites, 100.);
    auto cells = diagram.compute();

    assert(cells.size() == 2);

    for (nat_t c = 0; c < cells.size(); ++c)
    {
      const auto& cell = cells[c];
      const Point2D& own = cell.site;
      const Point2D& other = sites[c == 0 ? 1 : 0];

      for (nat_t v = 0; v < cell.polygon.size(); ++v)
      {
        real_t d_own = sq_dist(cell.polygon[v], own);
        real_t d_other = sq_dist(cell.polygon[v], other);

        assert(d_own <= d_other + 1e-6);
      }
    }

    cout << "VoronoiDiagram: two sites Everything ok!\n";
  }

  // The fundamental correctness property for a random point set: every
  // vertex of every cell must be at least as close to its own site as
  // to every other site (that's exactly what half-plane clipping
  // against every other site's bisector is supposed to guarantee).
  {
    rng_t rng(2024);
    DynArray<Point2D> sites;

    for (int_t i = 0; i < 25; ++i)
    {
      sites.append(Point2D(real_t(random_uniform(rng, 200)) - 100.,
                           real_t(random_uniform(rng, 200)) - 100.));
    }

    VoronoiDiagram<Point2D> diagram(sites, 500.);
    auto cells = diagram.compute();

    assert(cells.size() == sites.size());

    for (nat_t c = 0; c < cells.size(); ++c)
    {
      const auto& cell = cells[c];
      assert(!cell.polygon.is_empty());

      for (nat_t v = 0; v < cell.polygon.size(); ++v)
      {
        real_t d_own = sq_dist(cell.polygon[v], cell.site);

        for (nat_t other = 0; other < sites.size(); ++other)
        {
          if (other == c)
          {
            continue;
          }

          real_t d_other = sq_dist(cell.polygon[v], sites[other]);
          assert(d_own <= d_other + 1e-6);
        }
      }
    }

    cout << "VoronoiDiagram: random sites, cell-boundary invariant Everything ok!\n";
  }

  // Fewer than two sites must be rejected outright (a Voronoi diagram
  // over a single site is degenerate — the whole plane).
  {
    bool threw = false;

    try
    {
      DynArray<Point2D> one_site = {Point2D(0., 0.)};
      VoronoiDiagram<Point2D> diagram(one_site, 10.);
    }
    catch (const domain_error&)
    {
      threw = true;
    }

    assert(threw);

    cout << "VoronoiDiagram: rejects fewer than two sites Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
