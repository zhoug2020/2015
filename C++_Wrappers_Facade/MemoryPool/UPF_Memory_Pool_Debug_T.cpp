/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Debug_T.cpp                                                                *
*       CREATE DATE     : 2007-11-21                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池调试版本的实现![CN]                                                              *
*********************************************************************************************************************/
#include <assert.h>                        /* for assert */
#include <ctype.h>                         /* for isprint */
#include <memory.h>                        /* for memset */
#include <string.h>                        /* for strncat, strlen */
#include <stdio.h>                         /* for BUFSIZ, snprintf, sprintf */
#include <stdlib.h>                        /* for getenv, atoi */
#include "UPF_Memory_Pool_Debug_T.h"
#include "UPF_Memory_Pool_Util.h"
#include "UPF_IMemory_Pool_Info_Manager.h"
#include "UPF_Memory_Pool_Debug_Print_Impl.h"

#if defined(WIN32) || defined(_WIN32)
#define snprintf       _snprintf     /* on win32 platform, be _snprintf */
#endif


/** 
 * 构造函数, 用于初始化内部状态, 同时读取dump type环境变量和
 * debug print type环境变量, 并依据debug print type环境变量的值
 * 创建debug print实例, 并启动Dump线程.
 * 
 * 
 * @param memory_pool              [in]  实现UPF_IMemory_Pool接口的实例, 它必须不为0, 
 *                                       它的生存期由UPF_Memory_Pool_Debug管理，当
 *                                       UPF_Memory_Pool_Debug析构时, 会自动释放这个实例.
 */
template < class Lock >
UPF_Memory_Pool_Debug_T< Lock >::UPF_Memory_Pool_Debug_T( 
    UPF_IMemory_Pool * memory_pool )
{   
    assert( memory_pool != 0 );

    m_memory_pool      = memory_pool;

    m_head_debug_node  = 0;

    m_total_allocated_size  = 0;
    m_total_allocated_count = 0;    

    // no startup dump thread on WINCE platform
#if defined(UPF_OS_IS_WINNT)
    
    m_dump_memory_type = DUMP_DETAIL;
    m_debug_print      = 0;

    // read dump type env var
    read_memory_dump_type_from_env();
    
    // create debug print instance.
    create_debug_print_from_env();


    // start a dump memory thread for respond client's dump request 
    this->start_dump_memory_thread();
#endif
}


/** 
 * 添加内存池调试信息节点到内部的双向循环链表中
 * 
 * @param file_name          [in]      源文件名
 * @param line_no            [in]      行号 
 * @param allocate_time      [in]      内存块申请时间
 * @param size               [in]      Client端分配的内存的大小
 * @param new_node           [in,out]  新创建的节点
 * @param head_node          [in,out]  双向链表的头节点
 * 
 * @return 一个新添加的节点
 */
template < class Lock >
inline 
typename UPF_Memory_Pool_Debug_T< Lock >::UPF_Memory_Pool_Debug_Node*
UPF_Memory_Pool_Debug_T< Lock >::add_node( const char * file_name,
                                           int          line_no,
                                           time_t       allocate_time,
                                           size_t       size,
                                           UPF_Memory_Pool_Debug_Node * new_node,
                                           UPF_Memory_Pool_Debug_Node * head_node )
{
    new_node->file_name     = file_name;
    new_node->line_no       = line_no;
    new_node->allocate_time = allocate_time;
    new_node->size          = size;
    
    if ( head_node != 0 )
    {
        new_node->previous_node = head_node->previous_node;
        
        head_node->previous_node->next_node = new_node;

        head_node->previous_node = new_node;
        new_node->next_node      = head_node;
    }
    else
    {
        new_node->previous_node = new_node;
        new_node->next_node     = new_node;
    }

    m_total_allocated_count += 1;
    m_total_allocated_size  += size;

    return new_node;
}

/** 
 * 从内部的双向链表中移去该节点
 * 
 * @param node            [in,out]    要移去的节点
 */
template < class Lock >
inline void 
UPF_Memory_Pool_Debug_T< Lock >::remove_node( UPF_Memory_Pool_Debug_Node * node )
{
    m_total_allocated_count -= 1;
    m_total_allocated_size  -= node->size;

    if ( node->previous_node != node )
    {
        node->previous_node->next_node = node->next_node;
    }

    if ( node->next_node != node )
    {
        node->next_node->previous_node = node->previous_node;
    }

    if ( m_head_debug_node == node )
    {
        if ( node->previous_node == node && node->previous_node == node->next_node )
        {
            m_head_debug_node = 0;
        }
        else
        {
            m_head_debug_node = node->next_node;
        }
    }
}

