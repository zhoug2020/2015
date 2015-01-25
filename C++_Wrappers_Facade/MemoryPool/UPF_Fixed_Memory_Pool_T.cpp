/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Fixed_Memory_Pool_T.cpp                                                                *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]固定对象大小的内存池的实现![CN]                                                        *
*********************************************************************************************************************/
#include <assert.h>                      /* for assert */ 
#include "UPF_Fixed_Memory_Pool_T.h"

// Constant definitions.
const char * const FIXED_MEMORY_POOL_TYPE = "Fixed";

/** 
 * 构造函数，用于内存池的初始化, 由于分配出去的每个Object还要包含Object size的信息
 * 所以需要多分配4 bytes用于记录Object size, 根据object_size + 4算出对齐后的对象大小，
 * 然后在计算 出实际的经过对齐后的内存块大小和每个内存块中实际存放的对象个数.
 * 
 * @param memory_pool_name              [in]   内存池的名称
 * @param object_size                   [in]   对象的大小, 缺省值为16 - 4. 这个object_size还
 *                                             需要经过字节对齐，默认为对齐到16字节, 如果对象的大小
 *                                             被Client端设为0, 那内部实现默认设为1.
 * @param num_objects_per_block         [in]   每个内存块分配的对象个数, 缺省值为256
 * @param allocator_type                [in]  allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
 *                                            UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 */
template < class Allocator, class Lock, class RecordPolicy > 
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::UPF_Fixed_Memory_Pool_T( 
    const char *          memory_pool_name, 
    size_t                object_size,
    size_t                num_objects_per_block,
    toplevel_allocator_t  allocator_type )

    : UPF_Memory_Pool_Base< Allocator, Lock, RecordPolicy >( memory_pool_name,
                                                             MAX_MEMORY_REQ_SIZE,
                                                             FIXED_MEMORY_POOL_TYPE,
                                                             allocator_type )
{
    // reset internal states.
    this->reset();

	if ( 0 == object_size )
	{
		object_size = 1;
	}

    // calculate aligned object size.
    m_align_object_size = UPF_MEMORY_ALIGN( SIZEOF_ID_ALLOC_SIZE_FIELD + object_size, 
                                            DEFAULT_OBJECT_SIZE );

    // calculate aligned block size.
    m_align_block_size  = UPF_MEMORY_ALIGN(
         m_align_object_size * num_objects_per_block, UPF_MEMORY_PAGE_SIZE );

    // calculate number of objects that really stored in a memory block.
    m_num_objects_per_block =
         ( m_align_block_size - BLOCK_OVERHEAD ) / m_align_object_size;

    
}

/** 
 * 析构函数，用于关闭内存池，释放所管理的内存.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::~UPF_Fixed_Memory_Pool_T( void )
{
    this->destroy_memory_blocks();
}

/** 
 * 从内存池中分配内存.
 * 
 * @param size      [in]  要申请的内存的大小，不能大于初始指定的object_size
 *                        经过对齐后的大小 - SIZEOF_ID_ALLOC_SIZE_FIELD(4), 参见
 *                        UPF_Fixed_Memory_Pool::UPF_Fixed_Memory_Pool(),
 *                        如果size为0, 那么默认分配1字节的内存.
 * 
 * @retval 指向内存的有效指针   分配内存成功
 * @retval 0                   分配内存失败
 */
template < class Allocator, class Lock, class RecordPolicy > 
void *
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::allocate( size_t size )
{
    void * result = 0;

    if ( 0 == size )
    {
        size = 1;
    }

    // size argument must less than or equal to m_align_object_size
    assert( size <= ( m_align_object_size - SIZEOF_ID_ALLOC_SIZE_FIELD ) );

    if ( size <= ( m_align_object_size - SIZEOF_ID_ALLOC_SIZE_FIELD )  )
    {
    
        UPF_Guard< Lock > guard( this->m_lock );

        // if m_free_list_head be 0, then create memory block.
        if ( 0 == m_free_list_head )
        {
            void * mem_block = this->create_memory_block();
    
            // if mem_block not 0, then add mem_block into memory block link list.
            if ( mem_block )
            {
                this->set_next_block_ptr( mem_block, m_first_block );
    
                m_first_block    = mem_block;
                m_free_list_head = mem_block;
            }         
        }
    
        if ( m_free_list_head )
        {
            // get next object ptr from free object link list
            void* next_object_ptr = this->get_next_object_ptr( m_free_list_head );
    
            // set allocated memory size.
            *( reinterpret_cast< size_t * >( m_free_list_head ) ) =
                UPF_MMORY_POOL_COMBINE_ID_SIZE( this->m_memory_pool_id, size );
    
            // set return memory ptr.
            result = reinterpret_cast< char * >( m_free_list_head ) +
                     SIZEOF_ID_ALLOC_SIZE_FIELD;
    
            // set m_free_list_head be next object ptr.
            m_free_list_head = next_object_ptr;
    
            // set allocated memory size.
            m_recorder.set_allocated_memory_size(
                m_recorder.get_allocated_memory_size() + size );
    
            // set wasted memory size.
            m_recorder.set_wasted_memory_size(
                m_recorder.get_wasted_memory_size() +
                 ( m_align_object_size - size ) );
        }
    }

    return result;
}

