#include "UPF_Object_Manager_Base.h"


UPF_Object_Manager_Base::UPF_Object_Manager_Base (void)
  : object_manager_state_ (OBJ_MAN_UNINITIALIZED)
  , dynamically_allocated_ (false)
  , next_ (0)
{
}

UPF_Object_Manager_Base::~UPF_Object_Manager_Base (void)
{
#if defined (UPF_HAS_NONSTATIC_OBJECT_MANAGER)
  // Clear the flag so that fini () doesn't delete again.
  dynamically_allocated_ = false;
#endif /* UPF_HAS_NONSTATIC_OBJECT_MANAGER */
}

int
UPF_Object_Manager_Base::starting_up_i ()
{
  return object_manager_state_ < OBJ_MAN_INITIALIZED;
}

int
UPF_Object_Manager_Base::shutting_down_i ()
{
  return object_manager_state_ > OBJ_MAN_INITIALIZED;
}