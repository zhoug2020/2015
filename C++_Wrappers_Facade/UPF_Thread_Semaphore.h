#ifndef UPF_THREAD_SEMAPHORE_H_
#define UPF_THREAD_SEMAPHORE_H_

#include "UPF_Semaphore.h"

class /*UPF_Export*/ UPF_Thread_Semaphore : public UPF_Semaphore
{
public:
 
  UPF_Thread_Semaphore (unsigned int count = 1,
                        const UPF_TCHAR *name = 0,
                        void * = 0,
                        int max = 0x7FFFFFFF);


  ~UPF_Thread_Semaphore (void);


  void dump (void) const;


  UPF_ALLOC_HOOK_DECLARE;
};
#endif 