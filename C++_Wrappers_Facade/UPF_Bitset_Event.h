#ifndef UPF_BITSET_EVENT_H_
#define UPF_BITSET_EVENT_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "UPF_Thread_Mutex.h"

#define		MAX_NAME_LEN 16

/////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////
// bitset_event
class  UPF_Export bitset_event
{
public:
	bitset_event():is_set_(true),event_handle_(0){}

	bool						is_set_;
	UPF_event_t					event_handle_;	
};

/////////////////////////////////////////////////////////////////////////
// UPF_Bitset_Event
class UPF_Export UPF_Bitset_Event  
{
friend class UPF_Bitset_Event_Manager;

public:

	enum SetOption
	{	
		SET_OPT_OR		= 0x0,
	    SET_OPT_AND		= 0x1,
		SET_OPT_REPLACE	= 0x2, 
		SET_OPT_XOR	    = 0x3
	};


	enum WaitOption
	{    		
		WAIT_OPT_AND		= 0x0,
		WAIT_OPT_TIMEOUT	= 0x1,
		WAIT_OPT_OR			= 0x2,
		WAIT_OPT_RESET		= 0x4 
	};

	UPF_Bitset_Event();
	UPF_Bitset_Event(const char * name);
	~UPF_Bitset_Event();	

	 int set_event(u_int set_option, u_long set_bits,u_long * prev_bits = 0);
	 int wait_event(u_int wait_option,u_long wait_bits,u_long * hit_bits = 0,u_int msec_timeout = -1);
	 unsigned long get_event_bits();
	 unsigned long get_os_event(u_long event_bit);
	 int close();
public:
	 char				m_name[MAX_NAME_LEN];
 	 bitset_event		bitset_event_handle_[32];
	 unsigned int		event_pattern_;
	 unsigned int		create_pattern_;
	 UPF_Thread_Mutex	mutex_;
};



#endif
