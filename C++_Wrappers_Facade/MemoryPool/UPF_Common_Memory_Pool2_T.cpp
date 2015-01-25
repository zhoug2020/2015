/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Common_Memory_Pool2_T.cpp                                                              *
*       CREATE DATE     : 2007-11-14                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]可变对象大小的内存池实现![CN]                                                          *
*********************************************************************************************************************/

#include <assert.h>                 /* for assert */
#include <memory.h>                 /* for memset */

#include "UPF_Common_Memory_Pool2_T.h"
#include "UPF_Memory_Pool_Util.h"   /* for get_free_small_block_index,
                                           get_free_small_block_size. */

// Constant definitions.

// type desc
const char * const COMMON_MEMORY_POOL2_TYPE = "Common2";

/** 
 * 初始化内部状态.
 * 
 * @param memory_pool_name              [in]  内存池名称
 * @param allocated_memory_limit        [in]  Client端分配内存的上限值
 * @param allocator_type                [in]  allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
 *                                            UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 */
template < class Allocator, class Lock, class RecordPolicy >
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::UPF_Common_Memory_Pool2_T(
     const char *          memory_pool_name,
     size_t                allocated_memory_limit,
     toplevel_allocator_t  allocator_type )

    : UPF_Memory_Pool_Base< Allocator, Lock, RecordPolicy >( memory_pool_name,
                                                             allocated_memory_limit,
                                                             COMMON_MEMORY_POOL2_TYPE,
                                                             allocator_type )
{
    this->reset();
}

/** 
 * 清除内存池内部申请的所有内存
 * 
 * @param memory_pool_name    [in]  内存池名称
 */
template < class Allocator, class Lock, class RecordPolicy >
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::~UPF_Common_Memory_Pool2_T( void )
{
    this->destroy_memory_space();
}

/** 
 * 从内存池中分配内存.
 * 
 * @param size       [in]  要申请的内存的大小, 如果size为0, 那么默认分配1字节的内存.
 * 
 * @retval 指向内存的有效指针   分配内存成功
 * @retval 0                    分配内存失败
 */
