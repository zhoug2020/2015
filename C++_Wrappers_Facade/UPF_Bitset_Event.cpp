#include "UPF_Bitset_Event.h"

UPF_Bitset_Event::UPF_Bitset_Event():event_pattern_(0),create_pattern_(0)
{
}
UPF_Bitset_Event::UPF_Bitset_Event(const char * name):event_pattern_(0),create_pattern_(0)
{
	memset(m_name,0,MAX_NAME_LEN);
	UPF_OS::strncpy(m_name,name,MAX_NAME_LEN-1);
}
UPF_Bitset_Event::~UPF_Bitset_Event()
{
	this->close();
}


int UPF_Bitset_Event::set_event(u_int set_option, u_long set_bits,u_long * prev_bits)
{	
	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);

	u_int	pattern_filter  = 0x1;
	u_long  pattern_new		= 0;
	u_long  pattern_xor		= 0;
	u_long  pattern_loop_out= 0;
	u_long  pattern_create  = 0;

	u_long	event_pattern	= event_pattern_;


	switch( set_option )
	{
		case SET_OPT_OR:
			pattern_new = event_pattern | set_bits;
			break;
		case SET_OPT_AND:
			pattern_new = event_pattern & set_bits;
			break;
		case SET_OPT_REPLACE:
			pattern_new = set_bits;
			break;
		case SET_OPT_XOR:
			pattern_new = event_pattern ^ set_bits;
			break;

		default: // 默认参数为OR
			pattern_new = event_pattern | set_bits;
	}


	pattern_xor		 = pattern_new  ^ event_pattern;
	pattern_loop_out = pattern_xor;

	pattern_create   = create_pattern_;

	
	for(u_int cnt = 0; (cnt <32)&&(pattern_loop_out != 0); ++cnt,pattern_filter <<= 1)
	{		
		if(( pattern_xor & pattern_filter ) != 0 ) 
		{	
			if(( event_pattern & pattern_filter ) == 0 )
			{			
				
				if(( pattern_create & pattern_filter) == 0 )
				{		
					UPF_OS::event_init(&bitset_event_handle_[cnt].event_handle_,true,true);					
					pattern_create |= pattern_filter;
				}
				else 
				{					
					UPF_OS::event_signal(&bitset_event_handle_[cnt].event_handle_);
				}
			}
			
			else 
			{
				UPF_OS::event_reset(&bitset_event_handle_[cnt].event_handle_);
			}
		}
		
		pattern_loop_out >>=1;		
	}
		
	create_pattern_	= pattern_create;
	event_pattern_	= pattern_new;


	if(prev_bits!=0)
		*prev_bits = event_pattern;

	return 0;

}
 int UPF_Bitset_Event::wait_event(u_int wait_option,u_long wait_bits,u_long * hit_bits,u_int msec_timeout)
{
	 
	
    UPF_event_t  wait_event[32] = {0};  //  TArray better ;wangy
    int          position[32]   = {0}; 
	
	u_int  cnt					= 0; // for loop
	u_int  wait_event_index		= 0;
	u_long hit_pattern		    = 0;
	int    result				= -1; 

	u_int  pattern_filter		= 0x1;
	u_long pattern_create		= 0;
	u_long pattern_temp			= 0;

	mutex_.acquire();
	
	pattern_temp	 = wait_bits;
	pattern_create   = create_pattern_;

	for( cnt = 0; (cnt <32)&&(pattern_temp!=0); ++cnt,pattern_filter<<=1)
	{
		if( (pattern_filter & wait_bits)!=0 )
		{
			bitset_event_handle_[cnt].is_set_ = true;
			
			if(bitset_event_handle_[cnt].event_handle_ ==0)
			{
				UPF_OS::event_init(&bitset_event_handle_[cnt].event_handle_,true);
				pattern_create|=pattern_filter;
			}
			
			wait_event[wait_event_index] = bitset_event_handle_[cnt].event_handle_;
			position[wait_event_index] = cnt;
			++wait_event_index;
		}
	
		pattern_temp >>= 1;	
	}

	create_pattern_	= pattern_create;

	mutex_.release();

	u_int time_out = (-1);

	if(wait_option & WAIT_OPT_TIMEOUT)
		time_out = msec_timeout;


	if(wait_option & WAIT_OPT_OR)
	{	
		u_long  array_index = 0;
		result = UPF_OS::wait_for_single_event_in_array(wait_event_index,wait_event,&array_index,time_out);
	
		if( result==0)
			hit_pattern = 0x00000001 << position[array_index];
	}
	else // if(wait_option & WAIT_OPT_AND)
	{
		result = UPF_OS::wait_for_multiple_events_in_array(wait_event_index,wait_event,time_out);
		if( result==0)
			hit_pattern = wait_bits;
	}

	mutex_.acquire();

	if(hit_bits !=0 )
		*hit_bits = (event_pattern_ & wait_bits)|hit_pattern;

	if( wait_option & WAIT_OPT_RESET) 
	{
		for(cnt = 0; cnt <wait_event_index; ++cnt)
		{
			UPF_OS::event_reset	(&wait_event[cnt]);

			pattern_filter = 0x00000001 << position[cnt];
		
			event_pattern_ &= ~ pattern_filter;
		}
	}

	mutex_.release();

	return result;	
}

unsigned long 
UPF_Bitset_Event::get_event_bits()
{
	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);
	return event_pattern_;

}
	 
unsigned long
UPF_Bitset_Event::get_os_event(u_long event_bit)
{
	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);
	
	u_long idx;for( idx = 0;event_bit!=0;++idx)
	{
		event_bit >>=1;
	}
	
	if(idx >= sizeof(u_long)*8 || idx==0)
		return 0;

	return (unsigned int)(bitset_event_handle_[idx-1].event_handle_);
	
}
int UPF_Bitset_Event::close()
{
	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);

	for(int idx = 0; idx<32;++idx)
	{
		if(bitset_event_handle_[idx].event_handle_!=0)
		{
			UPF_OS::event_destroy(&bitset_event_handle_[idx].event_handle_);
			bitset_event_handle_[idx].event_handle_ = 0;
		}
	}
	
	return 0;
}
