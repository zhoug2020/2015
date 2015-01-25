// UPF_One_Level_Memory_Manager.cpp: implementation of the UPF_One_Level_Memory_Manager class.
//
//////////////////////////////////////////////////////////////////////

//#include "stdafx.h"
#include "UPF_One_Level_Memory_Manager.h"
#include <windows.h>


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
//#define PAGE_SIZE		(0x1000)
//#define PAGE_ALIGN(Va)  ((ULONG)(PAGE_SIZE+Va-1) & ~(Va - 1))
UPF_One_Level_Memory_Manager UPF_One_Level_Memory_Allocator::sm_one_level_mem_mgr;
UPF_Thread_Mutex	UPF_One_Level_Memory_Allocator::sm_manage_lock;

UPF_One_Level_Memory_Manager::UPF_One_Level_Memory_Manager():m_mem_base_addr(0)
{
	init();
}

UPF_One_Level_Memory_Manager::~UPF_One_Level_Memory_Manager()
{

}
void   UPF_One_Level_Memory_Manager::init()
{
	using namespace partial_utils;
	m_mem_base_addr = VirtualAlloc(0,MEMORY_BUFFER_SIZE,MEM_RESERVE, PAGE_NOACCESS);
	m_max_range_idx = (MEMORY_BUFFER_SIZE/DEFAULT_RANGE_SIZE) -1 ;
	

	// init ranges
	m_ranges.reserve(m_max_range_idx+1);
	for(size_t i= 0; i< (m_max_range_idx +1);++i)
	{
		range_t rg;
		rg.begin_addr	= (size_t)m_mem_base_addr +i*DEFAULT_RANGE_SIZE; 	
		rg.end_addr		= rg.begin_addr + DEFAULT_RANGE_SIZE;
		m_ranges.push_back(rg);
	}


	// init buckets
	m_buckets.reserve(512/4 + 1);
	for(size_t i= 0; i< (MAX_BUCKET_COUNT);++i)
	{
		bucket_t bucket_item;
		bucket_item.size = (i+1)*4*1024;
		m_buckets.push_back(bucket_item);
	}



	memory_block_t	first_block;
	first_block.addr = 0;
	first_block.size = 0;
	m_blocks.push_back(first_block);

	m_blocks.front().addr			= (size_t)m_mem_base_addr;
	m_blocks.front().size			= MEMORY_BUFFER_SIZE;

	
	m_ranges.front().block_indicator_list.push_back(m_blocks.begin());
	m_buckets[LAST_BUCKET_INDEX].block_indicator_list.push_back(m_blocks.begin());
	m_blocks.front().range_idx		= 0;
	m_blocks.front().bucket_idx     = LAST_BUCKET_INDEX;

	m_blocks.front().range_block_idctor		=	m_ranges.front().block_indicator_list.begin();
	m_blocks.front().bucket_block_idctor	=	m_buckets[LAST_BUCKET_INDEX].block_indicator_list.begin();

}

