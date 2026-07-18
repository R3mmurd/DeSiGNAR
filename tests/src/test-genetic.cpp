/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <genetic.hpp>
#include <linearalgebra.hpp>

using namespace Designar;

int main()
{
    // Sphere function sum(x_i^2) over R^3, minimum 0 at the origin,
    // maximized via its negation. Fixed seed so this is deterministic.
    constexpr nat_t DIM = 3;
    constexpr real_t LO = -5.0;
    constexpr real_t HI = 5.0;

    auto init_fn = [&](rng_t& rng)
    {
        Vector<real_t> v(DIM, 0.0);

        for (nat_t i = 0; i < DIM; ++i)
        {
            v[i] = random_uniform<real_t>(rng, LO, HI);
        }

        return v;
    };

    auto fitness_fn = [](const Vector<real_t>& v)
    {
        real_t s = 0;

        for (nat_t i = 0; i < v.size(); ++i)
        {
            s += v[i] * v[i];
        }

        return -s;
    };

    auto crossover_fn = [](const Vector<real_t>& a, const Vector<real_t>& b,
                           rng_t& rng)
    {
        Vector<real_t> child(a.size(), 0.0);

        for (nat_t i = 0; i < a.size(); ++i)
        {
            real_t t = random(rng);
            child[i] = t * a[i] + (1 - t) * b[i];
        }

        return child;
    };

    auto mutate_fn = [](Vector<real_t>& v, rng_t& rng)
    {
        nat_t i = random_uniform<nat_t>(rng, v.size());
        v[i] += random_uniform<real_t>(rng, -0.5, 0.5);
    };

    auto ga = make_genetic_algorithm<Vector<real_t>>(
        100, 0.8, 0.2, 3, 20260718, init_fn, fitness_fn, crossover_fn,
        mutate_fn);

    assert(ga.generation() == 0);
    assert(ga.population_view().size() == 100);

    ga.run(300);

    assert(ga.generation() == 300);
    // Loose enough not to be flaky across compilers/platforms, tight
    // enough that a broken selection/crossover/mutation step would fail
    // it (a correctly-working GA converges to within 1e-6 of 0 on this
    // easy a benchmark well before 300 generations).
    assert(ga.best_fitness() > -0.01);
    assert(ga.best_individual().size() == DIM);

    // Elitism: best-ever fitness must never regress across generations.
    auto ga2 = make_genetic_algorithm<Vector<real_t>>(
        50, 0.8, 0.3, 2, 7, init_fn, fitness_fn, crossover_fn, mutate_fn);

    real_t prev_best = ga2.best_fitness();

    for (nat_t g = 0; g < 100; ++g)
    {
        ga2.step();
        assert(ga2.best_fitness() >= prev_best);
        prev_best = ga2.best_fitness();
    }

    return 0;
}
