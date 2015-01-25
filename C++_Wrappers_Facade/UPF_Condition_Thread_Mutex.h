#ifndef UPF_CONDITION_THREAD_MUTEX_H_
#define UPF_CONDITION_THREAD_MUTEX_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "UPF_Thread_Mutex.h"

class /*UPF_Export*/ UPF_Condition_Attributes
{
public:
  /// Constructor
  UPF_Condition_Attributes (int type = UPF_DEFAULT_SYNCH_TYPE);

  /// Destructor
  ~UPF_Condition_Attributes (void);

private:
  friend class UPF_Condition_Thread_Mutex;

  /// The attributes
  UPF_condattr_t attributes_;

private:
  // = Prevent assignment and initialization.
  void operator= (const UPF_Condition_Attributes &);
  UPF_Condition_Attributes (const UPF_Condition_Attributes &);
};

class /*UPF_Export*/ UPF_Condition_Thread_Mutex
{
public:
  /// Initialize the condition variable.
  UPF_Condition_Thread_Mutex (UPF_Thread_Mutex &m,
                              const UPF_TCHAR *name = 0,
                              void *arg = 0);

  /// Initialize the condition variable.
  UPF_Condition_Thread_Mutex (UPF_Thread_Mutex &m,
                              UPF_Condition_Attributes &attributes,
                              const UPF_TCHAR *name = 0,
                              void *arg = 0);

  /// Implicitly destroy the condition variable.
  ~UPF_Condition_Thread_Mutex (void);

  /**
   * Explicitly destroy the condition variable.  Note that only one
   * thread should call this method since it doesn't protect against
   * race conditions.
   */
  int remove (void);

#if 0 // wangy
  /**
   * Block on condition, or until absolute time-of-day has passed.  If
   * abstime == 0 use "blocking" <wait> semantics.  Else, if @a abstime
   * != 0 and the call times out before the condition is signaled
   * <wait> returns -1 and sets errno to ETIME.
   */
  int wait (const UPF_Time_Value *abstime);

#endif


  /// Block on condition.
  int wait (void);

#if 0
  /**
   * Block on condition or until absolute time-of-day has passed.  If
   * abstime == 0 use "blocking" wait() semantics on the <mutex>
   * passed as a parameter (this is useful if you need to store the
   * <Condition> in shared memory).  Else, if @a abstime != 0 and the
   * call times out before the condition is signaled <wait> returns -1
   * and sets errno to ETIME.
   */
  int wait (UPF_Thread_Mutex &mutex, const UPF_Time_Value *abstime = 0);

#endif 
  /// Signal one waiting thread.
  int signal (void);

  /// Signal *all* waiting threads.
  int broadcast (void);

  /// Returns a reference to the underlying mutex;
  UPF_Thread_Mutex &mutex (void);

  /// Dump the state of an object.
  void dump (void) const;

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;

protected:
  /// Condition variable.
  UPF_cond_t cond_;

  /// Reference to mutex lock.
  UPF_Thread_Mutex &mutex_;

  /// Keeps track of whether <remove> has been called yet to avoid
  /// multiple <remove> calls, e.g., explicitly and implicitly in the
  /// destructor.  This flag isn't protected by a lock, so make sure
  /// that you don't have multiple threads simultaneously calling
  /// <remove> on the same object, which is a bad idea anyway...
  bool removed_;

private:
  // = Prevent assignment and initialization.
  void operator= (const UPF_Condition_Thread_Mutex &);
  UPF_Condition_Thread_Mutex (const UPF_Condition_Thread_Mutex &);
};

#endif




