#ifndef  UPF_STATIC_OBJECT_LOCK_H_
#define	 UPF_STATIC_OBJECT_LOCK_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "UPF_Recursive_Thread_Mutex.h"

class /*UPF_Export*/ UPF_Static_Object_Lock
{
public:
  /// Static lock access point.
  static UPF_Recursive_Thread_Mutex *instance (void);

  /// For use only by UPF_Object_Manager to clean up lock if it
  /// what dynamically allocated.
  static void cleanup_lock (void);
};


#endif 	