/** 
 *  释放内存, 把ptr所指向的内存归还给内存池, ptr所指向的内存必须为调用
 *  UPF_Fixed_Memory_Pool::allocate所申请的内存.
 * 
 * @param ptr        [in] 调用UPF_Fixed_Memory_Pool::allocate所申请的内存
 * @param size       [in] 内存大小
 */
template < class Allocator, class Lock, class RecordPolicy > 
void
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::deallocate( void* ptr, 
                                                                    size_t /* size */ )
{
    if ( ptr != 0 )
    {
        // offset back 4 bytes from ptr for get allocated size.
        size_t * original_ptr = reinterpret_cast< size_t * >(
            static_cast< char * >( ptr ) - SIZEOF_ID_ALLOC_SIZE_FIELD );

        // if ptr equal to m_free_list_head, then ptr be freed twice, 
        // this is error!!
        assert( m_free_list_head != original_ptr );

        size_t allocated_size = UPF_MEMORY_POOL_ALLOC_SIZE( *original_ptr );

        UPF_Guard< Lock > guard( this->m_lock );

        // add ptr into free object link list, then set m_free_list_head be ptr.
        this->set_next_object_ptr( original_ptr,  m_free_list_head );
        m_free_list_head = original_ptr;

        // set allocated memory size.
        m_recorder.set_allocated_memory_size(
            m_recorder.get_allocated_memory_size() - allocated_size );

        // set wasted memory size.
        m_recorder.set_wasted_memory_size(
            m_recorder.get_wasted_memory_size() -
             ( m_align_object_size - allocated_size ) );
    }

    return;
}

/** 
 * 创建一个内存块, 并把内存块中的每个对象链接起来.
 *  
 * @retval 新创建的内存块指针   内存块创建成功
 *         0                    内存块创建失败
 */
template < class Allocator, class Lock, class RecordPolicy >
void * 
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::create_memory_block( void )
{
    void * mem_block_base = Allocator::allocate( m_align_block_size,
                                                 m_allocator_type );

    if ( mem_block_base )
    {
        unsigned char * object_ptr = 
            static_cast< unsigned char * >( mem_block_base );

        for ( int i = 0; i < m_num_objects_per_block; ++i )
        {
            if ( i != m_num_objects_per_block - 1 )
            {
                this->set_next_object_ptr( object_ptr, 
                                           object_ptr + m_align_object_size );
            }
            else
            {
                this->set_next_object_ptr( object_ptr, 0 );
            }

            object_ptr += m_align_object_size;
        }

        // set managed memory size
        m_recorder.set_managed_memory_size(
            m_recorder.get_managed_memory_size() + m_align_block_size );
    }

    return mem_block_base;
}

/** 
 * 取得内存块中对象的next ptr
 *  
 * @param object_ptr    [in]   当前对象的ptr.
 * 
 * @return 该对象的next ptr
 */
template < class Allocator, class Lock, class RecordPolicy >
inline void *
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::get_next_object_ptr( 
    void * object_ptr )
{
    assert( object_ptr );

     // the next ptr field of the object located at first four bytes of the object.
    int * object_next_ptr_field = static_cast< int * >( object_ptr );
    
    return reinterpret_cast< void * >( *object_next_ptr_field );
}



