/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file linearalgebra.hpp
    @brief Basic linear algebra support: `Vector<T>` (a plain alias
    for `DynArray<T>`, plus free-function dot/norm/elementwise
    operators) and `Matrix<T>` (a dense, row-major matrix supporting
    addition, multiplication, transpose, and trace), together with
    `determinant()`, `solve()` (a linear system via Gaussian
    elimination with partial pivoting), and `inverse()` (built on top
    of `solve()`, one identity column at a time). These last three
    are templated but only meaningful for a field-like `T` (division
    must make sense) — use `real_t` unless you have a specific reason
    not to; `Matrix<T>`'s own arithmetic (`+`, `-`, `*`, transpose,
    trace) has no such restriction and works for any `T` a `DynArray`
    would accept.
    @ingroup LinearAlgebra
*/

#pragma once

#include <cmath>
#include <stdexcept>

#include <array.hpp>

namespace Designar
{
    /** `Vector<T>` is nothing more than `DynArray<T>` under a
        domain-specific name — the arithmetic below is what actually
        makes it usable as a mathematical vector rather than just a
        resizable array. */
    template <typename T>
    using Vector = DynArray<T>;

    template <typename T>
    Vector<T> operator+(const Vector<T>& u, const Vector<T>& v)
    {
        if (u.size() != v.size())
        {
            throw std::domain_error("Vector::operator+: size mismatch");
        }

        Vector<T> result(u.size(), T());

        for (nat_t i = 0; i < u.size(); ++i)
        {
            result[i] = u[i] + v[i];
        }

        return result;
    }

    template <typename T>
    Vector<T> operator-(const Vector<T>& u, const Vector<T>& v)
    {
        if (u.size() != v.size())
        {
            throw std::domain_error("Vector::operator-: size mismatch");
        }

        Vector<T> result(u.size(), T());

        for (nat_t i = 0; i < u.size(); ++i)
        {
            result[i] = u[i] - v[i];
        }

        return result;
    }

    template <typename T>
    Vector<T> operator*(const T& scalar, const Vector<T>& v)
    {
        Vector<T> result(v.size(), T());

        for (nat_t i = 0; i < v.size(); ++i)
        {
            result[i] = scalar * v[i];
        }

        return result;
    }

    template <typename T>
    Vector<T> operator*(const Vector<T>& v, const T& scalar)
    {
        return scalar * v;
    }

    template <typename T>
    T dot(const Vector<T>& u, const Vector<T>& v)
    {
        if (u.size() != v.size())
        {
            throw std::domain_error("dot: size mismatch");
        }

        T result = T();

        for (nat_t i = 0; i < u.size(); ++i)
        {
            result += u[i] * v[i];
        }

        return result;
    }

    template <typename T>
    T norm2(const Vector<T>& v)
    {
        return dot(v, v);
    }

    template <typename T>
    real_t norm(const Vector<T>& v)
    {
        return std::sqrt(real_t(norm2(v)));
    }

    /** A dense, row-major matrix of `T`. Storage is a single flat
        `DynArray<T>` (row `i`, column `j` lives at `i * cols + j`)
        rather than an array of rows, so `Matrix` need not worry about
        row-length invariants the way a naive "array of arrays"
        representation would. */
    template <typename T>
    class Matrix
    {
    public:
        using ItemType = T;

    private:
        nat_t rows;
        nat_t cols;
        DynArray<T> data;

        void validate_dims(nat_t i, nat_t j) const
        {
            if (i >= rows || j >= cols)
            {
                throw std::out_of_range("Matrix: index out of range");
            }
        }

    public:
        Matrix(nat_t _rows, nat_t _cols, const T& fill = T())
            : rows(_rows), cols(_cols), data(_rows * _cols, fill)
        {
            if (rows == 0 || cols == 0)
            {
                throw std::domain_error("Matrix: dimensions must be positive");
            }
        }

        static Matrix identity(nat_t n)
        {
            Matrix result(n, n, T());

            for (nat_t i = 0; i < n; ++i)
            {
                result(i, i) = T(1);
            }

            return result;
        }

        nat_t num_rows() const
        {
            return rows;
        }

