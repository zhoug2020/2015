
/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Common_Global_Memory_Pool_T.h                                                          *
*       CREATE DATE     : 2008-7-18                                                                                  *
*       MODULE          : Memory pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]可变对象大小的全局内存池![CN]                                                          *
*********************************************************************************************************************/
#ifndef UPF_COMMON_GLOBAL_MEMORY_POOL_T_H
#define UPF_COMMON_GLOBAL_MEMORY_POOL_T_H

#include "UPF_IGlobal_Memory_Pool.h"
#include "UPF_Memory_Pool_Define.h"     /* for MAX_SMALL_BLOCK_SIZE,
                                               MAX_SMALL_BLOCKS_INDEX,
                                               MIN_ALLOC_MEM_SIZE,
                                               MIN_ALLOC_GRANULARITY,
                                               UPF_MEMORY_ALIGN_LACK_ONE,
                                               UPF_MEMORY_ALIGN,
                                               UPF_MEMORY_ALIGN_DEFAULT,
                                               MAX_MEMORY_REQ_SIZE,
                                               MESSAGE_ASSERT */

/** 
 * @class UPF_Common_Global_Memory_Pool_T
 * 
 * @brief 可变内存池，当申请的内存小于16K时，由该内存池管理, 当大于16K时，交由
 * 系统管理.
 * 
 * 大概的管理策略是, 内部保存一个128个元素的数组m_free_small_block_list,每个元素
 * 管理固定大小的内存块, 具体数据请参见下表:<br><pre>
 * -------------------------------------------
 * |  数组索引  |   分配粒度    |  管理范围   |
 * -------------------------------------------
 * |  1-32      |      8        |   1-256     | 
 * |  33-48     |      16       |  257-512    |
 * |  49-64     |      32       |  513-1024   | 
 * |  65-80     |      64       | 1025-2048   |
 * |  81-96     |     128       | 2049-4096   |
 * | 97-112     |     256       | 4097-8192   |
 * | 113-128    |     512       | 8193-16384  |
 * -------------------------------------------
 * </pre>
 * 
 * 当Client端申请的内存小于16K时, 根据申请的内存大小找到在m_free_small_block_list
 * 中的相应索引，然后判断在该位置上是否有空闲的内存块，如果有，则分配出去，否则
 * 从系统分配出一个大的内存块，最小为2个页面大小(8K), 然后从这个大内存块中划分出
 * 相应的块，在进行分配.
 * 
 * 当Client端申请的内存大于16k时，则直接交给系统去分配.
 * 
 * 在分配内存时, 分配给Client端的内存会比请求的大小多出4字节，这部分内容用于记录
 * 分配给Client端的内存大小.
 * 
 * 当Client端释放内存时, 根据这个内存指针, 向后偏移4个字节, 取出该内存块的大小,
 * 然后根据这个内存大小进行判断, 如果小于16K, 算出在m_free_small_block_list
 * 中的相应索引, 然后把该内存块添加到空闲内存块链表中, 如果大于16K, 则交由系统去
 * 释放.
 *
 * UPF_Common_Global_Memory_Pool_T默认使用的toplevel allocator type是
 * UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Common_Global_Memory_Pool_T : public UPF_IGlobal_Memory_Pool
{
public:
    /// 初始化内部状态.
    UPF_Common_Global_Memory_Pool_T( void );

    /// 清除内存池内部申请的所有内存
    ~UPF_Common_Global_Memory_Pool_T( void );

public:
    /// 从内存池中分配内存.
    virtual void* allocate( size_t size, void * hint_ptr );
    
    /// 释放内存.
    virtual void deallocate( void* ptr, size_t size, void * hint_ptr );

    /// 从可变大小的内存池中重新分配内存.
    virtual void * reallocate( void * ptr, size_t new_size, void * hint_ptr );

private:
    /// 小内存块的节点类型 (memory size <= 16K)
    struct Small_Block_Node
    {
        union
        {
            /// 内存块的大小和所属的内存池ID
            size_t id_alloc_size;

            /// 当在free状态时, 用于指向下一个Small Block.
            Small_Block_Node * next_block_node;

        };
        
        /// 当该内存块被Client端使用时, 用于指明被Client端使用的内存.
        char               client_data[1];
    };

    /// 大内存块的节点类型 (memory size > 16K), Large_Block_Node的最后一个字段
    /// 必须和Small_Block_Node的第一个字段匹配, 因为在释放内存时，会根据内存
    /// 指针向后偏移4个字节, 来确定该内存的大小.
    struct Large_Block_Node
    {
        /// 内存块的大小和所属的内存池ID
        size_t id_alloc_size;
    };

    /// 当每次分配内存时, 如果没有可用内存, 需要从系统分配出一大块内存, 最小为
    /// 两个页面大小, Memory_Node用于记录已分配内存的链表.
    struct Memory_Node
    {
        Memory_Node * next_memory_node;
        size_t        memory_size;
    };

private:            
    enum  { 
        /* 每个分配出去的内存块的记录信息字段的大小, 通常为4. */
        SIZEOF_ID_ALLOC_SIZE_FIELD = sizeof( size_t ) 
        };

    enum {
        /* Large_Block_Node的大小 */
        LARGE_BLOCK_NODE_SIZE = sizeof( Large_Block_Node )
    };

    enum {
        /* Memory_Node的大小*/
        MEOMRY_NODE_SIZE = UPF_MEMORY_ALIGN_DEFAULT( sizeof(Memory_Node) )
    };
    
private:
    /// 从系统中申请更多的内存.
    void * get_memory_space( size_t size, RecordPolicy * size_recorder );

    /// 从m_free_small_block_list中取得空闲的small block.
    void * get_avail_memory_from_free_block_list( size_t   size, 
                                                  size_t & bytes_to_get );

    /// 把多余的内存分配到空闲的内存块列表中.
    void place_remainder_memory_on_block_list( size_t bytes_left );
    
    /// 清除内存池内部申请的所有内存
    void destroy_memory_space( void );

    /// 重置内部状态
    void reset( void );

private:
    /// 用于存放free small block, 具体请参见UPF_Common_Memory_Pool的说明.
    Small_Block_Node * m_free_small_block_list[ MAX_SMALL_BLOCKS_INDEX ];

    /// 用于保存一个指向大内存块链表的头指针.
    Memory_Node * m_first_memory_node;

    /// 指向可用内存的起始位置.
    char *        m_start_avail_memory;

    /// 指向可用内存的终止位置.
    char *        m_end_avail_memory;

    /// 线程锁, 用于线程同步.
    mutable Lock  m_lock;
   
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Common_Global_Memory_Pool_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_COMMON_GLOBAL_MEMORY_POOL_T_H */

