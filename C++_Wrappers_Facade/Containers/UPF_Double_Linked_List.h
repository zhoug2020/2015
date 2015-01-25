#ifndef UPF_DOUBLE_LINKED_LIST_H_
#define UPF_DOUBLE_LINKED_LIST_H_
#include "..\Malloc_Base.h"

template <class T>
class UPF_Double_Linked_List;


template <class T>
class UPF_Double_Linked_List_Iterator_Base
{
public:
  // = Iteration methods.

  /// Passes back the {entry} under the iterator. Returns 0 if the
  /// iteration has completed, otherwise 1
  int next (T *&) const;

  /**
   * @deprecated Return the address of next (current) unvisited item in
   * the list. 0 if there is no more element available.
   */
  T *next (void) const;

  /// Returns 1 when all items have been seen, else 0.
  int done (void) const;

  /// STL-like iterator dereference operator: returns a reference
  /// to the node underneath the iterator.
  T & operator* (void) const ;

  /**
   * Retasks the iterator to iterate over a new
   * Double_Linked_List. This allows clients to reuse an iterator
   * without incurring the constructor overhead. If you do use this,
   * be aware that if there are more than one reference to this
   * iterator, the other "clients" may be very bothered when their
   * iterator changes.  @@ Here be dragons. Comments?
   */
  void reset (UPF_Double_Linked_List<T> &);

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;

protected:
  // = Initialization methods.

  /// Constructor
  UPF_Double_Linked_List_Iterator_Base (const UPF_Double_Linked_List<T> &);

  /// Copy constructor.
  UPF_Double_Linked_List_Iterator_Base (const
                                        UPF_Double_Linked_List_Iterator_Base<T>
                                        &iter);

  // = Iteration methods.
  /**
   * Move to the first element of the list. Returns 0 if the list is
   * empty, else 1.
   * @note the head of the UPF_DLList is actually a null entry, so the
   * first element is actually the 2n'd entry
   */
  int go_head (void);

  /// Move to the last element of the list. Returns 0 if the list is
  /// empty, else 1.
  int go_tail (void);

  /**
   * Check if we reach the end of the list.  Can also be used to get
   * the *current* element in the list.  Return the address of the
   * current item if there are still elements left , 0 if we run out
   * of element.
   */
  T *not_done (void) const ;

  /// Advance to the next element in the list.  Return the address of the
  /// next element if there are more, 0 otherwise.
  T *do_advance (void);

  /// Retreat to the previous element in the list.  Return the address
  /// of the previous element if there are more, 0 otherwise.
  T *do_retreat (void);

  /// Dump the state of an object.
  void dump_i (void) const;

  /// Remember where we are.
  T *current_;

  const UPF_Double_Linked_List<T> *dllist_;
};

/**
 * @class UPF_Double_Linked_List_Iterator
 *
 * @brief Implements an iterator for a double linked list ADT
 *
 * Iterate thru the double-linked list.  This class provides
 * an interface that let users access the internal element
 * addresses directly. Notice {class T} must declare
 * UPF_Double_Linked_List&lt;T&gt;,
 * UPF_Double_Linked_List_Iterator_Base &lt;T&gt; and
 * UPF_Double_Linked_List_Iterator as friend classes and class T
 * should also have data members T* next_ and T* prev_.
 */
template <class T>
class UPF_Double_Linked_List_Iterator : public UPF_Double_Linked_List_Iterator_Base <T>
{
public:
  // = Initialization method.
  UPF_Double_Linked_List_Iterator (const UPF_Double_Linked_List<T> &);

  /**
   * Retasks the iterator to iterate over a new
   * Double_Linked_List. This allows clients to reuse an iterator
   * without incurring the constructor overhead. If you do use this,
   * be aware that if there are more than one reference to this
   * iterator, the other "clients" may be very bothered when their
   * iterator changes.
   * @@ Here be dragons. Comments?
   */
  void reset (UPF_Double_Linked_List<T> &);

