/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <genetic.hpp>
#include <linearalgebra.hpp>

using namespace Designar;

int main()
{
    // Minimize the sphere function sum(x_i^2) over R^3 (optimum: the
    // origin, value 0) by maximizing its negation.
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

    ga.run(300);

    cout << "Best fitness after " << ga.generation()
         << " generations: " << ga.best_fitness() << endl;
    cout << "Best individual: ";

    for (real_t v : ga.best_individual())
    {
        cout << v << " ";
    }

    cout << endl;

    return 0;
}
