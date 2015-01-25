#include "UPF_Thread_Manager.h"
#include "UPF_Thread.h"
#include "..\OS_Adapter\OS_Adapter_Common\UPF_Guard_T.h"

#include "UPF_Static_Object_Lock.h"
#include "UPF_At_Thread_Exit.h"

#include <memory>

UPF_Thread_Descriptor_Base::UPF_Thread_Descriptor_Base (void): 
									UPF_OS_Thread_Descriptor (),
									thr_id_ (UPF_OS::NULL_thread),
									thr_handle_ (UPF_OS::NULL_hthread),
									grp_id_ (0),
									thr_state_ (UPF_Thread_Manager::UPF_THR_IDLE),
									task_ (0),
									next_ (0),
									prev_ (0)
{
}


UPF_Thread_Descriptor_Base::~UPF_Thread_Descriptor_Base (void)
{
}

bool UPF_Thread_Descriptor_Base::operator== ( const UPF_Thread_Descriptor_Base &rhs) const
{
	return
		UPF_OS::thr_cmp (this->thr_handle_, rhs.thr_handle_)
		&& UPF_OS::thr_equal (this->thr_id_, rhs.thr_id_);
}

bool UPF_Thread_Descriptor_Base::operator!=(const UPF_Thread_Descriptor_Base &rhs) const
{
	return !(*this == rhs);
}

UPF_Task_Base * UPF_Thread_Descriptor_Base::task (void) const
{
	UPF_TRACE ("UPF_Thread_Descriptor_Base::task");
	return this->task_;
}

// Group ID.

int
UPF_Thread_Descriptor_Base::grp_id (void) const
{
	UPF_TRACE ("UPF_Thread_Descriptor_Base::grp_id");
	return grp_id_;
}

// Current state of the thread.
UPF_UINT32 UPF_Thread_Descriptor_Base::state (void) const
{
	UPF_TRACE ("UPF_Thread_Descriptor_Base::state");
	return thr_state_;
}

// Reset this base descriptor.
void
UPF_Thread_Descriptor_Base::reset (void)
{
	UPF_TRACE ("UPF_Thread_Descriptor_Base::reset");
	this->thr_id_		= UPF_OS::NULL_thread;
	this->thr_handle_ = UPF_OS::NULL_hthread;
	this->grp_id_		= 0;
	this->thr_state_	= UPF_Thread_Manager::UPF_THR_IDLE;
	this->task_		= 0;
	this->flags_		= 0;
}
/******************************************************************/


UPF_Thread_Descriptor::UPF_Thread_Descriptor (void)
  :
#if 0  // wangy
   log_msg_ (0),
#endif
    at_exit_list_ (0),
    terminated_ (false)
{
  UPF_TRACE ("UPF_Thread_Descriptor::UPF_Thread_Descriptor");
  UPF_NEW (this->sync_,UPF_DEFAULT_THREAD_MANAGER_LOCK);
}
UPF_Thread_Descriptor::~UPF_Thread_Descriptor (void)
{
  delete this->sync_;
}



// Unique thread id.
 UPF_thread_t
UPF_Thread_Descriptor::self (void) const
{
  UPF_TRACE ("UPF_Thread_Descriptor::self");
  return this->thr_id_;
}

// Unique kernel-level thread handle.

 void
UPF_Thread_Descriptor::self (UPF_hthread_t &handle)
{
  UPF_TRACE ("UPF_Thread_Descriptor::self");
  handle = this->thr_handle_;
}

#if 0  // wangy
void
UPF_Thread_Descriptor::log_msg_cleanup (UPF_Log_Msg* log_msg)

{
  log_msg_ = log_msg;
}
#endif 
// Set the <next_> pointer
 void
UPF_Thread_Descriptor::set_next (UPF_Thread_Descriptor *td)
{
  UPF_TRACE ("UPF_Thread_Descriptor::set_next");
  this->next_ = td;
}

// Get the <next_> pointer
 UPF_Thread_Descriptor *
UPF_Thread_Descriptor::get_next (void) const
{
  UPF_TRACE ("UPF_Thread_Descriptor::get_next");
  return static_cast<UPF_Thread_Descriptor * UPF_CAST_CONST> (this->next_);
}

// Reset this thread descriptor
 void
