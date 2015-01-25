#ifndef UPF_LOCK_H_
#define UPF_LOCK_H_


#include "..\OS_Adapter\UPF_OS.h"

class UPF_Export UPF_Lock
{
public:
  /// CE needs a default ctor here.
  UPF_Lock (void);

  /// Noop virtual destructor
  virtual ~UPF_Lock (void);

  /**
   * Explicitly destroy the lock.  Note that only one thread should
   * call this method since it doesn't protect against race
   * conditions.
   */
  virtual int remove (void) = 0;

  /// Block the thread until the lock is acquired.  Returns -1 on
  /// failure.
  virtual int acquire (void) = 0;

  /**
   * Conditionally acquire the lock (i.e., won't block).  Returns -1
   * on failure.  If we "failed" because someone else already had the
   * lock, @c errno is set to @c EBUSY.
   */
  virtual int tryacquire (void) = 0;

  /// Release the lock.  Returns -1 on failure.
  virtual int release (void) = 0;

  /**
   * Block until the thread acquires a read lock.  If the locking
   * mechanism doesn't support read locks then this just calls
   * <acquire>.  Returns -1 on failure.
   */
  virtual int acquire_read (void) = 0;

  /**
   * Block until the thread acquires a write lock.  If the locking
   * mechanism doesn't support read locks then this just calls
   * <acquire>.  Returns -1 on failure.
   */
  virtual int acquire_write (void) = 0;

  /**
   * Conditionally acquire a read lock.  If the locking mechanism
   * doesn't support read locks then this just calls <acquire>.
   * Returns -1 on failure.  If we "failed" because someone else
   * already had the lock, @c errno is set to @c EBUSY.
   */
  virtual int tryacquire_read (void) = 0;

  /**
   * Conditionally acquire a write lock.  If the locking mechanism
   * doesn't support read locks then this just calls <acquire>.
   * Returns -1 on failure.  If we "failed" because someone else
   * already had the lock, @c errno is set to @c EBUSY.
   */
  virtual int tryacquire_write (void) = 0;

  /**
   * Conditionally try to upgrade a lock held for read to a write lock.
   * If the locking mechanism doesn't support read locks then this just
   * calls <acquire>. Returns 0 on success, -1 on failure.
   */
  virtual int tryacquire_write_upgrade (void) = 0;
};


#endif 