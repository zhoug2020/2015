/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Base_T.h                                                                   *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ��ʵ�ֵĻ��ඨ��![CN]                                                              *
*********************************************************************************************************************/
#ifndef UPF_MEMORY_POOL_BASE_T_H
#define UPF_MEMORY_POOL_BASE_T_H

#include <malloc.h>   /* for malloc, free, realloc */
#include <stdio.h>    /* for _snprintf */

#include "UPF_IMemory_Pool.h"
#include "UPF_IMemory_Pool_Info.h"
#include "UPF_Thread_Sync.h"
#include "UPF_Memory_Pool_Define.h"   /* for MAX_MEMORY_REQ_SIZE */
#include "UPF_Memory_Pool_ID_Manager.h"
#include "UPF_Memory_Pool_Util.h"

/** 
 * @class  MallocAllocator 
 *  
 * @brief һ���ڴ�������������ڴ�ʹ��malloc, �ͷ��ڴ�ʹ��free. 
 * 
 */
struct UPF_Malloc_Allocator
{
    static void * allocate( size_t size, toplevel_allocator_t /* allocator_type */)
    {
        return ::malloc( size );
    }

    static void deallocate( void * ptr, 
                            size_t /* size */, 
                            toplevel_allocator_t /* allocator_type */ )
    {
        ::free( ptr );
    }

    static void * reallocate( void * ptr, 
                              size_t new_size,
                              toplevel_allocator_t /* allocator_type */ )
    {
        return ::realloc( ptr, new_size );
    }

};

struct UPF_Heap_Allocator_Impl
{
	UPF_Heap_Allocator_Impl()
	{
		m_heap_handle = 0;
		m_heap_handle = HeapCreate(0, 1024*1024, 0);
	}
	void * allocate( size_t size )
	{
		return HeapAlloc(m_heap_handle, 0, size);
		
	}
	void deallocate( void * ptr )
	{
		HeapFree(m_heap_handle, 0, ptr);
		
	}
	void * reallocate( void * ptr, size_t new_size )
	{
		return HeapReAlloc(m_heap_handle, 0, ptr, new_size );
	}

static	UPF_Heap_Allocator_Impl * instance()
	{
		static UPF_Heap_Allocator_Impl heap_allocator;
		return &heap_allocator;
	}

	 HANDLE  m_heap_handle;

};

struct UPF_Heap_Allocator
{
    static void * allocate( size_t size, toplevel_allocator_t /* allocator_type */ )
    {
	return	UPF_Heap_Allocator_Impl::instance()->allocate(size);
    }

    static void deallocate( void *    ptr, 
                            size_t /* size */, 
                            toplevel_allocator_t /* allocator_type */ )
    {
        UPF_Heap_Allocator_Impl::instance()->deallocate(ptr);
    }