UPF_Thread_Descriptor::reset (UPF_Thread_Manager *tm)
{
  UPF_TRACE ("UPF_Thread_Descriptor::reset");
  this->UPF_Thread_Descriptor_Base::reset ();
  this->at_exit_list_ = 0;
    // Start the at_exit hook list.
  this->tm_ = tm;

#if 0  // wangy
    // Setup the Thread_Manager.
  this->log_msg_ = 0;
#endif

  this->terminated_ = false;
}
void UPF_Thread_Descriptor::at_pop (int apply )
{
  
	UPF_TRACE ("UPF_Thread_Descriptor::at_pop");
  // Get first at from at_exit_list
  UPF_At_Thread_Exit* at = this->at_exit_list_;
  // Remove at from at_exit list
  this->at_exit_list_ = at->next_;
  // Apply if required
  if (apply)
   {
     at->apply ();
     // Do the apply method
     at->was_applied (1);
     // Mark at has been applied to avoid double apply from
     // at destructor
   }
  // If at is not owner delete at.
  if (!at->is_owner ())
   delete at;
}

void
UPF_Thread_Descriptor::at_push (UPF_At_Thread_Exit* cleanup, int is_owner)
{
  UPF_TRACE ("UPF_Thread_Descriptor::at_push");
  cleanup->is_owner (is_owner);
  cleanup->td_ = this;
  cleanup->next_ = at_exit_list_;
  at_exit_list_ = cleanup;
}

int
UPF_Thread_Descriptor::at_exit (UPF_At_Thread_Exit& cleanup)
{
  UPF_TRACE ("UPF_Thread_Descriptor::at_exit");
  at_push (&cleanup, 1);
  return 0;
}

int
UPF_Thread_Descriptor::at_exit (UPF_At_Thread_Exit* cleanup)
{
  UPF_TRACE ("UPF_Thread_Descriptor::at_exit");
  if (cleanup==0)
   return -1;
  else
   {
     this->at_push (cleanup);
     return 0;
   }
}

void
UPF_Thread_Descriptor::do_at_exit ()
{
  UPF_TRACE ("UPF_Thread_Descriptor::do_at_exit");
  while (at_exit_list_!=0)
    this->at_pop ();
}

/******************************************************************/

 // Process-wide Thread Manager.
UPF_Thread_Manager *UPF_Thread_Manager::thr_mgr_ = 0;

// Controls whether the Thread_Manager is deleted when we shut down
// (we can only delete it safely if we created it!)
int UPF_Thread_Manager::delete_thr_mgr_ = 0;


UPF_Thread_Manager::UPF_Thread_Manager (size_t prealloc,
                                        size_t lwm,
                                        size_t inc,
                                        size_t hwm)
  : grp_id_ (1),
    automatic_wait_ (1)
#if defined (UPF_HAS_THREADS)
    , zero_cond_ (lock_)
#endif /* UPF_HAS_THREADS */
    , thread_desc_freelist_ (UPF_FREE_LIST_WITH_POOL,
                             prealloc, lwm, hwm, inc)
{
  UPF_TRACE ("UPF_Thread_Manager::UPF_Thread_Manager");
}

UPF_Thread_Manager::~UPF_Thread_Manager (void)
{
  UPF_TRACE ("UPF_Thread_Manager::~UPF_Thread_Manager");
  this->close ();
}
int
UPF_Thread_Manager::close ()
{
  UPF_TRACE ("UPF_Thread_Manager::close");

  // Clean up the thread descriptor list.
  if (this->automatic_wait_)
  {
	  // this->wait (0, 1);  wangy because time_value  
  }
  else
    {
      UPF_MT (UPF_GUARD_RETURN (UPF_Thread_Mutex, UPF_mon, this->lock_, -1));

      this->remove_thr_all ();
    }

  return 0;
}



UPF_Thread_Manager * UPF_Thread_Manager::instance (void)
{
	UPF_TRACE ("UPF_Thread_Manager::instance");

  if (UPF_Thread_Manager::thr_mgr_ == 0)
    {
      // Perform Double-Checked Locking Optimization.
      UPF_MT (UPF_GUARD_RETURN (UPF_Recursive_Thread_Mutex, UPF_mon,
                                *UPF_Static_Object_Lock::instance (), 0));

      if (UPF_Thread_Manager::thr_mgr_ == 0)
        {
          UPF_NEW_RETURN (UPF_Thread_Manager::thr_mgr_,
                          UPF_Thread_Manager,
                          0);
          UPF_Thread_Manager::delete_thr_mgr_ = 1;
        }
    }

  return UPF_Thread_Manager::thr_mgr_;
}
UPF_Thread_Manager * UPF_Thread_Manager::instance (UPF_Thread_Manager *)
{
	return UPF_Thread_Manager::instance() ;// wangy
}

