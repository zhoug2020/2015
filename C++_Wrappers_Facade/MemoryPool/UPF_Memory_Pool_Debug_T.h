/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Debug_T.h                                                                  *
*       CREATE DATE     : 2007-11-21                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池调试版本的定义![CN]                                                              *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_DEBUG_T_H
#define UPF_MEMORY_POOL_DEBUG_T_H

#include "UPF_IMemory_Pool.h"
#include "UPF_IMemory_Pool_Info.h"
#include "UPF_Memory_Pool_Base_T.h"        /* for UPF_Scope_Guard, Null Lock */

#include <windows.h>                       /* for CreateThread, 
                                                  CreateEvent,
                                                  WaitForSingleObject. */

#include <time.h>                          /* for time_t, 
                                                  strftime, 
                                                  time, 
                                                  localtime. */

#include <stdio.h>

class UPF_IMemory_Pool_Debug_Print;

// type desc
const char * const DEBUG_MEMORY_POOL_TYPE = "Debug";


/** 
 * @class UPF_Memory_Pool_Debug_T
 * 
 * @brief 内存池调试版本, 基本上是一个内存池的包装版本，但实现了allocate_debug,
 * 当每次调用allocate_debug分配内存时, 额外多分配20字节的内存, 用于保存已分配的内存
 * 所在的文件名, 行号, 大小(当使用固定大小的内存池时, 必须保证为原来大小加上
 * 20字节), 然后把这个分配的节点添加到一个用于保存已分配节点的双向链表中，
 * 当释放内存时, 从双向链表中移除该节点. 在程序终止时，就可以根据这个链表来查看
 * 是否有内存泄露.
 * 
 *  - Lock          用于线程同步锁.
 * 
 */
