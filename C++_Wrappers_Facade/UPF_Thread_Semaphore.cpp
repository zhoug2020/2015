#include "UPF_Thread_Semaphore.h"

UPF_Thread_Semaphore::UPF_Thread_Semaphore (unsigned int count,
                                            const UPF_TCHAR *name,
                                            void *arg,
                                            int max)
  : UPF_Semaphore (count, USYNC_THREAD, name, arg, max)
{
// UPF_TRACE ("UPF_Thread_Semaphore::UPF_Thread_Semaphore");
}

UPF_Thread_Semaphore::~UPF_Thread_Semaphore (void)
{
}