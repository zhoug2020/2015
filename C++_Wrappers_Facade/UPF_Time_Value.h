#ifndef UPF_TIME_VALUE_H_
#define UPF_TIME_VALUE_H_


class ACE_Export ACE_Time_Value
{
public:

  /// Constant "0".
  static const ACE_Time_Value zero;

  /**
   * Constant for maximum time representable.  Note that this time is
   * not intended for use with <select> or other calls that may have
   * *their own* implementation-specific maximum time representations.
   * Its primary use is in time computations such as those used by the
   * dynamic subpriority strategies in the ACE_Dynamic_Message_Queue
   * class.
   */
  static const ACE_Time_Value max_time;

  // = Initialization methods.

  /// Default Constructor.
  ACE_Time_Value (void);

  /// Constructor.
  explicit ACE_Time_Value (time_t sec, suseconds_t usec = 0);

  // = Methods for converting to/from various time formats.

  /// Construct the ACE_Time_Value from a timeval.
  explicit ACE_Time_Value (const struct timeval &t);

  /// Construct the ACE_Time_Value object from a timespec_t.
  explicit ACE_Time_Value (const timespec_t &t);

# if defined (ACE_WIN32)
  /// Construct the ACE_Time_Value object from a Win32 FILETIME
  explicit ACE_Time_Value (const FILETIME &ft);
# endif /* ACE_WIN32 */

  /// Initializes the ACE_Time_Value from seconds and useconds.
  void set (time_t sec, suseconds_t usec);

  /// Initializes the ACE_Time_Value from a double, which is assumed to be
  /// in second format, with any remainder treated as microseconds.
  void set (double d);

  /// Initializes the ACE_Time_Value from a timeval.
  void set (const timeval &t);

  /// Initializes the ACE_Time_Value object from a timespec_t.
  void set (const timespec_t &t);

# if defined (ACE_WIN32)
  ///  Initializes the ACE_Time_Value object from a Win32 FILETIME.
  void set (const FILETIME &ft);
# endif /* ACE_WIN32 */

  /// Converts from ACE_Time_Value format into milli-seconds format.
  /**
   * @return Sum of second field (in milliseconds) and microsecond field
   *         (in milliseconds).  Note that this method can overflow if
   *         the second and microsecond field values are large, so use
   *         the msec (ACE_UINT64 &ms) method instead.
   *
   * @note The semantics of this method differs from the sec() and
   *       usec() methods.  There is no analogous "millisecond"
   *       component in an ACE_Time_Value.
   */
  unsigned long msec (void) const;

#if !defined (ACE_LACKS_LONGLONG_T)
  /**
   * @return Sum of second field (in milliseconds) and microsecond field
   *         (in milliseconds) and return them via the @param ms parameter.
   *
   * @note The semantics of this method differs from the sec() and
   *       usec() methods.  There is no analogous "millisecond"
   *       component in an ACE_Time_Value.
   */
  void msec (ACE_UINT64 &ms) const;

#endif /*ACE_LACKS_LONGLONG_T */

  /// Converts from milli-seconds format into ACE_Time_Value format.
  /**
   * @note The semantics of this method differs from the sec() and
   *       usec() methods.  There is no analogous "millisecond"
   *       component in an ACE_Time_Value.
   */
  void msec (long);

  /// Returns the value of the object as a timespec_t.
  operator timespec_t () const;

  /// Returns the value of the object as a timeval.
  operator timeval () const;

  /// Returns a pointer to the object as a timeval.
  operator const timeval *() const;

# if defined (ACE_WIN32)
  /// Returns the value of the object as a Win32 FILETIME.
  operator FILETIME () const;
# endif /* ACE_WIN32 */

  // = The following are accessor/mutator methods.

  /// Get seconds.
  /**
   * @return The second field/component of this ACE_Time_Value.
   *
   * @note The semantics of this method differs from the msec()
   *       method.
   */
  time_t sec (void) const;

  /// Set seconds.
  void sec (time_t sec);

  /// Get microseconds.
  /**
   * @return The microsecond field/component of this ACE_Time_Value.
   *
   * @note The semantics of this method differs from the msec()
   *       method.
   */
  suseconds_t usec (void) const;

  /// Set microseconds.
  void usec (suseconds_t usec);

  /**
   * @return Sum of second field (in microseconds) and microsecond field
   *         and return them via the @param usec parameter.
   */
  void to_usec (ACE_UINT64 &usec) const;

  // = The following arithmetic methods operate on ACE_Time_Value's.

  /// Add @a tv to this.
  ACE_Time_Value &operator += (const ACE_Time_Value &tv);

  /// Add @a tv to this.
  ACE_Time_Value &operator += (time_t tv);

  /// Assign @ tv to this
  ACE_Time_Value &operator = (const ACE_Time_Value &tv);

  /// Assign @ tv to this
  ACE_Time_Value &operator = (time_t tv);

  /// Subtract @a tv to this.
  ACE_Time_Value &operator -= (const ACE_Time_Value &tv);

