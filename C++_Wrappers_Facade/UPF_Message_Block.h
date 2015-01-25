#ifndef  UPF_MESSAGE_BLOCK_H_
#define  UPF_MESSAGE_BLOCK_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "Malloc_Base.h"
#include "UPF_Lock.h"

class /*UPF_Export wangy*/ UPF_Message_Block
{
public:
  friend class UPF_Data_Block;

  enum
  {
    // = Data and proto
    /// Undifferentiated data message
    MB_DATA     = 0x01,
    /// Undifferentiated protocol control
    MB_PROTO    = 0x02,

    // = Control messages
    /// Line break (regular and priority)
    MB_BREAK    = 0x03,
    /// Pass file pointer
    MB_PASSFP   = 0x04,
    /// Post an event to an event queue
    MB_EVENT    = 0x05,
    /// Generate process signal
    MB_SIG      = 0x06,
    /// ioctl; set/get params
    MB_IOCTL    = 0x07,
    /// Set various stream head options
    MB_SETOPTS  = 0x08,

    // = Control messages
    /// Acknowledge ioctl (high priority; go to head of queue)
    MB_IOCACK   = 0x81,
    /// Negative ioctl acknowledge
    MB_IOCNAK   = 0x82,
    /// Priority proto message
    MB_PCPROTO  = 0x83,
    /// Generate process signal
    MB_PCSIG    = 0x84,
    /// Generate read notification
    MB_READ     = 0x85,
    /// Flush your queues
    MB_FLUSH    = 0x86,
    /// Stop transmission immediately
    MB_STOP     = 0x87,
    /// Restart transmission after stop
    MB_START    = 0x88,
    /// Line disconnect
    MB_HANGUP   = 0x89,
    /// Fatal error used to set u.u_error
    MB_ERROR    = 0x8a,
    /// Post an event to an event queue
    MB_PCEVENT  = 0x8b,

    // = Message class masks
    /// Normal priority message mask
    MB_NORMAL   = 0x00,
    /// High priority control message mask
    MB_PRIORITY = 0x80,
    /// User-defined message mask
    MB_USER     = 0x200
  };

  typedef int UPF_Message_Type;
  typedef unsigned long Message_Flags;

  enum
  {
    /// Don't delete the data on exit since we don't own it.
    DONT_DELETE = 01,
    /// user defined flags start here
    USER_FLAGS = 0x1000
  };

  // = Initialization and termination.
  /// Create an empty message.
  UPF_Message_Block (UPF_Allocator *message_block_allocator = 0);

  /**
   * Create an UPF_Message_Block that owns the specified UPF_Data_Block
   * without copying it. If the @a flags is set to @c DONT_DELETE we
   * don't delete the UPF_Data_Block. It is left to the client's
   * responsibility to take care of the memory allocated for the
   * data_block
   */
  UPF_Message_Block (UPF_Data_Block *,
                     Message_Flags flags = 0,
                     UPF_Allocator *message_block_allocator = 0);

