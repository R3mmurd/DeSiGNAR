/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <closestpair.hpp>
#include <random.hpp>

using namespace Designar;
using namespace std;

int main()
{
  // closest pair: small hand-verifiable case
  DynArray<Point2D> pts = {
      Point2D(0., 0.), Point2D(10., 10.), Point2D(0.1, 0.1), Point2D(5., 5.)};
  auto result = ClosestPair<Point2D>::compute(pts);
  assert(num_equal(result.distance, Point2D(0., 0.).distance_with(Point2D(0.1, 0.1))));

  // cross-check against brute force on random point sets
  rng_t rng(4242);
  for (int_t trial = 0; trial < 100; ++trial)
  {
    nat_t n = random_uniform(rng, 40) + 2;
    DynArray<Point2D> random_pts;
    for (nat_t i = 0; i < n; ++i)
      random_pts.append(Point2D(real_t(random_uniform(rng, 1000)),
                                real_t(random_uniform(rng, 1000))));

    real_t brute_best = std::numeric_limits<real_t>::max();
    for (nat_t i = 0; i < random_pts.size(); ++i)
      for (nat_t j = i + 1; j < random_pts.size(); ++j)
        brute_best = std::min(brute_best, random_pts[i].distance_with(random_pts[j]));

    auto dc_result = ClosestPair<Point2D>::compute(random_pts);
    assert(num_equal(dc_result.distance, brute_best));
  }

  cout << "ClosestPair: Everything ok!\n";
  return 0;
}
