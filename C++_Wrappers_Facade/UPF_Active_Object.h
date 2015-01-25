#ifndef UPF_ACTIVE_OBJECT_H_
#define UPF_ACTIVE_OBJECT_H_

#include "..\OS_Adapter\UPF_OS.h"

class UPF_Export UPF_Active_Object_Base
{
public:
	 virtual int svc (void);
	 virtual int activate (	long flags = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
							int force_active = 0,
							long priority = UPF_DEFAULT_THREAD_PRIORITY,
                            UPF_hthread_t * thread_handle= 0,
							void *stack = 0,
							size_t stack_size = 0,
							UPF_thread_t * thread_id = 0);
  
	 static UPF_THR_FUNC_RETURN svc_run (void * args);


};

#endif