  /// Move to the first element in the list.  Returns 0 if the
  /// list is empty, else 1.
  int first (void);

  /// Move forward by one element in the list.  Returns 0 when all the
  /// items in the list have been seen, else 1.
  int advance (void);

  /**
   * Advance the iterator while removing the original item from the
   * list.  Return a pointer points to the original (removed) item.
   * If {dont_remove} equals 0, this function behaves like {advance}
   * but return 0 (NULL) instead.
   */
  T* advance_and_remove (int dont_remove);

  // = STL-style iteration methods

  /// Prefix advance.
  UPF_Double_Linked_List_Iterator<T> & operator++ (void);

  /// Postfix advance.
  UPF_Double_Linked_List_Iterator<T> operator++ (int);

  /// Prefix reverse.
  UPF_Double_Linked_List_Iterator<T> & operator-- (void);

  /// Postfix reverse.
  UPF_Double_Linked_List_Iterator<T> operator-- (int);

  /// Dump the state of an object.
  void dump (void) const;

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;
};
template <class T>
class UPF_Double_Linked_List_Reverse_Iterator : public UPF_Double_Linked_List_Iterator_Base <T>
{
public:
  // = Initialization method.
  UPF_Double_Linked_List_Reverse_Iterator (UPF_Double_Linked_List<T> &);

  /**
   * Retasks the iterator to iterate over a new
   * Double_Linked_List. This allows clients to reuse an iterator
   * without incurring the constructor overhead. If you do use this,
   * be aware that if there are more than one reference to this
   * iterator, the other "clients" may be very bothered when their
   * iterator changes.
   * @@ Here be dragons. Comments?
   */
  void reset (UPF_Double_Linked_List<T> &);

  /// Move to the first element in the list.  Returns 0 if the
  /// list is empty, else 1.
  int first (void);

  /// Move forward by one element in the list.  Returns 0 when all the
  /// items in the list have been seen, else 1.
  int advance (void);

  /**
   * Advance the iterator while removing the original item from the
   * list.  Return a pointer points to the original (removed) item.
   * If {dont_remove} equals 0, this function behaves like {advance}
   * but return 0 (NULL) instead.
   */
  T* advance_and_remove (int dont_remove);

  // = STL-style iteration methods

  /// Prefix advance.
  UPF_Double_Linked_List_Reverse_Iterator<T> & operator++ (void);

  /// Postfix advance.
  UPF_Double_Linked_List_Reverse_Iterator<T> operator++ (int);

  /// Prefix reverse.
  UPF_Double_Linked_List_Reverse_Iterator<T> & operator-- (void);

  /// Postfix reverse.
  UPF_Double_Linked_List_Reverse_Iterator<T> operator-- (int);

  /// Dump the state of an object.
  void dump (void) const;

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;
};



template <class T>
class UPF_Double_Linked_List
{
public:
  friend class UPF_Double_Linked_List_Iterator_Base<T>;
  friend class UPF_Double_Linked_List_Iterator<T>;
  friend class UPF_Double_Linked_List_Reverse_Iterator<T>;

  // Trait definition.
  typedef UPF_Double_Linked_List_Iterator<T> ITERATOR;
  typedef UPF_Double_Linked_List_Reverse_Iterator<T> REVERSE_ITERATOR;

  // = Initialization and termination methods.
  /// construction.  Use user specified allocation strategy
  /// if specified.
  /**
   * Initialize an empy list using the allocation strategy specified by the user.
   * If none is specified, then use default allocation strategy.
   */
  UPF_Double_Linked_List (UPF_Allocator *the_allocator = 0);

  /// Copy constructor.
  /**
   * Create a double linked list that is a copy of the provided
   * parameter.
   */
  UPF_Double_Linked_List (const UPF_Double_Linked_List<T> &);

