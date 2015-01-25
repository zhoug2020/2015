#ifndef UPF_AT_THREAD_EXIT_H_
#define UPF_AT_THREAD_EXIT_H_

#include "..\OS_Adapter\UPF_OS.h"

class UPF_Thread_Manager;
class UPF_Thread_Descriptor;

class /*UPF_Export*/ UPF_At_Thread_Exit
{
  friend class UPF_Thread_Descriptor;
  friend class UPF_Thread_Manager;
public:
  /// Default constructor
  UPF_At_Thread_Exit (void);

  /// The destructor
  virtual ~UPF_At_Thread_Exit (void);

  /// At_Thread_Exit has the ownership?
  int is_owner (void) const;

  /// Set the ownership of the At_Thread_Exit.
  int is_owner (int owner);

  /// This At_Thread_Exit was applied?
  int was_applied (void) const;

  /// Set applied state of At_Thread_Exit.
  int was_applied (int applied);

protected:
  /// The next At_Thread_Exit hook in the list.
  UPF_At_Thread_Exit *next_;

  /// Do the apply if necessary
  void do_apply (void);

  /// The apply method.
  virtual void apply (void) = 0;

  /// The Thread_Descriptor where this at is registered.
  UPF_Thread_Descriptor* td_;

  /// The at was applied?
  int was_applied_;

  /// The at has the ownership of this?
  int is_owner_;
};

class /*UPF_Export*/ UPF_At_Thread_Exit_Func : public UPF_At_Thread_Exit
{
public:
   /// Constructor
   UPF_At_Thread_Exit_Func (void *object,
                            UPF_CLEANUP_FUNC func,
                            void *param = 0);

  virtual ~UPF_At_Thread_Exit_Func (void);

protected:
   /// The object to be cleanup
   void *object_;

   /// The cleanup func
   UPF_CLEANUP_FUNC func_;

   /// A param if required
   void *param_;

   /// The apply method
   void apply (void);
};

#endif