void * 
UPF_One_Level_Memory_Manager::allocate(size_t size)
{
	size_t return_addr = 0;
	size_t align_size = PAGE_ALIGN(size);

	bucket_iterator right_bucket = find_right_bucket(align_size);

	if(right_bucket != m_buckets.end())
	{
		// standard bucket
		if(partial_utils::is_standard_bucket(*right_bucket))
		{
	
			right_bucket = this->next_not_empty_bucket(right_bucket);
			if(right_bucket == m_buckets.end())
			{
				//__asm int 3
				return 0;
			}

			block_iterator block_itr = right_bucket->block_indicator_list.front();
			return_addr = block_itr->addr;

	//		VirtualAlloc((void *)return_addr,align_size,MEM_RESERVE, PAGE_NOACCESS);
			if(VirtualAlloc((void *)return_addr,align_size,MEM_COMMIT,PAGE_READWRITE)==0)
			{
				//__asm int 3
				return 0;
			}

			if(block_itr->size > align_size)
			{
				block_itr->addr += align_size;
				block_itr->size -= align_size;
				this->adjust_block(block_itr);
			}
			else
				remove_block(block_itr);		
		}
	else
		{

			block_iterator   block_itr;
			block_indicator_iterator   block_idctor_itr = m_buckets[LAST_BUCKET_INDEX].block_indicator_list.begin();
			
			for (;block_idctor_itr != m_buckets[LAST_BUCKET_INDEX].block_indicator_list.end();++block_idctor_itr)
			{
				block_itr = (*block_idctor_itr);
				if(block_itr->size >= align_size)
						break;		
			}

			if(block_idctor_itr == m_buckets[LAST_BUCKET_INDEX].block_indicator_list.end())
			{
				//__asm int 3
				return 0;

			}
			
			return_addr = block_itr->addr;
//			VirtualAlloc((void *)return_addr,align_size,MEM_RESERVE, PAGE_NOACCESS);
			if(VirtualAlloc((void *)return_addr,align_size,MEM_COMMIT,PAGE_READWRITE)==0)
			{
					return 0;
			}

			if ((block_itr->addr - (size_t)m_mem_base_addr) > MEMORY_BUFFER_SIZE )
			{
				//__asm int 3
				return 0;
			}
			if ( block_itr->size < align_size)
			{				
				//__asm int 3
				return 0;
			}
			
			block_itr->addr += align_size;
			block_itr->size -= align_size;
			
			if (block_itr->size!=0)
			{
				adjust_block(block_itr);
			}else
			{
				remove_block(block_itr);	
			}
		
		
		}
	}
	if(m_blocks.empty())
	{
		//__asm int 3
		return 0;
	}
	return (void *)return_addr;
}
void   
UPF_One_Level_Memory_Manager::deallocate(void * ptr, size_t size)
{
	memory_block_t	memory_block;
	memory_block.addr = (size_t)ptr;
	memory_block.size = PAGE_ALIGN(size);

//	VirtualFree(ptr,0,MEM_RELEASE);
	VirtualFree(ptr,PAGE_ALIGN(size),MEM_DECOMMIT);
	
	if(m_blocks.empty())
	{
		insert_block_to_empty_list(memory_block);
		return ;
	}

	m_helper_blocks.clear();
	m_helper_blocks.push_back(memory_block);
	range_iterator range = find_right_range(memory_block.addr);

	block_indicator_iterator where_block_idct_itr = 
		partial_utils::order_query(
		range->block_indicator_list,m_helper_blocks.begin(),
		block_indicator_less_finder());

	block_iterator where_block_itr = get_block_by_indicator(where_block_idct_itr,range);
	
	block_iterator block_itr = (this->m_blocks.insert(where_block_itr,memory_block));
	//check_blocks();
	bool is_merge_prev	= false;
	bool is_merge_next	= false;
	block_iterator cur_block_itr = block_itr;

	if (cur_block_itr!=m_blocks.begin())
	{
		block_iterator prev_block_itr = partial_utils::prev(cur_block_itr);
		if (cur_block_itr->addr ==(prev_block_itr->addr+prev_block_itr->size))
		{
			prev_block_itr->size += cur_block_itr->size;
			m_blocks.erase(cur_block_itr);
			is_merge_prev  = true;
			cur_block_itr  = prev_block_itr;
			
		}	
	}
	if (cur_block_itr!=m_blocks.end())
	{	
		block_iterator next_block_itr = partial_utils::next(cur_block_itr);
		if (next_block_itr->addr ==(cur_block_itr->addr+cur_block_itr->size) )
		{

			next_block_itr->addr = cur_block_itr->addr;
			next_block_itr->size += cur_block_itr->size;
			if(!is_merge_prev)
			{
				m_blocks.erase(cur_block_itr);
				
			}else
			{
				remove_block(cur_block_itr);
			}

			is_merge_next = true;
			cur_block_itr  = next_block_itr;
		}

	}

	if(!(is_merge_prev||is_merge_next))
	{
		memory_block_t & block = (*block_itr);

		block_itr->range_block_idctor = range->block_indicator_list.insert(where_block_idct_itr,block_itr);
		block_itr->range_idx		= partial_utils::range_index((size_t)m_mem_base_addr,block.addr,DEFAULT_RANGE_SIZE);
		
		bucket_iterator bucket = find_right_bucket(block.size);
		if ( bucket->size!=block_itr->size &&  bucket->size <=512*1024)
		{
			//__asm int 3
			return;
		}


		bucket->block_indicator_list.push_front(block_itr);
		//check_bucket(*bucket);
		block_itr->bucket_block_idctor	= bucket->block_indicator_list.begin();
		block_itr->bucket_idx			= partial_utils::bucket_index(block.size,STANDARD_BUCKET_SIZE);
	//	this->adjust_block(block_itr);
	}else
	{
		this->adjust_block(cur_block_itr);
	}

}	

