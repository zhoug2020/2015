#include "UPF_Active_Object.h"
#include "UPF_Thread.h"


int UPF_Active_Object_Base::svc (void)
{
	return 0;
}
int UPF_Active_Object_Base::activate (	long			flags,
									int				force_active,
									long			priority,
									UPF_hthread_t *	thread_handle,
									void *			stack,
									size_t			stack_size,
									UPF_thread_t *	thread_id )
{



return 	UPF_Thread::spawn(	UPF_Active_Object_Base::svc_run,
							(void *) this,
							flags,
							thread_id,
							thread_handle,
							priority,
							stack,
							stack_size
						  );

}


UPF_THR_FUNC_RETURN  UPF_Active_Object_Base::svc_run (void * args)
{
#ifdef	UPF_CATCH_THREAD_EXCEPTION

	UPF_Active_Object_Base * active_object_ptr = static_cast<UPF_Active_Object_Base *>(args);
	__try
	{
		int svc_status = active_object_ptr->svc();

		return static_cast<UPF_THR_FUNC_RETURN>(svc_status);
	}
	__except(EXCEPTION_EXECUTE_HANDLER)
	{
		return -1;
	}

#else

	UPF_Active_Object_Base * active_object_ptr = static_cast<UPF_Active_Object_Base *>(args);
	int svc_status = active_object_ptr->svc();

	return static_cast<UPF_THR_FUNC_RETURN>(svc_status);

#endif // UPF_CATCH_THREAD_EXCEPTION

}