        nat_t num_cols() const
        {
            return cols;
        }

        bool is_square() const
        {
            return rows == cols;
        }

        T& operator()(nat_t i, nat_t j)
        {
            validate_dims(i, j);
            return data[i * cols + j];
        }

        const T& operator()(nat_t i, nat_t j) const
        {
            validate_dims(i, j);
            return data[i * cols + j];
        }

        bool operator==(const Matrix& m) const
        {
            if (rows != m.rows || cols != m.cols)
            {
                return false;
            }

            for (nat_t i = 0; i < data.size(); ++i)
            {
                if (data[i] < m.data[i] || m.data[i] < data[i])
                {
                    return false;
                }
            }

            return true;
        }

        bool operator!=(const Matrix& m) const
        {
            return !(*this == m);
        }

        Matrix operator+(const Matrix& m) const
        {
            if (rows != m.rows || cols != m.cols)
            {
                throw std::domain_error(
                    "Matrix::operator+: dimension mismatch");
            }

            Matrix result(rows, cols, T());

            for (nat_t i = 0; i < data.size(); ++i)
            {
                result.data[i] = data[i] + m.data[i];
            }

            return result;
        }

        Matrix operator-(const Matrix& m) const
        {
            if (rows != m.rows || cols != m.cols)
            {
                throw std::domain_error(
                    "Matrix::operator-: dimension mismatch");
            }

            Matrix result(rows, cols, T());

            for (nat_t i = 0; i < data.size(); ++i)
            {
                result.data[i] = data[i] - m.data[i];
            }

            return result;
        }

        Matrix operator*(const T& scalar) const
        {
            Matrix result(rows, cols, T());

            for (nat_t i = 0; i < data.size(); ++i)
            {
                result.data[i] = data[i] * scalar;
            }

            return result;
        }

        friend Matrix operator*(const T& scalar, const Matrix& m)
        {
            return m * scalar;
        }

        Matrix operator*(const Matrix& m) const
        {
            if (cols != m.rows)
            {
                throw std::domain_error(
                    "Matrix::operator*: dimension mismatch");
            }

            Matrix result(rows, m.cols, T());

            for (nat_t i = 0; i < rows; ++i)
            {
                for (nat_t k = 0; k < cols; ++k)
                {
                    const T& a_ik = (*this)(i, k);

                    for (nat_t j = 0; j < m.cols; ++j)
                    {
                        result(i, j) += a_ik * m(k, j);
                    }
                }
            }

            return result;
        }

        Vector<T> operator*(const Vector<T>& v) const
        {
            if (cols != v.size())
            {
                throw std::domain_error(
                    "Matrix::operator*: dimension mismatch");
            }

            Vector<T> result(rows, T());

            for (nat_t i = 0; i < rows; ++i)
            {
                T acc = T();

                for (nat_t j = 0; j < cols; ++j)
                {
                    acc += (*this)(i, j) * v[j];
                }

                result[i] = acc;
            }

            return result;
        }

        Matrix transpose() const
        {
            Matrix result(cols, rows, T());

            for (nat_t i = 0; i < rows; ++i)
            {
                for (nat_t j = 0; j < cols; ++j)
                {
                    result(j, i) = (*this)(i, j);
                }
            }

            return result;
        }

        T trace() const
        {
            if (!is_square())
            {
                throw std::domain_error("Matrix::trace: matrix must be square");
            }

            T result = T();

            for (nat_t i = 0; i < rows; ++i)
            {
                result += (*this)(i, i);
            }

            return result;
        }
    };