template < class Lock >
class UPF_Memory_Pool_Debug_T : public UPF_IMemory_Pool,
                                public UPF_IMemory_Pool_Info
{
public:
     /// 构造函数, 用于初始化内部状态    
    UPF_Memory_Pool_Debug_T( UPF_IMemory_Pool * memory_pool );

    /// 析构函数，释放内部资源.
    ~UPF_Memory_Pool_Debug_T( void );

public:
    /************************ Implements UPF_IMemory_Pool ********************/

    /// 从内存池中分配内存.
    virtual void* allocate( size_t size );

    /// allocate的调试版本.
    virtual void* allocate_debug( size_t      size, 
                                  const char* file_name, 
                                  int         line_no );

    /// 从内存池中重新分配内存.
    virtual void * reallocate( void * ptr,
                               size_t new_size );

    /// reallocate的调试版本.
    virtual void * reallocate_debug( void *       ptr,
                                     size_t       new_size,
                                     const char * file_name,
                                     int          line_no );
    /// 释放内存.
    virtual void deallocate( void* ptr, size_t size );

    /// 判断内存块是否存在于内存池中.
    virtual bool is_memory_block_exist( void * ptr );

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
     * 获取内存池的类型，当前支持两种，固定大小的，可变大小的.
     * 
     * @return 内存池的类型.
     */
    virtual const char * get_memory_pool_name( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_memory_pool_name();
    }

    /**
     * 获取内存池的类型，当前支持两种，固定大小的，可变大小的.
     * 
     * @return 内存池的类型.
     */
    virtual const char * get_memory_pool_type( void ) const
    {
        return DEBUG_MEMORY_POOL_TYPE;
    }

    /** 
     * 获得内存池所管理的内存大小, 即内存池从系统中申请的所有内存的大小.
     *  
     * @return 内存池所管理的内存大小
     */
    virtual size_t get_managed_memory_size( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_managed_memory_size();
    }

    /** 
     * 获得内存池中当前已分配给Client端的内存大小, 这个值能表示在某一时刻Client端
     * 所使用的内存大小.
     * 
     * @return  分配给Client端的内存大小. 
     */
    virtual size_t get_allocated_memory_size( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_allocated_memory_size();
    }


    /** 
     *  获取分配给Client端的内存大小的峰值, 这个值能表示在某一时刻Client端使用的
     *  内存大小的最大值.
     * 
     * @return 分配给Client端的内存大小的峰值
     */
    virtual size_t get_allocated_memory_peak( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_allocated_memory_peak();
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
        return m_memory_pool->get_memory_pool_info()->get_wasted_memory_size();
    }

    /**
     * 获取内存的最大限制值.
     */
    virtual size_t get_allocated_memory_limit( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_allocated_memory_limit();
    }

    virtual toplevel_allocator_t  get_allocator_type( void ) const
    {
        return m_memory_pool->get_memory_pool_info()->get_allocator_type();
    }


public:
    /// 打印所管理的内存块的概要信息, 包括内存的大小, filename, lineno.
    void print_allocated_memory_block_summary( FILE * fp );

public:
    /// 内存池调试信息节点结构
    struct UPF_Memory_Pool_Debug_Node
    {
        const char *                 file_name;
        int                          line_no;        
        time_t                       allocate_time;
        UPF_Memory_Pool_Debug_Node * previous_node;
        UPF_Memory_Pool_Debug_Node * next_node;
        size_t                       size;
    };

public:
    /// 内存池调试信息节点结构的大小
    enum { SIZE_OF_DEBUG_NODE = ( sizeof( UPF_Memory_Pool_Debug_Node ) ) };

private:
    /// DUMP 内存的类型
    enum UPF_Dump_Memory_Type
    {
        DUMP_DETAIL  = 1,   /**< dupm内存块详细信息.                 */
        DUMP_SUMMARY = 2,   /**< dump内存块概要信息.                 */
        DUMP_ALL     = 3    /**< 既dump内存块详细信息又dump概要信息. */
    };

    enum UPF_Memory_Pool_Debug_Print_Type
    {
        DEBUG_PRINT_OUTPUTDEBUGSTRING = 1, /**< 调用OutputDebugString输出.       */
        DEBUG_PRINT_CONSOLE = 2,           /**< 输出到控制台.       */
        DEBUG_PRINT_FILE    = 3,           /**< 输出到文件.         */
        DEBUG_PRINT_ALL     = 4            /**< 输出到控制台和文件. */
    };

private:

    /// 添加内存池调试信息节点到内部的双向链表中
    UPF_Memory_Pool_Debug_Node *  add_node( const char * file_name,
                                            int          line_no,
                                            time_t       allocate_time,
                                            size_t       size,
                                            UPF_Memory_Pool_Debug_Node * new_node,
                                            UPF_Memory_Pool_Debug_Node * head_node );

    /// 从内部的双向链表中移去该节点
    void remove_node( UPF_Memory_Pool_Debug_Node * node );

private:

#ifdef UPF_OS_IS_WINNT
    /// dump memory 线程函数
    static DWORD WINAPI dump_memory_thread_proc( LPVOID lpParameter );

    /// 启动一个dump memory 线程
    void start_dump_memory_thread( void );

    /// 停止一个dump memory 线程
    void stop_dump_memory_thread( void );

    /// Dump 已经分配的内存块
    void dump_allocated_memory_block( void );

    /// Dump已经分配的内存块的详细信息, 包括已分配的内存块的内容.
    void dump_allocated_memory_block_detail( const char * memory_pool_name );

    /// Dump已经分配的内存块的概要信息
    void dump_allocated_memory_block_summary( const char * memory_pool_name );

    /// Dump start_memory_ptr和end_memory_ptr之间的内存的内容
    void dump_memory( unsigned char * start_memory_ptr,
                      unsigned char * end_memory_ptr );

    /// 用于指示dump memory 线程是否终止
    bool is_dump_memory_thread_stopped( void ) const
    {
        return m_is_dump_memory_thread_stopped;
    }

    /// 返回dump memory 事件对象.
    HANDLE & dump_memory_event( void ) 
    {
         return m_dump_memory_event;
    }

    /// 从环境变量中读取dump type值.
    void read_memory_dump_type_from_env( void );

    /// 从环境变量中读取debug print type值, 并创建相应的debug print实例.
    void create_debug_print_from_env( void );

    /// 取得配置信息
    static char * get_profile_string( const char * entry_name );

#endif /* UPF_OS_IS_WINNT */


private:
    /// 存放实现UPF_IMemory_Pool接口的实例, 它的生存期由
    /// UPF_Memory_Pool_Debug来管理
    UPF_IMemory_Pool * m_memory_pool;

    /// 用于保存已分配节点的双向循环链表的头指针.
    UPF_Memory_Pool_Debug_Node * m_head_debug_node;

    /// 线程同步锁
    Lock m_lock;

#ifdef UPF_OS_IS_WINNT

    /// dump memory 线程句柄
    HANDLE       m_dump_memory_thread_handle;

    /// dump memory 线程是否终止的标志
    bool         m_is_dump_memory_thread_stopped;

    /// dump memory 线程中用于相应通知的事件对象.
    HANDLE       m_dump_memory_event;

    /// DUMP内存的类型
    UPF_Dump_Memory_Type m_dump_memory_type;

    /// 输出Debug信息的接口
    UPF_IMemory_Pool_Debug_Print * m_debug_print;

#endif /* UPF_OS_IS_WINNT */

    /// 总共分配出去的内存大小
    size_t       m_total_allocated_size;

    /// 总共分配出去的内存块个数
    size_t       m_total_allocated_count;
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_Debug_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_MEMORY_POOL_DEBUG_T_H */



