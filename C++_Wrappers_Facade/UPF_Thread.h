#ifndef UPF_THREAD_H_
#define UPF_THREAD_H_

#include "..\OS_Adapter\UPF_OS.h"

class UPF_Thread_Adapter;

/** 
 * @class UPF_Thread
 * 
 */
class UPF_Export UPF_Thread
{
public:
 /** 
  * 创建线程
  * 
  * @param func
  * @param arg
  * @param flags
  * @param t_id
  * @param t_handle
  * @param priority
  * @param stack
  * @param stack_size
  * @param thread_adapter
  * 
  * @return int
  */
 static int spawn ( UPF_THR_FUNC func,
                    void *arg = 0,
                    long flags = THR_NEW_LWP | THR_JOINABLE,
                    UPF_thread_t *t_id = 0,
                    UPF_hthread_t *t_handle = 0,
                    long priority = UPF_DEFAULT_THREAD_PRIORITY,
                    void *stack = 0,
                    size_t stack_size = UPF_DEFAULT_THREAD_STACKSIZE,
                    UPF_Thread_Adapter *thread_adapter = 0);

  /** 
   * 创建多个线程
   * 
   * @param n
   * @param func
   * @param arg
   * @param flags
   * @param priority
   * @param stack
   * @param stack_size
   * @param thread_adapter
   * 
   * @return size_t
   */
  static size_t spawn_n (size_t n,
                         UPF_THR_FUNC func,
                         void *arg = 0,
                         long flags = THR_NEW_LWP | THR_JOINABLE,
                         long priority = UPF_DEFAULT_THREAD_PRIORITY,
                         void *stack[] = 0,
                         size_t stack_size[] = 0,
                         UPF_Thread_Adapter *thread_adapter = 0);

  /** 
   * 创建多个线程
   * 
   * @param thread_ids
   * @param n
   * @param func
   * @param arg
   * @param flags
   * @param priority
   * @param stack
   * @param stack_size
   * @param thread_handles
   * @param thread_adapter
   * 
   * @return size_t
   */
  static size_t spawn_n (UPF_thread_t thread_ids[],
                         size_t n,
                         UPF_THR_FUNC func,
                         void *arg,
                         long flags,
                         long priority = UPF_DEFAULT_THREAD_PRIORITY,
                         void *stack[] = 0,
                         size_t stack_size[] = 0,
                         UPF_hthread_t thread_handles[] = 0,
                         UPF_Thread_Adapter *thread_adapter = 0);


  static int join (UPF_hthread_t,UPF_THR_FUNC_RETURN * = 0);
  static int resume (UPF_hthread_t);
  static int suspend (UPF_hthread_t);
  static int cancel (UPF_hthread_t t_id);
  static int kill (UPF_thread_t, int signum);
  static int getprio (UPF_hthread_t ht_id, int &priority);
  static int getprio (UPF_hthread_t ht_id, int &priority, int &policy);
  static int setprio (UPF_hthread_t ht_id, int priority, int policy = -1);
  static void yield (void);


  static int keycreate (UPF_thread_key_t *keyp,UPF_THR_DEST destructor,void * inst = 0);
  static int keyfree (UPF_thread_key_t key);
  static int setspecific (UPF_thread_key_t key,void *value);
  static int getspecific (UPF_thread_key_t key,void **valuep);

  static UPF_thread_t self (void);


};

#endif 