  /// Assignment operator.
  /**
   * Perform a deep copy of the provided list by first deleting the nodes of the
   * lhs and then copying the nodes of the rhs.
   */
  void operator= (const UPF_Double_Linked_List<T> &);

  /// Destructor.
  /**
   * Clean up the memory allocated for the nodes of the list.
   */
  ~UPF_Double_Linked_List (void);

  // = Check boundary conditions.

  /// Returns 1 if the container is empty, 0 otherwise.
  /**
   * Performs constant time check to determine if the list is empty.
   */
  int is_empty (void) const;

  /// The list is unbounded, so this always returns 0.
  /**
   * Since the list is unbounded, the method simply returns 0.
   */
  int is_full (void) const;

  // = Classic queue operations.

  /// Adds @a new_item to the tail of the list. Returns the new item
  /// that was inserted.
  /**
   * Provides constant time insertion at the end of the list structure.
   */
  T *insert_tail (T *new_item);

  /// Adds @a new_item to the head of the list.Returns the new item that
  /// was inserted.
  /**
   * Provides constant time insertion at the head of the list.
   */
  T *insert_head (T *new_item);

  /// Removes the head of the list and returns a pointer to that item.
  /**
   * Removes and returns the first {item} in the list.  Returns
   * internal node's address on success, 0 if the queue was empty.
   * This method will *not* free the internal node.
   */
  T* delete_head (void);

  /// Removes the tail of the list and returns a pointer to that item.
  /**
   * Removes and returns the last {item} in the list.  Returns
   * internal nodes's address on success, 0 if the queue was
   * empty. This method will *not* free the internal node.
   */
  T *delete_tail (void);

  // = Additional utility methods.

  ///Empty the list.
  /**
   * Reset the {UPF_Double_Linked_List} to be empty.
   * Notice that since no one is interested in the items within,
   * This operation will delete all items.
   */
  void reset (void);

  /// Get the {slot}th element in the set.  Returns -1 if the element
  /// isn't in the range {0..{size} - 1}, else 0.
  /**
   * Iterates through the list to the desired index and assigns the provides pointer
   * with the address of the node occupying that index.
   */
  int get (T *&item, size_t slot = 0);

  /// The number of items in the queue.
  /**
   * Constant time call to return the current size of the list.
   */
  size_t size (void) const;

  /// Dump the state of an object.
  void dump (void) const;

  /// Use DNode address directly.
  /**
   * Constant time removal of an item from the list using it's address.
   */
  int remove (T *n);

  /// Declare the dynamic allocation hooks.
  UPF_ALLOC_HOOK_DECLARE;

protected:
  /// Delete all the nodes in the list.
  /**
   * Removes and deallocates memory for all of the list nodes.
   */
  void delete_nodes (void);

  /// Copy nodes from {rhs} into this list.
  /**
   * Copy the elements of the provided list by allocated new nodes and assigning
   * them with the proper data.
   */
  void copy_nodes (const UPF_Double_Linked_List<T> &rhs);

  /// Setup header pointer.  Called after we create the head node in ctor.
  /**
   * Initialize the head pointer so that the list has a dummy node.
   */
  void init_head (void);

  ///Constant time insert a new item into the list structure.
  /**
   * Insert a @a new_item into the list.  It will be added before
   * or after @a old_item.  Default is to insert the new item *after*
   * {head_}.  Return 0 if succeed, -1 if error occured.
   */
  int insert_element (T *new_item,
                      int before = 0,
                      T *old_item = 0);

  ///Constant time delete an item from the list structure.
  /**
   * Remove @a item from the list.  Return 0 if succeed, -1 otherwise.
   * Notice that this function checks if item is {head_} and either its
   * {next_} or {prev_} is NULL.  The function resets item's {next_} and
   * {prev_} to 0 to prevent clobbering the double-linked list if a user
   * tries to remove the same node again.
   */
  int remove_element (T *item);

  /// Head of the circular double-linked list.
  T *head_;

