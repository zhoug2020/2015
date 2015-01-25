#include "Malloc_Base.h"
#include "UPF_Recursive_Thread_Mutex.h"
#include "..\OS_Adapter\OS_Adapter_Common\UPF_Guard_T.h"
#include "UPF_Static_Object_Lock.h"
#include "Malloc_Allocator.h"



// Process-wide UPF_Allocator.
UPF_Allocator *UPF_Allocator::allocator_ = 0;

// Controls whether the Allocator is deleted when we shut down (we can
// only delete it safely if we created it!)  This is no longer used;
// see UPF_Allocator::instance (void).
int UPF_Allocator::delete_allocator_ = 0;


UPF_Allocator *
UPF_Allocator::instance (void)
{
  //  UPF_TRACE ("UPF_Allocator::instance");

  if (UPF_Allocator::allocator_ == 0)
    {
      // Perform Double-Checked Locking Optimization.
      UPF_MT (UPF_GUARD_RETURN (UPF_Recursive_Thread_Mutex, UPF_mon,
                                *UPF_Static_Object_Lock::instance (), 0));

      if (UPF_Allocator::allocator_ == 0)
        {
          // Have a seat.  We want to avoid ever having to delete the
          // UPF_Allocator instance, to avoid shutdown order
          // dependencies.  UPF_New_Allocator never needs to be
          // destroyed:  its destructor is empty and its instance
          // doesn't have any state.  Therefore, sizeof
          // UPF_New_Allocator is equal to sizeof void *.  It's
          // instance just contains a pointer to its virtual function
          // table.
          //
          // So, we allocate space for the UPF_New_Allocator instance
          // in the data segment.  Because its size is the same as
          // that of a pointer, we allocate it as a pointer so that it
          // doesn't get constructed statically.  We never bother to
          // destroy it.
          static void *allocator_instance = 0;

          // Check this critical assumption.  We put it in a variable
          // first to avoid stupid compiler warnings that the
          // condition may always be true/false.
#         if !defined (UPF_NDEBUG)
          int assertion = (sizeof allocator_instance ==
                           sizeof (UPF_New_Allocator));
          UPF_ASSERT (assertion);
#         endif /* !UPF_NDEBUG */

          // Initialize the allocator_instance by using a placement
          // new.
          UPF_Allocator::allocator_ =
            new (&allocator_instance) UPF_New_Allocator;
        }
    }

  return UPF_Allocator::allocator_;
}

UPF_Allocator *
UPF_Allocator::instance (UPF_Allocator *r)
{
  UPF_TRACE ("UPF_Allocator::instance");
  UPF_MT (UPF_GUARD_RETURN (UPF_Recursive_Thread_Mutex, UPF_mon,
                            *UPF_Static_Object_Lock::instance (), 0));
  UPF_Allocator *t = UPF_Allocator::allocator_;

  // We can't safely delete it since we don't know who created it!
  UPF_Allocator::delete_allocator_ = 0;

  UPF_Allocator::allocator_ = r;
  return t;
}

void
UPF_Allocator::close_singleton (void)
{
  UPF_TRACE ("UPF_Allocator::close_singleton");

  UPF_MT (UPF_GUARD (UPF_Recursive_Thread_Mutex, UPF_mon,
                     *UPF_Static_Object_Lock::instance ()));

  if (UPF_Allocator::delete_allocator_)
    {
      // This should never be executed....  See the
      // UPF_Allocator::instance (void) method for an explanation.
      delete UPF_Allocator::allocator_;
      UPF_Allocator::allocator_ = 0;
      UPF_Allocator::delete_allocator_ = 0;
    }
}

UPF_Allocator::~UPF_Allocator (void)
{
  UPF_TRACE ("UPF_Allocator::~UPF_Allocator");
}

UPF_Allocator::UPF_Allocator (void)
{
  UPF_TRACE ("UPF_Allocator::UPF_Allocator");
}