bucket_iterator  
UPF_One_Level_Memory_Manager::find_right_bucket(size_t size)
{

	bucket_iterator itr = m_buckets.end();
	size_t bucket_idx	= 0;

	if (size > 512*1024)
		bucket_idx =  LAST_BUCKET_INDEX;
	else
	bucket_idx =  (size/STANDARD_BUCKET_SIZE)-1;

	if( bucket_idx < m_buckets.size())
	{
		itr = m_buckets.begin();
		std::advance(itr,bucket_idx);
	}

	return itr;

}
range_iterator	 
UPF_One_Level_Memory_Manager::find_right_range(size_t addr)
{
	range_iterator	itr  = m_ranges.end();
	size_t range_idx  = ((addr-(size_t)m_mem_base_addr)/ DEFAULT_RANGE_SIZE);
	
	if (range_idx < m_ranges.size())
	{
		itr = m_ranges.begin();
		std::advance(itr,range_idx);
	
	}
	return itr;

}


void					
UPF_One_Level_Memory_Manager::remove_block(block_iterator  & block_itr)
{

	m_ranges[block_itr->range_idx].block_indicator_list.erase(block_itr->range_block_idctor);
	m_buckets[block_itr->bucket_idx].block_indicator_list.erase(block_itr->bucket_block_idctor);
	m_blocks.erase(block_itr);

}


void	
UPF_One_Level_Memory_Manager::adjust_block(block_iterator & block_itr)
{
	size_t old_range_idx = block_itr->range_idx;
	size_t cur_range_idx = partial_utils::range_index((size_t)m_mem_base_addr,block_itr->addr,DEFAULT_RANGE_SIZE);

	if (old_range_idx!=cur_range_idx)
	{
		m_helper_blocks.clear();
		m_helper_blocks.push_back(*block_itr);	

		m_ranges[old_range_idx].block_indicator_list.erase(block_itr->range_block_idctor);

		block_indicator_iterator where_block_idct_itr  = 
			partial_utils::order_query(
			m_ranges[cur_range_idx].block_indicator_list,
			m_helper_blocks.begin(),
			block_indicator_less_finder());

		block_itr->range_idx			= cur_range_idx;
		block_itr->range_block_idctor	= m_ranges[cur_range_idx].block_indicator_list.insert(where_block_idct_itr,block_itr);
		
	
	}

	size_t old_bucket_idx = block_itr->bucket_idx;
	size_t cur_bucket_idx = partial_utils::bucket_index(block_itr->size,STANDARD_BUCKET_SIZE);

	if (old_bucket_idx!=cur_bucket_idx)
	{
		if ( m_buckets[cur_bucket_idx].size!=block_itr->size && m_buckets[cur_bucket_idx].size <=512*1024)
		{
			//__asm int 3
			return ;
		}
		m_buckets[old_bucket_idx].block_indicator_list.erase(block_itr->bucket_block_idctor);
		block_itr->bucket_idx = cur_bucket_idx;
		block_itr->bucket_block_idctor = m_buckets[cur_bucket_idx].block_indicator_list.insert(m_buckets[cur_bucket_idx].block_indicator_list.begin(),block_itr);
		//check_bucket(m_buckets[cur_bucket_idx]);
	
	}
}

