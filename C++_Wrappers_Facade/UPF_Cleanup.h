#ifndef UPF_CLEANUP_H_
#define UPF_CLEANUP_H_

#include "..\OS_Adapter\UPF_OS.h"

class /*UPF_Export*/ UPF_Cleanup
{
public:
  /// No-op constructor.
  UPF_Cleanup (void);

  /// Destructor.
  virtual ~UPF_Cleanup (void);

  /// Cleanup method that, by default, simply deletes itself.
  virtual void cleanup (void *param = 0);
};

// Adapter for cleanup, used by UPF_Object_Manager.
extern "C" UPF_Export
void UPF_CLEANUP_DESTROYER_NAME (UPF_Cleanup *, void *param = 0);



/**
 * @class UPF_Cleanup_Info
 *
 * @brief Hold cleanup information for thread/process
 */
class /*UPF_Export*/ UPF_Cleanup_Info
{
public:
  /// Default constructor.
  UPF_Cleanup_Info (void);

  /// Equality operator.
  bool operator== (const UPF_Cleanup_Info &o) const;

  /// Inequality operator.
  bool operator!= (const UPF_Cleanup_Info &o) const;

  /// Point to object that gets passed into the <cleanup_hook_>.
  void *object_;

  /// Cleanup hook that gets called back.
  UPF_CLEANUP_FUNC cleanup_hook_;

  /// Parameter passed to the <cleanup_hook_>.
  void *param_;
};


#if 0 //wangy
class UPF_Cleanup_Info_Node;

/**
 * @class UPF_OS_Exit_Info
 *
 * @brief Hold Object Manager cleanup (exit) information.
 *
 * For internal use by the ACE library, only.
 */
class UPF_Export UPF_OS_Exit_Info
{
public:
  /// Default constructor.
  UPF_OS_Exit_Info (void);

  /// Destructor.
  ~UPF_OS_Exit_Info (void);

  /// Use to register a cleanup hook.
  int at_exit_i (void *object, UPF_CLEANUP_FUNC cleanup_hook, void *param);

  /// Look for a registered cleanup hook object.  Returns 1 if already
  /// registered, 0 if not.
  int find (void *object);

  /// Call all registered cleanup hooks, in reverse order of
  /// registration.
  void call_hooks ();

private:
  /**
   * Keeps track of all registered objects.  The last node is only
   * used to terminate the list (it doesn't contain a valid
   * UPF_Cleanup_Info).
   */
  UPF_Cleanup_Info_Node *registered_objects_;


#endif  //wangy 


#endif