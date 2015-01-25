#include "UPF_At_Thread_Exit.h"
#include "UPF_Thread_Manager.h"

UPF_At_Thread_Exit::UPF_At_Thread_Exit (void)
  : next_ (0),
    td_ (0),
    was_applied_ (0),
    is_owner_ (1)
{
}

UPF_At_Thread_Exit::~UPF_At_Thread_Exit (void)
{
  this->do_apply ();
}

int
UPF_At_Thread_Exit::was_applied() const

{
   return was_applied_;
}

int
UPF_At_Thread_Exit::was_applied (int applied)
{
  was_applied_ = applied;
  if (was_applied_)
    td_ = 0;
  return was_applied_;
}

int
UPF_At_Thread_Exit::is_owner() const
{
  return is_owner_;
}

int
UPF_At_Thread_Exit::is_owner (int owner)
{
  is_owner_ = owner;
  return is_owner_;
}

void
UPF_At_Thread_Exit::do_apply (void)
{
  if (!this->was_applied_ && this->is_owner_)
    td_->at_pop();
}

/*---------------------------------------------------------------------------------*/

UPF_At_Thread_Exit_Func::UPF_At_Thread_Exit_Func (void *object,
                                                  UPF_CLEANUP_FUNC func,
                                                  void *param)
  : object_(object),
    func_(func),
    param_(param)
{
}
UPF_At_Thread_Exit_Func::~UPF_At_Thread_Exit_Func (void)
{
  this->do_apply ();
}

void
UPF_At_Thread_Exit_Func::apply (void)
{
  this->func_ (this->object_, this->param_);
}