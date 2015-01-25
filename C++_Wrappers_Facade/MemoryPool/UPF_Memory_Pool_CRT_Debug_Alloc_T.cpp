/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_CRT_Debug_Alloc_T.cpp                                                      *
*       CREATE DATE     : 2008-1-9                                                                                   *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]使用CRT Debug堆来管理内存的UPF_IMemory_Pool的实现![CN]                                 *
*********************************************************************************************************************/
#include <crtdbg.h>    /* for malloc_dbg, 
                              realloc_dbg,
                              free_dbg */

#include "UPF_Memory_Pool_CRT_Debug_Alloc_T.h"

const  char * const CRT_DEBUG_ALLOC_MEMORY_POOL_TYPE = "CRTDebugAlloc";

/** 
 * 初始化crt debug堆的一些标志变量.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
void 
UPF_Memory_Pool_CRT_Debug_Alloc_T<
    Allocator, Lock, RecordPolicy >::init_crt_debug( void )
{
    static bool sl_init_crt_debug_flag = false;

    if ( false == sl_init_crt_debug_flag )
    {
        _CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | 
                        _CRTDBG_LEAK_CHECK_DF );

        sl_init_crt_debug_flag = true;
    }

    return;
}

/** 
 * 初始化内部状态.
 * 
 * @param memory_pool_name          [in]  内存池名称.
 * @param allocated_memory_limit    [in]  Client端分配内存的上限值.
 */
template < class Allocator, class Lock, class RecordPolicy >
UPF_Memory_Pool_CRT_Debug_Alloc_T<
    Allocator, Lock, RecordPolicy >::UPF_Memory_Pool_CRT_Debug_Alloc_T( 
        const char *          memory_pool_name,
        size_t                allocated_memory_limit,
        toplevel_allocator_t  allocator_type )

    : UPF_Memory_Pool_Base< Allocator, Lock, RecordPolicy>( memory_pool_name,
                                                            allocated_memory_limit,
                                                            CRT_DEBUG_ALLOC_MEMORY_POOL_TYPE,
                                                            allocator_type
                                                           )
{
    SELF_CLASS::init_crt_debug();

    this->reset_counter();
}

/** 
 * 分配内存.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
void *
UPF_Memory_Pool_CRT_Debug_Alloc_T< 
    Allocator, Lock, RecordPolicy >::allocate_debug( size_t       size,
                                                     const char * file_name,
                                                     int          line_no )
{
    void * result = 0;

    UPF_Guard< Lock > guard( this->m_lock );

    size_t request_size = CRT_DEBUG_BLOCK_NODE_SIZE + size;

    CRT_Debug_Block_Node * block_node_ptr = 
        static_cast< CRT_Debug_Block_Node * >(
            _malloc_dbg( request_size, _NORMAL_BLOCK, file_name, line_no ) );

    if ( 0 != block_node_ptr )
    {
        block_node_ptr->id_alloc_size  = 
            UPF_MMORY_POOL_COMBINE_ID_SIZE( this->m_memory_pool_id, size );

        m_recorder.set_managed_memory_size(
            m_recorder.get_managed_memory_size() + request_size );

        m_recorder.set_allocated_memory_size(
            m_recorder.get_allocated_memory_size() + size );

        m_recorder.set_wasted_memory_size(
            m_recorder.get_wasted_memory_size() + CRT_DEBUG_BLOCK_NODE_SIZE );

        result = reinterpret_cast< char * >( block_node_ptr ) + 
            CRT_DEBUG_BLOCK_NODE_SIZE;
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
UPF_Memory_Pool_CRT_Debug_Alloc_T< 
    Allocator, Lock, RecordPolicy >::reallocate_debug( void *       ptr,
                                                       size_t       new_size,
                                                       const char * file_name,
                                                       int          line_no )
{
    // if ptr is 0, then call allocate to alloc memory.
    if ( 0 == ptr )
    {
        return this->allocate_debug( new_size,
                                     file_name,
                                     line_no );
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

    CRT_Debug_Block_Node * block_node_ptr = 
        reinterpret_cast< CRT_Debug_Block_Node * >(
            static_cast< char * >( ptr ) - CRT_DEBUG_BLOCK_NODE_SIZE );

    size_t old_alloc_size = UPF_MEMORY_POOL_ALLOC_SIZE( 
                                block_node_ptr->id_alloc_size );

    CRT_Debug_Block_Node * new_block_node_ptr = 
        static_cast< CRT_Debug_Block_Node * >(
            _realloc_dbg( block_node_ptr, 
                          CRT_DEBUG_BLOCK_NODE_SIZE + new_size, 
                          _NORMAL_BLOCK, 
                          file_name, 
                          line_no ) );

    if ( 0 != new_block_node_ptr )
    {
        new_block_node_ptr->id_alloc_size = 
            UPF_MMORY_POOL_COMBINE_ID_SIZE( this->m_memory_pool_id, new_size );

        m_recorder.set_managed_memory_size(
            m_recorder.get_managed_memory_size() - old_alloc_size + new_size );

        m_recorder.set_allocated_memory_size(
            m_recorder.get_allocated_memory_size() - old_alloc_size + new_size );

        result = reinterpret_cast< char * >( new_block_node_ptr ) + 
            CRT_DEBUG_BLOCK_NODE_SIZE;
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
UPF_Memory_Pool_CRT_Debug_Alloc_T< 
    Allocator, Lock, RecordPolicy >::deallocate( void * ptr, 
                                                 size_t /* size */ )
{
    UPF_Guard< Lock > guard( this->m_lock );

    if ( 0 != ptr )
    {
        CRT_Debug_Block_Node * block_node_ptr = 
            reinterpret_cast< CRT_Debug_Block_Node * >(
                static_cast< char * >( ptr ) - CRT_DEBUG_BLOCK_NODE_SIZE );

        size_t alloc_size = UPF_MEMORY_POOL_ALLOC_SIZE( 
                                block_node_ptr->id_alloc_size );

        _free_dbg( block_node_ptr, _NORMAL_BLOCK );

        m_recorder.set_managed_memory_size(
            m_recorder.get_managed_memory_size() - alloc_size - 
            CRT_DEBUG_BLOCK_NODE_SIZE );

        m_recorder.set_allocated_memory_size(
            m_recorder.get_allocated_memory_size() - alloc_size );

        m_recorder.set_wasted_memory_size(
            m_recorder.get_wasted_memory_size() - CRT_DEBUG_BLOCK_NODE_SIZE );
    }

    return;
}

