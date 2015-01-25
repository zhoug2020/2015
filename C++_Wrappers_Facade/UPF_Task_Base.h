#ifndef UPF_TASK_BASE_H_
#define UPF_TASK_BASE_H_


#include "..\OS_Adapter\UPF_OS.h"

namespace UPF_Task_Flags
{
  enum
  {
    /// Identifies a Task as being the "reader" in a Module.
    UPF_READER     = 01,
    /// Just flush data messages in the queue.
    UPF_FLUSHDATA  = 02,
    /// Flush all messages in the Queue.
    UPF_FLUSHALL   = 04,
    /// Flush read queue
    UPF_FLUSHR     = 010,
    /// Flush write queue
    UPF_FLUSHW     = 020,
    /// Flush both queues
    UPF_FLUSHRW    = 030
  };
}


class /*UPF_Export wangy */ UPF_Task_Base // : public UPF_Service_Object ->wangy
{
public:
 
  UPF_Task_Base (UPF_Thread_Manager * = 0);

  virtual ~UPF_Task_Base (void);

  
  virtual int open (void *args = 0);

 
  virtual int close (u_long flags = 0);

 
  virtual int module_closed (void);

#if 0  // wangy
  virtual int put (UPF_Message_Block *, UPF_Time_Value * = 0);
#endif 
 
  virtual int svc (void);

 
  virtual int activate (long flags = THR_NEW_LWP | THR_JOINABLE | THR_INHERIT_SCHED,
                        int n_threads = 1,
                        int force_active = 0,
                        long priority = UPF_DEFAULT_THREAD_PRIORITY,
                        int grp_id = -1,
                        UPF_Task_Base *task = 0,
                        UPF_hthread_t thread_handles[] = 0,
                        void *stack[] = 0,
                        size_t stack_size[] = 0,
                        UPF_thread_t thread_ids[] = 0);

  virtual int wait (void);

 
  virtual int suspend (void);
 
  virtual int resume (void);


  int grp_id (void) const;

  void grp_id (int);

 
  UPF_Thread_Manager *thr_mgr (void) const;


  void thr_mgr (UPF_Thread_Manager *);


  int is_reader (void) const;


  int is_writer (void) const;

  
  size_t thr_count (void) const;

 
  UPF_thread_t last_thread (void) const;

 
  static UPF_THR_FUNC_RETURN svc_run (void *);

  
  static void cleanup (void *object, void *params);

protected:
 
  size_t thr_count_;


  UPF_Thread_Manager *thr_mgr_;


  u_long flags_;


  int grp_id_;

#if defined (UPF_MT_SAFE) && (UPF_MT_SAFE != 0)
  /// Protect the state of a Task during concurrent operations, but
  /// only if we're configured as MT safe...
  UPF_Thread_Mutex lock_;
#endif 

 
  UPF_thread_t  last_thread_id_;

private:


  UPF_Task_Base &operator= (const UPF_Task_Base &);
  UPF_Task_Base (const UPF_Task_Base &);
};

#endif 