/** 
 * 调用allocate_debug来从内存池中分配内存, 但文件名, 行号等信息会丢失.
 * 
 * @param size    [in] 需要分配的内存的大小.
 * 
 * @retval 一个指向所申请的内存的指针    申请内存成功. 
 * @retval NULL                          申请内存失败. 
 */
template < class Lock >
void* 
UPF_Memory_Pool_Debug_T< Lock >::allocate( size_t size )
{
    return allocate_debug( size, "", 0 );
}

/** 
 * 从内存池中重新分配内存, 但文件名, 行号等信息会丢失.
 * 
 * @param ptr           [in] 调用allocate_debug或reallocate_debug所申请的内存.
 * @param new_size      [in] 新的内存大小
 * 
 * @retval 一个指向所申请的内存的指针    重新申请内存成功. 
 * @retval 0                             重新申请内存失败.
 * 
 */
template < class Lock >
void* 
UPF_Memory_Pool_Debug_T< Lock >::reallocate( void * ptr,
                                             size_t new_size )
{
    return reallocate_debug( ptr, new_size, "", 0 );
}


/** 
 * allocate的调试版本, 分配内存时，会在头部多分配SIZE_OF_DEBUG_NODE(20) bytes的
 * 内存用于存放信息, 然后把它加到内部的用于记录已分配内存的循环链表中.
 * 
 * @param size          [in] 需要分配的内存的大小.
 * @param file_name     [in] 调用allocate_debug的源文件名
 * @param line_no       [in] 调用allocate_debug的行号
 * 
 * @retval 一个指向所申请的内存的指针    申请内存成功. 
 * @retval NULL                          申请内存失败. 
 */
template < class Lock >
void* 
UPF_Memory_Pool_Debug_T< Lock >::allocate_debug( size_t      size, 
                                                 const char* file_name, 
                                                 int         line_no )
{
    if ( size > static_cast< size_t >( MAX_MEMORY_REQ_SIZE ) )
    {
        return 0;
    }

    if ( 0 == size )
    {
        size = 1;
    }

    UPF_Guard< Lock > guard( m_lock );

    void * ptr = m_memory_pool->allocate( SIZE_OF_DEBUG_NODE + size );

    if ( ptr != 0 )
    {
        if ( 0 == file_name )
        {
            file_name = "";
        }

#if defined(UPF_OS_IS_WINNT)

        m_head_debug_node = add_node( file_name,  
                                      line_no, 
                                      time( 0 ),
                                      size,  
                                      static_cast< UPF_Memory_Pool_Debug_Node * >( ptr ),
                                      m_head_debug_node );
#elif defined(UPF_OS_IS_WINCE)
        m_head_debug_node = add_node( file_name,  
                                      line_no, 
                                      GetTickCount(),
                                      size,  
                                      static_cast< UPF_Memory_Pool_Debug_Node * >( ptr ),
                                      m_head_debug_node );
#endif
    
        
        return ( static_cast< char * >(ptr) + SIZE_OF_DEBUG_NODE );
    }
    else
    {
        return 0;
    }

}

/** 
 * reallocate的调试版本, 主要用于跟踪当前分配内存的位置，可以用于
 * 内存泄露的检查.
 * 
 * @param ptr           [in] 调用allocate_debug或reallocate_debug所申请的内存.
 * @param new_size      [in] 新的内存大小
 * @param file_name     [in] 调用reallocate_debug的源文件名
 * @param line_no       [in] 调用reallocate_debug的行号
 * 
 * @retval 一个指向所申请的内存的指针    重新申请内存成功. 
 * @retval 0                             重新申请内存失败.
 * 
 */