  /// Size of this list.
  size_t size_;

  /// Allocation Strategy of the queue.
  UPF_Allocator *allocator_;
};

UPF_ALLOC_HOOK_DEFINE(UPF_Double_Linked_List_Iterator_Base)

template <class T>
UPF_Double_Linked_List_Iterator_Base<T>::UPF_Double_Linked_List_Iterator_Base (const UPF_Double_Linked_List<T> &dll)
  : current_ (0), dllist_ (&dll)
{
  // Do nothing
}

template <class T>
UPF_Double_Linked_List_Iterator_Base<T>::UPF_Double_Linked_List_Iterator_Base (const UPF_Double_Linked_List_Iterator_Base<T> &iter)
  : current_ (iter.current_),
    dllist_ (iter.dllist_)
{
  // Do nothing
}


template <class T> T *
UPF_Double_Linked_List_Iterator_Base<T>::next (void) const
{
  return this->not_done ();
}

template <class T> int
UPF_Double_Linked_List_Iterator_Base<T>::next (T *&ptr) const
{
  ptr = this->not_done ();
  return ptr ? 1 : 0;
}


template <class T> int
UPF_Double_Linked_List_Iterator_Base<T>::done (void) const
{
  return this->not_done () ? 0 : 1;
}

template <class T> T &
UPF_Double_Linked_List_Iterator_Base<T>::operator* (void) const
{
  return *(this->not_done ());
}

// @@ Is this a valid retasking? Make sure to check with Purify and
// whatnot that we're not leaking memory or doing any other screwing things.
template <class T> void
UPF_Double_Linked_List_Iterator_Base<T>::reset (UPF_Double_Linked_List<T> &dll)
{
  current_ = 0;
  dllist_ = &dll;
}

 template <class T> int
UPF_Double_Linked_List_Iterator_Base<T>::go_head (void)
{
  this->current_ = static_cast<T*> (dllist_->head_->next_);
  return this->current_ ? 1 : 0;
}

template <class T> int
UPF_Double_Linked_List_Iterator_Base<T>::go_tail (void)
{
  this->current_ = static_cast<T*> (dllist_->head_->prev_);
  return this->current_ ? 1 : 0;
}

template <class T> T *
UPF_Double_Linked_List_Iterator_Base<T>::not_done (void) const
{
  if (this->current_ != this->dllist_->head_)
    return this->current_;
  else
    return 0;
}

template <class T> T *
UPF_Double_Linked_List_Iterator_Base<T>::do_advance (void)
{
  if (this->not_done ())
    {
      this->current_ = static_cast<T*> (this->current_->next_);
      return this->not_done ();
    }
  else
    return 0;
}

template <class T> T *
UPF_Double_Linked_List_Iterator_Base<T>::do_retreat (void)
{
  if (this->not_done ())
    {
      this->current_ = static_cast<T*> (this->current_->prev_);
      return this->not_done ();
    }
  else
    return 0;
}

template <class T> void
UPF_Double_Linked_List_Iterator_Base<T>::dump_i (void) const
{
  UPF_DEBUG ((LM_DEBUG, UPF_BEGIN_DUMP, this));
  UPF_DEBUG ((LM_DEBUG,  UPF_TEXT ("current_ = %x"), this->current_));
  UPF_DEBUG ((LM_DEBUG, UPF_END_DUMP));
}

//--------------------------------------------------
UPF_ALLOC_HOOK_DEFINE(UPF_Double_Linked_List_Iterator)

template <class T>
UPF_Double_Linked_List_Iterator<T>::UPF_Double_Linked_List_Iterator (const UPF_Double_Linked_List<T> &dll)
  : UPF_Double_Linked_List_Iterator_Base <T> (dll)
{
  this->current_ = static_cast<T*> (dll.head_->next_);
  // Advance current_ out of the null area and onto the first item in
  // the list
}