  /// Substract @a tv to this.
  ACE_Time_Value &operator -= (time_t tv);

  /**
    \brief Multiply the time value by the @a d factor.
    \note The result of the operator is valid for results from range
    < (ACE_INT32_MIN, -999999), (ACE_INT32_MAX, 999999) >. Result
    outside this range are saturated to a limit.
     */
  ACE_Time_Value &operator *= (double d);

  /// Increment microseconds as postfix.
  /**
   * @note The only reason this is here is to allow the use of ACE_Atomic_Op
   * with ACE_Time_Value.
   */
  ACE_Time_Value operator++ (int);

  /// Increment microseconds as prefix.
  /**
   * @note The only reason this is here is to allow the use of ACE_Atomic_Op
   * with ACE_Time_Value.
   */
  ACE_Time_Value &operator++ (void);

  /// Decrement microseconds as postfix.
  /**
   * @note The only reason this is here is to allow the use of ACE_Atomic_Op
   * with ACE_Time_Value.
   */
  ACE_Time_Value operator-- (int);

  /// Decrement microseconds as prefix.
  /**
   * @note The only reason this is here is to allow the use of ACE_Atomic_Op
   * with ACE_Time_Value.
   */
  ACE_Time_Value &operator-- (void);

  /// Adds two ACE_Time_Value objects together, returns the sum.
  friend ACE_Export ACE_Time_Value operator + (const ACE_Time_Value &tv1,
                                               const ACE_Time_Value &tv2);

  /// Subtracts two ACE_Time_Value objects, returns the difference.
  friend ACE_Export ACE_Time_Value operator - (const ACE_Time_Value &tv1,
                                               const ACE_Time_Value &tv2);

  /// True if @a tv1 < @a tv2.
  friend ACE_Export bool operator < (const ACE_Time_Value &tv1,
                                     const ACE_Time_Value &tv2);

  /// True if @a tv1 > @a tv2.
  friend ACE_Export bool operator > (const ACE_Time_Value &tv1,
                                     const ACE_Time_Value &tv2);

  /// True if @a tv1 <= @a tv2.
  friend ACE_Export bool operator <= (const ACE_Time_Value &tv1,
                                      const ACE_Time_Value &tv2);

  /// True if @a tv1 >= @a tv2.
  friend ACE_Export bool operator >= (const ACE_Time_Value &tv1,
                                      const ACE_Time_Value &tv2);

  /// True if @a tv1 == @a tv2.
  friend ACE_Export bool operator == (const ACE_Time_Value &tv1,
                                      const ACE_Time_Value &tv2);

  /// True if @a tv1 != @a tv2.
  friend ACE_Export bool operator != (const ACE_Time_Value &tv1,
                                      const ACE_Time_Value &tv2);

  //@{
  /// Multiplies the time value by @a d
  friend ACE_Export ACE_Time_Value operator * (double d,
                                               const ACE_Time_Value &tv);

  friend ACE_Export ACE_Time_Value operator * (const ACE_Time_Value &tv,
                                               double d);
  //@}

  /// Dump is a no-op.
  /**
   * The dump() method is a no-op.  It's here for backwards compatibility
   * only, but does not dump anything. Invoking logging methods here
   * violates layering restrictions in ACE because this class is part
   * of the OS layer and @c ACE_Log_Msg is at a higher level.
   */
  void dump (void) const;

# if defined (ACE_WIN32)
  /// Const time difference between FILETIME and POSIX time.
#  if defined (ACE_LACKS_LONGLONG_T)
  static const ACE_U_LongLong FILETIME_to_timval_skew;
#  else
  static const DWORDLONG FILETIME_to_timval_skew;
#  endif // ACE_LACKS_LONGLONG_T
# endif /* ACE_WIN32 */

private:
  /// Put the timevalue into a canonical form.
  void normalize (void);

  /// Store the values as a timeval.
#if defined (ACE_HAS_TIME_T_LONG_MISMATCH)
  // Windows' timeval is non-conformant, so swap in a struct that conforms
  // to the proper data types to represent the entire time range that this
  // class's API can accept.
  // Also, since this class can supply a pointer to a timeval that things
  // like select() expect, we need the OS-defined one as well. To make this
  // available, use a real timeval called ext_tv_ and set it up when needed.
  // Since this is most often for relative times that don't approach 32 bits
  // in size, reducing a time_t to fit should be no problem.
  struct {
    time_t tv_sec;
    suseconds_t tv_usec;
  } tv_;
  timeval ext_tv_;
#else
  timeval tv_;
#endif /* ACE_HAS_TIME_T_LONG_MISMATCH */
};

ACE_END_VERSIONED_NAMESPACE_DECL

#if defined (__ACE_INLINE__)
#include "ace/Time_Value.inl"
#endif /* __ACE_INLINE__ */

#if defined (ACE_WIN32) && defined (_WIN32_WCE)
}
#endif


#endif 