void			 
UPF_One_Level_Memory_Manager::insert_block_to_empty_list(memory_block_t & mem_block)
{
	// assert m_blocks.empty !!
	// assert range->block_indicator_list.empty !!
	// assert bucket->block_indicator_list.empty !!

	m_blocks.push_back(mem_block);
	//check_blocks();

	block_iterator & block_itr  = m_blocks.begin();;
	
	range_iterator range = find_right_range(block_itr->addr);
	range->block_indicator_list.push_front(block_itr);

	block_itr->range_idx = partial_utils::range_index((size_t)m_mem_base_addr,block_itr->addr,DEFAULT_RANGE_SIZE);
	block_itr->range_block_idctor = range->block_indicator_list.begin();

	bucket_iterator bucket = find_right_bucket(block_itr->size);
	bucket->block_indicator_list.push_front(block_itr);
	//check_bucket(*bucket);
	block_itr->bucket_idx		= partial_utils::bucket_index(block_itr->size,STANDARD_BUCKET_SIZE);
	block_itr->bucket_block_idctor = bucket->block_indicator_list.begin();

}
bucket_iterator  
UPF_One_Level_Memory_Manager::next_not_empty_bucket(bucket_iterator  cur_bucket_itr)
{
	while(cur_bucket_itr!=this->m_buckets.end())
	{
		if(!cur_bucket_itr->block_indicator_list.empty())
		{	
			if(cur_bucket_itr->block_indicator_list.size()==0)
			{
				//__asm int 3
			}
			break;
		}
		++ cur_bucket_itr;
	}

	return cur_bucket_itr;
}

block_iterator	 
UPF_One_Level_Memory_Manager::get_block_by_indicator(block_indicator_iterator & cur_indicator,range_iterator  & own_range)
{

	block_iterator where_block_itr;
	/*if (cur_indicator == own_range->block_indicator_list.end() && !own_range->block_indicator_list.empty())
	{
			where_block_itr = own_range->block_indicator_list.back();
			return partial_utils::next(where_block_itr);
	}*/
	if (cur_indicator == own_range->block_indicator_list.end())
	{			
		range_iterator prev_range  = own_range;
		range_iterator next_range  = own_range;
		while (1)
		{
			if (!prev_range->block_indicator_list.empty()) 
			{
				where_block_itr  = prev_range->block_indicator_list.back();
				where_block_itr  = partial_utils::next(where_block_itr);
				break;
			}
			if(!next_range->block_indicator_list.empty() )
			{
				where_block_itr  = next_range->block_indicator_list.front();
				break;
			}


			if(prev_range!=this->m_ranges.begin())
				prev_range = this->prev_range(prev_range);
			if(next_range!=this->m_ranges.end())
				next_range = this->next_range(next_range);

		}
	}else
	{
		where_block_itr  = (*cur_indicator);
	}

	return where_block_itr;
}

range_iterator	        
UPF_One_Level_Memory_Manager::next_range( range_iterator  & cur_range)
{
	return partial_utils::next(cur_range);
}

range_iterator			 
UPF_One_Level_Memory_Manager::prev_range( range_iterator & cur_range)
{
	return partial_utils::prev(cur_range);
}

bucket_iterator  		
UPF_One_Level_Memory_Manager:: next_bucket(bucket_iterator & cur_bucket)
{
	return partial_utils::next(cur_bucket);
}

bucket_iterator  		 
UPF_One_Level_Memory_Manager::prev_bucket(bucket_iterator &cur_bucket)
{
		return partial_utils::prev(cur_bucket);
}	
void					
UPF_One_Level_Memory_Manager::check_blocks()
{
	block_iterator itr		=  m_blocks.begin();
	block_iterator itr_next =  partial_utils::next(itr);

	size_t size		= itr->size;
	for (;itr!=m_blocks.end();++itr,itr_next =  partial_utils::next(itr))
	{	
		if (itr_next == m_blocks.end())
		{
			return;
		}
		if(itr->addr > itr_next->addr)
		{
			//__asm int 3
		}

	}


}

void					
UPF_One_Level_Memory_Manager::check_bucket( bucket_t  & cur_bucket)
{
	if (cur_bucket.size >512*1024)
	{
		return ;
	}
	block_indicator_iterator itr = cur_bucket.block_indicator_list.begin();
	for (;itr!=cur_bucket.block_indicator_list.end();++itr)
	{
		if ((*itr)->size!= cur_bucket.size)
		{
			//__asm int 3
		}
	}

}