  /**
   * Create an UPF_Message_Block that refers to @a data without
   * copying it. The @a data memory will not be freed when this block is
   * destroyed; memory management of @a data is left to the caller.
   * Note that the @c size of the new UPF_Message_Block will be @a size, but
   * the @c length will be 0 until the write pointer is changed.
   */
  UPF_Message_Block (const char *data,
                     size_t size = 0,
                     unsigned long priority = UPF_DEFAULT_MESSAGE_BLOCK_PRIORITY);


#if 0 // wangy
  /**
   * Create an initialized message of type @a type containing @a size
   * bytes.  The @a cont argument initializes the continuation field in
   * the UPF_Message_Block.  If @a data == 0 then this block allocates and
   * owns the block's memory, using @a allocator to get the data if it's
   * non-0.  If @a data != 0 then this block refers to that memory until
   * this this block ceases to exist; this object will not free @a data on
   * destruction.  If @a locking_strategy is non-0 then this is used
   * to protect regions of code that access shared state (e.g.,
   * reference counting) from race conditions.  Note that the @c size
   * of the UPF_Message_Block will be @a size, but the @c length will be 0
   * until the write pointer is set. The @a data_block_allocator is used to
   * allocate the data blocks while the @a allocator_strategy is used
   * to allocate the buffers contained by those. The
   * @a message_block_allocator is used to allocate new UPF_Message_Block
   * objects when the duplicate() method is called. If a
   * @a message_block_allocator is given, this UPF_Message_Block and
   * future UPF_Message_Block objects created by duplicate() will be
   * freed using this allocator when they are released.
   * @note If you use this allocator, the UPF_Message_Block you created
   * should have been created using this allocator because it will be
   * released to the same allocator.
   */
  UPF_Message_Block (size_t size,
                     UPF_Message_Type type = MB_DATA,
                     UPF_Message_Block *cont = 0,
                     const char *data = 0,
                     UPF_Allocator *allocator_strategy = 0,
                     UPF_Lock *locking_strategy = 0,
                     unsigned long priority = UPF_DEFAULT_MESSAGE_BLOCK_PRIORITY,
                     const UPF_Time_Value &execution_time = UPF_Time_Value::zero,
                     const UPF_Time_Value &deadline_time = UPF_Time_Value::max_time,
                     UPF_Allocator *data_block_allocator = 0,
                     UPF_Allocator *message_block_allocator = 0);


#endif 

  /**
   * A copy constructor. This constructor is a bit different. If the
   * incoming Message Block has a data block from the stack this
   * constructor does a deep copy ie. allocates a new data block on
   * the heap and does a copy of the data from the incoming message
   * block. As a final note, the alignment information is used to
   * align the data block if it is created afresh. If the incoming
   * @a mb has a data block has a data block allocated from the heap,
   * then this constructor just duplicates (ie. a shallow copy) the
   * data block of the incoming @a mb.
   */
  UPF_Message_Block (const UPF_Message_Block &mb,
                     size_t align);

  /**
   * Create a Message Block that assumes it has ownership of @a data,
   * but in reality it doesnt (i.e., cannot delete it since it didn't
   * malloc it!).  Note that the @c size of the Message_Block will
   * be @a size, but the @a length  will be 0 until <wr_ptr> is set.
   */
  int init (const char *data,
            size_t size = 0);


#if 0 // wangy
  /**
   * Create an initialized message of type @a type containing @a size
   * bytes.  The @a cont argument initializes the continuation field in
   * the <Message_Block>.  If @a data == 0 then we create and own the
   * @a data, using @a allocator_strategy to get the data if it's non-0.  If
   * @a data != 0 we assume that we have ownership of the @a data till
   * this object ceases to exist  (and don't delete it during
   * destruction).  If @a locking_strategy is non-0 then this is used
   * to protect regions of code that access shared state (e.g.,
   * reference counting) from race conditions.  Note that the @a size
   * of the <Message_Block> will be @a size, but the @a length will be 0
   * until  <wr_ptr> is set. The @a data_block_allocator is use to
   * allocate the data blocks while the @a allocator_strategy is used
   * to allocate the buffers contained by those.
   */
  int init (size_t size,
            UPF_Message_Type type = MB_DATA,
            UPF_Message_Block *cont = 0,
            const char *data = 0,
            UPF_Allocator *allocator_strategy = 0,
            UPF_Lock *locking_strategy = 0,
            unsigned long priority = UPF_DEFAULT_MESSAGE_BLOCK_PRIORITY,
            const UPF_Time_Value &execution_time = UPF_Time_Value::zero,
            const UPF_Time_Value &deadline_time = UPF_Time_Value::max_time,
            UPF_Allocator *data_block_allocator = 0,
            UPF_Allocator *message_block_allocator = 0);
#endif

  /**
   * Delete all the resources held in the message.
   *
   * Note that <release()> is designed to release the continuation
   * chain; the destructor is not. See <release()> for details.
   */
  virtual ~UPF_Message_Block (void);

  // = Message Type accessors and mutators.

  /// Get type of the message.
  UPF_Message_Type msg_type (void) const;

