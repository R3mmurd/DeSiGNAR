/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGMATH_H
# define DSGMATH_H

# include <types.H>

namespace Designar
{

  constexpr real_t PI      = 3.1415926535897932384626433832795028841971693993751;
  constexpr real_t PI_2    = PI/2.;
  constexpr real_t PI_4    = PI/4.;
  constexpr real_t PI3_2   = 3.*PI/2.;
  constexpr real_t EPSILON = 1e-6;
  constexpr real_t INF     = std::numeric_limits<real_t>::infinity();

  enum class Sign { NEGATIVE, POSITIVE, ZERO };

  template <typename T>
  Sign sign(T);

  template <typename T>
  bool is_positive(T);

  template <typename T>
  bool is_negative(T);

  template <typename T>
  T abs(T);

  template <typename T>
  bool real_equal(T, T);

  template <typename T>
  bool num_equal(T, T);

  template <>
  bool num_equal<float>(float, float);
  
  template <>
  bool num_equal<double>(double, double);

  template <>
  bool num_equal<long double>(long double, long double);
  
  template <class NumberType> class GenPoint2D;

  template <typename NumberType>
  real_t area_of_parallelogram(const GenPoint2D<NumberType> &,
			       const GenPoint2D<NumberType> &,
			       const GenPoint2D<NumberType> &);
  
  template <typename BT, typename ET>
  BT fast_integral_pow(BT, ET);

  template <typename BT, typename ET>
  BT pow(BT, ET);

  template <typename T>
  T abs(T x)
  {
    static_assert(std::is_floating_point<T>::value,
		  "Template argument must be a floating point type");
    return (x < T(0) ? -x : x);
  }

  template <typename T>
  bool real_equal(T a, T b)
  {
    static_assert(std::is_floating_point<T>::value,
		  "Template argument must be a floating point type");
    return abs(a-b) <= EPSILON;
  }

  template <typename T>
  bool num_equal(T a, T b)
  {
    static_assert(std::is_integral<T>::value,
		  "Template argument must be an integral type");
    return a == b;
  }

  template <typename T>
  Sign sign(T n)
  {
    if (n < T(0))
      return Sign::NEGATIVE;
    else if (n > T(0))
      return Sign::POSITIVE;
    return Sign::ZERO;
  }

  template <typename T>
  bool is_positive(T n)
  {
    return sign(n) == Sign::POSITIVE;
  }

  template <typename T>
  bool is_negative(T n)
  {
    return sign(n) == Sign::NEGATIVE;
  }

  template <typename NumberType>
  real_t area_of_parallelogram(const GenPoint2D<NumberType> & a,
			       const GenPoint2D<NumberType> & b,
			       const GenPoint2D<NumberType> & c)
  {
    return (b.get_x() - a.get_x()) * (c.get_y() - a.get_y()) -
      (c.get_x() - a.get_x()) * (b.get_y() - a.get_y());
  }
  
  template <typename BT, typename ET>
  BT fast_integral_pow(BT base, ET exp)
  {
    static_assert(std::is_integral<BT>::value and std::is_integral<ET>::value,
    		  "Arguments must be integral types");
    static_assert(std::is_unsigned<ET>::value,
    		  "Exponent must be an unsigned type");
    
    BT ret_val = BT(1);
    
    while (exp > ET(0))
      {
	if (exp & ET(1))
	  ret_val *= base;
	
	exp >>= 1;
	base *= base;
      }

    return ret_val;
  }

  template <typename BT, typename ET>
  struct FastIntegralPow
  {
    BT operator () (BT base, ET exp)
    {
      return fast_integral_pow(base, exp);
    }
  };

  template <typename BT, typename ET>
  struct StdPow
  {
    BT operator () (BT base, ET exp)
    {
      return std::pow(base, exp);
    }
  };

  template <typename BT, typename ET>
  BT pow(BT base, ET exp)
  {
    auto fct = typename std::conditional<std::is_integral<BT>::value and
					 std::is_integral<ET>::value and
					 std::is_unsigned<ET>::value,
					 FastIntegralPow<BT, ET>,
					 StdPow<BT, ET>>::type();
    return fct(base, exp);    
  }
 
} // end namespace Designar

# endif // DSGMATH_H
