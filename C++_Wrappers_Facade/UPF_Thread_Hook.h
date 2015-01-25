#ifndef UPF_THREAD_HOOK_H_
#define UPF_THREAD_HOOK_H_

#include "..\OS_Adapter\UPF_OS.h"

class /*UPF_Export*/ UPF_Thread_Hook
{

public:

  /// Destructor.
  virtual ~UPF_Thread_Hook (void);

  /**
   * This method can be overridden in a subclass to customize this
   * pre-function call "hook" invocation that can perform
   * initialization processing before the thread entry point <func>
   * method is called back.  The @a func and @a arg passed into the
   * start hook are the same as those passed by the application that
   * spawned the thread.
   */
  virtual UPF_THR_FUNC_RETURN start (UPF_THR_FUNC func,
                                     void *arg);

  /// sets the system wide thread hook, returns the previous thread
  /// hook or 0 if none is set.
  static UPF_Thread_Hook *thread_hook (UPF_Thread_Hook *hook);

  /// Returns the current system thread hook.
  static UPF_Thread_Hook *thread_hook (void);
};

#endif

