/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Heap_Alloc_T.cpp                                                           *
*       CREATE DATE     : 2008-4-14                                                                                  *
*       MODULE          : MemoryPool                                                                                 *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]使用HeapAlloc系列函数分配内存.![CN]                                                    *
*********************************************************************************************************************/
#include "UPF_Memory_Pool_Heap_Alloc_T.h"
#include "UPF_Memory_Pool_Util.h"
#include <assert.h>

// constant definitions.
const  char * const HEAP_ALLOC_MEMORY_POOL_TYPE = "HeapAlloc";

// static member definitions.
template < class Allocator, class Lock, class RecordPolicy >
HANDLE 
UPF_Memory_Pool_Heap_Alloc_T< Allocator, Lock, RecordPolicy >::sm_pool_heap = NULL;

template < class Allocator, class Lock, class RecordPolicy >
UPF_Thread_Mutex  
UPF_Memory_Pool_Heap_Alloc_T< Allocator, Lock, RecordPolicy >::sm_pool_heap_lock;


/** 
 * 初始化内部状态.
 * 
 * @param memory_pool_name          [in]  内存池名称.
 * @param allocated_memory_limit    [in]  Client端分配内存的上限值.
 */
template < class Allocator, class Lock, class RecordPolicy >
UPF_Memory_Pool_Heap_Alloc_T<
    Allocator, Lock, RecordPolicy >::UPF_Memory_Pool_Heap_Alloc_T( 
        const char *          memory_pool_name,
        size_t                allocated_memory_limit,
        toplevel_allocator_t  allocator_type )

    : UPF_Memory_Pool_Base< Allocator, Lock, RecordPolicy>( memory_pool_name,
                                                            allocated_memory_limit,
                                                            HEAP_ALLOC_MEMORY_POOL_TYPE,
                                                            allocator_type
                                                           )
{

    if ( NULL == sm_pool_heap )
    {
        UPF_Guard< Lock > guard( sm_pool_heap_lock );

        if ( NULL == sm_pool_heap )
        {
            sm_pool_heap = HeapCreate( 0, 1024 * 1024, 0 );

            if ( NULL == sm_pool_heap )
            {
                UPF_Memory_Pool_Util::report( "HeapCreate failed!" );
            }
        }
    }

    this->reset_counter();
}

/** 
 * 分配内存.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
void *
UPF_Memory_Pool_Heap_Alloc_T< 
    Allocator, Lock, RecordPolicy >::allocate( size_t size )
{
    assert( sm_pool_heap != NULL );

    void * result = 0;

    UPF_Guard< Lock > guard( this->m_lock );

    size_t request_size = HEAP_ALLOC_BLOCK_NODE_SIZE + size;


    Heap_Alloc_Block_Node * block_node_ptr = 
        static_cast< Heap_Alloc_Block_Node * >(
            HeapAlloc( sm_pool_heap, 0, request_size ) );


    if ( 0 != block_node_ptr )
    {
        block_node_ptr->id_alloc_size = 
            UPF_MMORY_POOL_COMBINE_ID_SIZE( this->m_memory_pool_id, size );

        m_recorder.set_managed_memory_size(
            m_recorder.get_managed_memory_size() + request_size );

        m_recorder.set_allocated_memory_size(
            m_recorder.get_allocated_memory_size() + size );

        m_recorder.set_wasted_memory_size(
            m_recorder.get_wasted_memory_size() + HEAP_ALLOC_BLOCK_NODE_SIZE );

        result = reinterpret_cast< char * >( block_node_ptr ) + 
            HEAP_ALLOC_BLOCK_NODE_SIZE;
    }

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

/** 
 * 重新分配内存.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
void *
UPF_Memory_Pool_Heap_Alloc_T< 
    Allocator, Lock, RecordPolicy >::reallocate( void * ptr,
                                                 size_t new_size )
{
    assert( sm_pool_heap != NULL );

    // if ptr is 0, then call allocate to alloc memory.
    if ( 0 == ptr )
    {
        return this->allocate( new_size );
    }

    // if ptr isn't 0 and new_size is 0, then call deallocate to free memory
    // and return 0
    if ( 0 == new_size )
    {
        this->deallocate( ptr, 0 );
        return 0;
    }

    UPF_Guard< Lock > guard( this->m_lock );

    void * result = 0;

    Heap_Alloc_Block_Node * block_node_ptr = 
        reinterpret_cast< Heap_Alloc_Block_Node * >(
            static_cast< char * >( ptr ) - HEAP_ALLOC_BLOCK_NODE_SIZE );

    size_t old_alloc_size = UPF_MEMORY_POOL_ALLOC_SIZE(
                                block_node_ptr->id_alloc_size );

    Heap_Alloc_Block_Node * new_block_node_ptr = 
        static_cast< Heap_Alloc_Block_Node * >(
            HeapReAlloc( sm_pool_heap, 
                         0, 
                         block_node_ptr,
                         HEAP_ALLOC_BLOCK_NODE_SIZE + new_size ) );

    if ( 0 != new_block_node_ptr )
    {
        new_block_node_ptr->id_alloc_size = 
            UPF_MMORY_POOL_COMBINE_ID_SIZE( this->m_memory_pool_id, new_size );        

        m_recorder.set_managed_memory_size(
            m_recorder.get_managed_memory_size() - old_alloc_size + new_size );

        m_recorder.set_allocated_memory_size(
            m_recorder.get_allocated_memory_size() - old_alloc_size + new_size );

        result = reinterpret_cast< char * >( new_block_node_ptr ) + 
            HEAP_ALLOC_BLOCK_NODE_SIZE;
    }

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

/** 
 * 释放内存.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
void 
UPF_Memory_Pool_Heap_Alloc_T< 
    Allocator, Lock, RecordPolicy >::deallocate( void * ptr, 
                                                 size_t /* size */ )
{
    assert( sm_pool_heap != NULL );

    UPF_Guard< Lock > guard( this->m_lock );

    if ( 0 != ptr )
    {
        Heap_Alloc_Block_Node * block_node_ptr = 
            reinterpret_cast< Heap_Alloc_Block_Node * >(
                static_cast< char * >( ptr ) - HEAP_ALLOC_BLOCK_NODE_SIZE );

        size_t alloc_size = UPF_MEMORY_POOL_ALLOC_SIZE(
                                block_node_ptr->id_alloc_size );

        HeapFree( sm_pool_heap, 0, block_node_ptr );

        m_recorder.set_managed_memory_size(
            m_recorder.get_managed_memory_size() - alloc_size - 
            HEAP_ALLOC_BLOCK_NODE_SIZE );

        m_recorder.set_allocated_memory_size(
            m_recorder.get_allocated_memory_size() - alloc_size );

        m_recorder.set_wasted_memory_size(
            m_recorder.get_wasted_memory_size() - HEAP_ALLOC_BLOCK_NODE_SIZE );
    }

    return;
}