  /// Set type of the message.
  void msg_type (UPF_Message_Type type);

  /// Find out what type of message this is.
  int is_data_msg (void) const;

  /// Find out what class of message this is (there are two classes,
  /// @c normal messages and @c high-priority messages).
  UPF_Message_Type msg_class (void) const;

  // = Message flag accessors and mutators.
  /// Bitwise-or the @a more_flags into the existing message flags and
  /// return the new value.
  Message_Flags set_flags (Message_Flags more_flags);

  /// Clear the message flag bits specified in @a less_flags and return
  /// the new value.
  Message_Flags clr_flags (Message_Flags less_flags);

  /// Get the current message flags.
  Message_Flags flags (void) const;

  // = Data Block flag accessors and mutators.
  /// Bitwise-or the <more_flags> into the existing message flags and
  /// return the new value.
  /* @todo: I think the following set of methods could not be used at
   *  all. May be they are useless. Let us have it so that we dont
   *  mess up memory management of the Message_Block. Somebody correct
   *  me if I am totally totally wrong..
   */
  Message_Flags set_self_flags (UPF_Message_Block::Message_Flags more_flags);

  /// Clear the message flag bits specified in @a less_flags and return
  /// the new value.
  Message_Flags clr_self_flags (UPF_Message_Block::Message_Flags less_flags);

  /// Get the current message flags.
  Message_Flags self_flags (void) const;

  /// Get priority of the message.
  unsigned long msg_priority (void) const;

  /// Set priority of the message.
  void msg_priority (unsigned long priority);


#if 0 // wangy
  /// Get execution time associated with the message.
  const UPF_Time_Value &msg_execution_time (void) const;

  /// Set execution time associated with the message.
  void msg_execution_time (const UPF_Time_Value &et);

  /// Get absolute time of deadline associated with the message.
  const UPF_Time_Value &msg_deadline_time (void) const;

  /// Set absolute time of deadline associated with the message.
  void msg_deadline_time (const UPF_Time_Value &dt);

#endif 


  // = Deep copy and shallow copy methods.

  /// Return an exact "deep copy" of the message, i.e., create fresh
  /// new copies of all the Data_Blocks and continuations.
  virtual UPF_Message_Block *clone (Message_Flags mask = 0) const;

  /// Return a "shallow" copy that increments our reference count by 1.
  virtual UPF_Message_Block *duplicate (void) const;

  /**
   * Return a "shallow" copy that increments our reference count by 1.
   * This is similar to CORBA's <_duplicate> method, which is useful
   * if you want to eliminate lots of checks for NULL @a mb pointers
   * before calling <_duplicate> on them.
   */
  static UPF_Message_Block *duplicate (const UPF_Message_Block *mb);

  /**
   * Decrease the shared UPF_Data_Block's reference count by 1.  If the
   * UPF_Data_Block's reference count goes to 0, it is deleted.
   * In all cases, this UPF_Message_Block is deleted - it must have come
   * from the heap, or there will be trouble.
   *
   * release() is designed to release the continuation chain; the
   * destructor is not.  If we make the destructor release the
   * continuation chain by calling release() or delete on the message
   * blocks in the continuation chain, the following code will not
   * work since the message block in the continuation chain is not off
   * the heap:
   *
   *  UPF_Message_Block mb1 (1024);
   *  UPF_Message_Block mb2 (1024);
   *
   *  mb1.cont (&mb2);
   *
   * And hence, call release() on a dynamically allocated message
   * block. This will release all the message blocks in the
   * continuation chain.  If you call delete or let the message block
   * fall off the stack, cleanup of the message blocks in the
   * continuation chain becomes the responsibility of the user.
   *
   * @retval 0, always, and the object this method was invoked on is no
   *            longer valid.
   */
  virtual UPF_Message_Block *release (void);

  /**
   * This behaves like the non-static method <release>, except that it
   * checks if @a mb is 0.  This is similar to <CORBA::release>, which
   * is useful if you want to eliminate lots of checks for NULL
   * pointers before calling <release> on them.  Returns @a mb.
   */
  static UPF_Message_Block *release (UPF_Message_Block *mb);