template <class T> void
UPF_Double_Linked_List_Iterator<T>::reset (UPF_Double_Linked_List<T> &dll)
{
  this->UPF_Double_Linked_List_Iterator_Base <T>::reset (dll);
  this->current_ = static_cast<T*> (dll.head_->next_);
  // Advance current_ out of the null area and onto the first item in
  // the list
}

template <class T> int
UPF_Double_Linked_List_Iterator<T>::first (void)
{
  return this->go_head ();
}

template <class T> int
UPF_Double_Linked_List_Iterator<T>::advance (void)
{
  return this->do_advance () ? 1 : 0;
}

template <class T> T*
UPF_Double_Linked_List_Iterator<T>::advance_and_remove (int dont_remove)
{
  T* item = 0;
  if (dont_remove)
    this->do_advance ();
  else
    {
      item = this->next ();
      this->do_advance ();
      // It seems dangerous to remove nodes in an iterator, but so it goes...
      UPF_Double_Linked_List<T> *dllist =
        const_cast<UPF_Double_Linked_List<T> *> (this->dllist_);
      dllist->remove (item);
    }
  return item;
}

template <class T> void
UPF_Double_Linked_List_Iterator<T>::dump (void) const
{
#if defined (UPF_HAS_DUMP)
  this->dump_i ();
#endif /* UPF_HAS_DUMP */
}

// Prefix advance.

template <class T>
UPF_Double_Linked_List_Iterator<T> &
UPF_Double_Linked_List_Iterator<T>::operator++ (void)
{
  this->do_advance ();
  return *this;
}


// Postfix advance.

template <class T>
UPF_Double_Linked_List_Iterator<T>
UPF_Double_Linked_List_Iterator<T>::operator++ (int)
{
  UPF_Double_Linked_List_Iterator<T> retv (*this);
  this->do_advance ();
  return retv;
}


// Prefix reverse.

template <class T>
UPF_Double_Linked_List_Iterator<T> &
UPF_Double_Linked_List_Iterator<T>::operator-- (void)
{
  this->do_retreat ();
  return *this;
}


// Postfix reverse.

template <class T>
UPF_Double_Linked_List_Iterator<T>
UPF_Double_Linked_List_Iterator<T>::operator-- (int)
{
  UPF_Double_Linked_List_Iterator<T> retv (*this);
  this->do_retreat ();
  return retv;
}


//--------------------------------------------------
UPF_ALLOC_HOOK_DEFINE(UPF_Double_Linked_List_Reverse_Iterator)

  template <class T>
UPF_Double_Linked_List_Reverse_Iterator<T>::UPF_Double_Linked_List_Reverse_Iterator (UPF_Double_Linked_List<T> &dll)
  : UPF_Double_Linked_List_Iterator_Base <T> (dll)
{
  this->current_ = static_cast<T*> (dll.head_->prev_);
  // Advance current_ out of the null area and onto the last item in
  // the list
}

template <class T> void
UPF_Double_Linked_List_Reverse_Iterator<T>::reset (UPF_Double_Linked_List<T> &dll)
{
  this->UPF_Double_Linked_List_Iterator_Base <T>::reset (dll);
  this->current_ = static_cast<T*> (dll.head_->prev_);
  // Advance current_ out of the null area and onto the last item in
  // the list
}

template <class T> int
UPF_Double_Linked_List_Reverse_Iterator<T>::first (void)
{
  return this->go_tail ();
}

template <class T> int
UPF_Double_Linked_List_Reverse_Iterator<T>::advance (void)
{
  return this->do_retreat () ? 1 : 0;
}

template <class T> T*
UPF_Double_Linked_List_Reverse_Iterator<T>::advance_and_remove (int dont_remove)
{
  T* item = 0;
  if (dont_remove)
    this->do_retreat ();
  else
    {
      item = this->next ();
      this->do_retreat ();
      // It seems dangerous to remove nodes in an iterator, but so it goes...
      UPF_Double_Linked_List<T> *dllist =
        const_cast<UPF_Double_Linked_List<T> *> (this->dllist_);
      dllist->remove (item);
    }
  return item;
}

