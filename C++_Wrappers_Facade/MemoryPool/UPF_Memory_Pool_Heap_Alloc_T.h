/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Heap_Alloc_T.h                                                             *
*       CREATE DATE     : 2008-4-14                                                                                  *
*       MODULE          : MemoryPool                                                                                 *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]使用HeapAlloc系列函数分配内存.![CN]                                                    *
*********************************************************************************************************************/
#ifndef UPF_MEMORY_POOL_HEAP_ALLOC_T_H
#define UPF_MEMORY_POOL_HEAP_ALLOC_T_H

#include "UPF_Memory_Pool_Base_T.h"
#include <windows.h>

/** 
 * @class UPF_Memory_Pool_Heap_Alloc_T
 * 
 * @brief 使用HeapAlloc系列函数分配内存, 主要用于pageheap.exe检查内存破坏.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Memory_Pool_Heap_Alloc_T : public UPF_Memory_Pool_Base< Allocator,
                                                                  Lock,
                                                                  RecordPolicy >
{
public:
    /// 初始化内部状态.
    UPF_Memory_Pool_Heap_Alloc_T( const char *          memory_pool_name,
                                  size_t                allocated_memory_limit = MAX_MEMORY_REQ_SIZE,
                                  toplevel_allocator_t  allocator_type = UPF_TOPLEVEL_PERMANENT_ALLOCATOR );

public:
    virtual void * allocate( size_t size );

    virtual void * reallocate( void * ptr,
                               size_t new_size );

    virtual void deallocate( void * ptr, size_t size );

private:
    /// Heap Alloc内存块的节点类型, 最后一个字段为alloc size字段, 因为在释放内存时，
    /// 会根据内存指针向后偏移4个字节, 来确定该内存的大小.
    struct Heap_Alloc_Block_Node
    {
        /// 内存块的大小和所属的内存池ID
        size_t id_alloc_size;
    };

    /// Heap_Alloc_Block_Node的大小定义
    enum { HEAP_ALLOC_BLOCK_NODE_SIZE = sizeof( Heap_Alloc_Block_Node ) };

private:
    typedef UPF_Memory_Pool_Heap_Alloc_T< Allocator,
                                          Lock,
                                          RecordPolicy > SELF_CLASS;

private:
    /// 用于保存自定义堆的句柄.
    static HANDLE sm_pool_heap;

    /// 用于自定义堆的lock
    static UPF_Thread_Mutex  sm_pool_heap_lock;
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_Heap_Alloc_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_MEMORY_POOL_HEAP_ALLOC_T_H */