template < class Lock >
void* 
UPF_Memory_Pool_Debug_T< Lock >::reallocate_debug( void *       ptr,
                                                   size_t       new_size,
                                                   const char * file_name,
                                                   int          line_no )
{
    void * result = 0;

    // according to UPF_IMemory_Pool::reallocate functions's specification
    // if ptr is 0, call allocate_debug to allocate memory,
    // if new size is 0, call deallocate to free memory.
    // otherwise, calll m_memory_pool's reallocate to reallocate memory.

    if ( 0 == ptr )
    {
        return this->allocate_debug( new_size, file_name, line_no );
    }

    if ( 0 == new_size )
    {
        this->deallocate( ptr, 0 );
        return 0;
    }

    if ( new_size > static_cast< size_t >( MAX_MEMORY_REQ_SIZE ) )
    {
        return 0;
    }

    UPF_Guard< Lock > guard( m_lock );
    
    UPF_Memory_Pool_Debug_Node * debug_node = 
        reinterpret_cast< UPF_Memory_Pool_Debug_Node * >(
        static_cast< char * >( ptr ) - SIZE_OF_DEBUG_NODE );
    
    remove_node( debug_node );
    
    void * new_ptr = m_memory_pool->reallocate( debug_node, 
                                                SIZE_OF_DEBUG_NODE + new_size );
    
    // if ptr is not 0, then reallocate memory succeed,
    // then add the debug info into internal debug info link list,
    // if ptr is 0, then reallocate memory failed, because 
    // UPF_IMemory_Pool::reallocate function specify that when
    // reallocate failed, original memory is not freed, so 
    // add original debug node info into internal debug info link list again.

    if ( new_ptr != 0 )
    {
        if ( 0 == file_name )
        {
            file_name = "";
        }

#if defined(UPF_OS_IS_WINNT)
        m_head_debug_node = add_node( file_name,  
                                      line_no,  
                                      time( 0 ),
                                      new_size,
                                      static_cast< UPF_Memory_Pool_Debug_Node * >( new_ptr ),
                                      m_head_debug_node );
#elif defined(UPF_OS_IS_WINCE)
        m_head_debug_node = add_node( file_name,  
                                      line_no,  
                                      GetTickCount(),
                                      new_size,
                                      static_cast< UPF_Memory_Pool_Debug_Node * >( new_ptr ),
                                      m_head_debug_node );
#endif
        
        result = ( static_cast< char * >(new_ptr) + SIZE_OF_DEBUG_NODE );            
    }
    else
    {
        m_head_debug_node = add_node( debug_node->file_name,
                                      debug_node->line_no,
                                      debug_node->allocate_time,
                                      debug_node->size,
                                      debug_node,
                                      m_head_debug_node );

        result = 0;
    }

    
    return result;
}

/** 
 * 释放内存, 把该内存节点从已分配内存的循环链表中移除.
 * 
 * @param ptr     [in] 调用allocate_debug或reallocate_debug所申请的内存的指针.
 * @param size    [in] ptr所指向的内存的大小.
 */
template < class Lock >
void
UPF_Memory_Pool_Debug_T< Lock >::deallocate( void* ptr, size_t size )
{
    if ( ptr != 0 )
    {
        UPF_Guard< Lock > guard( m_lock );
    
        UPF_Memory_Pool_Debug_Node * debug_node = 
            reinterpret_cast< UPF_Memory_Pool_Debug_Node * >(
                static_cast< char * >( ptr ) - SIZE_OF_DEBUG_NODE );
    
        remove_node( debug_node );
    
        m_memory_pool->deallocate( debug_node, 0 );
    }
}

/** 
 * 析构函数，如果有内存泄露, 则打印出泄露信息, 最终释放m_memory_pool,
 * 
 */
template < class Lock >
UPF_Memory_Pool_Debug_T< Lock >::~UPF_Memory_Pool_Debug_T( void )
{
    
#if defined(UPF_OS_IS_WINNT)

     // stop dump memory thread
    this->stop_dump_memory_thread();

    UPF_Guard< Lock > guard( m_lock );
    
    if ( m_head_debug_node != 0 )
    {
        const char * memory_pool_name = get_memory_pool_name();

        assert( m_debug_print != 0 );

        m_debug_print->on_before_print( memory_pool_name );
        
        m_debug_print->print( "\n[%s] UPF Memory Pool already detected memory leaks!!!\n",
                              memory_pool_name );
        
        m_debug_print->print( 
            "-------------------------------------------------------------------\n" );
        
        bool first_node = true;
        
        for ( UPF_Memory_Pool_Debug_Node * node = m_head_debug_node;
              ( first_node || node != m_head_debug_node );
              node = node->next_node, first_node = false )
        {
            m_debug_print->print( "%s (%d): leak %u bytes.\n", 
                                  node->file_name,
                                  node->line_no,
                                  node->size );
        }
        
        m_debug_print->print( 
            "-------------------------------------------------------------------\n\n" );
        
        m_debug_print->on_after_print( true );
        
    }

    delete m_debug_print;

#endif /* UPF_OS_IS_WINNT */

    // must delete m_memory_pool at last.
    delete m_memory_pool;
}


