/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp                                             *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池信息监视策略的实现![CN]                                                          *
*********************************************************************************************************************/

#include "UPF_Memory_Pool_Info_Record_Monitor_Policy.h"
#include "UPF_Memory_Pool_Util.h"

#include <string.h>             /* for strncpy */
#include <stdio.h>              /* for _snprintf, _vsnprintf, BUFSIZ */
#include <new>                  /* for placement new */
#include <stdarg.h>             /* for va_list */

// Static data definitions
HWND UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::sm_monitor_window = NULL;

BOOL UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::sm_monitor_app_existed = TRUE;

HWND UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::sm_monitor_window = NULL;

BOOL UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::sm_monitor_app_existed = TRUE;


// Constant definitions

#define MONITOR_APP_NAME            "UPF_Memory_Pool_Monitor.exe"

#define MONITOR_APP_WINDOW_TITLE    "UPF Memory Pool Monitor"

// 内存映射对象名字前缀.
#define MMAP_NAME_PREFIX            "UPF_MMAP_@#$%"


// Help function definitions.

#if defined(UPF_OS_IS_WINNT)
/* WINNT Platform */
#  include "WinNT/UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp"
#elif defined(UPF_OS_IS_WINCE)
/* WINCE Platform */
#  include "WinCE/UPF_Memory_Pool_Info_Record_Monitor_Policy.cpp"
#endif 

/********** UPF_Memory_Pool_Info_Record_Monitor_Policy implementation **********/

/** 
 * 构造函数，用于初始化内部状态
 * 
 */
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy()
{
    // initialize copy data
    m_copy_data.cbData = sizeof( m_info_data );
    m_copy_data.lpData = &m_info_data;

    m_last_memory_peak = 0;
}

/** 
 * 设置内存池的名字
 * 
 * @param memory_pool_name   [in]   内存池的名字
 * 
 */
void 
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::set_memory_pool_name(
     const char * memory_pool_name )
{
    // set info data
    strncpy( m_info_data.memory_pool_name, 
             memory_pool_name, 
             MAX_POOL_NAME_LENGTH );

    m_info_data.memory_pool_name[ MAX_POOL_NAME_LENGTH ] = '\0';

}

/** 
 * 发送内存池管理的内存的大小信息给监视程序
 * 
 * @param size        [in]   内存池管理的内存的大小
 */
void 
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::set_managed_memory_size( 
    size_t size )
{
    Base_Class::set_managed_memory_size( size );

    set_send_info( MANAGED_MEMORY_SIZE, size );

    UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( m_copy_data );
}

/** 
 * 发送内存池分配给Client端的内存大小信息给监视程序
 * 
 * @param size       [in]  内存池分配给Client端的内存大小
 */
void 
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::set_allocated_memory_size(
     size_t size )
{
    Base_Class::set_allocated_memory_size( size );

    set_send_info( ALLOCATED_MEMORY_SIZE, size );

    UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( m_copy_data );

    // if memory peak already changed, then send memory peak size info.
    size_t memory_peak_size = Base_Class::get_allocated_memory_peak();

    if ( m_last_memory_peak < memory_peak_size )
    {
        m_last_memory_peak = memory_peak_size;

        set_send_info( ALLOCATED_MEMORY_PEAK, m_last_memory_peak );

        UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( m_copy_data );
    }
}

/** 
 * 发送内存池中浪费的内存大小信息给监视程序
 * 
 * @param size       [in]  内存池中浪费的内存大小
 */
void 
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::set_wasted_memory_size( 
    size_t size )
{
    Base_Class::set_wasted_memory_size( size );

    set_send_info( WASTED_MEMORY_SIZE, size );

    UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( m_copy_data );
}

/** 
 *  发送内存池信息给监视程序
 *  
 *  当第一次发送消息时，因为监视程序可能已经运行, 所以首先查找监视程序的主窗口,
 *  如果查找不到, 则启动监视程序UPF_Memory_Pool_Monitor.exe), 如果启动成功
 *  则查找主窗口句柄，如果一切顺利的话，则发送WM_COPYDATA, 如果失败，则设置
 *  sm_monitor_app_existed为FALSE, 以表示监视程序不存在，并且以后也不在查找.
 * 
 * @param copy_data          [in]   要发送的WM_COPYDATA消息数据
 */
void
UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy::send_memory_info( 
    COPYDATASTRUCT & copy_data )
{
    if ( TRUE == sm_monitor_app_existed )
    {
        if ( ( NULL == sm_monitor_window || 
              ::IsWindow( sm_monitor_window ) == FALSE) )
        {
            if ( startup_monitor_app( sm_monitor_window ) == false )
            {
                sm_monitor_app_existed = FALSE;
                return;
            }
        }

        SendMessage( sm_monitor_window, WM_COPYDATA, 0, (LPARAM) &copy_data );
    }
}


/********** UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy implementation **********/

