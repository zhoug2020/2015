/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Base_T.h                                                                   *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池实现的基类定义![CN]                                                              *
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
 * @brief 一个内存分配器，分配内存使用malloc, 释放内存使用free. 
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
 * @brief  内存池实现的基类定义, 实现了UPF_IMemory_Pool接口和
 * UPF_IMemory_Pool_Info的所有方法, 其他内存池的实现可以继承这个类, 
 * 只实现自己必要的方法,  或者也可以直接实现UPF_IMemory_Pool接口和UPF_IMemory_Pool_Info.
 * 
 * Allocator     内存分配器，用于分配内存，释放内存, 需要支持的函数接口如下:
 *               static void * allocate( size_t size );
 *               static void deallocate( void * ptr );
 * 
 * Lock          线程锁，用于线程同步, 需要支持的函数接口如下:
 *               void lock( void );
 *               void unlock( void );
 * 
 * RecordPolicy  一个策略类, 用于记录内存池的分配信息, 需要支持的函数接口如下:
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
 * 其中的get_allocated_memory_peak方法为返回某一时刻Client端所使用的内存的峰值,
 * 这个值由set_allocated_memory_size实现来计算而得, 应属于计算字段,
 * 所以RecordPolicy无需提供set_allocated_memory_leak方法.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Memory_Pool_Base : public UPF_IMemory_Pool, 
                             public UPF_IMemory_Pool_Info
{
public:

    /**
     * 初始化操作，设置内存池的名字, 类型, Client端申请内存最大值的限制.
     *  
     * @param memory_pool_name         [in] 内存池名.
     * @param allocated_memory_limit   [in] Client端申请内存的上限值.
     * @param memory_pool_type         [in] 内存池类型, 缺省为空串.
     * @param allocator_type           [in] allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
     *                                      UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
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
     * 析构函数
     * 
     */
    ~UPF_Memory_Pool_Base( void )
    {
        UPF_Memory_Pool_ID_Manager::release_id( m_memory_pool_id );
    }

public:
   enum  { 
        /* 每个分配出去的内存块的记录信息字段的大小, 通常为4. */
        SIZEOF_ID_ALLOC_SIZE_FIELD = sizeof( size_t ) 
    };

    /************************ Implements UPF_IMemory_Pool ********************/

    /** 
     * 从内存池中分配内存.
     * 
     * @param size    [in] 需要分配的内存的大小.
     * 
     * @retval 一个指向所申请的内存的指针    申请内存成功. 
     * @retval 0                             申请内存失败. 
     */
    virtual void * allocate( size_t size )
    {
        return Allocator::allocate( size, m_allocator_type );
    }

    /** 
     * allocate的调试版本.
     * 
     * 基类的实现只是调用Allocator::allocate来申请内存, 具体的调试版本实现由子类来
     * 实现。
     * 
     * @param size          [in] 需要分配的内存的大小.
     * @param file_name     [in] 调用allocate_debug的源文件名
     * @param line_no       [in] 调用allocate_debug的行号
     * 
     * @retval 一个指向所申请的内存的指针    申请内存成功. 
     * @retval 0                             申请内存失败. 
     */
    virtual void * allocate_debug( size_t               size, 
                                   const char *         file_name,
                                   int                  line_no )
    {
        return this->allocate( size );
    }

    /** 
     * 释放内存.
     * 
     * @param ptr     [in] 调用allocate所申请的内存的指针.
     * @param size    [in] ptr所指向的内存的大小.
     */
    virtual void deallocate( void *               ptr, 
                             size_t               size )
    {        
        Allocator::deallocate( ptr, size, m_allocator_type );
    }

    /** 
     * 从内存池中重新分配内存
     * 
     * @param ptr           [in] 调用allocate或reallocate所申请的内存.
     * @param new_size      [in] 新的内存大小
     * 
     * @retval 一个指向所申请的内存的指针    重新申请内存成功. 
     * @retval 0                             重新申请内存失败.
     * 
     */
    virtual void * reallocate( void *               ptr,
                               size_t               new_size )
    {
        return Allocator::reallocate( ptr, new_size, m_allocator_type );
    }

    /** 
     * reallocate的调试版本, 主要用于跟踪当前分配内存的位置，可以用于
     * 内存泄露的检查.
     * 
     * @param ptr           [in] 调用allocate或reallocate所申请的内存.
     * @param new_size      [in] 新的内存大小
     * @param file_name     [in] 调用reallocate_debug的源文件名
     * @param line_no       [in] 调用reallocate_debug的行号
     * 
     * @retval 一个指向所申请的内存的指针    重新申请内存成功. 
     * @retval 0                             重新申请内存失败.
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
     * 判断内存块是否存在于内存池中, 默认实现为返回true.
     * 
     * @param ptr     [in] 一个指向内存块的地址.
     * 
     * @retval true  内存块存在于该内存池中.
     * @retval false 内存块不存在于该内存池中.
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
     * 获取指向UPF_IMemory_Pool_Info实例的指针.
     * 
     * 
     * @return 指向UPF_IMemory_Pool_Info实例的指针.
     */
    UPF_IMemory_Pool_Info * get_memory_pool_info( void )
    {
        return static_cast< UPF_IMemory_Pool_Info * >( this );
    }

    /******************* Implements UPF_IMemory_Pool_Info ********************/

    /** 
     * 返回UPF_IMemory_Pool接口指针.
     * 
     * @return 实现UPF_IMemory_Pool接口的指针.
     */
    virtual UPF_IMemory_Pool * get_memory_pool( void )
    {
        return static_cast< UPF_IMemory_Pool * >( this );
    }

    /**
     * 获取内存池的名字.
     * 
     * @return 内存池的名字.
     */
    virtual const char * get_memory_pool_name( void ) const
    {
        return m_memory_pool_name;
    }

    /**
     * 获取内存池的类型，当前支持两种，固定大小的，可变大小的.
     * 
     * @return 内存池的类型.
     */
    virtual const char * get_memory_pool_type( void ) const
    {
        return m_memory_pool_type;
    }

    /** 
     * 获得内存池所管理的内存大小, 即内存池从系统中申请的所有内存的大小.
     *  
     * @return 内存池所管理的内存大小
     */
    virtual size_t get_managed_memory_size( void ) const
    {
        UPF_Guard< Lock > guard( this->m_lock );

        return m_recorder.get_managed_memory_size();
    }

    /** 
     * 获得内存池中当前已分配给Client端的内存大小, 这个值能表示在某一时刻Client端
     * 所使用的内存大小.
     * 
     * @return  分配给Client端的内存大小. 
     */
    virtual size_t get_allocated_memory_size( void ) const
    {
        UPF_Guard< Lock > guard( this->m_lock );

        return m_recorder.get_allocated_memory_size();
    }


    /** 
     *  获取分配给Client端的内存大小的峰值, 这个值能表示在某一时刻Client端使用的
     *  内存大小的最大值.
     * 
     * @return 分配给Client端的内存大小的峰值
     */
    virtual size_t get_allocated_memory_peak( void ) const
    {
        UPF_Guard< Lock > guard( this->m_lock );

        return m_recorder.get_allocated_memory_peak();
    }

    /** 
     * 获取内存池中浪费的内存大小, 因为内存池中的每个节点大小都是按照一定分配粒度
     * 对齐的, 所以每次分配可能有浪费的内存, 使用这个接口函数可以获取某一时刻内存池
     * 中所浪费的内存大小.
     * 
     * @return 内存池中浪费的内存大小
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
     * 重置计数器为0.
     * 
     */
    void reset_counter( void )
    {
        m_recorder.set_managed_memory_size( 0 );
        m_recorder.set_allocated_memory_size( 0 );
        m_recorder.set_wasted_memory_size( 0 );
    }

protected:
    /// 线程锁, 用于线程同步.
    mutable Lock          m_lock;

    /// 内存池名字.
    const char *          m_memory_pool_name;

    /// 内存池类型
    const char *          m_memory_pool_type;

    /// 用于保存内存池的id
    size_t                m_memory_pool_id;

    /// 记录内存池的分配信息.
    RecordPolicy          m_recorder;

    /// 记录allocator的类型.
    toplevel_allocator_t  m_allocator_type;

private:
    /// 禁止拷贝和赋值.
    UPF_Memory_Pool_Base( const UPF_Memory_Pool_Base< Allocator,  
                                                      Lock, 
                                                      RecordPolicy >& );

    UPF_Memory_Pool_Base< Allocator,  Lock, RecordPolicy >& operator =(
        const UPF_Memory_Pool_Base< Allocator,  Lock, RecordPolicy >& );

};

#endif /* UPF_MEMORY_POOL_BASE_T_H */

