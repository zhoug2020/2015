#include "UPF_Thread.h"


int UPF_Thread::spawn (UPF_THR_FUNC func,
                   void *arg,
                   long flags,
                   UPF_thread_t *t_id,
                   UPF_hthread_t *t_handle,
                   long priority,
                   void *thr_stack,
                   size_t thr_stack_size,
                   UPF_Thread_Adapter *thread_adapter)
{

	return UPF_OS::thr_create (func,
                             arg,
                             flags,
                             t_id,
                             t_handle,
                             priority,
                             thr_stack,
                             thr_stack_size,
                             thread_adapter);
}

size_t UPF_Thread::spawn_n (size_t n,
                     UPF_THR_FUNC func,
                     void *arg,
                     long flags,
                     long priority,
                     void *stack[],
                     size_t stack_size[],
                     UPF_Thread_Adapter *thread_adapter)
{
 // UPF_TRACE ("UPF_Thread::spawn_n");
 
  UPF_thread_t t_id;
  size_t i;

  for (i = 0; i < n; i++)
    // Bail out if error occurs.
    if (UPF_OS::thr_create (func,
                            arg,
                            flags,
                            &t_id,
                            0,
                            priority,
                            stack == 0 ? 0 : stack[i],
                            stack_size == 0 ? UPF_DEFAULT_THREAD_STACKSIZE : stack_size[i],
                            thread_adapter) != 0)
      break;

  return i;
}

size_t UPF_Thread::spawn_n (UPF_thread_t thread_ids[],
                     size_t n,
                     UPF_THR_FUNC func,
                     void *arg,
                     long flags,
                     long priority,
                     void *stack[],
                     size_t stack_size[],
                     UPF_hthread_t thread_handles[],
                     UPF_Thread_Adapter *thread_adapter)
{
//  UPF_TRACE ("UPF_Thread::spawn_n");
  size_t i;

  for (i = 0; i < n; i++)
    {
      UPF_thread_t t_id;
      UPF_hthread_t t_handle;

      int result =
        UPF_OS::thr_create (func,
                            arg,
                            flags,
                            &t_id,
                            &t_handle,
                            priority,
                            stack == 0 ? 0 : stack[i],
                            stack_size == 0 ? UPF_DEFAULT_THREAD_STACKSIZE : stack_size[i],
                            thread_adapter);

      if (result == 0)
        {
          if (thread_ids != 0)
            thread_ids[i] = t_id;
          if (thread_handles != 0)
            thread_handles[i] = t_handle;
        }
      else
        // Bail out if error occurs.
        break;
    }

  return i;
}
int UPF_Thread::join (UPF_hthread_t wait_for,UPF_THR_FUNC_RETURN * status)
{
	  return UPF_OS::thr_join (wait_for, status);
}
int UPF_Thread::resume (UPF_hthread_t t_id)
{
	  return UPF_OS::thr_continue (t_id);
}
int UPF_Thread::suspend (UPF_hthread_t t_id)
{
	 return UPF_OS::thr_suspend (t_id);
}

int UPF_Thread::cancel (UPF_hthread_t t_id)
{
	return UPF_OS::thr_cancel(t_id);
}
int UPF_Thread::kill (UPF_thread_t t_id, int signum)
{
	 return UPF_OS::thr_kill (t_id, signum);
}


int UPF_Thread::getprio (UPF_hthread_t ht_id, int &priority)
{
	return UPF_OS::thr_getprio (ht_id, priority);
}
int UPF_Thread::getprio (UPF_hthread_t ht_id, int &priority, int &policy)
{
	return UPF_OS::thr_getprio (ht_id, priority,policy);
}

int UPF_Thread::setprio (UPF_hthread_t ht_id, int priority, int policy )
{
	return UPF_OS::thr_setprio (ht_id, priority, policy);
}

void UPF_Thread::yield (void)
{
	 UPF_OS::thr_yield ();
}


UPF_INLINE int
UPF_Thread::keycreate (UPF_thread_key_t *keyp, UPF_THR_DEST destructor,void *inst)
{
  return UPF_OS::thr_key_create (keyp, destructor, inst);
}



UPF_INLINE int
UPF_Thread::keyfree (UPF_thread_key_t key)
{
  UPF_TRACE ("UPF_Thread::keyfree");
  return UPF_OS::thr_key_free (key);
}


UPF_INLINE int
UPF_Thread::setspecific (UPF_thread_key_t key, void *value)
{
  return UPF_OS::thr_set_specific (key, value);
}



UPF_INLINE int
UPF_Thread::getspecific (UPF_thread_key_t key, void **valuep)
{
  return UPF_OS::thr_get_specific (key, valuep);
}

UPF_INLINE UPF_thread_t 
UPF_Thread::self (void)
{
	return UPF_OS::thr_self();
}
