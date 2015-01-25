#ifndef UPF_THREAD_MANAGER_H_
#define UPF_THREAD_MANAGER_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "UPF_Thread_Mutex.h"
#include "UPF_Condition_Thread_Mutex.h"

#include "Containers\UPF_Double_Linked_List.h"
#include "Containers\UPF_Free_List.h"
#include "UPF_Cleanup.h"




#if !defined (UPF_DEFAULT_THREAD_MANAGER_PREALLOC)
# define UPF_DEFAULT_THREAD_MANAGER_PREALLOC 0
#endif /* UPF_DEFAULT_THREAD_MANAGER_PREALLOC */

#if !defined (UPF_DEFAULT_THREAD_MANAGER_LWM)
# define UPF_DEFAULT_THREAD_MANAGER_LWM 1
#endif /* UPF_DEFAULT_THREAD_MANAGER_LWM */

#if !defined (UPF_DEFAULT_THREAD_MANAGER_INC)
# define UPF_DEFAULT_THREAD_MANAGER_INC 1
#endif /* UPF_DEFAULT_THREAD_MANAGER_INC */

#if !defined (UPF_DEFAULT_THREAD_MANAGER_HWM)
# define UPF_DEFAULT_THREAD_MANAGER_HWM UPF_DEFAULT_FREE_LIST_HWM
// this is a big number
#endif /* UPF_DEFAULT_THREAD_MANAGER_HWM */




class  UPF_Task_Base;

class UPF_Export UPF_Thread_Descriptor_Base : public UPF_OS_Thread_Descriptor
{

  friend class UPF_Thread_Manager;
  friend class UPF_Double_Linked_List<UPF_Thread_Descriptor_Base>;
  friend class UPF_Double_Linked_List_Iterator_Base<UPF_Thread_Descriptor_Base>;
  friend class UPF_Double_Linked_List_Iterator<UPF_Thread_Descriptor_Base>;
  friend class UPF_Double_Linked_List<UPF_Thread_Descriptor>;
  friend class UPF_Double_Linked_List_Iterator_Base<UPF_Thread_Descriptor>;
  friend class UPF_Double_Linked_List_Iterator<UPF_Thread_Descriptor>;


public:
  UPF_Thread_Descriptor_Base (void);
  ~UPF_Thread_Descriptor_Base (void);

  // = We need the following operators to make Borland happy.

  /// Equality operator.
  bool operator== (const UPF_Thread_Descriptor_Base &rhs) const;

  /// Inequality operator.
  bool operator!= (const UPF_Thread_Descriptor_Base &rhs) const;

  /// Group ID.
  int grp_id (void) const;

  /// Current state of the thread.
  UPF_UINT32 state (void) const;

  /// Return the pointer to an UPF_Task_Base or NULL if there's no
  /// UPF_Task_Base associated with this thread.;
  UPF_Task_Base *task (void) const;

protected:
  /// Reset this base thread descriptor.
  void reset (void);

  /// Unique thread ID.
  UPF_thread_t thr_id_;

  /// Unique handle to thread (used by Win32 and AIX).
  UPF_hthread_t thr_handle_;

  /// Group ID.
  int grp_id_;

  /// Current state of the thread.
  UPF_UINT32 thr_state_;

  /// Pointer to an UPF_Task_Base or NULL if there's no
  /// UPF_Task_Base.
  UPF_Task_Base *task_;

  /// We need these pointers to maintain the double-linked list in a
  /// thread managers.
  UPF_Thread_Descriptor_Base *next_;
  UPF_Thread_Descriptor_Base *prev_;
};
class /*UPF_Export*/ UPF_Thread_Descriptor :public UPF_Thread_Descriptor_Base
{
	
  friend class UPF_At_Thread_Exit;
  friend class UPF_Thread_Manager;
  friend class UPF_Double_Linked_List<UPF_Thread_Descriptor>;
  friend class UPF_Double_Linked_List_Iterator<UPF_Thread_Descriptor>;
public:
  // = Initialization method.
  UPF_Thread_Descriptor (void);

  // = Accessor methods.
  /// Unique thread id.
  UPF_thread_t self (void) const;

  /// Unique handle to thread (used by Win32 and AIX).
  void self (UPF_hthread_t &);

