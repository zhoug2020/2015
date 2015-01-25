#ifndef UPF_MALLOC_ALLOCATOR_H_
#define UPF_MALLOC_ALLOCATOR_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "Malloc_Base.h"

class /*UPF_Export*/ UPF_New_Allocator : public UPF_Allocator
{
public:
  /// These methods are defined.
  virtual void *malloc (size_t nbytes);
  virtual void *calloc (size_t nbytes, char initial_value = '\0');
  virtual void *calloc (size_t n_elem, size_t elem_size, char initial_value = '\0');
  virtual void free (void *ptr);

  /// These methods are no-ops.
  virtual int remove (void);
  virtual int bind (const char *name, void *pointer, int duplicates = 0);
  virtual int trybind (const char *name, void *&pointer);
  virtual int find (const char *name, void *&pointer);
  virtual int find (const char *name);
  virtual int unbind (const char *name);
  virtual int unbind (const char *name, void *&pointer);
  virtual int sync (ssize_t len = -1, int flags = MS_SYNC);
  virtual int sync (void *addr, size_t len, int flags = MS_SYNC);
  virtual int protect (ssize_t len = -1, int prot = PROT_RDWR);
  virtual int protect (void *addr, size_t len, int prot = PROT_RDWR);

  virtual void dump (void) const;

private:
  // DO NOT ADD ANY STATE (DATA MEMBERS) TO THIS CLASS!!!!  See the
  // <UPF_Allocator::instance> implementation for explanation.
};

class /*UPF_Export*/ UPF_Static_Allocator_Base : public UPF_Allocator
{
public:
  UPF_Static_Allocator_Base (char *buffer, size_t size);
  virtual void *malloc (size_t nbytes);
  virtual void *calloc (size_t nbytes, char initial_value = '\0');
  virtual void *calloc (size_t n_elem, size_t elem_size, char initial_value = '\0');
  virtual void free (void *ptr);
  virtual int remove (void);
  virtual int bind (const char *name, void *pointer, int duplicates = 0);
  virtual int trybind (const char *name, void *&pointer);
  virtual int find (const char *name, void *&pointer);
  virtual int find (const char *name);
  virtual int unbind (const char *name);
  virtual int unbind (const char *name, void *&pointer);
  virtual int sync (ssize_t len = -1, int flags = MS_SYNC);
  virtual int sync (void *addr, size_t len, int flags = MS_SYNC);
  virtual int protect (ssize_t len = -1, int prot = PROT_RDWR);
  virtual int protect (void *addr, size_t len, int prot = PROT_RDWR);
#if defined (UPF_HAS_MALLOC_STATS)
  virtual void print_stats (void) const;
#endif /* UPF_HAS_MALLOC_STATS */
  virtual void dump (void) const;

protected:
  /// Don't allow direct instantiations of this class.
  UPF_Static_Allocator_Base (void);

  /// Pointer to the buffer.
  char *buffer_;

  /// Size of the buffer.
  size_t size_;

  /// Pointer to the current offset in the <buffer_>.
  size_t offset_;
};


#endif