    static void * reallocate( void *                  ptr, 
                              size_t                  new_size, 
                              toplevel_allocator_t /* allocator_type */ )
    {
      return   UPF_Heap_Allocator_Impl::instance()->reallocate(ptr,new_size);
    }

};

 
/** 
 * @class UPF_Memory_Pool_Base 
 *  
 * @brief  �ڴ��ʵ�ֵĻ��ඨ��, ʵ����UPF_IMemory_Pool�ӿں�
 * UPF_IMemory_Pool_Info�����з���, �����ڴ�ص�ʵ�ֿ��Լ̳������, 
 * ֻʵ���Լ���Ҫ�ķ���,  ����Ҳ����ֱ��ʵ��UPF_IMemory_Pool�ӿں�UPF_IMemory_Pool_Info.
 * 
 * Allocator     �ڴ�����������ڷ����ڴ棬�ͷ��ڴ�, ��Ҫ֧�ֵĺ����ӿ�����:
 *               static void * allocate( size_t size );
 *               static void deallocate( void * ptr );
 * 
 * Lock          �߳����������߳�ͬ��, ��Ҫ֧�ֵĺ����ӿ�����:
 *               void lock( void );
 *               void unlock( void );
 * 
 * RecordPolicy  һ��������, ���ڼ�¼�ڴ�صķ�����Ϣ, ��Ҫ֧�ֵĺ����ӿ�����:
 * 
 *                void   set_memory_pool_name( const char * name );
 *                void   set_meomry_pool_type( const char * type );
 * 
 *                size_t get_managed_memory_size( void ) const;
 *                void   set_managed_memory_size( size_t size );
 * 
 *                size_t get_allocated_memory_size( void ) const;
 *                void set_allocated_memory_size( size_t size );
 * 
 *                size_t get_allocated_memory_peak( void ) const;
 * 
 *                size_t get_wasted_memory_size( void ) const;
 *                void set_wasted_memory_size( size_t size );
 * 
 *                size_t get_allocated_memory_limit( void ) const;
 *                void set_allocated_memory_limit( size_t size );
 * 
 * ���е�get_allocated_memory_peak����Ϊ����ĳһʱ��Client����ʹ�õ��ڴ�ķ�ֵ,
 * ���ֵ��set_allocated_memory_sizeʵ�����������, Ӧ���ڼ����ֶ�,
 * ����RecordPolicy�����ṩset_allocated_memory_leak����.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Memory_Pool_Base : public UPF_IMemory_Pool, 
                             public UPF_IMemory_Pool_Info
{
public:

    /**
     * ��ʼ�������������ڴ�ص�����, ����, Client�������ڴ����ֵ������.
     *  
     * @param memory_pool_name         [in] �ڴ����.
     * @param allocated_memory_limit   [in] Client�������ڴ������ֵ.
     * @param memory_pool_type         [in] �ڴ������, ȱʡΪ�մ�.
     * @param allocator_type           [in] allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
     *                                      UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
     */
    UPF_Memory_Pool_Base( const char *          memory_pool_name,
                          size_t                allocated_memory_limit = MAX_MEMORY_REQ_SIZE,
                          const char *          memory_pool_type = "",
                          toplevel_allocator_t  allocator_type = UPF_TOPLEVEL_PERMANENT_ALLOCATOR )

        : m_memory_pool_name( memory_pool_name ),
          m_memory_pool_type( memory_pool_type ),
          m_allocator_type( allocator_type )
    {
        // record memory pool's name and type.
        if ( allocator_type >= UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES )
        {            
            // if allocator_type is greatter than UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES
            // then the memory pool was consider as transaction memory pool.
            // then memory pool name will changed.
            char new_memory_pool_name[ MAX_POOL_NAME_LENGTH ];

           _snprintf( new_memory_pool_name,
                      sizeof( new_memory_pool_name ),
                      "%s_trans",
                      m_memory_pool_name );

           new_memory_pool_name[ MAX_POOL_NAME_LENGTH - 1 ] = '\0';

           m_recorder.set_memory_pool_name( new_memory_pool_name );
        }
        else
        {
            m_recorder.set_memory_pool_name( m_memory_pool_name );
        }

        m_recorder.set_memory_pool_type( m_memory_pool_type );

        // record allocated memory limit value.
        m_recorder.set_allocated_memory_limit( allocated_memory_limit );

        m_memory_pool_id = UPF_Memory_Pool_ID_Manager::get_id();
    
        if ( UPF_Memory_Pool_ID_Manager::INVALID_ID == m_memory_pool_id )
        {
            UPF_Memory_Pool_Util::report( "%s memory pool id acquire failed.", memory_pool_name ); 
        }
    }

    /** 
     * ��������
     * 
     */
    ~UPF_Memory_Pool_Base( void )
    {
        UPF_Memory_Pool_ID_Manager::release_id( m_memory_pool_id );
    }