  /// Dump the state of an object.
  void dump (void) const;


#if 0 // wangy
  /**
   * This cleanup function must be called only for UPF_TSS_cleanup.
   * The UPF_TSS_cleanup delegate Log_Msg instance destruction when
   * Log_Msg cleanup is called before terminate.
   */
  void log_msg_cleanup(UPF_Log_Msg* log_msg);

#endif 

  /**
   * Register an At_Thread_Exit hook and the ownership is acquire by
   * Thread_Descriptor, this is the usual case when the AT is dynamically
   * allocated.
   */
  int at_exit (UPF_At_Thread_Exit* cleanup);

  /// Register an At_Thread_Exit hook and the ownership is retained for the
  /// caller. Normally used when the at_exit hook is created in stack.
  int at_exit (UPF_At_Thread_Exit& cleanup);

  /**
   * Register an object (or array) for cleanup at thread termination.
   * "cleanup_hook" points to a (global, or static member) function
   * that is called for the object or array when it to be destroyed.
   * It may perform any necessary cleanup specific for that object or
   * its class.  "param" is passed as the second parameter to the
   * "cleanup_hook" function; the first parameter is the object (or
   * array) to be destroyed.  Returns 0 on success, non-zero on
   * failure: -1 if virtual memory is exhausted or 1 if the object (or
   * arrayt) had already been registered.
   */
  int at_exit (void *object,
               UPF_CLEANUP_FUNC cleanup_hook,
               void *param);

  /// Do nothing destructor to keep some compilers happy
  ~UPF_Thread_Descriptor (void);

  /**
   * Do nothing but to acquire the thread descriptor's lock and
   * release.  This will first check if the thread is registered or
   * not.  If it is already registered, there's no need to reacquire
   * the lock again.  This is used mainly to get newly spawned thread
   * in synch with thread manager and prevent it from accessing its
   * thread descriptor before it gets fully built.  This function is
   * only called from UPF_Log_Msg::thr_desc.
   */
  void acquire_release (void);
  void acquire (void);
  void release (void);

  /**
   * Set/get the @c next_ pointer.  These are required by the
   * UPF_Free_List.
   */
  void set_next (UPF_Thread_Descriptor *td);
  UPF_Thread_Descriptor *get_next (void) const;

private:
  /// Reset this thread descriptor.
  void reset (UPF_Thread_Manager *tm);

  /// Pop an At_Thread_Exit from at thread termination list, apply the at
  /// if apply is true.
  void at_pop (int apply = 1);

  /// Push an At_Thread_Exit to at thread termination list and set the
  /// ownership of at.
  void at_push (UPF_At_Thread_Exit* cleanup,
                int is_owner = 0);

  /// Run the AT_Thread_Exit hooks.
  void do_at_exit (void);

  /// Terminate realize the cleanup process to thread termination
  void terminate (void);

#if 0  //wangy
  /// Thread_Descriptor is the ownership of UPF_Log_Msg if log_msg_!=0
  /// This can occur because UPF_TSS_cleanup was executed before terminate.
  UPF_Log_Msg *log_msg_;
#endif

  /// The AT_Thread_Exit list
  UPF_At_Thread_Exit *at_exit_list_;

  /**
   * Stores the cleanup info for a thread.
   * @note This should be generalized to be a stack of UPF_Cleanup_Info's.
   */
  UPF_Cleanup_Info cleanup_info_;

  /// Pointer to an UPF_Thread_Manager or NULL if there's no
  /// UPF_Thread_Manager>
  UPF_Thread_Manager* tm_;

  /// Registration lock to prevent premature removal of thread descriptor.
  UPF_DEFAULT_THREAD_MANAGER_LOCK *sync_;

  /// Keep track of termination status.
  bool terminated_;
};



class  /*UPF_Export*/ UPF_Thread_Manager
{
public:
enum
  {
    /// Uninitialized.
    UPF_THR_IDLE = 0x00000000,

    /// Created but not yet running.
    UPF_THR_SPAWNED = 0x00000001,

    /// Thread is active (naturally, we don't know if it's actually
    /// *running* because we aren't the scheduler...).
    UPF_THR_RUNNING = 0x00000002,

    /// Thread is suspended.
    UPF_THR_SUSPENDED = 0x00000004,

