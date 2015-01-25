/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_CRT_Debug_Alloc_T.h                                                        *
*       CREATE DATE     : 2008-1-9                                                                                   *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]创建一个使用CRT Debug堆来管理内存的UPF_IMemory_Pool的实现![CN]                         *
********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_CRT_DEBUG_ALLOC_T_H
#define UPF_MEMORY_POOL_CRT_DEBUG_ALLOC_T_H

#include "UPF_Memory_Pool_Base_T.h" 

/** 
 * @class UPF_Memory_Pool_CRT_Debug_Alloc_T
 * 
 * @brief 实现UPF_IMemory_Pool接口，使用CRT Debug堆来管理内存.
 * 
 * @details 由于CRT Debug堆提供了内存越界校验，和内存泄露检查等功能，故这里使用
 * CRT Debug堆来管理内存，用于调试一些内存方面的错误。       
 * 当有内存泄露时，泄漏的信息会输出到调试器的Output窗口.
 * 
 * 对于内存使用信息的计数，由于这里用CRT Debug堆来管理，所以只有
 * allocate size 可用.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Memory_Pool_CRT_Debug_Alloc_T : public UPF_Memory_Pool_Base< Allocator,
                                                                       Lock,
                                                                       RecordPolicy >
{
public:
    /// 初始化内部状态.
    UPF_Memory_Pool_CRT_Debug_Alloc_T( const char *          memory_pool_name,
                                       size_t                allocated_memory_limit = MAX_MEMORY_REQ_SIZE,
                                       toplevel_allocator_t  allocator_type = UPF_TOPLEVEL_PERMANENT_ALLOCATOR );

public:
    virtual void * allocate( size_t size )
    {
        return allocate_debug( size, 0, 0 );
    }

    virtual void * reallocate( void * ptr,
                               size_t new_size )
    {
        return reallocate_debug( ptr, new_size, 0, 0 );
    }


    virtual void * allocate_debug( size_t       size, 
                                   const char * file_name,
                                   int          line_no );


    virtual void * reallocate_debug( void *       ptr,
                                     size_t       new_size,
                                     const char * file_name,
                                     int          line_no );

    virtual void deallocate( void * ptr, size_t size );

private:
    /// CRT内存块的节点类型, 最后一个字段为alloc size字段, 因为在释放内存时，
    /// 会根据内存指针向后偏移4个字节, 来确定该内存的大小.
    struct CRT_Debug_Block_Node
    {
        /// 内存块的大小和所属的内存池ID
        size_t id_alloc_size;
    };

    /// CRT_Debug_Block_Node的大小定义
    enum { CRT_DEBUG_BLOCK_NODE_SIZE = sizeof( CRT_Debug_Block_Node ) };

private:
    typedef UPF_Memory_Pool_CRT_Debug_Alloc_T< Allocator,
                                               Lock,
                                               RecordPolicy > SELF_CLASS;

private:
    /// 初始化crt debug堆的一些标志变量.
    static void init_crt_debug( void );
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_CRT_Debug_Alloc_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_MEMORY_POOL_CRT_DEBUG_ALLOC_T_H */