int UPF_Thread_Manager::spawn (UPF_THR_FUNC func,
							   void *args,
							   long flags,
							   UPF_thread_t *t_id,
							   UPF_hthread_t *t_handle,
							   long priority,
							   int grp_id,
							   void *stack,
							   size_t stack_size)
{
	//  UPF_TRACE ("UPF_Thread_Manager::spawn");
	
	UPF_MT (UPF_GUARD_RETURN (UPF_Thread_Mutex, UPF_mon, this->lock_, -1));
	
	if (grp_id == -1)
		grp_id = this->grp_id_++; // Increment the group id.
	
	if (priority != UPF_DEFAULT_THREAD_PRIORITY)
		UPF_CLR_BITS (flags, THR_INHERIT_SCHED);
	
	if (this->spawn_i (func,
		args,
		flags,
		t_id,
		t_handle,
		priority,
		grp_id,
		stack,
		stack_size,
		0) == -1)
		return -1;
	
	return grp_id;
}

int 
UPF_Thread_Manager::spawn_n (size_t n,
                             UPF_THR_FUNC func,
                             void *args,
                             long flags,
                             long priority,
                             int grp_id,
                             UPF_Task_Base *task,
                             UPF_hthread_t thread_handles[],
                             void *stack[],
                             size_t stack_size[])
{
	UPF_TRACE ("UPF_Thread_Manager::spawn_n");
	UPF_MT (UPF_GUARD_RETURN (UPF_Thread_Mutex, UPF_mon, this->lock_, -1));
	
	if (grp_id == -1)
		grp_id = this->grp_id_++; // Increment the group id.
	
	for (size_t i = 0; i < n; i++)
    {
		// @@ What should happen if this fails?! e.g., should we try to
		// cancel the other threads that we've already spawned or what?
		if (this->spawn_i (func,
			args,
			flags,
			0,
			thread_handles == 0 ? 0 : &thread_handles[i],
			priority,
			grp_id,
			stack == 0 ? 0 : stack[i],
			stack_size == 0 ? UPF_DEFAULT_THREAD_STACKSIZE : stack_size[i],
			task) == -1)
			return -1;
    }
	
	return grp_id;
}

int
UPF_Thread_Manager::spawn_n (UPF_thread_t thread_ids[],
                             size_t n,
                             UPF_THR_FUNC func,
                             void *args,
                             long flags,
                             long priority,
                             int grp_id,
                             void *stack[],
                             size_t stack_size[],
                             UPF_hthread_t thread_handles[],
                             UPF_Task_Base *task)
{
	UPF_TRACE ("UPF_Thread_Manager::spawn_n");
	UPF_MT (UPF_GUARD_RETURN (UPF_Thread_Mutex, UPF_mon, this->lock_, -1));
	
	if (grp_id == -1)
		grp_id = this->grp_id_++; // Increment the group id.
	
	for (size_t i = 0; i < n; i++)
    {
		// @@ What should happen if this fails?! e.g., should we try to
		// cancel the other threads that we've already spawned or what?
		if (this->spawn_i (func,
			args,
			flags,
			thread_ids == 0 ? 0 : &thread_ids[i],
			thread_handles == 0 ? 0 : &thread_handles[i],
			priority,
			grp_id,
			stack == 0 ? 0 : stack[i],
			stack_size == 0 ? UPF_DEFAULT_THREAD_STACKSIZE : stack_size[i],
			task) == -1)
			return -1;
    }
	
	return grp_id;
}


int UPF_Thread_Manager::spawn_i (UPF_THR_FUNC func,
								 void *args,
								 long flags,
								 UPF_thread_t *t_id,
								 UPF_hthread_t *t_handle,
								 long priority,
								 int grp_id,
								 void *stack,
								 size_t stack_size,
								 UPF_Task_Base *task)
{
	UPF_ASSERT (UPF_BIT_DISABLED (flags, THR_DAEMON));
	
	// Create a new thread running <func>.  *Must* be called with the
	// <lock_> held...
	// Get a "new" Thread Descriptor from the freelist.
	std::auto_ptr<UPF_Thread_Descriptor> new_thr_desc (this->thread_desc_freelist_.remove ());
	
	// Reset thread descriptor status
	new_thr_desc->reset (this);
	
	UPF_Thread_Adapter *thread_args = 0;
	
	UPF_NEW_RETURN (thread_args,
		UPF_Thread_Adapter (func,
		args,
		(UPF_THR_C_FUNC) UPF_THREAD_ADAPTER_NAME,
		this,
		new_thr_desc.get ()),
		-1);
	
	std::auto_ptr <UPF_Base_Thread_Adapter> auto_thread_args (static_cast<UPF_Base_Thread_Adapter *> (thread_args));
	UPF_TRACE ("UPF_Thread_Manager::spawn_i");
	UPF_hthread_t thr_handle;
	
	UPF_thread_t thr_id;
	if (t_id == 0)
		t_id = &thr_id;
	
	
	int const result = UPF_Thread::spawn (func,
									args,
									flags,
									t_id,
									&thr_handle,
									priority,
									stack,
									stack_size,
									thread_args);

  if (result != 0)
    {
      // _Don't_ clobber errno here!  result is either 0 or -1, and
      // UPF_OS::thr_create () already set errno!  D. Levine 28 Mar 1997
      // errno = result;
#if 0 // wangy
      UPF_Errno_Guard guard (errno);     // Lock release may smash errno
#endif 
	  new_thr_desc->sync_->release ();
      return -1;
  }
  auto_thread_args.release ();
  
  if (t_handle != 0)
	  UPF_OS::duplicate_handle(thr_handle,t_handle);


  return this->append_thr (*t_id,
                           thr_handle,
                           UPF_THR_SPAWNED,
                           grp_id,
                           task,
                           flags,
                           new_thr_desc.release ());
}

