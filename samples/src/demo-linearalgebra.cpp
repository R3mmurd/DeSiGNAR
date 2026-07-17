/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>

using namespace std;

#include <linearalgebra.hpp>

using namespace Designar;

namespace
{
    void print_matrix(const Matrix<real_t>& m)
    {
        for (nat_t i = 0; i < m.num_rows(); ++i)
        {
            for (nat_t j = 0; j < m.num_cols(); ++j)
            {
                cout << m(i, j) << " ";
            }

            cout << endl;
        }
    }
} // end anonymous namespace

int main()
{
    Matrix<real_t> A(2, 2, 0.0);
    A(0, 0) = 4;
    A(0, 1) = 7;
    A(1, 0) = 2;
    A(1, 1) = 6;

    cout << "A:\n";
    print_matrix(A);

    cout << "\ndeterminant(A) = " << determinant(A) << endl;

    Matrix<real_t> Ainv = inverse(A);
    cout << "\ninverse(A):\n";
    print_matrix(Ainv);

    cout << "\nA * inverse(A) (should be the identity):\n";
    print_matrix(A * Ainv);

    // Solve 2x + y = 5, x + y = 3.
    Matrix<real_t> S(2, 2, 0.0);
    S(0, 0) = 2;
    S(0, 1) = 1;
    S(1, 0) = 1;
    S(1, 1) = 1;

    Vector<real_t> b({5.0, 3.0});
    Vector<real_t> x = solve(S, b);

    cout << "\nSolving 2x + y = 5, x + y = 3:\n";
    cout << "x = " << x[0] << ", y = " << x[1] << endl;

    Vector<real_t> u({1.0, 2.0, 3.0});
    Vector<real_t> v({4.0, 5.0, 6.0});
    cout << "\ndot([1,2,3], [4,5,6]) = " << dot(u, v) << endl;
    cout << "norm([3,4]) = " << norm(Vector<real_t>({3.0, 4.0})) << endl;

    return 0;
}
