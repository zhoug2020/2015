// UPF_One_Level_Memory_Manager.h: interface for the UPF_One_Level_Memory_Manager class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_UPF_ONE_LEVEL_MEMORY_MANAGER_H__3D80FDA9_1109_4F49_BE4F_A179FCBA959F__INCLUDED_)
#define AFX_UPF_ONE_LEVEL_MEMORY_MANAGER_H__3D80FDA9_1109_4F49_BE4F_A179FCBA959F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
#include "../UPF_Thread_Mutex.h"

#include <list>
#include <vector>
#include <algorithm>


#define PAGE_SIZE         (0x1000)
#define PAGE_MASK         (~(PAGE_SIZE-1))
#define PAGE_ALIGN(addr)  (((addr)+PAGE_SIZE-1) & PAGE_MASK)


#define MEMORY_BUFFER_SIZE (30*1024*1024)	// 30	M
#define DEFAULT_RANGE_SIZE (512*1024)		// 512  k

#define MAX_BUCKET_COUNT	 (129)			// 512 / 4 +1
#define LAST_BUCKET_INDEX	 (128)			// 
#define STANDARD_BUCKET_SIZE (4*1024)		// one page


struct range_t;
struct bucket_t;
struct memory_block_t;


////////////////////////////////////////////////////////


typedef std::list<memory_block_t>				block_container_type;
typedef block_container_type::iterator			block_iterator;

typedef block_iterator							block_indicator ;  
typedef std::list<block_iterator>				block_indicator_container_type;
typedef std::list<block_iterator>::iterator		block_indicator_iterator;

struct memory_block_t
{
	size_t						addr;
	size_t						size;
	unsigned short				range_idx;
	unsigned short				bucket_idx;	
	block_indicator_iterator	range_block_idctor;
	block_indicator_iterator	bucket_block_idctor;
};


struct range_t
{
	size_t							begin_addr;
	size_t							end_addr;
	block_indicator_container_type	block_indicator_list;
};

struct bucket_t
{
	size_t						size;
	block_indicator_container_type	block_indicator_list;
};

typedef std::vector<range_t>						range_container_type;
typedef std::vector<bucket_t>						bucket_container_type;
typedef range_container_type::iterator				range_iterator;		
typedef bucket_container_type::iterator				bucket_iterator;


////////////////////////////////////////////////////////
namespace partial_utils
{
	static bool is_in_range(size_t addr,const range_t & rg)
	{
		return	( addr >=rg.begin_addr &&  addr < rg.end_addr);
	}
	static size_t	range_index(size_t base_addr,size_t addr,size_t range_size)
	{
		return (addr - base_addr)/range_size;	
	}
	
	static size_t	bucket_index(size_t size,size_t bucket_size)
	{
		if (size > 512*1024)
			return LAST_BUCKET_INDEX;
		return (size/bucket_size)-1;
	}

	static bool		is_standard_bucket(bucket_t & bucket_ref)
	{
		return (bucket_ref.size <= 512*1024);
	}

	template <class T>
	inline T next(T x) { return ++x; }

	template <class T, class Distance>
	inline T next(T x, Distance n)
	{
		std::advance(x, n);
		return x;
	}

	template <class T>
	inline T prev(T x) { return --x; }

	template <class T, class Distance>
	inline T prev(T x, Distance n)
	{
		std::advance(x, -n);
		return x;
	}

	template<class T>
	static void erase(T & container,size_t index)
	{
		T::iterator itr = container.begin();
		partial_utils::next(itr,index);
		container.erase(itr);
	}


	template<class ContainerT ,class FunctorT>
	inline void order_insert(const ContainerT & container,const typename ContainerT::value_type & vaule,FunctorT functor)
	{
		 ContainerT::iterator where_itr = std::lower_bound(container.begin(),container.end(),vaule,functor);
		container.insert(where_itr,vaule);
	}
	template<class ContainerT,class FunctorT>
	inline typename ContainerT::iterator  order_query( ContainerT & container, typename ContainerT::value_type & vaule,FunctorT functor) 
	{
		return std::lower_bound(container.begin(),container.end(),vaule,functor);
	}
	
}
////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
class UPF_One_Level_Memory_Manager  
{
public:
	struct block_indicator_less_finder
	{
		bool operator()( const block_indicator & block_idct1, const block_indicator & block_idct2)
		{
			return (block_idct1->addr < block_idct2->addr);

		}
	};
	//struct block_indicator_equal_finder
	//{
	//	block_indicator_equal_finder(){}
	//	block_indicator_equal_finder(const block_indicator & block_idct):m_block_idct(block_idct){}
	//	bool operator()( const block_indicator & block_idct1, const block_indicator & block_idct2) 
	//	{
	//		if(block_idct1->addr == block_idct2->addr)
	//			return true;
	//		return false;
	//	}
	//	bool operator()( const block_indicator & block_idct) 
	//	{
	//		if(block_idct->addr == m_block_idct->addr)
	//			return true;
	//		return false;
	//	}
	//	block_indicator m_block_idct;
	//};
public:
	UPF_One_Level_Memory_Manager();
	virtual ~UPF_One_Level_Memory_Manager();
	void   init();
	void * allocate(size_t size);
	void   deallocate(void * ptr, size_t size); 
private:
	bucket_iterator  find_right_bucket(size_t size);
	range_iterator	 find_right_range(size_t size);
	
	void			 remove_block(block_iterator & block_itr);
	void			 adjust_block(block_iterator & block_itr);
	void			 insert_block_to_empty_list(memory_block_t & mem_block);
	bucket_iterator  next_not_empty_bucket(bucket_iterator  cur_bucket_itr);
	block_iterator	 get_block_by_indicator(block_indicator_iterator &  cur_indicator,range_iterator & own_range);

	range_iterator	        next_range( range_iterator  &cur_range);
	range_iterator			prev_range( range_iterator  &cur_range);
	bucket_iterator  		next_bucket(bucket_iterator &cur_bucket);
	bucket_iterator  		prev_bucket(bucket_iterator &cur_bucket);

	void					check_blocks();
	void					check_bucket( bucket_t  &cur_bucket);
private:
	void *					m_mem_base_addr;
	range_container_type	m_ranges;
	bucket_container_type	m_buckets;
	block_container_type	m_blocks;
	block_container_type	m_helper_blocks;
	size_t					m_max_range_idx;


};
class time_tester
{
public:
	time_tester(const char * t):title(t){	d1 = ::GetTickCount();}
	~time_tester(){ d2 = ::GetTickCount(); UPF_OS::trace("%s:time = %d ms\n",title,d2-d1);}

	unsigned long d1;
	unsigned long  d2;
	const char * title;
};

class  UPF_One_Level_Memory_Allocator
{
public:
	typedef unsigned int toplevel_allocator_t;

	static void * allocate( size_t size, toplevel_allocator_t type )
	{
//		time_tester t("[UPF_1_M]Alloc:");
		UPF_Guard< UPF_Thread_Mutex > guard( sm_manage_lock );
		return sm_one_level_mem_mgr.allocate(size);
	}
	static void deallocate( void * ptr, size_t size, toplevel_allocator_t type )
	{ 
//			time_tester t("[UPF_1_M]deallocate:");
		UPF_Guard< UPF_Thread_Mutex > guard( sm_manage_lock );
		sm_one_level_mem_mgr.deallocate(ptr,size);
	}
	static void * reallocate( void * ptr, size_t new_size, toplevel_allocator_t type )
	{
		return 0;
	}
static  UPF_Thread_Mutex  sm_manage_lock;
static	UPF_One_Level_Memory_Manager sm_one_level_mem_mgr;
};
#endif 