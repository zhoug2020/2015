#include "UPF_Recursive_Thread_Mutex.h"


UPF_ALLOC_HOOK_DEFINE(UPF_Recursive_Thread_Mutex)

UPF_Recursive_Thread_Mutex::UPF_Recursive_Thread_Mutex (const UPF_TCHAR *name,
                                                        UPF_mutexattr_t *arg)
  : removed_ (false)
{
  // UPF_TRACE ("UPF_Recursive_Thread_Mutex::UPF_Recursive_Thread_Mutex");
   if (UPF_OS::recursive_mutex_init (&this->lock_,
                                     name,
                                     arg) == -1)
   
	 UPF_ERROR ((LM_ERROR,UPF_TEXT ("%p\n"),UPF_TEXT ("recursive_mutex_init")));
}

UPF_Recursive_Thread_Mutex::~UPF_Recursive_Thread_Mutex (void)
{
  // UPF_TRACE ("UPF_Recursive_Thread_Mutex::~UPF_Recursive_Thread_Mutex");
  this->remove ();
}

int
UPF_Recursive_Thread_Mutex::remove (void)
{
// UPF_TRACE ("UPF_Recursive_Thread_Mutex::remove");
  int result = 0;
  if (this->removed_ == false)
    {
      this->removed_ = true;
      result = UPF_OS::recursive_mutex_destroy (&this->lock_);
    }
  return result;
}

 UPF_recursive_thread_mutex_t &
UPF_Recursive_Thread_Mutex::mutex (void)
{
  return lock_;
}

 UPF_thread_mutex_t &
UPF_Recursive_Thread_Mutex::get_nesting_mutex (void)
{
#if defined (UPF_HAS_RECURSIVE_MUTEXES)
  return static_cast<UPF_thread_mutex_t &> (lock_);
#else
  return lock_.nesting_mutex_;
#endif /* UPF_HAS_RECURSIVE_MUTEXES */
}

 void
UPF_Recursive_Thread_Mutex::set_thread_id (UPF_thread_t t)
{
// UPF_TRACE ("UPF_Recursive_Thread_Mutex::set_thread_id");
#if defined (UPF_HAS_RECURSIVE_MUTEXES)
  UPF_UNUSED_ARG (t);
#else  /* ! UPF_HAS_RECURSIVE_MUTEXES */
  this->lock_.owner_id_ = t;
#endif /* ! UPF_HAS_RECURSIVE_MUTEXES */
}

 int
UPF_Recursive_Thread_Mutex::acquire (void)
{
  return UPF_OS::recursive_mutex_lock (&this->lock_);
}

int
UPF_Recursive_Thread_Mutex::release (void)
{
  return UPF_OS::recursive_mutex_unlock (&this->lock_);
}

int
UPF_Recursive_Thread_Mutex::tryacquire (void)
{
  return UPF_OS::recursive_mutex_trylock (&this->lock_);
}


#if 0   // wangy
int
UPF_Recursive_Thread_Mutex::acquire (UPF_Time_Value &tv)
{
  return UPF_OS::recursive_mutex_lock (&this->lock_, tv);
}

int
UPF_Recursive_Thread_Mutex::acquire (UPF_Time_Value *tv)
{
  return UPF_OS::recursive_mutex_lock (&this->lock_, tv);
}
#endif 

int
UPF_Recursive_Thread_Mutex::acquire_read (void)
{
  return this->acquire ();
}

 int
UPF_Recursive_Thread_Mutex::acquire_write (void)
{
  return this->acquire ();
}

 int
UPF_Recursive_Thread_Mutex::tryacquire_read (void)
{
  return this->tryacquire ();
}

 int
UPF_Recursive_Thread_Mutex::tryacquire_write (void)
{
  return this->tryacquire ();
}

 int
UPF_Recursive_Thread_Mutex::tryacquire_write_upgrade (void)
{
  return 0;
}
