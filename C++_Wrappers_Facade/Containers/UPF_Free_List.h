#ifndef UPF_FREE_LIST_H_
#define UPF_FREE_LIST_H_
#include "..\OS_Adapter\OS_Adapter_Common\UPF_Guard_T.h"

template <class T>
class UPF_Free_List
{
public:
 
  virtual ~UPF_Free_List (void);

  
  virtual void add (T *element) = 0;

 
  virtual T *remove (void) = 0;

  
  virtual size_t size (void) = 0;

  
  virtual void resize (size_t newsize) = 0;
};


template <class T, class UPF_LOCK>
class UPF_Locked_Free_List : public UPF_Free_List<T>
{
public:
  
  /// 构造函数
  UPF_Locked_Free_List (int mode = UPF_FREE_LIST_WITH_POOL,
                        size_t prealloc = UPF_DEFAULT_FREE_LIST_PREALLOC,
                        size_t lwm = UPF_DEFAULT_FREE_LIST_LWM,
                        size_t hwm = UPF_DEFAULT_FREE_LIST_HWM,
                        size_t inc = UPF_DEFAULT_FREE_LIST_INC);

  /// 析构函数 - 删除 free_list中的所有元素.
  virtual ~UPF_Locked_Free_List (void);

  /// 向free_list中插入一个元素 (if it isn't past the high
  /// water mark).
  virtual void add (T *element);

  /// Takes a element off the freelist and returns it.  It creates
  /// <inc> new elements if the size is at or below the low water mark.
  virtual T *remove (void);

  /// Returns the current size of the free list.
  virtual size_t size (void);

  /// Resizes the free list to @a newsize.
  virtual void resize (size_t newsize);

protected:
  /// Allocates @a n extra nodes for the freelist.
  virtual void alloc (size_t n);

  /// Removes and frees @a n nodes from the freelist.
  virtual void dealloc (size_t n);

  /// Free list operation mode, either UPF_FREE_LIST_WITH_POOL or
  /// UPF_PURE_FREE_LIST.
  int mode_;

  /// Pointer to the first node in the freelist.
  T *free_list_;

  /// Low water mark.
  size_t lwm_;

  /// High water mark.
  size_t hwm_;

  /// Increment value.
  size_t inc_;

  /// Keeps track of the size of the list.
  size_t size_;

  /// Synchronization variable for UPF_Timer_Queue.
  UPF_LOCK mutex_;

private:
  // = Don't allow these operations for now.
  UPF_UNIMPLEMENTED_FUNC (UPF_Locked_Free_List (const UPF_Locked_Free_List<T, UPF_LOCK> &))
  UPF_UNIMPLEMENTED_FUNC (void operator= (const UPF_Locked_Free_List<T, UPF_LOCK> &))
};
template <class T>
UPF_Free_List<T>::~UPF_Free_List (void)
{
}

// Default constructor that takes in a preallocation number
// (<prealloc>), a low and high water mark (<lwm> and <hwm>) and an
// increment value (<inc>)

template <class T, class UPF_LOCK>
UPF_Locked_Free_List<T, UPF_LOCK>::UPF_Locked_Free_List (int mode,
                                                     size_t prealloc,
                                                     size_t lwm,
                                                     size_t hwm,
                                                     size_t inc)
  : mode_ (mode),
    free_list_ (0),
    lwm_ (lwm),
    hwm_ (hwm),
    inc_ (inc),
    size_ (0)
{
  this->alloc (prealloc);
}

// Destructor - removes all the elements from the free_list

template <class T, class UPF_LOCK>
UPF_Locked_Free_List<T, UPF_LOCK>::~UPF_Locked_Free_List (void)
{
  if (this->mode_ != UPF_PURE_FREE_LIST)
    while (this->free_list_ != 0)
      {
        T *temp = this->free_list_;
        this->free_list_ = this->free_list_->get_next ();
        delete temp;
      }
}

// Inserts an element onto the free list (if we are allowed to manage
// elements withing and it pasts the high water mark, delete the
// element)

template <class T, class UPF_LOCK> void
UPF_Locked_Free_List<T, UPF_LOCK>::add (T *element)
{
  UPF_MT (UPF_GUARD (UPF_LOCK, UPF_mon, this->mutex_));

  // Check to see that we not at the high water mark.
  if (this->mode_ == UPF_PURE_FREE_LIST
      || this->size_ < this->hwm_)
    {
      element->set_next (this->free_list_);
      this->free_list_ = element;
      this->size_++;
    }
  else
    delete element;
}

// Takes a element off the freelist and returns it.  It creates <inc>
// new elements if we are allowed to do it and the size is at the low
// water mark.

template <class T, class UPF_LOCK> T *
UPF_Locked_Free_List<T, UPF_LOCK>::remove (void)
{
  UPF_MT (UPF_GUARD_RETURN (UPF_LOCK, UPF_mon, this->mutex_, 0));

  // If we are at the low water mark, add some nodes
  if (this->mode_ != UPF_PURE_FREE_LIST && this->size_ <= this->lwm_)
    this->alloc (this->inc_);

  // Remove a node
  T *temp = this->free_list_;

  if (temp != 0)
    {
      this->free_list_ = this->free_list_->get_next ();
      this->size_--;
    }

  return temp;
}


// Returns the current size of the free list

template <class T, class UPF_LOCK> size_t
UPF_Locked_Free_List<T, UPF_LOCK>::size (void)
{
  return this->size_;
}

// Resizes the free list to <newsize>

template <class T, class UPF_LOCK> void
UPF_Locked_Free_List<T, UPF_LOCK>::resize (size_t newsize)
{
  UPF_MT (UPF_GUARD (UPF_LOCK, UPF_mon, this->mutex_));

  // Check if we are allowed to resize
  if (this->mode_ != UPF_PURE_FREE_LIST)
    // Check to see if we grow or shrink
    if (newsize < this->size_)
      this->dealloc (this->size_ - newsize);
    else
      this->alloc (newsize - this->size_);
}

// Allocates <n> extra nodes for the freelist

template <class T, class UPF_LOCK> void
UPF_Locked_Free_List<T, UPF_LOCK>::alloc (size_t n)
{
  for (; n > 0; n--)
    {
      T *temp = 0;
      UPF_NEW (temp, T);
      temp->set_next (this->free_list_);
      this->free_list_ = temp;
      this->size_++;
    }
}

// Removes and frees <n> nodes from the freelist.

template <class T, class UPF_LOCK> void
UPF_Locked_Free_List<T, UPF_LOCK>::dealloc (size_t n)
{
  for (; this->free_list_ != 0 && n > 0;
       n--)
    {
      T *temp = this->free_list_;
      this->free_list_ = this->free_list_->get_next ();
      delete temp;
      this->size_--;
    }
}

#endif