template < class Allocator, class Lock, class RecordPolicy >
void*
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::allocate( size_t size )
{
    void * ret = 0;

    if ( size > static_cast< size_t >( MAX_MEMORY_REQ_SIZE ) )
    {
        return 0;
    }

    if ( 0 == size )
    {
        size = 1;
    }

    // because allocated memory need include size info,
    // so should allocate memory that's requested size is
    // size + SIZEOF_ALLOC_SIZE_FIELD.
    size_t request_size = size + SIZEOF_ID_ALLOC_SIZE_FIELD;

    UPF_Guard< Lock > guard( this->m_lock );

    if ( request_size > static_cast< size_t >( MAX_SMALL_BLOCK_SIZE ) )
    {
        void * allocated_mem = Allocator::allocate( LARGE_BLOCK_NODE_SIZE +
                                                    size,
                                                    m_allocator_type );

        if ( allocated_mem != 0 )
        {
            // set large block's size info
            Large_Block_Node * large_block = 
                static_cast< Large_Block_Node *>( allocated_mem );

            large_block->id_alloc_size  = UPF_MMORY_POOL_COMBINE_ID_SIZE( 
                                                this->m_memory_pool_id, size );

            // return the ptr that offset LARGE_BLOCK_NODE_SIZE bytes from allocated_mem.
            ret = static_cast< char * >( allocated_mem ) + LARGE_BLOCK_NODE_SIZE;

            // set managed memory size.
            m_recorder.set_managed_memory_size( 
                m_recorder.get_managed_memory_size() + 
                LARGE_BLOCK_NODE_SIZE + size );

            // set allocated memory size
            m_recorder.set_allocated_memory_size( 
                m_recorder.get_allocated_memory_size() + size );

            // set wasted memory size
            m_recorder.set_wasted_memory_size(
                m_recorder.get_wasted_memory_size() + 
                LARGE_BLOCK_NODE_SIZE );
            
        }        
    }
    else
    {
        size_t align_value = 0;

        int free_index = UPF_Memory_Pool_Util::get_free_small_block_index( 
            request_size, align_value );

        // the free_index ensure less than MAX_SMALL_BLOCKS_INDEX.
        Small_Block_Node * my_free_small_block = 
            m_free_small_block_list[ free_index ];

        // first find free small block from small block list, 
        // if not find free small block, then get from allocated memory space.
        if ( my_free_small_block != 0 )
        {
            m_free_small_block_list[ free_index ] = 
                my_free_small_block->next_block_node;
        }
        else
        {
            // the small memory block size allocated on free_index position of
            // m_free_small_block_list.
            size_t small_block_size = UPF_MEMORY_ALIGN_LACK_ONE( request_size,
                                                                 align_value );

            my_free_small_block = static_cast< Small_Block_Node * > (
                     this->refill( small_block_size, free_index ) );
        }

        if ( my_free_small_block != 0 )
        {
            // set small block's size info
            my_free_small_block->id_alloc_size = 
                UPF_MMORY_POOL_COMBINE_ID_SIZE( this->m_memory_pool_id, size );

            ret = reinterpret_cast< char * >( my_free_small_block ) + 
                    SIZEOF_ID_ALLOC_SIZE_FIELD;

            // set allocated memory size
            m_recorder.set_allocated_memory_size( 
                m_recorder.get_allocated_memory_size() + size );

            // set wasted memory size
            m_recorder.set_wasted_memory_size(
                m_recorder.get_wasted_memory_size() + 
                ( UPF_MEMORY_ALIGN_LACK_ONE( request_size, align_value ) - size) );
    
        }
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

    return ret;
}

/** 
 * 从系统中申请更多的内存块, 并加到内部的small block link list.
 * 
 * @param size          [in]     申请的内存块大小
 * @param free_index    [in]     空闲索引
 * 
 * @return void*
 */
template < class Allocator, class Lock, class RecordPolicy > 
void * 
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::refill( size_t size,
                                                                    int    free_index )
{
    int    num_objs      = 20;
    char * memory_block  = static_cast< char *>( this->allocate_blocks( size, num_objs ) );

    if ( 0 == memory_block  )
    {
        return 0;
    }

    Small_Block_Node * next_block = NULL;
    Small_Block_Node * cur_block  = NULL;
    Small_Block_Node * result     = NULL;

    if ( 1 == num_objs )
    {
        return memory_block;
    }

    result = reinterpret_cast< Small_Block_Node * >( memory_block );

    m_free_small_block_list[ free_index ] = next_block =
        reinterpret_cast< Small_Block_Node * >( memory_block + size );

    for ( int i = 1; ; i++ )
    {
        cur_block = next_block;
        next_block = reinterpret_cast< Small_Block_Node *>(
            reinterpret_cast< char * >( next_block ) + size);

        if ( i == num_objs - 1 )
        {
            cur_block->next_block_node = 0;
            break;
        }
        else
        {
            cur_block->next_block_node = next_block;
        }
    }

    return result;
}


/** 
 * 从系统中申请更多的内存, 如果当前已申请的内存中有剩余的内存, 则把这部份剩余的
 * 内存添加到空闲块列表中，然后从系统中申请一大块内存, 通常这大块内存应为页面大小
 * 的整数倍，但最小值为两个页面大小.
 * 
 * 如果系统没有可用内存了, 则从空闲块列表中寻找, 如果找到了, 就返回这个空闲块,
 * 否则返回0.
 * 
 * @param size     [in]   要获取的内存块大小, 应为8的倍数.
 * 
 * @retval 指向内存的有效指针   获取内存成功
 * @retval 0                    获取内存失败
 */
template < class Allocator, class Lock, class RecordPolicy > 
void * 
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::allocate_blocks( size_t size,
                                                                             int &  num_objs )
{
    // initialize result be 0.
    void   * result = 0;

    size_t total_bytes = size * num_objs;
    size_t bytes_left  = m_end_avail_memory - m_start_avail_memory;

    if ( bytes_left >= total_bytes )
    {
        result = m_start_avail_memory;
        m_start_avail_memory += total_bytes;

        return result;        
    }
    else if ( bytes_left >= size )
    {
        num_objs = static_cast< int >( bytes_left / size );
        total_bytes = size * num_objs;
        result = m_start_avail_memory;
        m_start_avail_memory += total_bytes;
        return result;
    }
    else
    {
        if ( bytes_left > 0 )
        {
            // if bytes_left greater than 0, then make this part of memory place
            // on the corresponding position of m_free_small_block_list.
			place_remainder_memory_on_block_list( bytes_left );

        }
    
        // set allocated memory size, min size is MIN_ALLOC_MEM_SIZE(8K)
         size_t bytes_to_get = UPF_MEMORY_ALIGN( 2 * total_bytes +
                                                 (m_heap_size >> 4) + 
                                                 MEOMRY_NODE_SIZE,
                                                 UPF_MEMORY_PAGE_SIZE );

        if ( bytes_to_get < MIN_ALLOC_MEM_SIZE )
        {
            bytes_to_get = MIN_ALLOC_MEM_SIZE;
        }
    
        m_start_avail_memory = static_cast< char * >(
             Allocator::allocate( bytes_to_get, m_allocator_type ) );
    
        if ( m_start_avail_memory != 0 )
        {
            // if allocate memory succeed, then add this memory node into
            // internal memory node link list.
            // This memory node link list used to save memory space info allocated
            // for free memory when destruction.
            Memory_Node * memory_node = 
                reinterpret_cast< Memory_Node * >( m_start_avail_memory );
    
            memory_node->next_memory_node = m_first_memory_node;
            memory_node->memory_size      = bytes_to_get;

            m_first_memory_node           = memory_node;
    
            // set m_start_avail_memory be the location that
            // offset MEOMRY_NODE_SIZE bytes from m_first_memory_node
            m_start_avail_memory = reinterpret_cast< char * >( m_first_memory_node ) +
                                   MEOMRY_NODE_SIZE;
    
            // set m_end_avail_memory ptr.
            m_end_avail_memory = reinterpret_cast< char * >( m_first_memory_node ) + 
                                 bytes_to_get;

            m_heap_size += bytes_to_get;

            m_recorder.set_managed_memory_size( 
                m_recorder.get_managed_memory_size() + bytes_to_get );

            m_recorder.set_wasted_memory_size(
                m_recorder.get_wasted_memory_size() + MEOMRY_NODE_SIZE );

            return this->allocate_blocks( size, num_objs );
        }
        else
        {
            // reset m_end_avail_memory
            m_end_avail_memory = 0;
    
            // if allocate memory failed, then call 
            // get_avail_memory_from_free_block_list to find free small block node from
            // m_free_small_block_list.
    
            m_start_avail_memory = reinterpret_cast< char * >(
                get_avail_memory_from_free_block_list( size, bytes_to_get ) );
    
            if ( m_start_avail_memory != 0 )
            {
                // set m_end_avail_memory ptr.
                m_end_avail_memory = m_start_avail_memory + bytes_to_get;

                return this->allocate_blocks( size, num_objs );
            }
        }
    }
   
    return result;
}

/** 
 * 从m_free_small_block_list中取得空闲的small block.
 * 
 * @param size             [in]    请求的内存大小
 * @param bytes_to_get     [out]   实际得到的内存大小
 * 
 * @retval 一个有效的内存指针   找到一个空闲的内存块
 * @retval 0                    没有找到空闲的内存块
 */
template < class Allocator, class Lock, class RecordPolicy > 
inline void * 
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::get_avail_memory_from_free_block_list(
     size_t size, size_t & bytes_to_get )
{

    // initialize variable's state be 0
    Small_Block_Node * my_free_small_block = 0;
    size_t align_value = 0;

    bytes_to_get = 0;

    int free_index = UPF_Memory_Pool_Util::get_free_small_block_index(
         size, align_value ) + 1;

    for ( ; free_index < MAX_SMALL_BLOCKS_INDEX; ++free_index )
    {               
        my_free_small_block = m_free_small_block_list[ free_index ];

        if ( my_free_small_block != 0 )
        {
            m_free_small_block_list[ free_index ] = 
                my_free_small_block->next_block_node;

            bytes_to_get = UPF_Memory_Pool_Util::get_free_small_block_size( free_index );

            break;
        }
    }

    return my_free_small_block;
}

/** 
 * 清除内存池内部申请的所有内存, 并重置内部的状态.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
void
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::destroy_memory_space( void )
{
    UPF_Guard< Lock > guard( this->m_lock );

    Memory_Node * current_memory_node = m_first_memory_node;
    Memory_Node * next_memory_node = 0;

    while ( current_memory_node != 0 )
    {
        next_memory_node = current_memory_node->next_memory_node;

        Allocator::deallocate( current_memory_node,
                               current_memory_node->memory_size,
                               m_allocator_type );

        current_memory_node = next_memory_node;
    }

    this->reset();
}

/** 
 *  释放内存, 把ptr所指向的内存归还给内存池, ptr所指向的内存必须为调用
 *  UPF_Common_Memory_Pool2::allocate所申请的内存.
 * 
 * @param ptr        [in] 调用UPF_Common_Memory_Pool2::allocate所申请的内存
 * @param size       [in] 内存大小
 */
template < class Allocator, class Lock, class RecordPolicy > 
void
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::deallocate( void* ptr, size_t /* size */)
{
    if ( ptr != 0 )
    {
        UPF_Guard< Lock > guard( this->m_lock );

        // offset 4 bytes back from ptr, get the size of the memory that pointed
        // by ptr.
        size_t * original_ptr = reinterpret_cast< size_t * >(
            static_cast< char * >( ptr ) - SIZEOF_ID_ALLOC_SIZE_FIELD );

        size_t alloc_size    = UPF_MEMORY_POOL_ALLOC_SIZE( *original_ptr ); 

        // alloc size can't be 0, if is 0, then this memory block was corrupted.
        assert( alloc_size != 0 );

        size_t original_size = ( alloc_size + SIZEOF_ID_ALLOC_SIZE_FIELD );

        // if alloc_size greater than MAX_SMALL_BLOCK_SIZE, then this part of
        // memory is large memory block, free the memory and return the memory
        // space to system directly, otherwise this part of memory is small
        // memory block, then add the memory into free small block link list.
        if ( original_size > static_cast< size_t >( MAX_SMALL_BLOCK_SIZE ) )
        {
            void * large_block = static_cast< char * >( ptr ) - LARGE_BLOCK_NODE_SIZE;

            Allocator::deallocate( large_block, 
                                   (alloc_size + LARGE_BLOCK_NODE_SIZE),
                                   m_allocator_type );            

            // set managed memory size.
            m_recorder.set_managed_memory_size( 
                m_recorder.get_managed_memory_size() - alloc_size - 
                LARGE_BLOCK_NODE_SIZE );

            // set allocated memory size
            m_recorder.set_allocated_memory_size( 
                m_recorder.get_allocated_memory_size() - alloc_size );

            // set wasted memory size
            m_recorder.set_wasted_memory_size(
                m_recorder.get_wasted_memory_size() - LARGE_BLOCK_NODE_SIZE );
        }
        else
        {
            Small_Block_Node * my_small_block_node = 
                reinterpret_cast< Small_Block_Node * >( original_ptr );

            size_t align_value = 0;

            int free_index = UPF_Memory_Pool_Util::get_free_small_block_index( 
                original_size, align_value );

            // if my_small_block_node equal to m_free_small_block_list[ free_index ], 
            // then ptr be freed twice, this is error!!
            assert( my_small_block_node != m_free_small_block_list[ free_index ] );
    
            my_small_block_node->next_block_node = m_free_small_block_list[ free_index ];
    
            m_free_small_block_list[ free_index ] = my_small_block_node;

            // set allocated memory size
            m_recorder.set_allocated_memory_size( 
                m_recorder.get_allocated_memory_size() - alloc_size );

            // set wasted memory size
            m_recorder.set_wasted_memory_size(
                m_recorder.get_wasted_memory_size() - 
                (UPF_MEMORY_ALIGN_LACK_ONE( original_size, align_value ) - alloc_size) );
        }
    }

    return;
}

/** 
 * 重置内部状态
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
inline void
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::reset( void )
{
    m_first_memory_node  = 0;
    m_start_avail_memory = 0;
    m_end_avail_memory   = 0;

    m_heap_size = 0;

    memset( m_free_small_block_list, 0, sizeof( m_free_small_block_list ) );

    this->reset_counter();
}

/** 
 * 从内存池中重新分配内存, 并保留原有的内容，当分配内存时，如果在原来的位置
 * 还有足够的内存可以分配, 则进行就地分配.
 * 
 * 有如下三种情况:
 *      -# 如果ptr为0, 则调用allocate进行分配内存.
 *      -# 如果ptr不为0, new_size为0, 则调用deallocate释放内存.
 *      -# 如果ptr不为0, new_size不为0, 则进行重新分配内存操作, 如果在原\n
 *         位置有可用的内存，则进行就地分配，否则重新分配一块内存, 并把原\n
 *         始内存的内容拷贝到新申请的内存中, 并释放原来的内存.
 * 
 * @note 如果新分配内存失败, 原有内存并不释放, 原来的指针仍然有效.
 * 
 * 
 * @param ptr           [in] 调用allocate或reallocate所申请的内存.
 * @param new_size      [in] 新的内存大小
 * 
 * @retval 一个指向所申请的内存的指针    重新申请内存成功. 
 * @retval 0                             重新申请内存失败, ptr所指向的内存\n
 *                                       并不释放, ptr仍然有效.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
void * 
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::reallocate( 
    void * ptr, size_t new_size )
{
    void * new_ptr = 0;

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

    if ( new_size > static_cast< size_t >( MAX_MEMORY_REQ_SIZE ) )
    {
        return 0;
    }
 
    UPF_Guard< Lock > guard( this->m_lock );

    size_t * old_original_ptr = reinterpret_cast< size_t * >(
        static_cast< char * >( ptr ) - SIZEOF_ID_ALLOC_SIZE_FIELD );

    size_t old_alloc_size  = UPF_MEMORY_POOL_ALLOC_SIZE( *old_original_ptr );

    // if old alloc size equal to new size, then return ptr directly.
    if ( old_alloc_size == new_size )
    {
        return ptr;
    }

    size_t old_original_size    = ( old_alloc_size + SIZEOF_ID_ALLOC_SIZE_FIELD );

    size_t new_request_size     = new_size + SIZEOF_ID_ALLOC_SIZE_FIELD;

    if ( old_original_size > static_cast< size_t >( MAX_SMALL_BLOCK_SIZE ) )
    {
        // if new request size greater than MAX_SMALL_BLOCK_SIZE, 
        // call Allocator::reallocate to realloate memory and update
        // memory size record.
        // if new request size less than MAX_SMALL_BLOCK_SIZE, then call allocate
        // to alloc new memory from memory pool and copy original memory's content.
        
        if ( new_request_size > static_cast< size_t >( MAX_SMALL_BLOCK_SIZE ) )
        {
            new_ptr = Allocator::reallocate( 
                static_cast< char * >( ptr ) - LARGE_BLOCK_NODE_SIZE,
                new_size + LARGE_BLOCK_NODE_SIZE,
                m_allocator_type );

            // if reallocate succeed, then set size info and update memory size
            // record.
            // if reallocate failed, then the original memory is not freed,
            // memory size record is not updated also.
            if ( new_ptr != 0 )
            {
                // set large block's size info
                Large_Block_Node * large_block = 
                    static_cast< Large_Block_Node *>( new_ptr );

                large_block->id_alloc_size = 
                    UPF_MMORY_POOL_COMBINE_ID_SIZE( this->m_memory_pool_id, new_size );
        
                // return the ptr that offset LARGE_BLOCK_NODE_SIZE bytes from new_ptr.
                new_ptr = static_cast< char * >( new_ptr ) + LARGE_BLOCK_NODE_SIZE;

                // update managed memory size
                m_recorder.set_managed_memory_size( 
                    m_recorder.get_managed_memory_size() - old_original_size + new_request_size );

                // update allocated memory size
                m_recorder.set_allocated_memory_size( 
                    m_recorder.get_allocated_memory_size() - old_alloc_size + new_size );

                // wasted memory size should not be updated, because old wasted 
                // memory size equal to new wasted memory size, they all be 
                // SIZEOF_ID_ALLOC_SIZE_FIELD(4).
            }
        }
        else
        {
            // note: because want to call allocate, deallocate, so here need release
            // lock for prevent from dead lock.
            guard.release();

            new_ptr = this->allocate( new_size );

            if ( new_ptr != 0 )
            {
                memcpy( new_ptr, ptr, new_size );
                this->deallocate( ptr, 0 );
            }                
        }
    }
    else
    {
        size_t align_value = 0;

        int free_index = UPF_Memory_Pool_Util::get_free_small_block_index(
             old_original_size, align_value );

        int new_free_index = UPF_Memory_Pool_Util::get_free_small_block_index(
             new_request_size, align_value );

        // if old original memory block can satisfy new memory request,
        // then use original memory that ptr points to and update
        // the original memory'size info.

        if ( free_index == new_free_index )
        {
            // update allocated memory size
            m_recorder.set_allocated_memory_size( 
                m_recorder.get_allocated_memory_size() - old_alloc_size + new_size );

            // update wasted memory size
            m_recorder.set_wasted_memory_size(
                m_recorder.get_wasted_memory_size() + old_alloc_size - new_size );
            
            // update the original memory'size info.
            *old_original_ptr = UPF_MMORY_POOL_COMBINE_ID_SIZE( 
                                    this->m_memory_pool_id, new_size );
            
            new_ptr = ptr;                
        }
        else
        {
            // note: because want to call allocate, deallocate, so here need release
            // lock for prevent from dead lock.
            guard.release();

            new_ptr = this->allocate( new_size );

            if ( new_ptr != 0 )
            {
                size_t copy_size = ( ( new_size > old_alloc_size ) ? old_alloc_size
                                                                   : new_size );

                memcpy( new_ptr, ptr, copy_size ); 

                this->deallocate( ptr, 0 );
            }                                
        }
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

    return new_ptr;
}

/** 
 * 当调用allocate申请内存时，m_start_avail_memory所指向的内存块中剩余的内存不能
 * 满足请求, 在申请一块大的内存来满足请求前, 调用place_remainder_memory_on_block_list
 * 来把这部分剩余的内存分配到空闲的内存块列表中.
 *
 * 因为内存池的最小分配粒度为8, 所以bytes_left肯定为8的倍数, 故这部分剩余的内存会正确的
 * 的分配到空闲的内存块列表中相应的位置上.
 *
 * @param bytes_left       [in]       m_start_avail_memory所指向的内存块中剩余的内存字节数
 *
 */
template < class Allocator, class Lock, class RecordPolicy >
void
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::
place_remainder_memory_on_block_list( size_t bytes_left )
{
    size_t align_value = 0;
    size_t small_block_size = 0;

	Small_Block_Node * my_free_small_block = 0;
	Small_Block_Node * small_block_left    = 0;

    int free_index;
 
    while ( bytes_left != 0 )
    {
        free_index = UPF_Memory_Pool_Util::get_nearest_free_small_block_index( 
            bytes_left, small_block_size );
    
        my_free_small_block = m_free_small_block_list[ free_index ];
    
        small_block_left =
             reinterpret_cast< Small_Block_Node * >( m_start_avail_memory );
    
        small_block_left->next_block_node = my_free_small_block;
    
        m_free_small_block_list[ free_index ] = small_block_left;

        bytes_left -= small_block_size;
        m_start_avail_memory += small_block_size;
    }

}

