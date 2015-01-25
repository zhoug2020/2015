#include "UPF_Thread_Mutex.h"

UPF_ALLOC_HOOK_DEFINE(UPF_Thread_Mutex)



UPF_Thread_Mutex::UPF_Thread_Mutex (const UPF_TCHAR *name, UPF_mutexattr_t *arg)
  : removed_ (false)
{
//  UPF_TRACE ("UPF_Thread_Mutex::UPF_Thread_Mutex");

#if 0   // wangy
  if (UPF_OS::thread_mutex_init (&this->lock_,0,name,arg) != 0)
    UPF_ERROR ((LM_ERROR,
                UPF_TEXT ("%p\n"),
                UPF_TEXT ("UPF_Thread_Mutex::UPF_Thread_Mutex")));

#endif 
	UPF_OS::thread_mutex_init (&this->lock_,0,name,arg);

}	


UPF_Thread_Mutex::~UPF_Thread_Mutex (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::~UPF_Thread_Mutex");
  this->remove ();
}

const UPF_thread_mutex_t & UPF_Thread_Mutex::lock (void) const
{
// UPF_TRACE ("UPF_Thread_Mutex::lock");
  return this->lock_;
}

int UPF_Thread_Mutex::acquire_read (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::acquire_read");
  return UPF_OS::thread_mutex_lock (&this->lock_);
}

int UPF_Thread_Mutex::acquire_write (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::acquire_write");
  return UPF_OS::thread_mutex_lock (&this->lock_);
}

int UPF_Thread_Mutex::tryacquire_read (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::tryacquire_read");
  return UPF_OS::thread_mutex_trylock (&this->lock_);
}

int UPF_Thread_Mutex::tryacquire_write (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::tryacquire_write");
  return UPF_OS::thread_mutex_trylock (&this->lock_);
}

int UPF_Thread_Mutex::tryacquire_write_upgrade (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::tryacquire_write_upgrade");
  return 0;
}

int UPF_Thread_Mutex::acquire (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::acquire");
  return UPF_OS::thread_mutex_lock (&this->lock_);
}


//int UPF_Thread_Mutex::acquire (UPF_Time_Value &tv);
//int UPF_Thread_Mutex::acquire (UPF_Time_Value *tv);

int UPF_Thread_Mutex::tryacquire (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::tryacquire");
  return UPF_OS::thread_mutex_trylock (&this->lock_);
}

int UPF_Thread_Mutex::release (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::release");
  return UPF_OS::thread_mutex_unlock (&this->lock_);
}
int UPF_Thread_Mutex::remove (void)
{
// UPF_TRACE ("UPF_Thread_Mutex::remove");
  int result = 0;
  if (this->removed_ == false)
    {
      this->removed_ = true;
      result = UPF_OS::thread_mutex_destroy (&this->lock_);
    }
  return result;
}