#ifdef UPF_OS_IS_WINNT

/** 
 * dump memory 线程函数, 等待一个Dump事件对象m_dump_memory_event, 如果得到通知, 则调用
 * dump_allocated_memory_block函数来dump已分配的内存内容.
 * 
 * @param lpParameter    [in] instance of UPF_Memory_Pool_Debug_T.
 * 
 * @return 0
 */
template < class Lock >
DWORD WINAPI 
UPF_Memory_Pool_Debug_T< Lock >::dump_memory_thread_proc( LPVOID lpParameter )
{
    UPF_Memory_Pool_Debug_T< Lock > * memory_pool_debug_instance =
        static_cast< UPF_Memory_Pool_Debug_T< Lock > * >( lpParameter );

    if ( memory_pool_debug_instance != 0 )
    {
        while ( ! memory_pool_debug_instance->is_dump_memory_thread_stopped() )
        {
            ::WaitForSingleObject( memory_pool_debug_instance->dump_memory_event(), 
                                   INFINITE );

            if ( memory_pool_debug_instance->is_dump_memory_thread_stopped() )
            {
                break;
            }

            memory_pool_debug_instance->dump_allocated_memory_block();
        }        
    }

    return 0;
}


/** 
 * 启动一个dump memory 线程.
 * 
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::start_dump_memory_thread( void )
{
    const char * const EVENT_SUFFIX = "_EventHAHA";

    char event_name[ BUFSIZ + 1 ];

    const char * memory_pool_name = get_memory_pool_name();

    // construct dump memory event name
    snprintf( event_name, 
              BUFSIZ, 
              "%.*s%s", 
              MAX_POOL_NAME_LENGTH, 
              memory_pool_name, 
              EVENT_SUFFIX );

    event_name[ BUFSIZ ] = '\0';


    // create dump memory event object.
    m_dump_memory_event = ::CreateEvent( NULL, FALSE, FALSE, UPF_A2T(event_name) );

    if ( m_dump_memory_event != NULL )
    {
        DWORD dwThreadId = 0;
        m_is_dump_memory_thread_stopped = false;

        // create dump memory thread.
        m_dump_memory_thread_handle = ::CreateThread( NULL,
                                                      0,
                                                      dump_memory_thread_proc,
                                                      this,
                                                      0,
                                                      &dwThreadId );

        if ( NULL == m_dump_memory_thread_handle )
        {
            ::CloseHandle( m_dump_memory_event );
            m_dump_memory_event = NULL;

            UPF_Memory_Pool_Util::display_os_error( "Create debug thread (%s) failed!",
                memory_pool_name );

        }
    }
    else
    {
        UPF_Memory_Pool_Util::display_os_error( "Create event (%s) failed!",
            event_name );
    }
}

/** 
 * 停止一个dump memory 线程
 * 
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::stop_dump_memory_thread( void )
{
    if ( m_dump_memory_thread_handle != NULL )
    {
        // set thread stop falg and send event notification to dump memory thread.
        m_is_dump_memory_thread_stopped = true;

        ::SetEvent( m_dump_memory_event );

        // wait for dump memory thread stop.
        ::WaitForSingleObject( m_dump_memory_thread_handle, 1000 );

        ::CloseHandle( m_dump_memory_event );
        ::CloseHandle( m_dump_memory_thread_handle );

        m_dump_memory_event         = NULL;
        m_dump_memory_thread_handle = NULL;
    }
}

/** 
 * dump 已经分配的内存块
 * 
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::dump_allocated_memory_block( void )
{
    UPF_Guard< Lock > guard( m_lock );

    const char * memory_pool_name = get_memory_pool_name();

    const char * ext_name = 0;

    // here, ext_name only be used in file print policy, 
    // when console print policy, ext_name be ommited.

    if ( (m_dump_memory_type & DUMP_SUMMARY) != 0 )
    {
        ext_name = ".csv";
    }
    else
    {
        ext_name = ".txt";
    }

    assert( m_debug_print != 0 );

    m_debug_print->on_before_print( memory_pool_name,
                                    ext_name );

    if ( m_head_debug_node != 0 )
    {        
        if ( (m_dump_memory_type & DUMP_SUMMARY) != 0 )
        {
            dump_allocated_memory_block_summary( memory_pool_name );
        }

        if ( (m_dump_memory_type & DUMP_DETAIL) != 0 )
        {
            dump_allocated_memory_block_detail( memory_pool_name );
        }
    }
    else
    {
        m_debug_print->print( "Memory pool name: %s\n", memory_pool_name );
        m_debug_print->print( "There are no allocated memory.\n" );
    }

    m_debug_print->on_after_print();
    
}

/** 
 * Dump已经分配的内存块的详细信息, 包括已分配的内存块的内容.
 * 
 * @param   memory_pool_name    [in]    内存池名
 * 
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::
dump_allocated_memory_block_detail( const char * memory_pool_name )
{
    m_debug_print->print( "Memory pool name: %s\n", memory_pool_name );

    m_debug_print->print( "Total allocated memory size: %u bytes\n", m_total_allocated_size );
    m_debug_print->print( "Total allocated block count: %u\n\n",     m_total_allocated_count );

    // because the link list is double direction cycle link list, 
    // the previous node of m_head_debug_node is tail node, 
    // the tail node is first allocated node, too.
    // so visit node from tail node to head node.
    UPF_Memory_Pool_Debug_Node * first_allocated_node = m_head_debug_node->previous_node;

    bool first_node = true;
    int count       = 1;

    for ( UPF_Memory_Pool_Debug_Node * node = first_allocated_node; 
          (first_node || node != first_allocated_node);
          node = node->previous_node, first_node = false )
    {

        m_debug_print->print( "%d. ", count++ );

        if ( node->file_name[0] != '\0' )
        {
            m_debug_print->print( "%s(%d) : ", node->file_name, node->line_no );
        }

        m_debug_print->print( "%u bytes\n\n", node->size );

        unsigned char * start_memory_ptr = 
            reinterpret_cast< unsigned char * >( node ) + SIZE_OF_DEBUG_NODE;

        dump_memory( start_memory_ptr, start_memory_ptr + node->size - 1 );
                     
    }
}

/** 
 * Dump已经分配的内存块的概要信息
 * 
 * @param   memory_pool_name    [in]    内存池名
 * 
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::
dump_allocated_memory_block_summary( const char * memory_pool_name )
{
    const int ALLOCATE_TIME_COL_SIZE = 19;
    const int LINE_NO_COL_SIZE       = 8;
    const int BLOCK_NO_COL_SIZE      = 8;

    const char MEMORY_POOL_NAME_HEADER[] = "Name";
    const char MEMORY_BLOCK_NO_HEADER[]  = "Block No";
    const char ALLOCATE_TIME_HEADER[]    = "Allocate Time";
    const char FILE_NAME_HEADER[]        = "File Name";
    const char LINE_NO_HEADER[]          = "Line No";
    const char ALLOCATE_SIZE_HEADER[]    = "Size(bytes)";


    int file_name_col_size = 0;  // need to calculate.

    UPF_Memory_Pool_Debug_Node * first_allocated_node = 0;

    // calculate file name col size.
    first_allocated_node = m_head_debug_node->previous_node;

    bool first_node       = true;
    int  file_name_length = 0;

    UPF_Memory_Pool_Debug_Node * node;for ( node = first_allocated_node; 
          (first_node || node != first_allocated_node);
          node = node->previous_node, first_node = false )
    {
        file_name_length = strlen( node->file_name );
        if ( file_name_col_size < file_name_length )
        {
            file_name_col_size = file_name_length;
        }
    }
    
    int memory_pool_name_col_size = strlen( memory_pool_name );

    // adjust column length
    if ( memory_pool_name_col_size < sizeof( MEMORY_POOL_NAME_HEADER ) - 1 )
    {
        memory_pool_name_col_size = sizeof( MEMORY_POOL_NAME_HEADER ) - 1;
    }

    if ( file_name_col_size < sizeof( FILE_NAME_HEADER ) - 1 )
    {
        file_name_col_size = sizeof( FILE_NAME_HEADER ) - 1;
    }

    // print header
    m_debug_print->print( "%-*s  %c%-*s  %c%-*s   %c%-*s   %c%-*s  %c%-s\n",
                          memory_pool_name_col_size,
                          MEMORY_POOL_NAME_HEADER,
                          m_debug_print->get_delimit_char(),
                          BLOCK_NO_COL_SIZE,
                          MEMORY_BLOCK_NO_HEADER,
                          m_debug_print->get_delimit_char(),
                          ALLOCATE_TIME_COL_SIZE,
                          ALLOCATE_TIME_HEADER,
                          m_debug_print->get_delimit_char(),
                          file_name_col_size,
                          FILE_NAME_HEADER,
                          m_debug_print->get_delimit_char(),
                          LINE_NO_COL_SIZE,
                          LINE_NO_HEADER,
                          m_debug_print->get_delimit_char(),
                          ALLOCATE_SIZE_HEADER );

    // print content

    first_node = true;

    char time_buf[ 20 ];  
    int  count = 1;

    for (  node = first_allocated_node; 
          (first_node || node != first_allocated_node);
          node = node->previous_node, first_node = false )
    {
        time_buf[ 0 ] = '\0';

        tm * now_tm = localtime( &node->allocate_time );

        if ( 0 != now_tm )
        {         
            if ( 0 == strftime( time_buf, 
                                sizeof( time_buf ), 
                                "%Y-%m-%d %H:%M:%S",
                                now_tm ) )
            {
                time_buf[ 0 ] = '\0';
            }

        }

        m_debug_print->print( "%*s  %c%-*d  %c%*s   %c%*s   %c%-*d  %c%-u\n",
                              memory_pool_name_col_size,
                              (first_node ? memory_pool_name : ""),
                              m_debug_print->get_delimit_char(),
                              BLOCK_NO_COL_SIZE,
                              count++,
                              m_debug_print->get_delimit_char(),
                              ALLOCATE_TIME_COL_SIZE,
                              time_buf,
                              m_debug_print->get_delimit_char(),
                              file_name_col_size,
                              node->file_name,
                              m_debug_print->get_delimit_char(),
                              LINE_NO_COL_SIZE,
                              node->line_no,
                              m_debug_print->get_delimit_char(),
                              node->size );
    }

}

/** 
 * Dump start_memory_ptr和end_memory_ptr之间的内存的内容
 * 
 * @param start_memory_ptr        [in]   需要Dump的内存的起始地址
 * @param end_memory_ptr          [in]   需要Dump的内存的终止地址
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::dump_memory( 
    unsigned char * start_memory_ptr, unsigned char * end_memory_ptr )
{
    const int BYTES_COUNT_EACH_LINE = 16;   // bytes count dumped of each line
    const int GAP_SIZE1             = 1;    // hex byte's gap
    const int GAP_SIZE2             = 4;    // hex section and data section's gap
    const int OFFSET_GAP            = 4;    // offset section and hex section's gap
    const int OFFSET_WIDTH          = 8;    // offset section's width.

    // hex section's width
    const int HEX_SECTION_WIDTH     =  BYTES_COUNT_EACH_LINE * 2 +
                                       ( BYTES_COUNT_EACH_LINE - 1 ) * GAP_SIZE1 +
                                       GAP_SIZE2;

    // one line's total width
    const int LINE_WIDTH            = OFFSET_WIDTH + OFFSET_GAP + 
                                      HEX_SECTION_WIDTH + BYTES_COUNT_EACH_LINE;

    // used to store a line's content
    char line_buffer[ HEX_SECTION_WIDTH + BYTES_COUNT_EACH_LINE + 1 ];

    // used to store a line end ptr of reading byte
    unsigned char * line_end_ptr = 0;

    // offset value of each line.
    size_t offset = 1;

    int i = 0;

    // print header
    m_debug_print->print( "%*s%*s", 
                          OFFSET_WIDTH, "Offset",
                          OFFSET_GAP,   " " );

    for ( i = 1; i <= BYTES_COUNT_EACH_LINE; ++i )
    {
        m_debug_print->print( "%2d%*s", i, GAP_SIZE1, " " );
    }

    m_debug_print->print( "%*s",  GAP_SIZE2, " " );
    m_debug_print->print( "Data\n" );

    for ( i = 0; i < LINE_WIDTH; ++i )
    {
        m_debug_print->print( "=" );
    }

    m_debug_print->print( "\n" );

    // print memory's content

    while ( start_memory_ptr <= end_memory_ptr )
    {

        // print offset value
        m_debug_print->print( "%*u%*s", 
                              OFFSET_WIDTH, offset,
                              OFFSET_GAP,   " " );

        // print a line
        line_end_ptr = start_memory_ptr + BYTES_COUNT_EACH_LINE - 1;

        if ( line_end_ptr > end_memory_ptr )
        {
            line_end_ptr = end_memory_ptr;
        }

        // first reset line buffer be space char.
        memset( line_buffer, ' ',  sizeof( line_buffer ) - 1 );

        for ( int index = 0; 
              start_memory_ptr <= line_end_ptr; 
              ++start_memory_ptr, ++index )
        {

            // fill hex section
            assert( index * ( 2 + GAP_SIZE1 ) <=  HEX_SECTION_WIDTH - GAP_SIZE2 - 2 );

            snprintf( &line_buffer[ index * ( 2 + GAP_SIZE1 ) ], 
				      2, 
				      "%02x", 
					  *start_memory_ptr );

            assert( (index + HEX_SECTION_WIDTH) <= sizeof( line_buffer ) - 2 );

            // fill data section
			if ( isprint( *start_memory_ptr ) )
			{
				line_buffer[ index + HEX_SECTION_WIDTH ] = 
                    static_cast< char >( *start_memory_ptr );
			}
			else
			{
                line_buffer[ index + HEX_SECTION_WIDTH ] = '.';
			}
        }

        line_buffer[ sizeof( line_buffer ) - 1 ] = '\0';

        m_debug_print->print( "%s\n", line_buffer );

        offset += BYTES_COUNT_EACH_LINE;        
    }

    for ( i = 0; i < LINE_WIDTH; ++i )
    {
        m_debug_print->print( "=" );
    }

    m_debug_print->print( "\n\n" );
}


/**
 * 从环境变量UPF_MEMORY_POOL_DUMP_TYPE中读取dump type值.
 *
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::
read_memory_dump_type_from_env( void )
{
    const char * const DUMP_TYPE_ENV_VAR = "UPF_MEMORY_POOL_DUMP_TYPE";

    m_dump_memory_type = DUMP_DETAIL;

    char * env_value = get_profile_string( DUMP_TYPE_ENV_VAR );
    
    if ( 0 != env_value )
    {
        int value = atoi( env_value );
        if ( value >= DUMP_DETAIL && value <= DUMP_ALL )
        {
            m_dump_memory_type =
                static_cast< UPF_Dump_Memory_Type >( value );
        }
    }
}

/**
 * 从环境变量UPF_MEMORY_POOL_DEBUG_PRINT_TYPE中读取debug print type值,
 * 并创建相应的debug print实例.
 *
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::
create_debug_print_from_env( void )
{
    const char * const DEBUG_PRINT_TYPE_ENV_VAR =
        "UPF_MEMORY_POOL_DEBUG_PRINT_TYPE";

    UPF_Memory_Pool_Debug_Print_Type 
        debug_print_type_value = DEBUG_PRINT_OUTPUTDEBUGSTRING;

    char * env_value = get_profile_string( DEBUG_PRINT_TYPE_ENV_VAR );

    if ( 0 != env_value )
    {
        int value = atoi( env_value );

        if ( value >= DEBUG_PRINT_OUTPUTDEBUGSTRING && value <= DEBUG_PRINT_ALL )
        {
            debug_print_type_value =  
                static_cast< UPF_Memory_Pool_Debug_Print_Type >( value );
        }
    }

    switch ( debug_print_type_value )
    {
        case DEBUG_PRINT_OUTPUTDEBUGSTRING:
            m_debug_print = new UPF_Memory_Pool_OutputDebugString_Print_Impl;
            break;

        case DEBUG_PRINT_CONSOLE:
            m_debug_print = new UPF_Memory_Pool_Debug_Console_Print_Impl;
            break;

        case DEBUG_PRINT_FILE:
            m_debug_print = new UPF_Memory_Pool_Debug_File_Print_Impl;
            break;

        case DEBUG_PRINT_ALL:
            m_debug_print = new UPF_Memory_Pool_Debug_Console_And_File_Print_Impl;
            break;

        default:
            assert( 0 );
            break;
    }
}

/** 
 * 取得配置信息, 在winnt下暂时用getenv, wince下未实现.
 * 
 * @param entry_name        [in]  配置入口名.
 *
 * @return 配置值
 *
 * @todo 以后考虑用配置文件代替.
 *
 */
