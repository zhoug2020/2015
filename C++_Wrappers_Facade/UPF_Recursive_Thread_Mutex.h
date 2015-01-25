#ifndef UPF_RECURSIVE_THREAD_MUTEX_H_
#define UPF_RECURSIVE_THREAD_MUTEX_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "..\OS_Adapter\OS_Adapter_Common\UPF_wchar.h"

class /*UPF_Export*/ UPF_Recursive_Thread_Mutex
{
public:
  /// Initialize a recursive mutex.
  UPF_Recursive_Thread_Mutex (const UPF_TCHAR *name = 0,
                              UPF_mutexattr_t *arg = 0);

  /// Implicitly release a recursive mutex.
  ~UPF_Recursive_Thread_Mutex (void);

  /**
   * Implicitly release a recursive mutex.  Note that only one thread
   * should call this method since it doesn't protect against race
   * conditions.
   */
  int remove (void);

  /**
   * Acquire a recursive mutex (will increment the nesting level and
   * not deadmutex if the owner of the mutex calls this method more
   * than once).
   */
  int acquire (void);

#if 0 // wangy
  /**
   * Block the thread until we acquire the mutex or until @a tv times
   * out, in which case -1 is returned with @c errno == @c ETIME.  Note
   * that @a tv is assumed to be in "absolute" rather than "relative"
   * time.  The value of @a tv is updated upon return to show the
   * actual (absolute) acquisition time.
   */
  int acquire (UPF_Time_Value &tv);

  /**
   * If @a tv == 0 the call <acquire()> directly.  Otherwise, Block the
   * thread until we acquire the mutex or until @a tv times out, in
   * which case -1 is returned with @c errno == @c ETIME.  Note that
   * <*tv> is assumed to be in "absolute" rather than "relative" time.
   * The value of <*tv> is updated upon return to show the actual
   * (absolute) acquisition time.
   */
  int acquire (UPF_Time_Value *tv);
#endif 
  /**
   * Conditionally acquire a recursive mutex (i.e., won't block).
   * Returns -1 on failure.  If we "failed" because someone else
   * already had the lock, @c errno is set to @c EBUSY.
   */
  int tryacquire (void);

  /**
   * Acquire mutex ownership.  This calls <acquire> and is only
   * here to make the <UPF_Recusive_Thread_Mutex> interface consistent
   * with the other synchronization APIs.
   */
  int acquire_read (void);

  /**
   * Acquire mutex ownership.  This calls <acquire> and is only
   * here to make the <UPF_Recusive_Thread_Mutex> interface consistent
   * with the other synchronization APIs.
   */
  int acquire_write (void);

  /**
   * Conditionally acquire mutex (i.e., won't block).  This calls
   * <tryacquire> and is only here to make the
   * <UPF_Recusive_Thread_Mutex> interface consistent with the other
   * synchronization APIs.  Returns -1 on failure.  If we "failed"
   * because someone else already had the lock, @c errno is set to
   * @c EBUSY.
   */
  int tryacquire_read (void);

  /**
   * Conditionally acquire mutex (i.e., won't block).  This calls
   * <tryacquire> and is only here to make the
   * <UPF_Recusive_Thread_Mutex> interface consistent with the other
   * synchronization APIs.  Returns -1 on failure.  If we "failed"
   * because someone else already had the lock, @c errno is set to
   * @c EBUSY.
   */
  int tryacquire_write (void);

  /**
   * This is only here to make the UPF_Recursive_Thread_Mutex
   * interface consistent with the other synchronization APIs.
   * Assumes the caller has already acquired the mutex using one of
   * the above calls, and returns 0 (success) always.
   */
  int tryacquire_write_upgrade (void);

  /**
   * Releases a recursive mutex (will not release mutex until all the
   * nesting level drops to 0, which means the mutex is no longer
   * held).
   */
  int release (void);

  /// Return the id of the thread that currently owns the mutex.
  UPF_thread_t get_thread_id (void);

  /**
   * Return the nesting level of the recursion.  When a thread has
   * acquired the mutex for the first time, the nesting level == 1.
   * The nesting level is incremented every time the thread acquires
   * the mutex recursively.  Note that if the UPF_HAS_RECURSIVE_MUTEXES
   * macro is enabled then this method may return -1 on platforms that
   * do not expose the internal count.
   */
  int get_nesting_level (void);

  /// Returns a reference to the recursive mutex;
  UPF_recursive_thread_mutex_t &mutex (void);

  /// Returns a reference to the recursive mutex's internal mutex;
  UPF_thread_mutex_t &get_nesting_mutex (void);

  /// Dump the state of an object.
  void dump (void) const;

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;

protected:
  // = This method should *not* be public (they hold no locks...)
  void set_thread_id (UPF_thread_t t);

  /// Recursive mutex.
  UPF_recursive_thread_mutex_t lock_;

  /// Keeps track of whether <remove> has been called yet to avoid
  /// multiple <remove> calls, e.g., explicitly and implicitly in the
  /// destructor.  This flag isn't protected by a lock, so make sure
  /// that you don't have multiple threads simultaneously calling
  /// <remove> on the same object, which is a bad idea anyway...
  bool removed_;

private:
  // = Prevent assignment and initialization.
  void operator= (const UPF_Recursive_Thread_Mutex &);
  UPF_Recursive_Thread_Mutex (const UPF_Recursive_Thread_Mutex &);
};

#endif 