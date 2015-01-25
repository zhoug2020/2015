/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Common_Memory_Pool2_T.cpp                                                              *
*       CREATE DATE     : 2007-11-14                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ɱ�����С���ڴ��ʵ��![CN]                                                          *
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
 * ��ʼ���ڲ�״̬.
 * 
 * @param memory_pool_name              [in]  �ڴ������
 * @param allocated_memory_limit        [in]  Client�˷����ڴ������ֵ
 * @param allocator_type                [in]  allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
 *                                            UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
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
 * ����ڴ���ڲ�����������ڴ�
 * 
 * @param memory_pool_name    [in]  �ڴ������
 */
template < class Allocator, class Lock, class RecordPolicy >
UPF_Common_Memory_Pool2_T< Allocator, Lock, RecordPolicy >::~UPF_Common_Memory_Pool2_T( void )
{
    this->destroy_memory_space();
}

/** 
 * ���ڴ���з����ڴ�.
 * 
 * @param size       [in]  Ҫ������ڴ�Ĵ�С, ���sizeΪ0, ��ôĬ�Ϸ���1�ֽڵ��ڴ�.
 * 
 * @retval ָ���ڴ����Чָ��   �����ڴ�ɹ�
 * @retval 0                    �����ڴ�ʧ��
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
 * ��ϵͳ�����������ڴ��, ���ӵ��ڲ���small block link list.
 * 
 * @param size          [in]     ������ڴ���С
 * @param free_index    [in]     ��������
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
 * ��ϵͳ�����������ڴ�, �����ǰ��������ڴ�����ʣ����ڴ�, ����ⲿ��ʣ���
 * �ڴ���ӵ����п��б��У�Ȼ���ϵͳ������һ����ڴ�, ͨ�������ڴ�ӦΪҳ���С
 * ��������������СֵΪ����ҳ���С.
 * 
 * ���ϵͳû�п����ڴ���, ��ӿ��п��б���Ѱ��, ����ҵ���, �ͷ���������п�,
 * ���򷵻�0.
 * 
 * @param size     [in]   Ҫ��ȡ���ڴ���С, ӦΪ8�ı���.
 * 
 * @retval ָ���ڴ����Чָ��   ��ȡ�ڴ�ɹ�
 * @retval 0                    ��ȡ�ڴ�ʧ��
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
 * ��m_free_small_block_list��ȡ�ÿ��е�small block.
 * 
 * @param size             [in]    ������ڴ��С
 * @param bytes_to_get     [out]   ʵ�ʵõ����ڴ��С
 * 
 * @retval һ����Ч���ڴ�ָ��   �ҵ�һ�����е��ڴ��
 * @retval 0                    û���ҵ����е��ڴ��
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
 * ����ڴ���ڲ�����������ڴ�, �������ڲ���״̬.
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
 *  �ͷ��ڴ�, ��ptr��ָ����ڴ�黹���ڴ��, ptr��ָ����ڴ����Ϊ����
 *  UPF_Common_Memory_Pool2::allocate��������ڴ�.
 * 
 * @param ptr        [in] ����UPF_Common_Memory_Pool2::allocate��������ڴ�
 * @param size       [in] �ڴ��С
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
 * �����ڲ�״̬
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
 * ���ڴ�������·����ڴ�, ������ԭ�е����ݣ��������ڴ�ʱ�������ԭ����λ��
 * �����㹻���ڴ���Է���, ����о͵ط���.
 * 
 * �������������:
 *      -# ���ptrΪ0, �����allocate���з����ڴ�.
 *      -# ���ptr��Ϊ0, new_sizeΪ0, �����deallocate�ͷ��ڴ�.
 *      -# ���ptr��Ϊ0, new_size��Ϊ0, ��������·����ڴ����, �����ԭ\n
 *         λ���п��õ��ڴ棬����о͵ط��䣬�������·���һ���ڴ�, ����ԭ\n
 *         ʼ�ڴ�����ݿ�������������ڴ���, ���ͷ�ԭ�����ڴ�.
 * 
 * @note ����·����ڴ�ʧ��, ԭ���ڴ沢���ͷ�, ԭ����ָ����Ȼ��Ч.
 * 
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
 * ������allocate�����ڴ�ʱ��m_start_avail_memory��ָ����ڴ����ʣ����ڴ治��
 * ��������, ������һ�����ڴ�����������ǰ, ����place_remainder_memory_on_block_list
 * �����ⲿ��ʣ����ڴ���䵽���е��ڴ���б���.
 *
 * ��Ϊ�ڴ�ص���С��������Ϊ8, ����bytes_left�϶�Ϊ8�ı���, ���ⲿ��ʣ����ڴ����ȷ��
 * �ķ��䵽���е��ڴ���б�����Ӧ��λ����.
 *
 * @param bytes_left       [in]       m_start_avail_memory��ָ����ڴ����ʣ����ڴ��ֽ���
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

