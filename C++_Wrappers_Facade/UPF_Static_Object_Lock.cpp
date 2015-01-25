#include "UPF_Static_Object_Lock.h"
#include "Managed_Object.h"



typedef UPF_Cleanup_Adapter<UPF_Recursive_Thread_Mutex> UPF_Static_Object_Lock_Type;
static UPF_Static_Object_Lock_Type *UPF_Static_Object_Lock_lock = 0;

// UPF_SHOULD_MALLOC_STATIC_OBJECT_LOCK isn't (currently) used by ACE.
// But, applications may find it useful for avoiding recursive calls
// if they have overridden operator new.  Thanks to Jody Hagins
// <jody@atdesk.com> for contributing it.

UPF_Recursive_Thread_Mutex *
UPF_Static_Object_Lock::instance (void)
{
  if (UPF_Object_Manager::starting_up ()  ||
      UPF_Object_Manager::shutting_down ())
    {
      // The preallocated UPF_STATIC_OBJECT_LOCK has not been
      // constructed yet.  Therefore, the program is single-threaded
      // at this point.  Or, the UPF_Object_Manager instance has been
      // destroyed, so the preallocated lock is not available.
      // Allocate a lock to use, for interface compatibility, though
      // there should be no contention on it.
      if (UPF_Static_Object_Lock_lock == 0)
        {

        UPF_NEW_RETURN (UPF_Static_Object_Lock_lock,
                        UPF_Cleanup_Adapter<UPF_Recursive_Thread_Mutex>,
                        0);

        }

      // Can't register with the UPF_Object_Manager here!  The lock's
      // declaration is visible to the UPF_Object_Manager destructor,
      // so it will clean it up as a special case.

      return &UPF_Static_Object_Lock_lock->object ();
    }
  else
    // Return the preallocated UPF_STATIC_OBJECT_LOCK.
    return
      UPF_Managed_Object<UPF_Recursive_Thread_Mutex>::get_preallocated_object
        (UPF_Object_Manager::UPF_STATIC_OBJECT_LOCK);
}

void
UPF_Static_Object_Lock::cleanup_lock (void)
{
# if defined(UPF_SHOULD_MALLOC_STATIC_OBJECT_LOCK)
    // It was malloc'd, so we need to explicitly call the dtor
    // and then free the memory.
    UPF_DES_FREE (UPF_Static_Object_Lock_lock,
                  UPF_OS::free,
                  UPF_Static_Object_Lock_Type);
# else  /* ! UPF_SHOULD_MALLOC_STATIC_OBJECT_LOCK */
    delete UPF_Static_Object_Lock_lock;
# endif /* ! UPF_SHOULD_MALLOC_STATIC_OBJECT_LOCK */
    UPF_Static_Object_Lock_lock = 0;
}
//#endif /* UPF_HAS_THREADS */