    /// Thread has been cancelled (which is an indiction that it needs to
    /// terminate...).
    UPF_THR_CANCELLED = 0x00000008,

    /// Thread has shutdown, but the slot in the thread manager hasn't
    /// been reclaimed yet.
    UPF_THR_TERMINATED = 0x00000010,

    /// Join operation has been invoked on the thread by thread manager.
    UPF_THR_JOINING = 0x10000000
  };

 UPF_Thread_Manager (size_t preaolloc = UPF_DEFAULT_THREAD_MANAGER_PREALLOC,
                      size_t lwm = UPF_DEFAULT_THREAD_MANAGER_LWM,
                      size_t inc = UPF_DEFAULT_THREAD_MANAGER_INC,
                      size_t hwm = UPF_DEFAULT_THREAD_MANAGER_HWM);
  ~UPF_Thread_Manager (void);


  static UPF_Thread_Manager *instance (void);
  static UPF_Thread_Manager *instance (UPF_Thread_Manager *);
  static void close_singleton (void);
 
  int close (void);
  int spawn (UPF_THR_FUNC func,
             void *arg = 0,
             long flags = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
             UPF_thread_t * = 0,
             UPF_hthread_t *t_handle = 0,
             long priority = UPF_DEFAULT_THREAD_PRIORITY,
             int grp_id = -1,
             void *stack = 0,
             size_t stack_size = UPF_DEFAULT_THREAD_STACKSIZE);

  int spawn_n (size_t n,
               UPF_THR_FUNC func,
               void *arg = 0,
               long flags = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
               long priority = UPF_DEFAULT_THREAD_PRIORITY,
               int grp_id = -1,
               UPF_Task_Base *task = 0,
               UPF_hthread_t thread_handles[] = 0,
               void *stack[] = 0,
               size_t stack_size[] = 0);

  int spawn_n (UPF_thread_t thread_ids[],
               size_t n,
               UPF_THR_FUNC func,
               void *arg,
               long flags,
               long priority = UPF_DEFAULT_THREAD_PRIORITY,
               int grp_id = -1,
               void *stack[] = 0,
               size_t stack_size[] = 0,
               UPF_hthread_t thread_handles[] = 0,
               UPF_Task_Base *task = 0);
#if 0	// wangy
  int wait (const UPF_Time_Value *timeout = 0,
            bool abandon_detached_threads = false,
            bool use_absolute_time = true);
#endif

protected:
	 /// Create a new thread (must be called with locks held).
	int spawn_i (UPF_THR_FUNC func,
               void *arg,
               long flags,
               UPF_thread_t * = 0,
               UPF_hthread_t *t_handle = 0,
               long priority = UPF_DEFAULT_THREAD_PRIORITY,
               int grp_id = -1,
               void *stack = 0,
               size_t stack_size = 0,
               UPF_Task_Base *task = 0);


	UPF_Thread_Descriptor *thread_descriptor (UPF_thread_t);
	UPF_Thread_Descriptor *hthread_descriptor (UPF_hthread_t);

	int append_thr (UPF_thread_t t_id,
                    UPF_hthread_t t_handle,
                    UPF_UINT32 thr_state,
                    int grp_id,
                    UPF_Task_Base *task = 0,
                    long flags = 0,
                    UPF_Thread_Descriptor *td = 0);


	UPF_Thread_Descriptor *find_thread (UPF_thread_t t_id);
	UPF_Thread_Descriptor *find_hthread (UPF_hthread_t h_id);

	void remove_thr (UPF_Thread_Descriptor *td,int close_handler);
	void remove_thr_all (void);



	int register_as_terminated (UPF_Thread_Descriptor *td);


	UPF_Double_Linked_List<UPF_Thread_Descriptor>		thr_list_;
	UPF_Double_Linked_List<UPF_Thread_Descriptor_Base>  terminated_thr_list_;

	int grp_id_;
	int automatic_wait_;
	UPF_Thread_Mutex lock_;
	UPF_Condition_Thread_Mutex zero_cond_;

	UPF_Locked_Free_List<UPF_Thread_Descriptor, UPF_SYNCH_MUTEX> thread_desc_freelist_;
private:

	  static UPF_Thread_Manager *	thr_mgr_;
	  static int					delete_thr_mgr_;
};	


#endif