/** 
 * 构造函数, 打开内存映射对象. 如果监视程序没有启动, 则启动监视程序.
 * 
 */
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy( void )
{
    m_mmap_id          = 0;
    m_mmap_data_handle = NULL;
    m_mmap_data        = NULL;
    m_last_memory_peak = 0;

    if ( TRUE == sm_monitor_app_existed )
    {
        if ( ( NULL == sm_monitor_window || 
              ::IsWindow( sm_monitor_window ) == FALSE) )
        {
            if ( startup_monitor_app( sm_monitor_window ) == false )
            {
                sm_monitor_app_existed = FALSE;
                return;
            }
        }

        // construct a memory map object name
        char mmap_name[ 100 ];

        m_mmap_id = UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::get_mmap_id();

        _snprintf( mmap_name, sizeof( mmap_name ) - 1, "%s_%d",
                   MMAP_NAME_PREFIX,
                   m_mmap_id );

        mmap_name[ sizeof( mmap_name ) - 1 ] = '\0';

        m_mmap_data_handle = ::CreateFileMapping( INVALID_HANDLE_VALUE, 
                                                  NULL, 
                                                  PAGE_READWRITE,
                                                  0,
                                                  sizeof( Memory_Pool_MMap_Data ),
                                                  UPF_A2T(mmap_name) );

        if ( NULL == m_mmap_data_handle )
        {
            UPF_Memory_Pool_Util::display_os_error( "Create File Mapping Object (%s) failed", mmap_name );
        }
        else
        {
            m_mmap_data = static_cast< Memory_Pool_MMap_Data * >(
                ::MapViewOfFile( m_mmap_data_handle, FILE_MAP_ALL_ACCESS, 0, 0, 0 ) );

            if ( NULL ==  m_mmap_data )
            {
                UPF_Memory_Pool_Util::display_os_error( "Map file view (%s) failed", mmap_name );

                ::CloseHandle( m_mmap_data_handle );

                m_mmap_data_handle = NULL;
            }
            else
            {
                new (m_mmap_data) Memory_Pool_MMap_Data;
            }
        }

    }    
}

/** 
 * 析构函数, 关闭内存映射对象, 并给监视程序主窗口发送WM_UPF_MMAP_REMOVE消息.
 * WM_UPF_MMAP_REMOVE消息的WPARAM参数为m_mmap_id.
 * 
 */
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::
~UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy( void )
{
    if ( m_mmap_data != NULL )
    {
        // send memory mapping object remove message.
        if ( sm_monitor_window != NULL && ::IsWindow( sm_monitor_window ) )
        {
            ::SendMessage( sm_monitor_window, WM_UPF_MMAP_REMOVE, m_mmap_id, 0 );
        }

        ::UnmapViewOfFile( m_mmap_data );

        ::CloseHandle( m_mmap_data_handle );

        m_mmap_data_handle = NULL;
        m_mmap_data        = NULL;
    }
}

/** 
 * 设置内存池的名字, 并给监视程序主窗口发送WM_UPF_MMAP_ADD消息.
 * WM_UPF_MMAP_ADD消息的WPARAM参数为m_mmap_id, 因为这时候已经有内存池的名字了
 * 所以这时候发送WM_UPF_MMAP_ADD消息是适合的, 监视程序收到消息后, 会取得内存映射对象
 * 的句柄, 然后刷新内存池分配数据.
 * 
 * @param memory_pool_name   [in]   内存池的名字
 * 
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_memory_pool_name( 
    const char * memory_pool_name )
{
    if ( m_mmap_data )
    {
        strncpy( m_mmap_data->m_memory_pool_name, 
                 memory_pool_name, 
                 MAX_POOL_NAME_LENGTH );

        m_mmap_data->m_memory_pool_name[ MAX_POOL_NAME_LENGTH ] = '\0';

        // send memory mapping object add message
        if ( sm_monitor_window != NULL && ::IsWindow( sm_monitor_window ) )
        {
            ::SendMessage( sm_monitor_window, WM_UPF_MMAP_ADD, m_mmap_id, 0 );
        }
    }
}

/** 
 * 设置内存池管理的内存的大小信息
 * 
 * @param size        [in]   内存池管理的内存的大小
 * 
 * @param size
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_managed_memory_size( 
    size_t size )
{
    Base_Class::set_managed_memory_size( size );

    if ( m_mmap_data != NULL  )
    {
        m_mmap_data->m_managed_memory_size = size;
    }
}

/** 
 * 设置内存池分配给Client端的内存大小信息
 * 
 * @param size       [in]   内存池分配给Client端的内存大小
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_allocated_memory_size(
     size_t size )    
{
    Base_Class::set_allocated_memory_size( size );

    if ( m_mmap_data != NULL  )
    {
        m_mmap_data->m_allocated_memory_size = size;
        m_mmap_data->m_memory_peak_size = Base_Class::get_allocated_memory_peak();
    }
}

/** 
 * 设置内存池中浪费的内存大小信息
 * 
 * @param size       [in]    内存池中浪费的内存大小
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_wasted_memory_size( 
    size_t size )
{
    Base_Class::set_wasted_memory_size( size );

    if ( m_mmap_data != NULL  )
    {
        m_mmap_data->m_wasted_memory_size = size;
    }
}

/** 
 * 设置内存池中分配内存的上限值.
 * 
 * @param size       [in]   内存池中分配内存的上限值
 */
void 
UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy::set_allocated_memory_limit( 
    size_t size )
{
    Base_Class::set_allocated_memory_limit( size );

    if ( m_mmap_data != NULL )
    {
        m_mmap_data->m_allocated_memory_limit = size;
    }
}
