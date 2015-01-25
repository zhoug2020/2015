/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Heap_Alloc_T.h                                                             *
*       CREATE DATE     : 2008-4-14                                                                                  *
*       MODULE          : MemoryPool                                                                                 *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]ʹ��HeapAllocϵ�к��������ڴ�.![CN]                                                    *
*********************************************************************************************************************/
#ifndef UPF_MEMORY_POOL_HEAP_ALLOC_T_H
#define UPF_MEMORY_POOL_HEAP_ALLOC_T_H

#include "UPF_Memory_Pool_Base_T.h"
#include <windows.h>

/** 
 * @class UPF_Memory_Pool_Heap_Alloc_T
 * 
 * @brief ʹ��HeapAllocϵ�к��������ڴ�, ��Ҫ����pageheap.exe����ڴ��ƻ�.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Memory_Pool_Heap_Alloc_T : public UPF_Memory_Pool_Base< Allocator,
                                                                  Lock,
                                                                  RecordPolicy >
{
public:
    /// ��ʼ���ڲ�״̬.
    UPF_Memory_Pool_Heap_Alloc_T( const char *          memory_pool_name,
                                  size_t                allocated_memory_limit = MAX_MEMORY_REQ_SIZE,
                                  toplevel_allocator_t  allocator_type = UPF_TOPLEVEL_PERMANENT_ALLOCATOR );

public:
    virtual void * allocate( size_t size );

    virtual void * reallocate( void * ptr,
                               size_t new_size );

    virtual void deallocate( void * ptr, size_t size );

private:
    /// Heap Alloc�ڴ��Ľڵ�����, ���һ���ֶ�Ϊalloc size�ֶ�, ��Ϊ���ͷ��ڴ�ʱ��
    /// ������ڴ�ָ�����ƫ��4���ֽ�, ��ȷ�����ڴ�Ĵ�С.
    struct Heap_Alloc_Block_Node
    {
        /// �ڴ��Ĵ�С���������ڴ��ID
        size_t id_alloc_size;
    };

    /// Heap_Alloc_Block_Node�Ĵ�С����
    enum { HEAP_ALLOC_BLOCK_NODE_SIZE = sizeof( Heap_Alloc_Block_Node ) };

private:
    typedef UPF_Memory_Pool_Heap_Alloc_T< Allocator,
                                          Lock,
                                          RecordPolicy > SELF_CLASS;

private:
    /// ���ڱ����Զ���ѵľ��.
    static HANDLE sm_pool_heap;

    /// �����Զ���ѵ�lock
    static UPF_Thread_Mutex  sm_pool_heap_lock;
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_Heap_Alloc_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_MEMORY_POOL_HEAP_ALLOC_T_H */

