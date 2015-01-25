/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Fixed_Memory_Pool_T.h                                                                  *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]固定对象大小的内存池实现![CN]                                                          *
*********************************************************************************************************************/

#ifndef UPF_FIXED_MEMORY_POOL_T_H
#define UPF_FIXED_MEMORY_POOL_T_H

#include <stddef.h>                     /* for size_t */

#include "UPF_Memory_Pool_Base_T.h"     /* for base class of UPF_Fixed_Memory_Pool */
#include "UPF_Memory_Pool_Define.h"     /* for DEFAULT_OBJECT_SIZE,
                                               DEFAULT_NUM_OBJECTS_PER_BLOCK,
                                               UPF_MEMORY_ALIGN,
                                               MAX_MEMORY_REQ_SIZE */

/** 
 * @class UPF_Fixed_Memory_Pool_T
 * 
 * @brief 固定对象大小的内存池实现
 * 
 * 一个空闲内存块由若干个对象组成, 每个对象头部存一指针指向下一对象，
 * 一个空闲内存块的尾部存一指针指向下一个空闲内存块, 内存块大概的布局如下:<br><pre>
 * object 1-> +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            |                    ptr of object 2                            |
 *            |                    unused memory                              |
 *            |                    unused memory                              |
 *            |                    ....                                       |
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * object 2 ->+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            |                    ptr of object 3                            |
 *            |                    unused memory                              |
 *            |                    unused memory                              |
 *            |                    ....                                       |
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ...........
 * object n ->+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            |                    NULL                                       |
 *            |                    unused memory                              |
 *            |                    unused memory                              |
 *            |                    ....                                       |
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 *            |                    ptr of block 2                             |
 *            +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
 * ............
 * </pre>
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Fixed_Memory_Pool_T : public UPF_Memory_Pool_Base< Allocator, 
                                                             Lock,
                                                             RecordPolicy >
{
public:
    /// 构造函数，用于内存池的初始化.
    UPF_Fixed_Memory_Pool_T( const char *          memory_pool_name,
                             size_t                object_size = DEFAULT_OBJECT_SIZE - SIZEOF_ALLOC_SIZE_FIELD,
                             size_t                num_objects_per_block = DEFAULT_NUM_OBJECTS_PER_BLOCK,
                             toplevel_allocator_t  allocator_type = UPF_TOPLEVEL_PERMANENT_ALLOCATOR);

    /// 析构函数，用于关闭内存池，释放所管理的内存.
    ~UPF_Fixed_Memory_Pool_T( void );

public:
    /// 从内存池中分配内存.
    virtual void* allocate( size_t size );
    
    /// 释放内存.
    virtual void deallocate( void* ptr, size_t size );

    /// 从固定大小的内存池中重新分配内存.
    virtual void * reallocate( void * ptr, size_t new_size );
   
private:
    enum { BLOCK_OVERHEAD = sizeof( int ) /**< 每个内存块需要保留一个4字节字段，
                                               用于指向下一个内存块. */
         };

private:
    /// 创建内存块.
    void * create_memory_block( void );

    /// 设置内存块中对象的next ptr.
    void set_next_object_ptr( void * object_ptr, void * next_object_ptr );

    /// 取得内存块中对象的next ptr
    void * get_next_object_ptr( void * object_ptr );

    /// 设置内存块的next ptr.
    void set_next_block_ptr( void * block_ptr, void * next_block_ptr );

    /// 取得内存块的next ptr
    void * get_next_block_ptr( void * block_ptr );

    /// 释放内存池中的所有内存块占用的内存, 并重置内部的状态.
    void destroy_memory_blocks( void );

    /// 重置内部的状态
    void reset( void );

private:
    /// 经过字节对齐后的对象大小
    size_t m_align_object_size;

    /// 经过字节对齐后的内存块大小
    size_t m_align_block_size;

    /// 根据m_align_object_size, m_align_block_size, BLOCK_OVERHEAD求出的
    /// 每个内存块实际存放的对象个数.
    int m_num_objects_per_block;

    /// 指向第一个内存块.
    void *          m_first_block;

    /// 指向第一个空闲块.
    void *          m_free_list_head;

};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Fixed_Memory_Pool_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_FIXED_MEMORY_POOL_T_H */
