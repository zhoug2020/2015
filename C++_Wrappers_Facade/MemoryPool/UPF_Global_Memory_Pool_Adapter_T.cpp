/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Global_Memory_Pool_Adapter_T.cpp                                                       *
*       CREATE DATE     : 2008-7-18                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]用于实现UPF_IMemory_Pool和全局内存池的一个adapter![CN]                                 *
*********************************************************************************************************************/
#include "UPF_Global_Memory_Pool_Adapter_T.h"

// Constant definitions.

// type desc

const  char * const GLOBAL_MEMORY_POOL_ADAPTER_TYPE = "Global";

/////////////////////////////////////////////////////////////////////////////

template < class Allocator, class Lock, class RecordPolicy >
UPF_Global_Memory_Pool_Adapter_T< Allocator, Lock, RecordPolicy >::
UPF_Global_Memory_Pool_Adapter_T(
     const char *          memory_pool_name,
     size_t                allocated_memory_limit,
     toplevel_allocator_t  allocator_type )

    : UPF_Memory_Pool_Base< Allocator, Lock, RecordPolicy>( memory_pool_name,
                                                            allocated_memory_limit, 
                                                            GLOBAL_MEMORY_POOL_ADAPTER_TYPE,
                                                            allocator_type )
{
    this->reset_counter();

    m_memory_pool    = UPF_Global_Memory_Pool_Creator::get_instance();

    m_recorder.set_memory_pool_id( this->m_memory_pool_id );
}


/////////////////////////////////////////////////////////////////////////////

template < class Allocator, class Lock, class RecordPolicy >
UPF_Global_Memory_Pool_Adapter_T< Allocator, Lock, RecordPolicy >::~UPF_Global_Memory_Pool_Adapter_T( void )
{
    UPF_Global_Memory_Pool_Creator::release_instance();
}


/////////////////////////////////////////////////////////////////////////////

template < class Allocator, class Lock, class RecordPolicy >
void*
UPF_Global_Memory_Pool_Adapter_T< Allocator, Lock, RecordPolicy >::allocate( size_t size )
{    
    void * result = m_memory_pool->allocate( size, &m_recorder );

#if 0
    MESSAGE_ASSERT( "Allocated memory size has exceeded the allocated memory limit!", 
                    m_recorder.get_allocated_memory_size() < 
                    m_recorder.get_allocated_memory_limit() );
#else
    if ( m_recorder.get_allocated_memory_size() >=
         m_recorder.get_allocated_memory_limit() )
    {
        UPF_Memory_Pool_Util::trace( 
            "[%s] Allocated memory size(%d) has exceeded the allocated memory limit(%d)!\n",
            this->get_memory_pool_info()->get_memory_pool_name(),
            m_recorder.get_allocated_memory_size(),
            m_recorder.get_allocated_memory_limit() );
    }
#endif

    return result;
}


/////////////////////////////////////////////////////////////////////////////

template < class Allocator, class Lock, class RecordPolicy > 
void
UPF_Global_Memory_Pool_Adapter_T< Allocator, Lock, RecordPolicy >::deallocate( void* ptr, size_t size )
{
    m_memory_pool->deallocate( ptr, size, &m_recorder );
}


/////////////////////////////////////////////////////////////////////////////

template < class Allocator, class Lock, class RecordPolicy >
void * 
UPF_Global_Memory_Pool_Adapter_T< Allocator, Lock, RecordPolicy >::reallocate( 
    void * ptr, size_t new_size )
{
    void * result = m_memory_pool->reallocate( ptr, new_size, &m_recorder );

#if 0
    MESSAGE_ASSERT( "Allocated memory size has exceeded the allocated memory limit!", 
                    m_recorder.get_allocated_memory_size() < 
                    m_recorder.get_allocated_memory_limit() );
#else
    if ( m_recorder.get_allocated_memory_size() >=
         m_recorder.get_allocated_memory_limit() )
    {
        UPF_Memory_Pool_Util::trace( 
            "[%s] Allocated memory size(%d) has exceeded the allocated memory limit(%d)!\n",
            this->get_memory_pool_info()->get_memory_pool_name(),
            m_recorder.get_allocated_memory_size(),
            m_recorder.get_allocated_memory_limit() );
    }
#endif

    return result;
}