template <class T> void
UPF_Double_Linked_List_Reverse_Iterator<T>::dump (void) const
{
#if defined (UPF_HAS_DUMP)
  this->dump_i ();
#endif /* UPF_HAS_DUMP */
}

// Prefix advance.

template <class T>
UPF_Double_Linked_List_Reverse_Iterator<T> &
UPF_Double_Linked_List_Reverse_Iterator<T>::operator++ (void)
{
  this->do_retreat ();
  return *this;
}


// Postfix advance.

template <class T>
UPF_Double_Linked_List_Reverse_Iterator<T>
UPF_Double_Linked_List_Reverse_Iterator<T>::operator++ (int)
{
  UPF_Double_Linked_List_Reverse_Iterator<T> retv (*this);
  this->do_retreat ();
  return retv;
}


// Prefix reverse.

template <class T>
UPF_Double_Linked_List_Reverse_Iterator<T> &
UPF_Double_Linked_List_Reverse_Iterator<T>::operator-- (void)
{
  this->do_advance ();
  return *this;
}


// Postfix reverse.

template <class T>
UPF_Double_Linked_List_Reverse_Iterator<T>
UPF_Double_Linked_List_Reverse_Iterator<T>::operator-- (int)
{
  UPF_Double_Linked_List_Reverse_Iterator<T> retv (*this);
  this->do_advance ();
  return retv;
}

//--------------------------------------------------
UPF_ALLOC_HOOK_DEFINE(UPF_Double_Linked_List)

 template <class T>
UPF_Double_Linked_List<T>:: UPF_Double_Linked_List (UPF_Allocator *alloc)
  : size_ (0), allocator_ (alloc)
{
  if (this->allocator_ == 0)
    this->allocator_ = UPF_Allocator::instance ();

  UPF_NEW_MALLOC (this->head_,
                  (T *) this->allocator_->malloc (sizeof (T)),
                  T);
  this->init_head ();
}

template <class T>
UPF_Double_Linked_List<T>::UPF_Double_Linked_List (const UPF_Double_Linked_List<T> &cx)
  : allocator_ (cx.allocator_)
{
  if (this->allocator_ == 0)
    this->allocator_ = UPF_Allocator::instance ();

  UPF_NEW_MALLOC (this->head_,
                  (T *) this->allocator_->malloc (sizeof (T)),
                  T);
  this->init_head ();
  this->copy_nodes (cx);
  this->size_ = cx.size_;
}

template <class T> void
UPF_Double_Linked_List<T>::operator= (const UPF_Double_Linked_List<T> &cx)
{
  if (this != &cx)
    {
      this->delete_nodes ();
      this->copy_nodes (cx);
    }
}

template <class T>
UPF_Double_Linked_List<T>::~UPF_Double_Linked_List (void)
{
  this->delete_nodes ();

  UPF_DES_FREE (head_,
                this->allocator_->free,
                T);

  this->head_ = 0;
}

template <class T> int
UPF_Double_Linked_List<T>::is_empty (void) const
{
  return this->size () ? 0 : 1;
}

template <class T> int
UPF_Double_Linked_List<T>::is_full (void) const
{
  return 0;                     // We have no bound.
}

template <class T> T *
UPF_Double_Linked_List<T>::insert_tail (T *new_item)
{
  // Insert it before <head_>, i.e., at tail.
  this->insert_element (new_item, 1);
  return new_item;
}

template <class T> T *
UPF_Double_Linked_List<T>::insert_head (T *new_item)
{
  this->insert_element (new_item); // Insert it after <head_>, i.e., at head.
  return new_item;
}