  // = Operations on Message data

  /**
   * Copies data into this UPF_Message_Block. Data is copied into the
   * block starting at the current write pointer.
   *
   * @param buf  Pointer to the buffer to copy from.
   * @param n    The number of bytes to copy.
   *
   * @retval 0  on success; the write pointer is advanced by @arg n.
   * @retval -1 if the amount of free space following the write pointer
   *            in the block is less than @arg n. Free space can be checked
   *            by calling space().
   */
  int copy (const char *buf, size_t n);

  /**
   * Copies a 0-terminated character string into this UPF_Message_Block.
   * The string is copied into the block starting at the current write
   * pointer. The 0-terminator is included in the copied data.
   *
   * @param buf  Pointer to the character string to copy from.
   *
   * @retval 0  on success; the write pointer is advanced by the string's
   *            length, including the 0 terminator.
   * @retval -1 if the amount of free space following the write pointer
   *            in the block is less than required to hold the entire string.
   *            Free space can be checked by calling space().
   */
  int copy (const char *buf);

  /// Normalizes data in the top-level <Message_Block> to align with the base,
  /// i.e., it "shifts" the data pointed to by <rd_ptr> down to the <base> and
  /// then readjusts <rd_ptr> to point to <base> and <wr_ptr> to point
  /// to <base> + the length of the moved data.  Returns -1 and does
  /// nothing if the <rd_ptr> is > <wr_ptr>, else 0 on success.
  int crunch (void);

  /// Resets the Message Block data to contain nothing, i.e., sets the
  /// read and write pointers to align with the base.
  void reset (void);

  /// Access all the allocators in the message block.
  /// @@todo: Not sure whether we would need finer control while
  /// trying to access allocators ie. a method for every allocator.
  /**
   * This method returns the allocators only from the first message
   * block in the chain.
   *
   * @param allocator_strategy Strategy used to allocate the
   *                           underlying buffer
   *
   * @param data_block_allocator Strategy used to allocate the
   *                             underlying data block
   *
   * @param message_block_allocator Strategy used to allocate the
   *                                message block
   */
  void access_allocators (UPF_Allocator *&allocator_strategy,
                          UPF_Allocator *&data_block_allocator,
                          UPF_Allocator *&message_block_allocator);

  /// Reset all the allocators in the message block.
  /// @@todo: Not sure whether we would need finer control while
  /// trying to reset allocators ie. a method for every allocator.
  /**
   * This method resets the allocators in all the message blocks in
   * the chain.
   */
  void reset_allocators (UPF_Allocator *allocator_strategy = 0,
                         UPF_Allocator *data_block_allocator = 0,
                         UPF_Allocator *message_block_allocator = 0);

  /// Get message data.
  char *base (void) const;

  /// Set message data (doesn't reallocate).
  void base (char *data,
             size_t size,
             Message_Flags = DONT_DELETE);

  /// Return a pointer to 1 past the end of the allocated data in a message.
  char *end (void) const;

  /**
   * Return a pointer to 1 past the end of the allotted data in a message.
   * Allotted data may be less than allocated data  if a value smaller than
   * capacity() to is passed to size().
   */
  char *mark (void) const;

  /// Get the read pointer.
  char *rd_ptr (void) const;

  /// Set the read pointer to @a ptr.
  void rd_ptr (char *ptr);

  /// Set the read pointer ahead @a n bytes.
  void rd_ptr (size_t n);

  /// Get the write pointer.
  char *wr_ptr (void) const;

  /// Set the write pointer to @a ptr.
  void wr_ptr (char *ptr);

  /// Set the write pointer ahead @a n bytes.  This is used to compute
  /// the <length> of a message.
  void wr_ptr (size_t n);

  /** @name Message length and size operations
   *
   * Message length is (wr_ptr - rd_ptr).
   *
   * Message size is capacity of the message, including data outside
   * the [rd_ptr,wr_ptr] range.
   */
  //@{
  /// Get the length of the message
  size_t length (void) const;

