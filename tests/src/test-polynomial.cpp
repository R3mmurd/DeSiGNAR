/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <cassert>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <polynomial.hpp>

using namespace std;
using namespace Designar;

namespace
{
    DynArray<int_t> naive_multiply(const DynArray<int_t>& a,
                                   const DynArray<int_t>& b)
    {
        DynArray<int_t> result(a.size() + b.size() - 1, int_t(0));

        for (nat_t i = 0; i < a.size(); ++i)
        {
            for (nat_t j = 0; j < b.size(); ++j)
            {
                result[i + j] += a[i] * b[j];
            }
        }

        return result;
    }
} // end anonymous namespace

int main()
{
    // multiply_polynomials must match the naive O(n^2) convolution.
    {
        srand(7);

        for (int trial = 0; trial < 20; ++trial)
        {
            nat_t na = 1 + nat_t(rand() % 30);
            nat_t nb = 1 + nat_t(rand() % 30);

            DynArray<int_t> a(na, int_t(0));
            DynArray<int_t> b(nb, int_t(0));

            for (nat_t i = 0; i < na; ++i)
            {
                a[i] = int_t(rand() % 21) - 10;
            }

            for (nat_t i = 0; i < nb; ++i)
            {
                b[i] = int_t(rand() % 21) - 10;
            }

            DynArray<int_t> naive = naive_multiply(a, b);
            DynArray<int_t> fast = multiply_polynomials(a, b);

            assert(naive.size() == fast.size());

            for (nat_t i = 0; i < naive.size(); ++i)
            {
                assert(naive[i] == fast[i]);
            }
        }
    }

    // fft/ifft round-trip.
    {
        DynArray<Complex> data(8, Complex());

        for (nat_t i = 0; i < 8; ++i)
        {
            data[i] = Complex(real_t(i + 1), 0);
        }

        DynArray<Complex> original = data;
        fft(data);
        ifft(data);

        for (nat_t i = 0; i < 8; ++i)
        {
            assert(std::abs(data[i].real() - original[i].real()) < 1e-6);
            assert(std::abs(data[i].imag() - original[i].imag()) < 1e-6);
        }
    }

    // fft on a non-power-of-two size must throw.
    {
        bool threw = false;

        try
        {
            DynArray<Complex> bad(3, Complex());
            fft(bad);
        }
        catch (const std::domain_error&)
        {
            threw = true;
        }

        assert(threw);
    }

    // Polynomial: evaluate() via Horner's method.
    {
        Polynomial<real_t> p(DynArray<real_t>({1.0, 2.0, 3.0})); // 1+2x+3x^2
        assert(std::abs(p.evaluate(2.0) - 17.0) < 1e-9);         // 1+4+12
        assert(p.degree() == 2);
    }

    // Polynomial: operator+ and operator* (FFT-based).
    {
        Polynomial<real_t> one_plus_x(DynArray<real_t>({1.0, 1.0}));
        Polynomial<real_t> squared = one_plus_x * one_plus_x;

        assert(squared.degree() == 2);
        assert(std::abs(squared[0] - 1.0) < 1e-6);
        assert(std::abs(squared[1] - 2.0) < 1e-6);
        assert(std::abs(squared[2] - 1.0) < 1e-6);

        // (1 + x) + (1 + 2x + x^2) = 2 + 3x + x^2
        Polynomial<real_t> sum = one_plus_x + squared;
        assert(sum.degree() == 2);
        assert(std::abs(sum[0] - 2.0) < 1e-6);
        assert(std::abs(sum[1] - 3.0) < 1e-6);
        assert(std::abs(sum[2] - 1.0) < 1e-6);
    }

    // Trailing-zero coefficients are trimmed: (1 - x) * (1 + x) = 1 - x^2,
    // whose x^1 coefficient is exactly zero.
    {
        Polynomial<real_t> one_minus_x(DynArray<real_t>({1.0, -1.0}));
        Polynomial<real_t> one_plus_x(DynArray<real_t>({1.0, 1.0}));
        Polynomial<real_t> product = one_minus_x * one_plus_x;

        assert(product.degree() == 2);
        assert(std::abs(product[0] - 1.0) < 1e-6);
        assert(std::abs(product[1]) < 1e-6);
        assert(std::abs(product[2] - (-1.0)) < 1e-6);
    }

    cout << "Everything ok!\n";

    return 0;
}