template <class T> T *
UPF_Double_Linked_List<T>::delete_head (void)
{
  if (this->is_empty ())
    return 0;

  T *temp = static_cast<T *> (this->head_->next_);
  // Detach it from the list.
  this->remove_element (temp);
  return temp;
}

template <class T> T *
UPF_Double_Linked_List<T>::delete_tail (void)
{
  if (this->is_empty ())
    return 0;

  T *temp = static_cast <T *> (this->head_->prev_);
  // Detach it from the list.
  this->remove_element (temp);
  return temp;
}

template <class T> void
UPF_Double_Linked_List<T>::reset (void)
{
  this->delete_nodes ();
}

template <class T> int
UPF_Double_Linked_List<T>::get (T *&item, size_t slot)
{
  UPF_Double_Linked_List_Iterator<T> iter (*this);

  for (size_t i = 0;
       i < slot && !iter.done ();
       i++)
    iter.advance ();

  item = iter.next ();
  return item ? 0 : -1;
}

template <class T> size_t
UPF_Double_Linked_List<T>::size (void) const
{
  return this->size_;
}

template <class T> void
UPF_Double_Linked_List<T>::dump (void) const
{
#if defined (UPF_HAS_DUMP)
  // Dump the state of an object.
#endif /* UPF_HAS_DUMP */
}

#if 0
template <class T> T *
UPF_Double_Linked_List<T>::find (const T &item)
{
  for (UPF_Double_Linked_List_Iterator<T> iter (*this);
       !iter.done ();
       iter.advance ())
    {
      T *temp = iter.next ();

      if (*temp == item)
        return temp;
    }

  return 0;
}

template <class T> int
UPF_Double_Linked_List<T>::remove (const T &item)
{
  T *temp = this->find (item);

  if (temp != 0)
    return this->remove (temp);
  else
    return -1;
}
#endif /* 0 */

template <class T> int
UPF_Double_Linked_List<T>::remove (T *n)
{
  return this->remove_element (n);
}

template <class T> void
UPF_Double_Linked_List<T>::delete_nodes (void)
{
  while (! this->is_empty ())
    {
      T * temp = static_cast<T*> (this->head_->next_);
      this->remove_element (temp);
      UPF_DES_FREE (temp,
                    this->allocator_->free,
                    T);
    }
}

template <class T> void
UPF_Double_Linked_List<T>::copy_nodes (const UPF_Double_Linked_List<T> &c)
{
  for (UPF_Double_Linked_List_Iterator<T> iter (c);
       !iter.done ();
       iter.advance ())
    {
      T* temp = 0;
      UPF_NEW_MALLOC (temp,
                      (T *)this->allocator_->malloc (sizeof (T)),
                      T (*iter.next ()));
      this->insert_tail (temp);
    }
}

template <class T> void
UPF_Double_Linked_List<T>::init_head (void)
{
  this->head_->next_ = this->head_;
  this->head_->prev_ = this->head_;
}

template <class T> int
UPF_Double_Linked_List<T>::insert_element (T *new_item,
                                           int before,
                                           T *old_item)
{
  if (old_item == 0)
    old_item = this->head_;

  if (before)
    old_item = static_cast<T *> (old_item->prev_);

  new_item->next_ = old_item->next_;
  new_item->next_->prev_ = new_item;
  new_item->prev_ = old_item;
  old_item->next_ = new_item;
  ++this->size_;
  return 0;                     // Well, what will cause errors here?
}

template <class T> int
UPF_Double_Linked_List<T>::remove_element (T *item)
{
  // Notice that you have to ensure that item is an element of this
  // list.  We can't do much checking here.

  if (item == this->head_ || item->next_ == 0
      || item->prev_ == 0 || this->size () == 0)      // Can't remove head
    return -1;

  item->prev_->next_ = item->next_;
  item->next_->prev_ = item->prev_;
  item->next_ = item->prev_ = 0; // reset pointers to prevent double removal.
  --this->size_;
  return 0;
}

#endif 