  /// Set the length of the message
  void length (size_t n);

  /// Get the length of the <Message_Block>s, including chained
  /// <Message_Block>s.
  size_t total_length (void) const;

  /// Get the total number of bytes in all <Message_Block>s, including
  /// chained <Message_Block>s.
  size_t total_size (void) const;

  /// Get the total number of bytes and total length in all
  /// <Message_Block>s, including chained <Message_Block>s.
  void total_size_and_length (size_t &mb_size,
                              size_t &mb_length) const;

  /// Get the number of bytes in the top-level <Message_Block> (i.e.,
  /// does not consider the bytes in chained <Message_Block>s).
  size_t size (void) const;

  /**
   * Set the number of bytes in the top-level <Message_Block>,
   * reallocating space if necessary.  However, the <rd_ptr_> and
   * <wr_ptr_> remain at the original offsets into the buffer, even if
   * it is reallocated.  Returns 0 if successful, else -1.
   */
  int size (size_t length);

  /// Get the number of allocated bytes in all <Message_Block>, including
  /// chained <Message_Block>s.
  size_t total_capacity (void) const;

  /// Get the number of allocated bytes in the top-level <Message_Block>.
  size_t capacity (void) const;

  /// Get the number of bytes available after the <wr_ptr_> in the
  /// top-level <Message_Block>.
  size_t space (void) const;
  //@}

  // = UPF_Data_Block methods.

  /**
   * Get a pointer to the data block. Note that the UPF_Message_Block
   * still references the block; this call does not change the reference
   * count.
   */
  UPF_Data_Block *data_block (void) const;

  /**
   * Set a new data block pointer. The original UPF_Data_Block is released
   * as a result of this call. If you need to keep the original block, call
   * <replUPF_data_block> instead. Upon return, this UPF_Message_Block
   * holds a pointer to the new UPF_Data_Block, taking over the reference
   * you held on it prior to the call.
   */
  void data_block (UPF_Data_Block *);

  /// Set a new data block pointer. A pointer to the original UPF_Data_Block
  /// is returned, and not released (as it is with <data_block>).
  UPF_Data_Block *replUPF_data_block (UPF_Data_Block*);

  // = The continuation field chains together composite messages.
  /// Get the continuation field.
  UPF_Message_Block *cont (void) const;

  /// Set the continuation field.
  void cont (UPF_Message_Block *);

  // = Pointer to the <Message_Block> directly ahead in the UPF_Message_Queue.
  /// Get link to next message.
  UPF_Message_Block *next (void) const;

  /// Set link to next message.
  void next (UPF_Message_Block *);

  // = Pointer to the <Message_Block> directly behind in the UPF_Message_Queue.
  /// Get link to prev message.
  UPF_Message_Block *prev (void) const;

  /// Set link to prev message.
  void prev (UPF_Message_Block *);

  // = The locking strategy prevents race conditions.
  /// Get the locking strategy.
  UPF_Lock *locking_strategy (void);

  /// Set a new locking strategy and return the hold one.
  UPF_Lock *locking_strategy (UPF_Lock *);

  /// Get the current reference count.
  int reference_count (void) const;

  /// Dump the state of an object.
  void dump (void) const;

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;

protected:

#if 0 // wangy
  // = Internal initialization methods.
  /// Perform the actual initialization.
  UPF_Message_Block (size_t size,
                     UPF_Message_Type type,
                     UPF_Message_Block *cont,
                     const char *data,
                     UPF_Allocator *allocator_strategy,
                     UPF_Lock *locking_strategy,
                     Message_Flags flags,
                     unsigned long priority,
                     const UPF_Time_Value &execution_time,
                     const UPF_Time_Value &deadline_time,
                     UPF_Data_Block *db,
                     UPF_Allocator *data_block_allocator,
                     UPF_Allocator *message_block_allocator);
#endif


