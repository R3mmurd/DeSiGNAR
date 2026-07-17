/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

#include <iostream>
#include <cassert>
#include <linearalgebra.hpp>

using namespace std;
using namespace Designar;

namespace
{
  bool close(real_t a, real_t b, real_t eps = 1e-9)
  {
    return std::abs(a - b) < eps;
  }
} // end anonymous namespace

int main()
{
  // Vector arithmetic.
  {
    Vector<real_t> u({1.0, 2.0, 3.0});
    Vector<real_t> v({4.0, 5.0, 6.0});

    Vector<real_t> sum = u + v;
    assert(close(sum[0], 5.0) && close(sum[1], 7.0) && close(sum[2], 9.0));

    Vector<real_t> diff = v - u;
    assert(close(diff[0], 3.0) && close(diff[1], 3.0) && close(diff[2], 3.0));

    Vector<real_t> scaled = 2.0 * u;
    assert(close(scaled[0], 2.0) && close(scaled[1], 4.0) && close(scaled[2], 6.0));

    // dot([1,2,3],[4,5,6]) = 4+10+18 = 32
    assert(close(dot(u, v), 32.0));

    // ||[3,4]|| = 5 (3-4-5 triangle)
    Vector<real_t> pyth({3.0, 4.0});
    assert(close(norm(pyth), 5.0));

    cout << "Vector: arithmetic Everything ok!\n";
  }

  // Matrix construction, indexing, identity.
  {
    Matrix<real_t> I = Matrix<real_t>::identity(3);

    for (nat_t i = 0; i < 3; ++i)
    {
      for (nat_t j = 0; j < 3; ++j)
      {
        assert(close(I(i, j), i == j ? 1.0 : 0.0));
      }
    }

    assert(close(I.trace(), 3.0));

    cout << "Matrix: identity/trace Everything ok!\n";
  }

  // Matrix +, -, scalar *, transpose (hand-verified 2x3 example).
  {
    Matrix<real_t> A(2, 3, 0.0);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(0, 2) = 3;
    A(1, 0) = 4;
    A(1, 1) = 5;
    A(1, 2) = 6;

    Matrix<real_t> B(2, 3, 1.0); // all-ones

    Matrix<real_t> sum = A + B;
    assert(close(sum(0, 0), 2) && close(sum(1, 2), 7));

    Matrix<real_t> diff = A - B;
    assert(close(diff(0, 0), 0) && close(diff(1, 2), 5));

    Matrix<real_t> scaled = A * 2.0;
    assert(close(scaled(0, 0), 2) && close(scaled(1, 2), 12));

    Matrix<real_t> At = A.transpose();
    assert(At.num_rows() == 3 && At.num_cols() == 2);

    for (nat_t i = 0; i < 2; ++i)
    {
      for (nat_t j = 0; j < 3; ++j)
      {
        assert(close(At(j, i), A(i, j)));
      }
    }

    cout << "Matrix: +/-/scalar*/transpose Everything ok!\n";
  }

  // Matrix * Matrix and Matrix * Vector (hand-verified 2x2 examples).
  {
    // A = [[1,2],[3,4]], B = [[5,6],[7,8]]
    // A*B = [[1*5+2*7, 1*6+2*8], [3*5+4*7, 3*6+4*8]] = [[19,22],[43,50]]
    Matrix<real_t> A(2, 2, 0.0);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;

    Matrix<real_t> B(2, 2, 0.0);
    B(0, 0) = 5;
    B(0, 1) = 6;
    B(1, 0) = 7;
    B(1, 1) = 8;

    Matrix<real_t> AB = A * B;
    assert(close(AB(0, 0), 19) && close(AB(0, 1), 22));
    assert(close(AB(1, 0), 43) && close(AB(1, 1), 50));

    // A * identity == A
    assert(A * Matrix<real_t>::identity(2) == A);

    // A * [1,1] = [1+2, 3+4] = [3,7]
    Vector<real_t> v({1.0, 1.0});
    Vector<real_t> Av = A * v;
    assert(close(Av[0], 3) && close(Av[1], 7));

    cout << "Matrix: matrix*matrix, matrix*vector Everything ok!\n";
  }

  // determinant: hand-computable 2x2 and 3x3 examples.
  {
    // det([[1,2],[3,4]]) = 1*4 - 2*3 = -2
    Matrix<real_t> A(2, 2, 0.0);
    A(0, 0) = 1;
    A(0, 1) = 2;
    A(1, 0) = 3;
    A(1, 1) = 4;
    assert(close(determinant(A), -2.0));

    // det(I_4) = 1
    assert(close(determinant(Matrix<real_t>::identity(4)), 1.0));

    // A classic singular matrix (rows 2x the first): det = 0
    Matrix<real_t> singular(2, 2, 0.0);
    singular(0, 0) = 1;
    singular(0, 1) = 2;
    singular(1, 0) = 2;
    singular(1, 1) = 4;
    assert(close(determinant(singular), 0.0));

    // A well-known 3x3 example: det([[6,1,1],[4,-2,5],[2,8,7]]) = -306
    Matrix<real_t> M3(3, 3, 0.0);
    M3(0, 0) = 6;
    M3(0, 1) = 1;
    M3(0, 2) = 1;
    M3(1, 0) = 4;
    M3(1, 1) = -2;
    M3(1, 2) = 5;
    M3(2, 0) = 2;
    M3(2, 1) = 8;
    M3(2, 2) = 7;
    assert(close(determinant(M3), -306.0));

    cout << "determinant: hand-verified 2x2/3x3 examples Everything ok!\n";
  }

  // solve: a hand-verifiable 2x2 system.
  {
    // 2x + y = 5
    //  x + y = 3
    // => x = 2, y = 1
    Matrix<real_t> A(2, 2, 0.0);
    A(0, 0) = 2;
    A(0, 1) = 1;
    A(1, 0) = 1;
    A(1, 1) = 1;

    Vector<real_t> b({5.0, 3.0});
    Vector<real_t> x = solve(A, b);

    assert(close(x[0], 2.0));
    assert(close(x[1], 1.0));

    bool threw = false;

    try
    {
      Matrix<real_t> singular(2, 2, 0.0);
      singular(0, 0) = 1;
      singular(0, 1) = 2;
      singular(1, 0) = 2;
      singular(1, 1) = 4;
      solve(singular, b);
    }
    catch (const domain_error&)
    {
      threw = true;
    }

    assert(threw);

    cout << "solve: hand-verified 2x2 system + singular detection Everything ok!\n";
  }

  // inverse: A * inverse(A) == I, on a hand-verifiable 2x2 example.
  {
    // inverse([[4,7],[2,6]]) = 1/10 * [[6,-7],[-2,4]]
    Matrix<real_t> A(2, 2, 0.0);
    A(0, 0) = 4;
    A(0, 1) = 7;
    A(1, 0) = 2;
    A(1, 1) = 6;

    Matrix<real_t> Ainv = inverse(A);
    assert(close(Ainv(0, 0), 0.6));
    assert(close(Ainv(0, 1), -0.7));
    assert(close(Ainv(1, 0), -0.2));
    assert(close(Ainv(1, 1), 0.4));

    Matrix<real_t> product = A * Ainv;

    for (nat_t i = 0; i < 2; ++i)
    {
      for (nat_t j = 0; j < 2; ++j)
      {
        assert(close(product(i, j), i == j ? 1.0 : 0.0));
      }
    }

    cout << "inverse: hand-verified 2x2 example, A*inverse(A)==I Everything ok!\n";
  }

  // Dimension-mismatch and shape errors are reported, not silently
  // miscomputed or crashed on.
  {
    bool threw = false;

    try
    {
      Matrix<real_t> a(2, 3, 0.0);
      Matrix<real_t> b(2, 3, 0.0);
      Matrix<real_t> product = a * b; // 3 != 2, invalid
    }
    catch (const domain_error&)
    {
      threw = true;
    }

    assert(threw);

    threw = false;

    try
    {
      Matrix<real_t> a(2, 3, 0.0);
      a.trace(); // not square
    }
    catch (const domain_error&)
    {
      threw = true;
    }

    assert(threw);

    cout << "Matrix: dimension-mismatch error checking Everything ok!\n";
  }

  cout << "Everything ok!\n";
  return 0;
}
