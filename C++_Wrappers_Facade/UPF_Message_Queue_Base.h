#ifndef UPF_Message_Queue_Base_h_
#define UPF_Message_Queue_Base_h_

#include "..\OS_Adapter\UPF_OS.h"
#include "UPF_Message_Block.h"

class /*UPF_Export*/ UPF_Message_Queue_Base
{
public:
  enum
  {
    // Default high and low watermarks.

    /// Default high watermark (16 K).
    DEFAULT_HWM = 16 * 1024,
    /// Default low watermark (same as high water mark).
    DEFAULT_LWM = 16 * 1024,

    // Queue states.  Before PULSED state was added, the activate()
    // and deactivate() methods returned WAS_INACTIVE or WAS_ACTIVE
    // to indicate the previous condition.  Now those methods
    // return the state the queue was previously in.  WAS_ACTIVE
    // and WAS_INACTIVE are defined to match previous semantics for
    // applications that don't use the PULSED state.

    /// @deprecated Use ACTIVATED instead.
    WAS_ACTIVE = 1,
    /// Message queue is active and processing normally
    ACTIVATED = 1,

    /// @deprecated Use DEACTIVATED instead.
    WAS_INACTIVE = 2,
    /// Queue is deactivated; no enqueue or dequeue operations allowed.
    DEACTIVATED = 2,

    /// Message queue was pulsed; enqueue and dequeue may proceed normally.
    PULSED = 3

  };

  UPF_Message_Queue_Base (void);

  /// Close down the message queue and release all resources.
  virtual int close (void) = 0;

  /// Close down the message queue and release all resources.
  virtual ~UPF_Message_Queue_Base (void);

  // = Enqueue and dequeue methods.

  /**
   * Retrieve the first UPF_Message_Block without removing it.  Note
   * that @a timeout uses <{absolute}> time rather than <{relative}>
   * time.  If the @a timeout elapses without receiving a message -1 is
   * returned and @c errno is set to @c EWOULDBLOCK.  If the queue is
   * deactivated -1 is returned and @c errno is set to <ESHUTDOWN>.
   * Otherwise, returns -1 on failure, else the number of items still
   * on the queue.
   */
  virtual int peek_dequeue_head (UPF_Message_Block *&first_item,
                                 UPF_Time_Value *timeout = 0) = 0;

  /**
   * Enqueue a <UPF_Message_Block *> into the tail of the queue.
   * Returns number of items in queue if the call succeeds or -1
   * otherwise.  These calls return -1 when queue is closed,
   * deactivated (in which case @c errno == <ESHUTDOWN>), when a signal
   * occurs (in which case @c errno == <EINTR>, or if the time
   * specified in timeout elapses (in which case @c errno ==
   * @c EWOULDBLOCK).
   */
  virtual int enqueue_tail (UPF_Message_Block *new_item,
                            UPF_Time_Value *timeout = 0) = 0;
  virtual int enqueue (UPF_Message_Block *new_item,
                       UPF_Time_Value *timeout = 0) = 0;

  /**
   * Dequeue and return the <UPF_Message_Block *> at the head of the
   * queue.  Returns number of items in queue if the call succeeds or
   * -1 otherwise.  These calls return -1 when queue is closed,
   * deactivated (in which case @c errno == <ESHUTDOWN>), when a signal
   * occurs (in which case @c errno == <EINTR>, or if the time
   * specified in timeout elapses (in which case @c errno ==
   * @c EWOULDBLOCK).
   */
#if 0  // wangy
  virtual int dequeue_head (UPF_Message_Block *&first_item,
                            UPF_Time_Value *timeout = 0) = 0;
  virtual int dequeue (UPF_Message_Block *&first_item,
                       UPF_Time_Value *timeout = 0) = 0;

#endif 

  virtual int dequeue_head (UPF_Message_Block *&first_item) = 0;
  virtual int dequeue (UPF_Message_Block *&first_item) = 0;


  // = Check if queue is full/empty.
  /// True if queue is full, else false.
  virtual int is_full (void) = 0;

  /// True if queue is empty, else false.
  virtual int is_empty (void) = 0;

  // = Queue statistic methods.

  /// Number of total bytes on the queue, i.e., sum of the message
  /// block sizes.
  virtual size_t message_bytes (void) = 0;

  /// Number of total length on the queue, i.e., sum of the message
  /// block lengths.
  virtual size_t message_length (void) = 0;

  /// Number of total messages on the queue.
  virtual size_t message_count (void) = 0;

  /// New value of the number of total bytes on the queue, i.e.,
  /// sum of the message block sizes.
  virtual void message_bytes (size_t new_size) = 0;

  /// New value of the number of total length on the queue, i.e.,
  /// sum of the message block lengths.
  virtual void message_length (size_t new_length) = 0;

  // = Activation control methods.

  /**
   * Deactivate the queue and wake up all threads waiting on the queue
   * so they can continue.  No messages are removed from the queue,
   * however.  Any other operations called until the queue is
   * activated again will immediately return -1 with @c errno
   * ESHUTDOWN.
   *
   * @retval  The queue's state before this call.
   */
  virtual int deactivate (void) = 0;

  /**
   * Reactivate the queue so that threads can enqueue and dequeue
   * messages again.
   *
   * @retval  The queue's state before this call.
   */
  virtual int activate (void) = 0;

  /**
   * Pulse the queue to wake up any waiting threads.  Changes the
   * queue state to PULSED; future enqueue/dequeue operations proceed
   * as in ACTIVATED state.
   *
   * @retval  The queue's state before this call.
   */
  virtual int pulse (void) = 0;

  /// Returns the current state of the queue.
  virtual int state (void);

  /// Returns 1 if the state of the queue is DEACTIVATED,
  /// and 0 if the queue's state is ACTIVATED or PULSED.
  virtual int deactivated (void) = 0;


#if 0  // wangy
  /// Get the notification strategy for the <Message_Queue>
  virtual UPF_Notification_Strategy * notification_strategy (void) = 0;

  /// Set the notification strategy for the <Message_Queue>
  virtual void notification_strategy (UPF_Notification_Strategy *s) = 0;

#endif 
  // = Notification hook.

  /// Dump the state of an object.
  virtual void dump (void) const = 0;

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;

private:
  // = Disallow copying and assignment.
  UPF_Message_Queue_Base (const UPF_Message_Queue_Base &);
  void operator= (const UPF_Message_Queue_Base &);

protected:
  /// Indicates the state of the queue, which can be
  /// <ACTIVATED>, <DEACTIVATED>, or <PULSED>.
  int state_;

};






#endif