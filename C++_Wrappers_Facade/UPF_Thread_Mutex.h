#ifndef UPF_THREAD_MUTEX_H_
#define UPF_THREAD_MUTEX_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "..\OS_Adapter\OS_Adapter_Common\UPF_wchar.h"

class UPF_Export UPF_Thread_Mutex
{
 friend class UPF_Condition_Thread_Mutex;
public:
  /// Constructor.
  UPF_Thread_Mutex (const UPF_TCHAR *name = 0,
                    UPF_mutexattr_t *attributes = 0);

  /// Implicitly destroy the mutex.
  ~UPF_Thread_Mutex (void);

 
  int remove (void);
 
  int acquire (void);
#if 0  //wangy 
  int acquire (ACE_Time_Value &tv);
  int acquire (ACE_Time_Value *tv);
#endif
  
  int tryacquire (void);
  int release (void);

  int acquire_read (void);
  int acquire_write (void);

  int tryacquire_read (void);
  int tryacquire_write (void);
  int tryacquire_write_upgrade (void);

 
  const UPF_thread_mutex_t &lock (void) const;

  void dump (void) const;

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;

  UPF_thread_mutex_t lock_;
  bool removed_;

private:
  // = Prevent assignment and initialization.
  void operator= (const UPF_Thread_Mutex &);
  UPF_Thread_Mutex (const UPF_Thread_Mutex &);

};

#endif