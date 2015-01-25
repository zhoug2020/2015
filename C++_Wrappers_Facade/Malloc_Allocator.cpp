#include "Malloc_Allocator.h"

void *
UPF_New_Allocator::malloc (size_t nbytes)
{
  char *ptr = 0;

  if (nbytes > 0)
    UPF_NEW_RETURN (ptr, char[nbytes], 0);
  return (void *) ptr;
}

void *
UPF_New_Allocator::calloc (size_t nbytes,
                           char initial_value)
{
  char *ptr = 0;

  UPF_NEW_RETURN (ptr, char[nbytes], 0);

  UPF_OS::memset (ptr, initial_value, nbytes);
  return (void *) ptr;
}

void *
UPF_New_Allocator::calloc (size_t n_elem, size_t elem_size, char initial_value)
{
  return UPF_New_Allocator::calloc (n_elem * elem_size, initial_value);
}

void
UPF_New_Allocator::free (void *ptr)
{
  delete [] (char *) ptr;
}

int
UPF_New_Allocator::remove (void)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::bind (const char *, void *, int)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::trybind (const char *, void *&)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::find (const char *, void *&)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::find (const char *)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::unbind (const char *)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::unbind (const char *, void *&)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::sync (ssize_t, int)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::sync (void *, size_t, int)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::protect (ssize_t, int)
{
  UPF_NOTSUP_RETURN (-1);
}

int
UPF_New_Allocator::protect (void *, size_t, int)
{
  UPF_NOTSUP_RETURN (-1);
}

#if defined (UPF_HAS_MALLOC_STATS)
void
UPF_New_Allocator::print_stats (void) const
{
}
#endif /* UPF_HAS_MALLOC_STATS */

void
UPF_New_Allocator::dump (void) const
{
#if defined (UPF_HAS_DUMP)
#endif /* UPF_HAS_DUMP */
}

/******************************************************************************/


void *
UPF_Static_Allocator_Base::malloc (size_t nbytes)
{
  if (this->offset_ + nbytes > this->size_)
    {
      errno = ENOMEM;
      return 0;
    }
  else
    {
      // Record the current offset, increment the offset by the number
      // of bytes requested, and return the original offset.
      char *ptr = &this->buffer_[this->offset_];
      this->offset_ += nbytes;
      return (void *) ptr;
    }
}

void *
UPF_Static_Allocator_Base::calloc (size_t nbytes,
                                   char initial_value)
{
  void *ptr = this->malloc (nbytes);

  UPF_OS::memset (ptr, initial_value, nbytes);
  return (void *) ptr;
}

void *
UPF_Static_Allocator_Base::calloc (size_t n_elem,
                                   size_t elem_size,
                                   char initial_value)
{
  return this->calloc (n_elem * elem_size, initial_value);
}

void
UPF_Static_Allocator_Base::free (void *ptr)
{
  // Check to see if ptr is within our pool?!
  UPF_UNUSED_ARG (ptr);
  UPF_ASSERT (ptr >= this->buffer_ && ptr < this->buffer_ + this->size_);
}

int
UPF_Static_Allocator_Base::remove (void)
{
  return -1;
}

int
UPF_Static_Allocator_Base::bind (const char *, void *, int)
{
  return -1;
}

int
UPF_Static_Allocator_Base::trybind (const char *, void *&)
{
  return -1;
}

int
UPF_Static_Allocator_Base::find (const char *, void *&)
{
  return -1;
}

int
UPF_Static_Allocator_Base::find (const char *)
{
  return -1;
}

int
UPF_Static_Allocator_Base::unbind (const char *)
{
  return -1;
}

int
UPF_Static_Allocator_Base::unbind (const char *, void *&)
{
  return -1;
}

int
UPF_Static_Allocator_Base::sync (ssize_t, int)
{
  return -1;
}

int
UPF_Static_Allocator_Base::sync (void *, size_t, int)
{
  return -1;
}

int
UPF_Static_Allocator_Base::protect (ssize_t, int)
{
  return -1;
}

int
UPF_Static_Allocator_Base::protect (void *, size_t, int)
{
  return -1;
}

#if defined (UPF_HAS_MALLOC_STATS)
void
UPF_Static_Allocator_Base::print_stats (void) const
{
}
#endif /* UPF_HAS_MALLOC_STATS */

void
UPF_Static_Allocator_Base::dump (void) const
{
#if defined (UPF_HAS_DUMP)
  UPF_TRACE ("UPF_Static_Allocator_Base::dump");

  UPF_DEBUG ((LM_DEBUG, UPF_BEGIN_DUMP, this));
  UPF_DEBUG ((LM_DEBUG, UPF_TEXT ("\noffset_ = %d"), this->offset_));
  UPF_DEBUG ((LM_DEBUG, UPF_TEXT ("\nsize_ = %d\n"), this->size_));
  UPF_HEX_DUMP ((LM_DEBUG, this->buffer_, this->size_));
  UPF_DEBUG ((LM_DEBUG, UPF_TEXT ("\n")));

  UPF_DEBUG ((LM_DEBUG, UPF_END_DUMP));
#endif /* UPF_HAS_DUMP */
}