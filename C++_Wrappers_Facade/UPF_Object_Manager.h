#ifndef UPF_OBJECT_MANAGER_H_
#define UPF_OBJECT_MANAGER_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "UPF_Object_Manager_Base.h"

class /*UPF_Export*/ UPF_Object_Manager  : public UPF_Object_Manager_Base
{
public:
	  /// Unique identifiers for preallocated objects.  Please see
  /// ace/Managed_Object.h for information on accessing preallocated
  /// objects.
  enum Preallocated_Object
    {
      UPF_FILECACHE_LOCK,
      UPF_STATIC_OBJECT_LOCK,
      UPF_MT_CORBA_HANDLER_LOCK,
      UPF_DUMP_LOCK,
      UPF_SIG_HANDLER_LOCK,
      UPF_SINGLETON_NULL_LOCK,
      UPF_SINGLETON_RECURSIVE_THREAD_LOCK,
      UPF_THREAD_EXIT_LOCK,
      UPF_TOKEN_MANAGER_CREATION_LOCK,
      UPF_TOKEN_INVARIANTS_CREATION_LOCK,
      UPF_PROACTOR_EVENT_LOOP_LOCK,

      // Hook for preallocated objects provided by application.
      // UPF_APPLICATION_PREALLOCATED_OBJECT_DECLARATIONS
      UPF_PREALLOCATED_OBJECTS  // This enum value must be last!
    };

  /// Unique identifiers for preallocated arrays.  Please see
  /// ace/Managed_Object.h for information on accessing preallocated
  /// arrays.
  enum Preallocated_Array
    {
      /// There currently are no preallocated arrays in the ACE
      /// library.  If the application doesn't have any, make sure
      /// the the preallocated_array size is at least one by declaring
      /// this dummy . . .
      UPF_EMPTY_PREALLOCATED_ARRAY,

      /// Hook for preallocated arrays provided by application.
      //UPF_APPLICATION_PREALLOCATED_ARRAY_DECLARATIONS

      UPF_PREALLOCATED_ARRAYS  // This enum value must be last!
    };



	static int starting_up (void);
	static int shutting_down (void);
	
	static UPF_Object_Manager *instance (void);

	/// Table of preallocated objects.
	static void *preallocated_object[UPF_PREALLOCATED_OBJECTS];
	
	/// Table of preallocated arrays.
	static void *preallocated_array[UPF_PREALLOCATED_ARRAYS];
	
private:
	/// Singleton pointer.
	static UPF_Object_Manager *instance_;
	
	
};



#endif 