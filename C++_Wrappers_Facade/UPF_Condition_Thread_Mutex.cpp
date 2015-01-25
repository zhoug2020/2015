#include "UPF_Condition_Thread_Mutex.h"


UPF_Condition_Attributes::UPF_Condition_Attributes (int type)
{
  (void) UPF_OS::condattr_init (this->attributes_, type);
}


UPF_Condition_Attributes::~UPF_Condition_Attributes (void)
{
  UPF_OS::condattr_destroy (this->attributes_);
}

UPF_ALLOC_HOOK_DEFINE(UPF_Condition_Thread_Mutex)
UPF_Condition_Thread_Mutex::UPF_Condition_Thread_Mutex (UPF_Thread_Mutex &m,
                                                        const UPF_TCHAR *name,
                                                        void *arg)
  : mutex_ (m),
    removed_ (false)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::UPF_Condition_Thread_Mutex");
  if (UPF_OS::cond_init (&this->cond_,
                         (short) USYNC_THREAD,
                         name,
                         arg) != 0)
    UPF_ERROR ((LM_ERROR,
                UPF_TEXT ("%p\n"),
                UPF_TEXT ("UPF_Condition_Thread_Mutex::UPF_Condition_Thread_Mutex")));
}

UPF_Condition_Thread_Mutex::
UPF_Condition_Thread_Mutex (UPF_Thread_Mutex &m,
                            UPF_Condition_Attributes &attributes,
                            const UPF_TCHAR *name,
                            void *arg)
  : mutex_ (m),
    removed_ (false)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::UPF_Condition_Thread_Mutex");
  if (UPF_OS::cond_init (&this->cond_, attributes.attributes_,
                         name, arg) != 0)
    UPF_ERROR ((LM_ERROR, UPF_TEXT ("%p\n"),
                UPF_TEXT ("UPF_Condition_Thread_Mutex::UPF_Condition_Thread_Mutex")));
}

UPF_Condition_Thread_Mutex::~UPF_Condition_Thread_Mutex (void)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::~UPF_Condition_Thread_Mutex");
  this->remove ();
}

int
UPF_Condition_Thread_Mutex::remove (void)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::remove");

  int result = 0;

  if (!this->removed_)
    {
      this->removed_ = true;

      while ((result = UPF_OS::cond_destroy (&this->cond_)) == -1
             && errno == EBUSY)
        {
          UPF_OS::cond_broadcast (&this->cond_);
          UPF_OS::thr_yield ();
        }
    }
  return result;

}


// Peform an "alertable" timed wait.  If the argument <abstime> == 0
// then we do a regular <cond_wait>, else we do a timed wait for up to
// <abstime> using the <cond_timedwait> function.

int
UPF_Condition_Thread_Mutex::wait (void)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::wait");
  return UPF_OS::cond_wait (&this->cond_, &this->mutex_.lock_);
}

#if 0	// wangy
int
UPF_Condition_Thread_Mutex::wait (UPF_Thread_Mutex &mutex,
                                  const UPF_Time_Value *abstime)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::wait");
  return UPF_OS::cond_timedwait (&this->cond_,
                                 &mutex.lock_,
                                 const_cast <UPF_Time_Value *> (abstime));
}

int
UPF_Condition_Thread_Mutex::wait (const UPF_Time_Value *abstime)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::wait");
  return this->wait (this->mutex_, abstime);
}

#endif 
int
UPF_Condition_Thread_Mutex::signal (void)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::signal");
  return UPF_OS::cond_signal (&this->cond_);
}

int
UPF_Condition_Thread_Mutex::broadcast (void)
{
// UPF_TRACE ("UPF_Condition_Thread_Mutex::broadcast");
  return UPF_OS::cond_broadcast (&this->cond_);
}