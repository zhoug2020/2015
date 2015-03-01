#ifndef UPF_TSS_T_H_
#define UPF_TSS_T_H_

#include "UPF_Thread_Mutex.h"

template <class TYPE>
class UPF_TSS
{
public:
	
	UPF_TSS (TYPE *ts_obj = 0);
	
	
	virtual ~UPF_TSS (void);
	
	TYPE *ts_object (TYPE *new_ts_obj);
	
	
	TYPE *operator-> () const;
	
	
	operator TYPE *(void) const;
	
	virtual TYPE *make_TSS_TYPE (void) const;
	
	void dump (void) const;
	
protected:
	
	TYPE *ts_get (void) const;
	
	int ts_init (void);
	
	UPF_Thread_Mutex keylock_;
	
	volatile int once_;
	
	UPF_thread_key_t key_;
	
	static void cleanup (void *ptr);

	
	UPF_UNIMPLEMENTED_FUNC (void operator= (const UPF_TSS<TYPE> &))
	UPF_UNIMPLEMENTED_FUNC (UPF_TSS (const UPF_TSS<TYPE> &))
		
		
};

template <class TYPE>
class UPF_TSS_Type_Adapter
{
public:
	UPF_TSS_Type_Adapter (const TYPE value = 0): value_ (value) {}
	operator TYPE () const { return value_; };
	operator TYPE &() { return value_; };
	
private:
	TYPE value_;
};

///////////////////////////////////////////////////////////////////////////


#endif