public:
   enum  { 
        /* ÿ�������ȥ���ڴ��ļ�¼��Ϣ�ֶεĴ�С, ͨ��Ϊ4. */
        SIZEOF_ID_ALLOC_SIZE_FIELD = sizeof( size_t ) 
    };

    /************************ Implements UPF_IMemory_Pool ********************/

    /** 
     * ���ڴ���з����ڴ�.
     * 
     * @param size    [in] ��Ҫ������ڴ�Ĵ�С.
     * 
     * @retval һ��ָ����������ڴ��ָ��    �����ڴ�ɹ�. 
     * @retval 0                             �����ڴ�ʧ��. 
     */
    virtual void * allocate( size_t size )
    {
        return Allocator::allocate( size, m_allocator_type );
    }

    /** 
     * allocate�ĵ��԰汾.
     * 
     * �����ʵ��ֻ�ǵ���Allocator::allocate�������ڴ�, ����ĵ��԰汾ʵ����������
     * ʵ�֡�
     * 
     * @param size          [in] ��Ҫ������ڴ�Ĵ�С.
     * @param file_name     [in] ����allocate_debug��Դ�ļ���
     * @param line_no       [in] ����allocate_debug���к�
     * 
     * @retval һ��ָ����������ڴ��ָ��    �����ڴ�ɹ�. 
     * @retval 0                             �����ڴ�ʧ��. 
     */
    virtual void * allocate_debug( size_t               size, 
                                   const char *         file_name,
                                   int                  line_no )
    {
        return this->allocate( size );
    }

    /** 
     * �ͷ��ڴ�.
     * 
     * @param ptr     [in] ����allocate��������ڴ��ָ��.
     * @param size    [in] ptr��ָ����ڴ�Ĵ�С.
     */
    virtual void deallocate( void *               ptr, 
                             size_t               size )
    {        
        Allocator::deallocate( ptr, size, m_allocator_type );
    }

    /** 
     * ���ڴ�������·����ڴ�
     * 
     * @param ptr           [in] ����allocate��reallocate��������ڴ�.
     * @param new_size      [in] �µ��ڴ��С
     * 
     * @retval һ��ָ����������ڴ��ָ��    ���������ڴ�ɹ�. 
     * @retval 0                             ���������ڴ�ʧ��.
     * 
     */
    virtual void * reallocate( void *               ptr,
                               size_t               new_size )
    {
        return Allocator::reallocate( ptr, new_size, m_allocator_type );
    }

    /** 
     * reallocate�ĵ��԰汾, ��Ҫ���ڸ��ٵ�ǰ�����ڴ��λ�ã���������
     * �ڴ�й¶�ļ��.
     * 
     * @param ptr           [in] ����allocate��reallocate��������ڴ�.
     * @param new_size      [in] �µ��ڴ��С
     * @param file_name     [in] ����reallocate_debug��Դ�ļ���
     * @param line_no       [in] ����reallocate_debug���к�
     * 
     * @retval һ��ָ����������ڴ��ָ��    ���������ڴ�ɹ�. 
     * @retval 0                             ���������ڴ�ʧ��.
     * 
     */
    virtual void * reallocate_debug( void *               ptr,
                                     size_t               new_size,
                                     const char *         file_name,
                                     int                  line_no )
    {
        return this->reallocate( ptr,  new_size );
    }

    /** 
     * �ж��ڴ���Ƿ�������ڴ����, Ĭ��ʵ��Ϊ����true.
     * 
     * @param ptr     [in] һ��ָ���ڴ��ĵ�ַ.
     * 
     * @retval true  �ڴ������ڸ��ڴ����.
     * @retval false �ڴ�鲻�����ڸ��ڴ����.
     */
    virtual bool is_memory_block_exist( void * ptr )
    {
        bool is_exist = false;
    
        if ( ptr != 0 )
        {
            size_t * id_size_ptr = reinterpret_cast< size_t * >(
                static_cast< char * >( ptr ) - SIZEOF_ID_ALLOC_SIZE_FIELD );
    
            size_t memory_pool_id = UPF_MEMORY_POOL_ID( *id_size_ptr );
    
            if ( memory_pool_id == m_memory_pool_id )
            {
                is_exist = true;
            }
        }
    
        return is_exist;
    }

    /** 
     * ��ȡָ��UPF_IMemory_Pool_Infoʵ����ָ��.
     * 
     * 
     * @return ָ��UPF_IMemory_Pool_Infoʵ����ָ��.
     */
    UPF_IMemory_Pool_Info * get_memory_pool_info( void )
    {
        return static_cast< UPF_IMemory_Pool_Info * >( this );
    }

    /******************* Implements UPF_IMemory_Pool_Info ********************/

    /** 
     * ����UPF_IMemory_Pool�ӿ�ָ��.
     * 
     * @return ʵ��UPF_IMemory_Pool�ӿڵ�ָ��.
     */
    virtual UPF_IMemory_Pool * get_memory_pool( void )
    {
        return static_cast< UPF_IMemory_Pool * >( this );
    }

    /**
     * ��ȡ�ڴ�ص�����.
     * 
     * @return �ڴ�ص�����.
     */
    virtual const char * get_memory_pool_name( void ) const
    {
        return m_memory_pool_name;
    }

    /**
     * ��ȡ�ڴ�ص����ͣ���ǰ֧�����֣��̶���С�ģ��ɱ��С��.
     * 
     * @return �ڴ�ص�����.
     */
    virtual const char * get_memory_pool_type( void ) const
    {
        return m_memory_pool_type;
    }

    /** 
     * ����ڴ����������ڴ��С, ���ڴ�ش�ϵͳ������������ڴ�Ĵ�С.
     *  
     * @return �ڴ����������ڴ��С
     */
    virtual size_t get_managed_memory_size( void ) const
    {
        UPF_Guard< Lock > guard( this->m_lock );

        return m_recorder.get_managed_memory_size();
    }

    /** 
     * ����ڴ���е�ǰ�ѷ����Client�˵��ڴ��С, ���ֵ�ܱ�ʾ��ĳһʱ��Client��
     * ��ʹ�õ��ڴ��С.
     * 
     * @return  �����Client�˵��ڴ��С. 
     */
    virtual size_t get_allocated_memory_size( void ) const
    {
        UPF_Guard< Lock > guard( this->m_lock );

        return m_recorder.get_allocated_memory_size();
    }


    /** 
     *  ��ȡ�����Client�˵��ڴ��С�ķ�ֵ, ���ֵ�ܱ�ʾ��ĳһʱ��Client��ʹ�õ�
     *  �ڴ��С�����ֵ.
     * 
     * @return �����Client�˵��ڴ��С�ķ�ֵ
     */
    virtual size_t get_allocated_memory_peak( void ) const
    {
        UPF_Guard< Lock > guard( this->m_lock );

        return m_recorder.get_allocated_memory_peak();
    }

    /** 
     * ��ȡ�ڴ�����˷ѵ��ڴ��С, ��Ϊ�ڴ���е�ÿ���ڵ��С���ǰ���һ����������
     * �����, ����ÿ�η���������˷ѵ��ڴ�, ʹ������ӿں������Ի�ȡĳһʱ���ڴ��
     * �����˷ѵ��ڴ��С.
     * 
     * @return �ڴ�����˷ѵ��ڴ��С
     */
    virtual size_t get_wasted_memory_size( void ) const
    {
        UPF_Guard< Lock > guard( this->m_lock );

        return m_recorder.get_wasted_memory_size();
    }


    virtual size_t get_allocated_memory_limit( void ) const
    {
        UPF_Guard< Lock > guard( this->m_lock );

        return m_recorder.get_allocated_memory_limit();
    }

    virtual toplevel_allocator_t  get_allocator_type( void ) const
    {
        return m_allocator_type;
    }

