#ifndef UPF_BITSET_EVENT_MANAGER_H_
#define UPF_BITSET_EVENT_MANAGER_H_

#include "..\OS_Adapter\UPF_OS.h"
#include "UPF_Bitset_Event.h"
#include "UPF_Thread_Mutex.h"

#include <vector>
#include <algorithm>
/////////////////////////////////////////////////////////////////////////
// UPF_Bitset_Event_Manager
namespace manager_helper
{
/**
 * @class Managerd的句柄管理的包装类.
 */
template<typename T>
struct HandleT
{
	u_int				 handle;	
	T * 				 class_ptr;	
};

/** 
 * @class name_finder
 * @brief 在Manager中按名称查找 
 *  
 * @remarks 
 * 			一元仿函数.(也可以考虑做成二元仿函数,则需要用bind族的仿函数适配器进行适配)
 */
struct name_finder
{
	name_finder(const char * name):target_name(name){}

	template<typename T>
	bool operator()(const HandleT<T> &handle_ref )const
	{
		return UPF_OS::strncmp(target_name,handle_ref.class_ptr->m_name,MAX_NAME_LEN-1)==0 ?true:false;
	}

	const char * target_name;
};
/** 
 * @class name_finder
 * @brief 在Manager中按handle查找
 *  
 * @remarks 
 *           一元仿函数.(也可以考虑做成二元仿函数,则需要用bind族的仿函数适配器进行适配)
 *  
 */
struct handle_finder
{
	handle_finder(u_int handle):m_handle(handle){
	}
	template<typename T>
	bool operator()(const HandleT<T> &handle_ref )const
	{
		return m_handle==handle_ref.handle?true:false;
	}

	u_int m_handle;
};

}

///////////////

typedef manager_helper::HandleT<UPF_Bitset_Event>	EventInfo;

class UPF_Export UPF_Bitset_Event_Manager
{
public:
static	UPF_Bitset_Event_Manager * instance();


	enum CreateResult
	{	
		CRT_RET_OK		= 0x0,
	    CRT_RET_EXIST	= 0x9,
	};
	enum OpenResult
	{	
		OPEN_RET_OK		  = 0x0,
	    OPEN_RET_NO_EXIST = 0xA,
	};
	enum WaitResult
	{
		WAIT_RET_OK 		= 0x0,
		WAIT_RET_TIMEOUT	= 0x10
	};


	typedef std::vector<EventInfo > ContainerType  ;

	UPF_Bitset_Event * get_bitset_event(int index);

	int open_bitset_event(const char * name,int * index);
	int create_bitset_event(const char * name);
	int set_bitset_event(int index, u_int set_option, u_long set_bits,u_long * prev_bits = 0);
	int wait_bitset_event(int index,u_int wait_option,u_long wait_bits,u_long * hit_bits = 0,u_int msec_timeout = -1);
	int close_bitset_event(int index);
	int destroy_bitset_event(const char * name);

	void dump(); 
private:	
	UPF_Bitset_Event_Manager();
	UPF_Bitset_Event *	get_class_pointer(u_int handle);

	UPF_Thread_Mutex	set_mutex_;
	UPF_Thread_Mutex	wait_mutex_;
	UPF_Thread_Mutex	mutex_;
	
	ContainerType		bitset_event_container_;  // custom alloctor wangy;	
												  // 最后不要忘记释放
	u_int				m_handle_count;
};




#endif