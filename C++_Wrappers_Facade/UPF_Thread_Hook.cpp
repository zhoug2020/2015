#include "UPF_Thread_Hook.h"

UPF_Thread_Hook::~UPF_Thread_Hook ()
{
}

UPF_THR_FUNC_RETURN
UPF_Thread_Hook::start (UPF_THR_FUNC func,
                        void *arg)
{
  return (func) (arg);
}

UPF_Thread_Hook *
UPF_Thread_Hook::thread_hook (UPF_Thread_Hook *hook)
{
	return static_cast<UPF_Thread_Hook *>(0); // wangy
//  return UPF_OS_Object_Manager::thread_hook (hook);
}

UPF_Thread_Hook *
UPF_Thread_Hook::thread_hook (void)
{
  return static_cast<UPF_Thread_Hook *>(0); // wangy
 // return UPF_OS_Object_Manager::thread_hook ();
}