/** 
 * 设置内存块中每个对象的next ptr.
 * 
 * @param object_ptr           [in, out]  当前对象的ptr.
 * @param next_object_ptr      [in]       下一对象的ptr, 可以是0.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
inline void
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::set_next_object_ptr( 
    void * object_ptr, void * next_object_ptr )
{
    assert( object_ptr );

    // the next ptr field of the object located at first four bytes of the object.
    int * object_next_ptr_field = static_cast< int * >( object_ptr );

    *object_next_ptr_field = reinterpret_cast< int >( next_object_ptr );

    return;
}

/** 
 * 取得内存块的next ptr
 *  
 * @param block_ptr    [in]   内存块的ptr.
 * 
 * @return 该内存块的next ptr
 */
template < class Allocator, class Lock, class RecordPolicy >
inline void *
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::get_next_block_ptr( void * block_ptr )
{
    assert( block_ptr );

    // the next ptr field of the block located at last four bytes of the block.
    int * block_next_ptr_field = reinterpret_cast< int * >(
        static_cast< unsigned char * >( block_ptr ) + m_align_block_size - BLOCK_OVERHEAD );

    return reinterpret_cast< void * >( *block_next_ptr_field );

}

/** 
 * 设置内存块的next ptr.
 * 
 * @param block_ptr           [in, out]  当前内存块的ptr.
 * @param next_block_ptr      [in]       下一内存块的ptr, 可以是0.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
inline void
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::set_next_block_ptr( 
    void * block_ptr, void * next_block_ptr )
{
    assert( block_ptr );

    // the next ptr field of the block located at last four bytes of the block.

    int * block_next_ptr_field = reinterpret_cast< int * >(
        static_cast< unsigned char* >( block_ptr ) + m_align_block_size - BLOCK_OVERHEAD );

    *block_next_ptr_field = reinterpret_cast< int >( next_block_ptr );

    return;
}

/** 
 * 释放内存池中的所有内存块占用的内存, 并重置内部的状态.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
inline void
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::destroy_memory_blocks( void )
{
    // before call close, if have some memory object that haven't freed, then error.

    UPF_Guard< Lock > guard( this->m_lock );

    void * next_block    = 0;
    void * current_block = m_first_block;

    while ( current_block )
    {
        next_block = this->get_next_block_ptr( current_block );

        Allocator::deallocate( current_block,
                               m_align_block_size,
                               m_allocator_type );

        current_block = next_block;
    }

    this->reset();
}

/** 
 * 重置内部的状态
 * 
 */
template < class Allocator, class Lock, class RecordPolicy >
void
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::reset( void )
{
    m_first_block          = 0;
    m_free_list_head       = 0;

    this->reset_counter();
}

/** 
 * 从固定大小的内存池中重新分配内存, 并保留原有的内容, 当分配内存时, 如果在原来的位置
 * 还有足够的内存可以分配, 则进行就地分配.
 * 
 * 有如下三种情况:
 *      -# 如果ptr为0, 则调用allocate进行分配内存.
 *      -# 如果ptr不为0, new_size为0, 则调用deallocate释放内存.
 *      -# 在ptr不为0, new_size不为0的情况下, 因为是固定大小的内存池, 所以只能就地\n
 *         分配, 如果新的内存大小超过了该内存池创建时所指定的对象大小, 则分配失败.
 * 
 * @note 如果新分配内存失败, 原有内存并不释放, 原来的指针仍然有效.
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
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::reallocate( 
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
 
    UPF_Guard< Lock > guard( this->m_lock );

    size_t * original_ptr = reinterpret_cast< size_t * >(
        static_cast< char * >( ptr ) - SIZEOF_ID_ALLOC_SIZE_FIELD );

    size_t old_alloc_size = UPF_MEMORY_POOL_ALLOC_SIZE( *original_ptr );

    // if old alloc size equal to new size, then return ptr directly.
    if ( old_alloc_size == new_size )
    {
        return ptr;
    }

    assert( new_size <= ( m_align_object_size - SIZEOF_ID_ALLOC_SIZE_FIELD ) );

    if ( new_size <= ( m_align_object_size - SIZEOF_ID_ALLOC_SIZE_FIELD ) )
    {
        // update size info
        *original_ptr = UPF_MMORY_POOL_COMBINE_ID_SIZE( 
                                    this->m_memory_pool_id, new_size );

        // update allocated memory size.
        m_recorder.set_allocated_memory_size(
            m_recorder.get_allocated_memory_size() - old_alloc_size + new_size );
    
        // update wasted memory size.
        m_recorder.set_wasted_memory_size(
            m_recorder.get_wasted_memory_size() + old_alloc_size - new_size );

        // set new ptr be old ptr directly.
        new_ptr = ptr;
    }

    return new_ptr;
}