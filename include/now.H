/*
  This file is part of Designar.
  
  Author: Alejandro Mujica (aledrums@gmail.com)
*/

# ifndef DSGNOW_H
# define DSGNOW_H

# include <types.H>

namespace Designar
{
  /** Class Now is a practical class for timing based in a high
   * resolution clock.
   *
   *  An instance of this class allows to measure elapsed time between
   *  two instants.
   *
   *  Usage example:
   *  \code{.cpp}
   *  Now now;
   *  now.start();
   *  // Any block of code
   *  cout << "Time elapsed: " << now.elapsed() << "ms\n";
   *  \endcode
   *  
   *  Furthermore you may use the static methods just like that:
   *  \code{.cpp}
   *  auto t = Now::current_time_point();
   *  // Any block of code
   *  cout << "Time elapsed: " << Now::elapsed(t) << "ms\n";
   *  \endcode
   *
   *  @ingroup utils
   *  @author Alejandro J. Mujica
   */
  class Now
  {
  public:
    /// Precision for timing.
    enum class Precision
      {
	HOURS,
	MINUTES,
        SECONDS,
        MILLISECONDS,
        MICROSECONDS,
	NANOSECONDS
      };

    static int to_int(Precision p)
    {
      return static_cast<int>(p);
    }
    
  private:
    time_point_t tp;

    Precision precision;

    static const double precision_values[];
    
  public:
    /// Gets the current time point.
    static time_point_t current_time_point()
    {
      return clock_t::now();
    }

    /** Calculates the time that has elapsed between two time points.
     * 
     * @param rtp Right time point.
     * @param ltp Left time point.
     * @param precision Type of precision for calculating the time.
     * @return The time calculated.
     * @see Precision
     */
    static double compute_time_diff(const time_point_t & rtp,
				    const time_point_t & ltp,
				    const Precision & precision)
    {
      duration_t et = std::chrono::duration_cast<std::chrono::nanoseconds>
	(rtp - ltp);
      return et.count() * precision_values[to_int(precision)];
    }

    /** \brief Builds a new object with default values.
     *
     *  Builds a new object with default time point and precision in
     *  MILLISECONDS. You may set the object to start counting time 
     *  immediately by setting the parameter start_now in true. This
     *  constructor can be used as parametric or default.
     *
     *  @param start_now If true, then the object starts counting the time 
     *  immediately. By default is false.
     *
     *  @see Precision
     */
    Now(bool start_now = false)
      : tp(), precision(Precision::MILLISECONDS)
    {
      if (start_now)
	start();
    }

    /** \brief Builds a new object with parametric precision time.
     *
     *  Builds a new object with default time point and precision given as a
     *  parameter. You may set the object to start counting time 
     *  immediately by setting the parameter start_now in true.
     *
     *  @param _precision The type of precision for timing.
     *  @param start_now If true, then the object starts counting the time 
     *  immediately. By default is false.
     *
     *  @see Precision
     */
    Now(const Precision & _precision, bool start_now = false)
      : tp(), precision(_precision)
    {
      if (start_now)
	start();
    }

    /** Gets the type of precision.
     * 
     *  @see Precision
     */
    const Precision & get_precision() const
    {
      return precision;
    }

    /** Sets the type of precision.
     * 
     *  @see Precision
     */
    void set_precision(const Precision & _precision)
    {
      precision = _precision;
    }

    /** Sets internally the current time point.
     *
     *  This method must be used before calling elapsed() or delta() the first
     *  time.
     *
     *  @return The current time point.
     *  @see current_time_point()
     */
    time_point_t start()
    {
      tp = this->current_time_point();
      return tp;
    }

    /** Calculates the time that has elapsed since the last time
     *  start(), elapsed() or delta() was called.
     *
     *  The value calculated depends of precision.
     *
     *  @return The time calculated.
     *  @see start(), delta(), Precision
     */
    double elapsed()
    {
      time_point_t ltp = tp;
      tp = this->current_time_point();
      return compute_time_diff(tp, ltp, precision);
    }

    /** Like elapsed().
     *
     * @see elapsed().
     */
    double delta()
    {
      return elapsed();
    }

    /** Calculates the time that has elapsed since a given time point.
     *
     *  This is a class method. It's useful when is required massive time
     *  calculations and is not desired massive object instances of Now.
     *
     *  @param tp Time point from which you want to calculate.
     *  @param precision The type of precision for timing, by default is
     *  MILLISECONDS.
     *  @return The time calculated.
     *  @see Precision
     */
    static double elapsed(const time_point_t & tp,
			  const Precision & precision = Precision::MILLISECONDS)
    {
      return compute_time_diff(current_time_point(), tp, precision);
    }

    /** Like elapsed(const time_point_t & tp,
     *               const Precision & precision = MILLISECONDS).
     *
     *  @see elapsed(const time_point_t & tp,
     *               const Precision & precision = MILLISECONDS)
     */
    static double delta(const time_point_t & tp,
			const Precision & precision = Precision::MILLISECONDS)
    {
      return elapsed(tp, precision);
    }
  };
  
} // End namespace Designar
  
# endif // DSGNOW_H
