#ifndef UPF_OBJECT_MANAGER_BASE_H_
#define  UPF_OBJECT_MANAGER_BASE_H_


#include "..\OS_Adapter\UPF_OS.h"
/**
 * @class UPF_Object_Manager_Base
 *
 * @brief Base class for UPF_Object_Manager(s).
 *
 * Encapsulates the most useful UPF_Object_Manager data structures.
 */
class /*UPF_Export*/ UPF_Object_Manager_Base
{
protected:
  /// Default constructor.
  UPF_Object_Manager_Base (void);

  /// Destructor.
  virtual ~UPF_Object_Manager_Base (void);

public:
  /**
   * Explicitly initialize.  Returns 0 on success, -1 on failure due
   * to dynamic allocation failure (in which case errno is set to
   * ENOMEM), or 1 if it had already been called.
   */
  virtual int init (void) = 0;

  /**
   * Explicitly destroy.  Returns 0 on success, -1 on failure because
   * the number of fini () calls hasn't reached the number of init ()
   * calls, or 1 if it had already been called.
   */
  virtual int fini (void) = 0;

  enum Object_Manager_State
    {
      OBJ_MAN_UNINITIALIZED = 0,
      OBJ_MAN_INITIALIZING,
      OBJ_MAN_INITIALIZED,
      OBJ_MAN_SHUTTING_DOWN,
      OBJ_MAN_SHUT_DOWN
    };

protected:
  /**
   * Returns 1 before UPF_Object_Manager_Base has been constructed.
   * This flag can be used to determine if the program is constructing
   * static objects.  If no static object spawns any threads, the
   * program will be single-threaded when this flag returns 1.  (Note
   * that the program still might construct some static objects when
   * this flag returns 0, if UPF_HAS_NONSTATIC_OBJECT_MANAGER is not
   * defined.)
   */
  int starting_up_i (void);

  /**
   * Returns 1 after UPF_Object_Manager_Base has been destroyed.  This
   * flag can be used to determine if the program is in the midst of
   * destroying static objects.  (Note that the program might destroy
   * some static objects before this flag can return 1, if
   * UPF_HAS_NONSTATIC_OBJECT_MANAGER is not defined.)
   */
  int shutting_down_i (void);

  /// State of the Object_Manager;
  Object_Manager_State object_manager_state_;

  /**
   * Flag indicating whether the UPF_Object_Manager was dynamically
   * allocated by ACE.  (If is was dynamically allocated by the
   * application, then the application is responsible for destroying
   * it.)
   */
  bool dynamically_allocated_;

  /// Link to next Object_Manager, for chaining.
  UPF_Object_Manager_Base *next_;
private:
  // Disallow copying by not implementing the following . . .
  UPF_Object_Manager_Base (const UPF_Object_Manager_Base &);
  UPF_Object_Manager_Base &operator= (const UPF_Object_Manager_Base &);
};

#endif