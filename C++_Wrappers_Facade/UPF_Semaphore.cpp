#include "UPF_Semaphore.h"

UPF_ALLOC_HOOK_DEFINE(UPF_Semaphore)
UPF_Semaphore::UPF_Semaphore (unsigned int count,
                              int type,
                              const UPF_TCHAR *name,
                              void *arg,
                              int max)
  : removed_ (false)
{

  if (UPF_OS::sema_init (&this->semaphore_, count, type,
                         name, arg, max) != 0)
   UPF_ERROR ((LM_ERROR,
                UPF_TEXT ("%p\n"),
                UPF_TEXT ("UPF_Semaphore::UPF_Semaphore")));
}

UPF_Semaphore::~UPF_Semaphore (void)
{

  this->remove ();
}

UPF_INLINE const UPF_sema_t &
UPF_Semaphore::lock (void) const
{
// UPF_TRACE ("UPF_Semaphore::lock");
  return this->semaphore_;
}

UPF_INLINE int
UPF_Semaphore::remove (void)
{
// UPF_TRACE ("UPF_Semaphore::remove");
  int result = 0;
  if (!this->removed_)
    {
      this->removed_ = true;
      result = UPF_OS::sema_destroy (&this->semaphore_);
    }
  return result;
}

UPF_INLINE int
UPF_Semaphore::acquire (void)
{
// UPF_TRACE ("UPF_Semaphore::acquire");
  return UPF_OS::sema_wait (&this->semaphore_);
}

#if 0 // wangy
UPF_INLINE int
UPF_Semaphore::acquire (UPF_Time_Value &tv)
{
// UPF_TRACE ("UPF_Semaphore::acquire");
  return UPF_OS::sema_wait (&this->semaphore_, tv);
}

UPF_INLINE int
UPF_Semaphore::acquire (UPF_Time_Value *tv)
{
// UPF_TRACE ("UPF_Semaphore::acquire");
  return UPF_OS::sema_wait (&this->semaphore_, tv);
}
#endif 

UPF_INLINE int
UPF_Semaphore::tryacquire (void)
{
// UPF_TRACE ("UPF_Semaphore::tryacquire");
  return UPF_OS::sema_trywait (&this->semaphore_);
}

UPF_INLINE int
UPF_Semaphore::release (void)
{
// UPF_TRACE ("UPF_Semaphore::release");
  return UPF_OS::sema_post (&this->semaphore_);
}

UPF_INLINE int
UPF_Semaphore::release (unsigned int release_count)
{
// UPF_TRACE ("UPF_Semaphore::release");
  return UPF_OS::sema_post (&this->semaphore_, release_count);
}

// Acquire semaphore ownership.  This calls <acquire> and is only
// here to make the <UPF_Semaphore> interface consistent with the
// other synchronization APIs.

UPF_INLINE int
UPF_Semaphore::acquire_read (void)
{
  return this->acquire ();
}

// Acquire semaphore ownership.  This calls <acquire> and is only
// here to make the <UPF_Semaphore> interface consistent with the
// other synchronization APIs.

UPF_INLINE int
UPF_Semaphore::acquire_write (void)
{
  return this->acquire ();
}

// Conditionally acquire semaphore (i.e., won't block).  This calls
// <tryacquire> and is only here to make the <UPF_Semaphore>
// interface consistent with the other synchronization APIs.

UPF_INLINE int
UPF_Semaphore::tryacquire_read (void)
{
  return this->tryacquire ();
}

// Conditionally acquire semaphore (i.e., won't block).  This calls
// <tryacquire> and is only here to make the <UPF_Semaphore>
// interface consistent with the other synchronization APIs.

UPF_INLINE int
UPF_Semaphore::tryacquire_write (void)
{
  return this->tryacquire ();
}

// This is only here to make the <UPF_Semaphore> interface consistent
// with the other synchronization APIs.  Assumes the caller has
// already acquired the semaphore using one of the above calls, and
// returns 0 (success) always.
UPF_INLINE int
UPF_Semaphore::tryacquire_write_upgrade (void)
{
  return 0;
}