template < class Lock >
char *
UPF_Memory_Pool_Debug_T< Lock >::get_profile_string( const char * entry_name )
{
    return getenv( entry_name );
}

#endif /* UPF_OS_IS_WINNT */


/** 
 * 判断内存块是否存在于内存池中.
 * 
 * @param ptr     [in] 一个指向内存块的地址.
 * 
 * @retval true  内存块存在于该内存池中.
 * @retval false 内存块不存在于该内存池中.
 */
template < class Lock >
bool 
UPF_Memory_Pool_Debug_T< Lock >::
is_memory_block_exist( void * ptr )
{
    if ( 0 != ptr )
    {
        return m_memory_pool->is_memory_block_exist( 
            static_cast< char * >( ptr ) - SIZE_OF_DEBUG_NODE );
    }
    else
    {
        return false;
    }
}

/**
 * 打印所管理的内存块的概要信息.
 */
template < class Lock >
void 
UPF_Memory_Pool_Debug_T< Lock >::print_allocated_memory_block_summary( FILE * fp )
{
    assert( fp != NULL );

    if ( fp != NULL )
    {
        UPF_Guard< Lock > guard( m_lock );

        const char * memory_pool_name = get_memory_pool_name();

        if ( m_head_debug_node != NULL )
        {                        
            const int ALLOCATE_TIME_COL_SIZE = 19;
            const int LINE_NO_COL_SIZE       = 8;
            const int BLOCK_NO_COL_SIZE      = 8;
            
            const char MEMORY_POOL_NAME_HEADER[] = "Name";
            const char MEMORY_BLOCK_NO_HEADER[]  = "Block No";
            const char ALLOCATE_TIME_HEADER[]    = "Allocate Time";
            const char FILE_NAME_HEADER[]        = "File Name";
            const char LINE_NO_HEADER[]          = "Line No";
            const char ALLOCATE_SIZE_HEADER[]    = "Size(bytes)";
                                              
            // print header
            fprintf( fp,
                    "%-15s,%-11s,%-23s,%-53s,%-11s,%-s\n",
                    MEMORY_POOL_NAME_HEADER,
                    MEMORY_BLOCK_NO_HEADER,
                    ALLOCATE_TIME_HEADER,
                    FILE_NAME_HEADER,
                    LINE_NO_HEADER,
                    ALLOCATE_SIZE_HEADER );
            
            // print content
            
            UPF_Memory_Pool_Debug_Node * first_allocated_node = 0;
            
            // calculate file name col size.
            first_allocated_node = m_head_debug_node->previous_node;
            
            bool first_node       = true;
            
            char time_buf[ 20 ];  
            int  count = 1;
            
            for ( UPF_Memory_Pool_Debug_Node* node = first_allocated_node; 
                    (first_node || node != first_allocated_node);
                    node = node->previous_node, first_node = false )
            {
                time_buf[ 0 ] = '\0';
                
#if defined( UPF_OS_IS_WINNT )
                
                tm * now_tm = localtime( &node->allocate_time );
                
                if ( 0 != now_tm )
                {         
                    if ( 0 == strftime( time_buf, 
                        sizeof( time_buf ), 
                        "%Y-%m-%d %H:%M:%S",
                        now_tm ) )
                    {
                        time_buf[ 0 ] = '\0';
                    }
                    
                }
#elif defined( UPF_OS_IS_WINCE )
                sprintf( time_buf, "%u", node->allocate_time );
#endif
                
                fprintf( fp, 
                        "%15s,%-11d,%23s,%53s,%-11d,%-u\n",
                        (first_node ? memory_pool_name : ""),
                        count++,
                        time_buf,
                        node->file_name,
                        node->line_no,
                        node->size );
            }
        }
        else
        {
            fprintf( fp, "Memory pool name: %s\n", memory_pool_name );
            fprintf( fp, "There are no allocated memory.\n" );
        }
    }
}