#define UPF_FIND(OP,INDEX) UPF_Thread_Descriptor *INDEX = OP; 

UPF_Thread_Descriptor *
UPF_Thread_Manager::thread_descriptor (UPF_thread_t thr_id)
{
  UPF_TRACE ("UPF_Thread_Manager::thread_descriptor");
  UPF_MT (UPF_GUARD_RETURN (UPF_Thread_Mutex, UPF_mon, this->lock_, 0));

  UPF_FIND (this->find_thread (thr_id), ptr);
  return ptr;
}

UPF_Thread_Descriptor *
UPF_Thread_Manager::hthread_descriptor (UPF_hthread_t thr_handle)
{
  UPF_TRACE ("UPF_Thread_Manager::hthread_descriptor");
  UPF_MT (UPF_GUARD_RETURN (UPF_Thread_Mutex, UPF_mon, this->lock_, 0));

  UPF_FIND (this->find_hthread (thr_handle), ptr);
  return ptr;
}

int UPF_Thread_Manager::append_thr (UPF_thread_t t_id,
									UPF_hthread_t t_handle,
									UPF_UINT32 thr_state,
									int grp_id,
									UPF_Task_Base *task,
									long flags,
									UPF_Thread_Descriptor *td)
{
	UPF_TRACE ("UPF_Thread_Manager::append_thr");
	UPF_Thread_Descriptor *thr_desc = 0;
	
	if (td == 0)
    {
		UPF_NEW_RETURN (thr_desc,
			UPF_Thread_Descriptor,
			-1);
		thr_desc->tm_ = this;
		// Setup the Thread_Manager.
    }
	else
		thr_desc = td;
	
	thr_desc->thr_id_ = t_id;
	thr_desc->thr_handle_ = t_handle;
	thr_desc->grp_id_ = grp_id;
	thr_desc->task_ = task;
	thr_desc->flags_ = flags;
	
	this->thr_list_.insert_head (thr_desc);
	UPF_SET_BITS (thr_desc->thr_state_, thr_state);
	thr_desc->sync_->release ();
	return 0;
}

UPF_Thread_Descriptor *UPF_Thread_Manager::find_thread (UPF_thread_t t_id)
{
	UPF_TRACE ("UPF_Thread_Manager::find_thread");
	
	for (UPF_Double_Linked_List_Iterator<UPF_Thread_Descriptor> iter (this->thr_list_);!iter.done ();iter.advance ())
		if (UPF_OS::thr_equal (iter.next ()->thr_id_, t_id))
			return iter.next ();

		return 0;
}
UPF_Thread_Descriptor *UPF_Thread_Manager::find_hthread (UPF_hthread_t h_id)
{
	for (UPF_Double_Linked_List_Iterator<UPF_Thread_Descriptor> iter (this->thr_list_);!iter.done ();iter.advance ())
		if (UPF_OS::thr_cmp (iter.next ()->thr_handle_, h_id))
			return iter.next ();
		
		return 0;
}
void UPF_Thread_Manager::remove_thr (UPF_Thread_Descriptor *td,int close_handler)
{
	td->tm_ = 0;
	this->thr_list_.remove (td);
	 
	if (close_handler != 0)
    ::CloseHandle (td->thr_handle_);

}

void UPF_Thread_Manager::remove_thr_all (void)
{
	UPF_Thread_Descriptor *td = 0;
	
	while ((td = this->thr_list_.delete_head ()) != 0)
    {
		this->remove_thr (td, 1);
    }
}

int UPF_Thread_Manager::register_as_terminated (UPF_Thread_Descriptor *td)
{
	UPF_Thread_Descriptor_Base *tdb = 0;
	UPF_NEW_RETURN (tdb, UPF_Thread_Descriptor_Base (*td), -1);

	this->terminated_thr_list_.insert_tail (tdb);
	
	return 0;
}	