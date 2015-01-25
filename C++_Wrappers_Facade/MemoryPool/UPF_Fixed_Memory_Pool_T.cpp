/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Fixed_Memory_Pool_T.cpp                                                                *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�̶������С���ڴ�ص�ʵ��![CN]                                                        *
*********************************************************************************************************************/
#include <assert.h>                      /* for assert */ 
#include "UPF_Fixed_Memory_Pool_T.h"

// Constant definitions.
const char * const FIXED_MEMORY_POOL_TYPE = "Fixed";

/** 
 * ���캯���������ڴ�صĳ�ʼ��, ���ڷ����ȥ��ÿ��Object��Ҫ����Object size����Ϣ
 * ������Ҫ�����4 bytes���ڼ�¼Object size, ����object_size + 4��������Ķ����С��
 * Ȼ���ڼ��� ��ʵ�ʵľ����������ڴ���С��ÿ���ڴ����ʵ�ʴ�ŵĶ������.
 * 
 * @param memory_pool_name              [in]   �ڴ�ص�����
 * @param object_size                   [in]   ����Ĵ�С, ȱʡֵΪ16 - 4. ���object_size��
 *                                             ��Ҫ�����ֽڶ��룬Ĭ��Ϊ���뵽16�ֽ�, �������Ĵ�С
 *                                             ��Client����Ϊ0, ���ڲ�ʵ��Ĭ����Ϊ1.
 * @param num_objects_per_block         [in]   ÿ���ڴ�����Ķ������, ȱʡֵΪ256
 * @param allocator_type                [in]  allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
 *                                            UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
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
 * �������������ڹر��ڴ�أ��ͷ���������ڴ�.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
UPF_Fixed_Memory_Pool_T< Allocator, Lock, RecordPolicy >::~UPF_Fixed_Memory_Pool_T( void )
{
    this->destroy_memory_blocks();
}

/** 
 * ���ڴ���з����ڴ�.
 * 
 * @param size      [in]  Ҫ������ڴ�Ĵ�С�����ܴ��ڳ�ʼָ����object_size
 *                        ���������Ĵ�С - SIZEOF_ID_ALLOC_SIZE_FIELD(4), �μ�
 *                        UPF_Fixed_Memory_Pool::UPF_Fixed_Memory_Pool(),
 *                        ���sizeΪ0, ��ôĬ�Ϸ���1�ֽڵ��ڴ�.
 * 
 * @retval ָ���ڴ����Чָ��   �����ڴ�ɹ�
 * @retval 0                   �����ڴ�ʧ��
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
 *  �ͷ��ڴ�, ��ptr��ָ����ڴ�黹���ڴ��, ptr��ָ����ڴ����Ϊ����
 *  UPF_Fixed_Memory_Pool::allocate��������ڴ�.
 * 
 * @param ptr        [in] ����UPF_Fixed_Memory_Pool::allocate��������ڴ�
 * @param size       [in] �ڴ��С
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
 * ����һ���ڴ��, �����ڴ���е�ÿ��������������.
 *  
 * @retval �´������ڴ��ָ��   �ڴ�鴴���ɹ�
 *         0                    �ڴ�鴴��ʧ��
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
 * ȡ���ڴ���ж����next ptr
 *  
 * @param object_ptr    [in]   ��ǰ�����ptr.
 * 
 * @return �ö����next ptr
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
 * �����ڴ����ÿ�������next ptr.
 * 
 * @param object_ptr           [in, out]  ��ǰ�����ptr.
 * @param next_object_ptr      [in]       ��һ�����ptr, ������0.
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
 * ȡ���ڴ���next ptr
 *  
 * @param block_ptr    [in]   �ڴ���ptr.
 * 
 * @return ���ڴ���next ptr
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
 * �����ڴ���next ptr.
 * 
 * @param block_ptr           [in, out]  ��ǰ�ڴ���ptr.
 * @param next_block_ptr      [in]       ��һ�ڴ���ptr, ������0.
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
 * �ͷ��ڴ���е������ڴ��ռ�õ��ڴ�, �������ڲ���״̬.
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
 * �����ڲ���״̬
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
 * �ӹ̶���С���ڴ�������·����ڴ�, ������ԭ�е�����, �������ڴ�ʱ, �����ԭ����λ��
 * �����㹻���ڴ���Է���, ����о͵ط���.
 * 
 * �������������:
 *      -# ���ptrΪ0, �����allocate���з����ڴ�.
 *      -# ���ptr��Ϊ0, new_sizeΪ0, �����deallocate�ͷ��ڴ�.
 *      -# ��ptr��Ϊ0, new_size��Ϊ0�������, ��Ϊ�ǹ̶���С���ڴ��, ����ֻ�ܾ͵�\n
 *         ����, ����µ��ڴ��С�����˸��ڴ�ش���ʱ��ָ���Ķ����С, �����ʧ��.
 * 
 * @note ����·����ڴ�ʧ��, ԭ���ڴ沢���ͷ�, ԭ����ָ����Ȼ��Ч.
 *  
 * @param ptr           [in] ����allocate��reallocate��������ڴ�.
 * @param new_size      [in] �µ��ڴ��С
 * 
 * @retval һ��ָ����������ڴ��ָ��    ���������ڴ�ɹ�. 
 * @retval 0                             ���������ڴ�ʧ��, ptr��ָ����ڴ�\n
 *                                       �����ͷ�, ptr��Ȼ��Ч.
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