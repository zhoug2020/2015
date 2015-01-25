#ifndef MANAGED_OBJECT_H_
#define MANAGED_OBJECT_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "UPF_Cleanup.h"
#include "UPF_Object_Manager.h"

template <class TYPE>
class UPF_Cleanup_Adapter : public UPF_Cleanup
{
public:
  /// Default constructor.
  UPF_Cleanup_Adapter (void);

  /// Virtual destructor, needed by some compilers for vtable placement.
  virtual ~UPF_Cleanup_Adapter (void);

  /// Accessor for contained object.
  TYPE &object (void);

private:
  UPF_UNIMPLEMENTED_FUNC (UPF_Cleanup_Adapter (const UPF_Cleanup_Adapter<TYPE> &))
  UPF_UNIMPLEMENTED_FUNC (void operator= (const UPF_Cleanup_Adapter<TYPE> &))

  /// Contained object.
  TYPE object_;
};

template <class TYPE>
class UPF_Managed_Object
{
public:
  static TYPE *get_preallocated_object (UPF_Object_Manager::Preallocated_Object identifier)
  {
    // The preallocated objects are in a separate, "read-only" array so
    // that this function doesn't need a lock.  Also, because it is
    // intended _only_ for use with hard-code values, it performs no
    // range checking on "id".

    // Cast the return type of the the object pointer based
    // on the type of the function template parameter.
    return &((UPF_Cleanup_Adapter<TYPE> *)
             UPF_Object_Manager::preallocated_object[identifier])->object ();
  }
  // Get the preallocated object identified by "id".  Returns a
  // pointer to the object.  Beware: no error indication is provided,
  // because it can _only_ be used for accessing preallocated objects.
  // @note The function definition is inlined here so that it compiles
  // on AIX 4.1 w/xlC v. 3.01.

  static TYPE *get_preallocated_array (UPF_Object_Manager::Preallocated_Array identifier)
  {
    // The preallocated array are in a separate, "read-only" array so
    // that this function doesn't need a lock.  Also, because it is
    // intended _only_ for use with hard-code values, it performs no
    // range checking on "id".

    // Cast the return type of the the object pointer based
    // on the type of the function template parameter.
    return &((UPF_Cleanup_Adapter<TYPE> *)
             UPF_Object_Manager::preallocated_array[identifier])->object ();
  }
  // Get the preallocated array identified by "id".  Returns a
  // pointer to the array.  Beware: no error indication is provided,
  // because it can _only_ be used for accessing preallocated arrays.
  // @note The function definition is inlined here so that it compiles
  // on AIX 4.1 w/xlC v. 3.01.

protected:

  // Disallow instantiation of this class.
  UPF_UNIMPLEMENTED_FUNC (UPF_Managed_Object (void))

private:

  UPF_UNIMPLEMENTED_FUNC (UPF_Managed_Object (const UPF_Managed_Object<TYPE> &))
  UPF_UNIMPLEMENTED_FUNC (void operator= (const UPF_Managed_Object<TYPE> &))
};



/////////////////////////////////////////////////////////////////
//
template <class TYPE>
UPF_Cleanup_Adapter<TYPE>::~UPF_Cleanup_Adapter (void)
{
}

template <class TYPE>
UPF_Cleanup_Adapter<TYPE>::UPF_Cleanup_Adapter (void)
  // Note: don't explicitly initialize "object_", because TYPE may not
  // have a default constructor.  Let the compiler figure it out . . .
{
}

template <class TYPE>
TYPE &
UPF_Cleanup_Adapter<TYPE>::object (void)
{
  return this->object_;
}


#endif