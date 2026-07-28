/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <polynomial.hpp>

using namespace Designar;

namespace
{
    void print_poly(const Polynomial<real_t>& p)
    {
        bool first = true;

        for (nat_t i = p.degree() + 1; i-- > 0;)
        {
            if (!(p[i] < 0.0) && !(0.0 < p[i]) && p.degree() > 0)
            {
                continue;
            }

            if (!first)
            {
                cout << " + ";
            }

            cout << p[i];

            if (i == 1)
            {
                cout << "x";
            }
            else if (i > 1)
            {
                cout << "x^" << i;
            }

            first = false;
        }

        cout << endl;
    }
} // end anonymous namespace

int main()
{
    // (1 + 2x) * (3 + 4x + 5x^2), via FFT-based multiplication.
    Polynomial<real_t> a(DynArray<real_t>({1.0, 2.0}));
    Polynomial<real_t> b(DynArray<real_t>({3.0, 4.0, 5.0}));
    Polynomial<real_t> product = a * b;

    cout << "a(x) = ";
    print_poly(a);
    cout << "b(x) = ";
    print_poly(b);
    cout << "a(x) * b(x) = ";
    print_poly(product);

    cout << "\na(2) = " << a.evaluate(2.0) << endl;
    cout << "product(2) = " << product.evaluate(2.0)
         << " (should equal a(2) * b(2) = " << a.evaluate(2.0) * b.evaluate(2.0)
         << ")" << endl;

    // multiply_polynomials directly on coefficient arrays, without the
    // Polynomial wrapper.
    DynArray<int_t> p1 = {1, 1, 1}; // 1 + x + x^2
    DynArray<int_t> p2 = {1, -1};  // 1 - x
    DynArray<int_t> result = multiply_polynomials(p1, p2);

    cout << "\n(1 + x + x^2)(1 - x) = ";

    for (nat_t i = 0; i < result.size(); ++i)
    {
        cout << result[i] << " ";
    }

    cout << "(coefficients, ascending degree)" << endl;

    return 0;
}
