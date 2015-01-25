#ifndef UPF_SEMAPHORE_H_
#define UPF_SEMAPHORE_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "..\OS_Adapter\OS_Adapter_Common\UPF_wchar.h"

class /*UPF_Export*/ UPF_Semaphore
{
public:
  // = Initialization and termination.
  /// Initialize the semaphore, with initial value of "count".
  UPF_Semaphore (unsigned int count = 1, // By default make this unlocked.
                 int type = USYNC_THREAD,
                 const UPF_TCHAR *name = 0,
                 void * = 0,
                 int max = 0x7fffffff);


  ~UPF_Semaphore (void);

  int remove (void);

  int acquire (void);

 #if 0  // wangy
 
  int acquire (UPF_Time_Value &tv);

  int acquire (UPF_Time_Value *tv);
#endif 

  
  int tryacquire (void);

  int release (void);

  int release (unsigned int release_count);

  int acquire_read (void);
 
  int acquire_write (void);
  
  int tryacquire_read (void);
  
  int tryacquire_write (void);

  int tryacquire_write_upgrade (void);

  void dump (void) const;

  UPF_ALLOC_HOOK_DECLARE;

  const UPF_sema_t &lock (void) const;

protected:
  UPF_sema_t semaphore_;

  bool removed_;

private:
  
  void operator= (const UPF_Semaphore &);
  UPF_Semaphore (const UPF_Semaphore &);
};


#endif