  /// Internal release implementation
  /// Returns 1 if the data block has to be destroyed.
  int release_i (UPF_Lock *lock);

#if 0 // wangy
  /// Perform the actual initialization.
  int init_i (size_t size,
              UPF_Message_Type type,
              UPF_Message_Block *cont,
              const char *data,
              UPF_Allocator *allocator_strategy,
              UPF_Lock *locking_strategy,
              Message_Flags flags,
              unsigned long priority,
              const UPF_Time_Value &execution_time,
              const UPF_Time_Value &deadline_time,
              UPF_Data_Block *db,
              UPF_Allocator *data_block_allocator,
              UPF_Allocator *message_block_allocator);

#endif 
  /// Pointer to beginning of next read.
  size_t rd_ptr_;

  /// Pointer to beginning of next write.
  size_t wr_ptr_;

  /// Priority of message.
  unsigned long priority_;

#if defined (UPF_HAS_TIMED_MESSAGE_BLOCKS)
  /// Execution time associated with the message.
  UPF_Time_Value execution_time_;

  /// Absolute deadline time for message.
  UPF_Time_Value deadline_time_;
#endif /* UPF_HAS_TIMED_MESSAGE_BLOCKS */

  // = Links to other UPF_Message_Block *s.
  /// Pointer to next message block in the chain.
  UPF_Message_Block *cont_;

  /// Pointer to next message in the list.
  UPF_Message_Block *next_;

  /// Pointer to previous message in the list.
  UPF_Message_Block *prev_;

  /// Misc flags (e.g., DONT_DELETE and USER_FLAGS).
  UPF_Message_Block::Message_Flags flags_;

  /// Pointer to the reference counted data structure that contains the
  /// actual memory buffer.
  UPF_Data_Block *data_block_;

  /// The allocator used to destroy ourselves when release is called
  /// and create new message blocks on duplicate.
  UPF_Allocator *message_block_allocator_;

private:
  // = Disallow these operations for now (use <clone> instead).
  UPF_Message_Block &operator= (const UPF_Message_Block &);
  UPF_Message_Block (const UPF_Message_Block &);
};

class UPF_Export UPF_Data_Block
{
public:
  // = Initialization and termination methods.
  /// Default "do-nothing" constructor.
  UPF_Data_Block (void);

  /// Initialize.
  UPF_Data_Block (size_t size,
                  UPF_Message_Block::UPF_Message_Type msg_type,
                  const char *msg_data,
                  UPF_Allocator *allocator_strategy,
                  UPF_Lock *locking_strategy,
                  UPF_Message_Block::Message_Flags flags,
                  UPF_Allocator *data_block_allocator);

  /// Delete all the resources held in the message.
  virtual ~UPF_Data_Block (void);

  /// Get type of the message.
  UPF_Message_Block::UPF_Message_Type msg_type (void) const;

  /// Set type of the message.
  void msg_type (UPF_Message_Block::UPF_Message_Type type);

  /// Get message data pointer
  char *base (void) const;

  /// Set message data pointer (doesn't reallocate).
  void base (char *data,
             size_t size,
             UPF_Message_Block::Message_Flags mflags = UPF_Message_Block::DONT_DELETE);

  /// Return a pointer to 1 past the end of the allocated data in a message.
  char *end (void) const;

  /**
   * Return a pointer to 1 past the end of the allotted data in a message.
   * The allotted data may be less than allocated data if <size()> is passed
   * an argument less than <capacity()>.
   */
  char *mark (void) const;

  // = Message size is the total amount of space alloted.

  /// Get the total amount of allotted space in the message.  The amount of
  /// allotted space may be less than allocated space.
  size_t size (void) const;

  /// Set the total amount of space in the message.  Returns 0 if
  /// successful, else -1.
  int size (size_t length);

  /// Get the total amount of allocated space.
  size_t capacity (void) const;

  /**
   * Return an exact "deep copy" of the message, i.e., create fresh
   * new copies of all the Data_Blocks and continuations.
   * Notice that Data_Blocks can act as "Prototypes", i.e. derived
   * classes can override this method and create instances of
   * themselves.
   */
  virtual UPF_Data_Block *clone (UPF_Message_Block::Message_Flags mask = 0) const;

