#include "UPF_Bitset_Event_Manager.h"

UPF_Bitset_Event_Manager::UPF_Bitset_Event_Manager():m_handle_count(0)
{
	bitset_event_container_.reserve(256);
}

UPF_Bitset_Event_Manager * UPF_Bitset_Event_Manager::instance()
{
	static UPF_Bitset_Event_Manager bitset_event_mgr;

	return &bitset_event_mgr;
}

UPF_Bitset_Event * UPF_Bitset_Event_Manager::get_bitset_event( int index)
{
	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);
	
	UPF_Bitset_Event * event_ptr = get_class_pointer(index);

	return event_ptr;
	
}
int  UPF_Bitset_Event_Manager::open_bitset_event(const char * name,int * index)
{

	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);

	ContainerType::iterator itr = std::find_if(	bitset_event_container_.begin(),
												bitset_event_container_.end(),
												manager_helper::name_finder(name));

	// 查找到~
	if(itr!=bitset_event_container_.end())
	{
		*index = itr->handle;
		return  OPEN_RET_OK;
	}

	return (OPEN_RET_NO_EXIST);



}
int UPF_Bitset_Event_Manager::create_bitset_event(const char * name)
{
	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);

	ContainerType::iterator itr = std::find_if(	bitset_event_container_.begin(),
												bitset_event_container_.end(),
												manager_helper::name_finder(name));

	// 查找到~
	if(itr!=bitset_event_container_.end())
	{
		return  CRT_RET_EXIST;
	}

	ContainerType::value_type value_info;

	value_info.handle		= ++ m_handle_count;
	value_info.class_ptr	= new UPF_Bitset_Event(name);
	
	bitset_event_container_.push_back(value_info);

	return CRT_RET_OK;
}
int UPF_Bitset_Event_Manager::set_bitset_event(int index,u_int set_option, u_long set_bits,u_long * prev_bits)
{
	UPF_Bitset_Event * event_ptr = 0;

	{
		UPF_Guard<UPF_Thread_Mutex> guard(mutex_);
		
		event_ptr = get_class_pointer(index);
		
		if(event_ptr==0)
			return (-1);
	}
		
	event_ptr->set_event(set_option,set_bits,prev_bits);

	return 0;
}
int UPF_Bitset_Event_Manager::wait_bitset_event(int index,u_int wait_option,u_long wait_bits,u_long * hit_bits ,u_int msec_timeout)
{

	UPF_Bitset_Event * event_ptr = 0;
	
	{
		UPF_Guard<UPF_Thread_Mutex> guard(mutex_);
	
		event_ptr = get_class_pointer(index);
		
		if(event_ptr==0)
			return (-1);
	}


	int ret  = event_ptr->wait_event(wait_option,wait_bits,hit_bits,msec_timeout);

	// -2 代表超时.请参考 UPF_OS::wait_event的说明
	if(ret == (-2))
	{
		return WAIT_RET_TIMEOUT;
	}
	return WAIT_RET_OK;
}

UPF_Bitset_Event *	UPF_Bitset_Event_Manager::get_class_pointer(u_int handle)
{
	using namespace manager_helper;
	ContainerType::iterator itr = std::find_if(bitset_event_container_.begin(),
											   bitset_event_container_.end(),
											   handle_finder(handle));

	if(itr!=bitset_event_container_.end())
	{
		return itr->class_ptr;
	}

	UPF_OS::trace("[UPF-BitEvent] get_class_pointer =0 (handle = %08X)!!!\n",handle);
	return static_cast<UPF_Bitset_Event * >(0);

}	
int UPF_Bitset_Event_Manager::close_bitset_event(int index)
{
	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);

	UPF_Bitset_Event * event_ptr = get_class_pointer(index);

	if(event_ptr!=0)
		return event_ptr->close();
	else
		return	(-1);

}	
int UPF_Bitset_Event_Manager::destroy_bitset_event(const char * name)
{	
	UPF_Guard<UPF_Thread_Mutex> guard(mutex_);


	ContainerType::iterator itr = std::find_if(	bitset_event_container_.begin(),
												bitset_event_container_.end(),
												manager_helper::name_finder(name));

	// 查找到~
	if(itr!=bitset_event_container_.end())
	{
		delete itr->class_ptr;	
		bitset_event_container_.erase(itr);
		return 0;
	}

	return (-1);

}