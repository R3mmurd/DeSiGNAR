/*
  This file is part of Designar.

  Author: Alejandro Mujica (aledrums@gmail.com)
*/

/** @file polynomial.hpp
    @brief Polynomial<T> (a plain coefficient-vector wrapper) plus the
    Fast Fourier Transform (`fft`/`ifft`, iterative Cooley-Tukey over
    `std::complex<real_t>`) and the FFT-based polynomial multiplication
    it exists to make possible: multiplying two degree-n polynomials
    directly is O(n^2) (every pair of coefficients), but evaluating
    both at n-th roots of unity (via FFT), multiplying pointwise
    (O(n)), and interpolating back (via the inverse FFT) does the same
    job in O(n lg n).
    @ingroup LinearAlgebra
*/

#pragma once

#include <cmath>
#include <complex>
#include <stdexcept>
#include <type_traits>

#include <array.hpp>
#include <math.hpp>

namespace Designar
{
    using Complex = std::complex<real_t>;

    /** In-place iterative Cooley-Tukey FFT — `a.size()` must be a power
        of two. First bit-reverses `a` (the standard trick that lets the
        usual recursive "split into evens/odds, recurse, combine" FFT be
        rewritten as a bottom-up iteration with no recursion overhead:
        after bit-reversal, every butterfly stage's inputs are already
        exactly where the recursive version would have grouped them),
        then combines adjacent pairs into blocks of increasing power-of-
        two size, each stage multiplying by the appropriate roots of
        unity. `invert` computes the inverse transform (conjugated root
        of unity, plus dividing every entry by `n` at the end) — the
        same butterfly network runs either direction. */
    inline void fft(DynArray<Complex>& a, bool invert)
    {
        nat_t n = a.size();

        if (n <= 1)
        {
            return;
        }

        if ((n & (n - 1)) != 0)
        {
            throw std::domain_error("fft: size must be a power of two");
        }

        for (nat_t i = 1, j = 0; i < n; ++i)
        {
            nat_t bit = n >> 1;

            for (; j & bit; bit >>= 1)
            {
                j ^= bit;
            }

            j ^= bit;

            if (i < j)
            {
                std::swap(a[i], a[j]);
            }
        }

        for (nat_t len = 2; len <= n; len <<= 1)
        {
            real_t ang = 2 * PI / real_t(len) * (invert ? -1 : 1);
            Complex wlen(std::cos(ang), std::sin(ang));

            for (nat_t i = 0; i < n; i += len)
            {
                Complex w(1);

                for (nat_t k = 0; k < len / 2; ++k)
                {
                    Complex u = a[i + k];
                    Complex v = a[i + k + len / 2] * w;
                    a[i + k] = u + v;
                    a[i + k + len / 2] = u - v;
                    w *= wlen;
                }
            }
        }

        if (invert)
        {
            for (nat_t i = 0; i < n; ++i)
            {
                a[i] /= real_t(n);
            }
        }
    }

    inline void fft(DynArray<Complex>& a)
    {
        fft(a, false);
    }

    inline void ifft(DynArray<Complex>& a)
    {
        fft(a, true);
    }

    /** Multiplies two coefficient sequences (`a[i]`/`b[i]` the
        coefficient of `x^i`) via FFT: zero-pad both up to a common
        power-of-two length comfortably larger than the true result's
        degree, transform, multiply pointwise, and inverse-transform —
        an O(n lg n) convolution instead of the O(n^2) direct one.
        Rounds back to the nearest integer when `T` is an integral type
        (the whole computation happens in floating point internally
        regardless of `T`, so integer coefficients need rounding to
        undo the accumulated floating-point error) or just takes the
        real part directly otherwise. */
    template <typename T>
    DynArray<T> multiply_polynomials(const DynArray<T>& a,
                                     const DynArray<T>& b)
    {
        if (a.is_empty() || b.is_empty())
        {
            return DynArray<T>();
        }

        nat_t result_size = a.size() + b.size() - 1;
        nat_t n = 1;

        while (n < result_size)
        {
            n <<= 1;
        }

        DynArray<Complex> fa(n, Complex());
        DynArray<Complex> fb(n, Complex());

        for (nat_t i = 0; i < a.size(); ++i)
        {
            fa[i] = Complex(real_t(a[i]), 0);
        }

        for (nat_t i = 0; i < b.size(); ++i)
        {
            fb[i] = Complex(real_t(b[i]), 0);
        }

        fft(fa);
        fft(fb);

        for (nat_t i = 0; i < n; ++i)
        {
            fa[i] *= fb[i];
        }

        ifft(fa);

        DynArray<T> result(result_size, T());

        for (nat_t i = 0; i < result_size; ++i)
        {
            real_t val = fa[i].real();

            if constexpr (std::is_integral<T>::value)
            {
                result[i] = T(std::llround(val));
            }
            else
            {
                result[i] = T(val);
            }
        }

        return result;
    }

    /** A polynomial as a plain coefficient vector, `coeffs[i]` the
        coefficient of `x^i` — thin enough that most of what makes it
        useful is the free functions above/below rather than member
        functions. */
    template <typename T>
    class Polynomial
    {
        DynArray<T> coeffs;

        /** Written without `==`/`!=` on `T` (a plain equality check
            would trip -Wfloat-equal for a floating-point `T`): a
            trailing coefficient is zero exactly when it's neither less
            than nor greater than `T()`. */
        void trim()
        {
            while (coeffs.size() > 1 &&
                  !(coeffs[coeffs.size() - 1] < T()) &&
                  !(T() < coeffs[coeffs.size() - 1]))
            {
                coeffs.remove_last();
            }
        }

    public:
        Polynomial() : coeffs(nat_t(1), T())
        {
            // empty
        }

        explicit Polynomial(DynArray<T> c) : coeffs(std::move(c))
        {
            if (coeffs.is_empty())
            {
                coeffs.append(T());
            }

            trim();
        }

        nat_t degree() const
        {
            return coeffs.size() - 1;
        }

        const DynArray<T>& coefficients() const
        {
            return coeffs;
        }

        T operator[](nat_t i) const
        {
            return i < coeffs.size() ? coeffs[i] : T();
        }

        /** Horner's method: O(degree) evaluation instead of computing
            each power of `x` from scratch. */
        T evaluate(const T& x) const
        {
            T result = coeffs[coeffs.size() - 1];

            for (nat_t i = coeffs.size() - 1; i-- > 0;)
            {
                result = result * x + coeffs[i];
            }

            return result;
        }

        Polynomial operator+(const Polynomial& p) const
        {
            nat_t n = std::max(coeffs.size(), p.coeffs.size());
            DynArray<T> result(n, T());

            for (nat_t i = 0; i < n; ++i)
            {
                result[i] = (*this)[i] + p[i];
            }

            return Polynomial(std::move(result));
        }

        /** Multiplies via multiply_polynomials() (FFT-based, O(n lg n))
            rather than the direct O(n^2) convolution — the reason this
            file exists. */
        Polynomial operator*(const Polynomial& p) const
        {
            return Polynomial(multiply_polynomials(coeffs, p.coeffs));
        }
    };

} // end namespace Designar