    /** Reduces `m` to row-echelon form in place via Gaussian
        elimination with partial pivoting (the largest-magnitude entry
        in each column is chosen as the pivot, swapping rows as needed,
        the standard defense against amplifying rounding error by
        dividing by a near-zero pivot). Returns the number of row swaps
        performed (needed by determinant() to get the overall sign
        right) and, via `singular`, whether a fully-zero pivot column
        was found (the matrix is singular — meaningless to keep
        eliminating). */
    template <typename T>
    nat_t gaussian_eliminate(Matrix<T>& m, bool& singular)
    {
        nat_t n = m.num_rows();
        nat_t swaps = 0;
        singular = false;

        for (nat_t col = 0; col < n && col < m.num_cols(); ++col)
        {
            nat_t pivot_row = col;
            T best = std::abs(m(col, col));

            for (nat_t r = col + 1; r < n; ++r)
            {
                T candidate = std::abs(m(r, col));

                if (candidate > best)
                {
                    best = candidate;
                    pivot_row = r;
                }
            }

            if (!(T() < best))
            {
                singular = true;
                continue;
            }

            if (pivot_row != col)
            {
                for (nat_t j = 0; j < m.num_cols(); ++j)
                {
                    std::swap(m(col, j), m(pivot_row, j));
                }

                ++swaps;
            }

            for (nat_t r = col + 1; r < n; ++r)
            {
                T factor = m(r, col) / m(col, col);

                for (nat_t j = col; j < m.num_cols(); ++j)
                {
                    m(r, j) -= factor * m(col, j);
                }
            }
        }

        return swaps;
    }

    /** The determinant, computed as the (sign-adjusted) product of the
        pivots Gaussian elimination leaves on the diagonal — equivalent
        to, but far cheaper than, cofactor expansion (O(n^3) instead of
        O(n!)). Throws if `m` is not square. */
    template <typename T>
    T determinant(const Matrix<T>& m)
    {
        if (!m.is_square())
        {
            throw std::domain_error("determinant: matrix must be square");
        }

        Matrix<T> work = m;
        bool singular = false;
        nat_t swaps = gaussian_eliminate(work, singular);

        if (singular)
        {
            return T();
        }

        T result = T(1);

        for (nat_t i = 0; i < work.num_rows(); ++i)
        {
            result *= work(i, i);
        }

        return (swaps % 2 == 0) ? result : -result;
    }

    /** Solves the linear system `A x = b` via Gaussian elimination with
        partial pivoting on the augmented matrix `[A | b]`, followed by
        back substitution. Throws if `A` is not square, if `b`'s size
        does not match, or if `A` is singular (no unique solution). */
    template <typename T>
    Vector<T> solve(const Matrix<T>& A, const Vector<T>& b)
    {
        if (!A.is_square())
        {
            throw std::domain_error("solve: matrix must be square");
        }

        nat_t n = A.num_rows();

        if (b.size() != n)
        {
            throw std::domain_error("solve: right-hand side size mismatch");
        }

        Matrix<T> augmented(n, n + 1, T());

        for (nat_t i = 0; i < n; ++i)
        {
            for (nat_t j = 0; j < n; ++j)
            {
                augmented(i, j) = A(i, j);
            }

            augmented(i, n) = b[i];
        }

        bool singular = false;
        gaussian_eliminate(augmented, singular);

        if (singular)
        {
            throw std::domain_error("solve: matrix is singular");
        }

        Vector<T> x(n, T());

        for (nat_t i = n; i > 0; --i)
        {
            nat_t row = i - 1;
            T acc = augmented(row, n);

            for (nat_t j = row + 1; j < n; ++j)
            {
                acc -= augmented(row, j) * x[j];
            }

            x[row] = acc / augmented(row, row);
        }

        return x;
    }

    /** The inverse, obtained by solving `A x_i = e_i` (the i-th
        identity column) for every column i and assembling the results
        — simple to follow and reuses solve() directly, at the cost of
        being roughly twice the work of specialized Gauss-Jordan
        matrix inversion; fine for a teaching-oriented implementation.
        Throws under the same conditions as solve(). */
    template <typename T>
    Matrix<T> inverse(const Matrix<T>& A)
    {
        if (!A.is_square())
        {
            throw std::domain_error("inverse: matrix must be square");
        }

        nat_t n = A.num_rows();
        Matrix<T> result(n, n, T());

        for (nat_t col = 0; col < n; ++col)
        {
            Vector<T> e(n, T());
            e[col] = T(1);

            Vector<T> x = solve(A, e);

            for (nat_t row = 0; row < n; ++row)
            {
                result(row, col) = x[row];
            }
        }

        return result;
    }

} // end namespace Designar