  /**
   * As clone above, but it does not copy the contents of the buffer,
   * i.e., create a new Data_Block of the same dynamic type, with the
   * same allocator, locking_strategy, and with the same amount of
   * storage available (if @a max_size is zero) but the buffer is unitialized.
   * If @a max_size is specified other than zero, it will be used when
   * creating the new data block.
   */
  virtual UPF_Data_Block *clone_nocopy (UPF_Message_Block::Message_Flags mask = 0,
                                        size_t max_size = 0) const;

  /// Return a "shallow" copy that increments our reference count by 1.
  UPF_Data_Block *duplicate (void);

  /**
   * Decrease the shared reference count by 1.  If the reference count
   * is > 0 then return this; else if reference count == 0 then delete
   * @c this and @a mb and return 0.  Behavior is undefined if reference
   * count < 0.
   */
  UPF_Data_Block *release (UPF_Lock *lock = 0);

  // = Message flag accessors and mutators.
  /// Bitwise-or the <more_flags> into the existing message flags and
  /// return the new value.
  UPF_Message_Block::Message_Flags set_flags (UPF_Message_Block::Message_Flags more_flags);

  /// Clear the message flag bits specified in <less_flags> and return
  /// the new value.
  UPF_Message_Block::Message_Flags clr_flags (UPF_Message_Block::Message_Flags less_flags);

  /// Get the current message flags.
  UPF_Message_Block::Message_Flags flags (void) const;

  /// Obtain the allocator strategy.
  UPF_Allocator *allocator_strategy (void) const;

  // = The locking strategy prevents race conditions.
  /// Get the locking strategy.
  UPF_Lock *locking_strategy (void);

  /// Set a new locking strategy and return the hold one.
  UPF_Lock *locking_strategy (UPF_Lock *);

  /// Dump the state of an object.
  void dump (void) const;

  /// Get the current reference count.
  int reference_count (void) const;

  /// Get the allocator used to create this object
  UPF_Allocator *data_block_allocator (void) const;

protected:
  /// Internal release implementation
  virtual UPF_Data_Block *release_i (void);

  /// Internal get the current reference count.
  int reference_count_i (void) const;

  /**
   * Decrease the reference count, but don't delete the object.
   * Returns 0 if the object should be removed.
   * If <lock> is equal to the locking strategy then we assume that
   * the lock is beign held by the current thread; this is used to
   * release all the data blocks in a chain while holding a single
   * lock.
   */
  friend class UPF_Message_Block;
  UPF_Data_Block *release_no_delete (UPF_Lock *lock);

  /// Type of message.
  UPF_Message_Block::UPF_Message_Type type_;

  /// Current size of message block.
  size_t cur_size_;

  /// Total size of buffer.
  size_t max_size_;

  /// Misc flags (e.g., DONT_DELETE and USER_FLAGS).
  UPF_Message_Block::Message_Flags flags_;

  /// Pointer To beginning of message payload.
  char *base_;

  // = Strategies.
  /**
   * Pointer to the allocator defined for this UPF_Data_Block.  Note
   * that this pointer is shared by all owners of this
   * UPF_Data_Block.
   */
  UPF_Allocator *allocator_strategy_;

  /**
   * Pointer to the locking strategy defined for this
   * UPF_Data_Block.  This is used to protect regions of code that
   * access shared UPF_Data_Block state.  Note that this lock is
   * shared by all owners of the UPF_Data_Block's data.
   */
  UPF_Lock *locking_strategy_;

  /**
   * Reference count for this UPF_Data_Block, which is used to avoid
   * deep copies (i.e., <clone>).  Note that this pointer value is
   * shared by all owners of the <Data_Block>'s data, i.e., all the
   * UPF_Message_Blocks.
   */
  int reference_count_;

  /// The allocator use to destroy ourselves.
  UPF_Allocator *data_block_allocator_;

private:
  // = Disallow these operations.
  UPF_Data_Block &operator= (const UPF_Data_Block &);
  UPF_Data_Block (const UPF_Data_Block &);
};


#endif 