protected:
    /** 
     * ���ü�����Ϊ0.
     * 
     */
    void reset_counter( void )
    {
        m_recorder.set_managed_memory_size( 0 );
        m_recorder.set_allocated_memory_size( 0 );
        m_recorder.set_wasted_memory_size( 0 );
    }

protected:
    /// �߳���, �����߳�ͬ��.
    mutable Lock          m_lock;

    /// �ڴ������.
    const char *          m_memory_pool_name;

    /// �ڴ������
    const char *          m_memory_pool_type;

    /// ���ڱ����ڴ�ص�id
    size_t                m_memory_pool_id;

    /// ��¼�ڴ�صķ�����Ϣ.
    RecordPolicy          m_recorder;

    /// ��¼allocator������.
    toplevel_allocator_t  m_allocator_type;

private:
    /// ��ֹ�����͸�ֵ.
    UPF_Memory_Pool_Base( const UPF_Memory_Pool_Base< Allocator,  
                                                      Lock, 
                                                      RecordPolicy >& );

    UPF_Memory_Pool_Base< Allocator,  Lock, RecordPolicy >& operator =(
        const UPF_Memory_Pool_Base< Allocator,  Lock, RecordPolicy >& );

};

#endif /* UPF_MEMORY_POOL_BASE_T_H */

