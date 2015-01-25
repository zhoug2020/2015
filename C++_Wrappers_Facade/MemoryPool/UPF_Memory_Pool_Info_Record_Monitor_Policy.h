/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Record_Monitor_Policy.h                                               *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]用于内存池信息记录, 当记录内存池信息时, 会把相应的数据发送给一个监视程序，由监视程序   *
*                             来实时显示数据, 仅用于win32 platform.                                                  *
*                         ![CN]                                                                                      *
*********************************************************************************************************************/

#ifndef UPF_Memory_Pool_Info_Record_Monitor_Policy_H
#define UPF_Memory_Pool_Info_Record_Monitor_Policy_H

#include "Memory_Pool_Info_Record_Policy.h"
#include "UPF_Memory_Pool_Define.h"

#include <stddef.h>            /* for size_t */
#include <windows.h>           /* for COPYDATASTRUCT */
#include <memory.h>            /* for memset */

/** 
 * @class UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy
 * 
 * @brief 一个内存池信息监视类, 通过发送WM_COPYDATA消息给一个
 *        UPF_Memory_Pool_Monitor.exe的程序, 由这个程序来显示数据.
 * 
 */
class UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy 
    : public UPF_Memory_Pool_Info_Record_Policy
{
public:
    typedef UPF_Memory_Pool_Info_Record_Policy Base_Class;

public:
    UPF_Memory_Pool_Info_Record_CDMsg_Monitor_Policy();

    void set_memory_pool_name( const char * memory_pool_name );
    

    void set_managed_memory_size( size_t size );

    void set_allocated_memory_size( size_t size );    

    void set_wasted_memory_size( size_t size );

private:

    enum Field_Type {
        MANAGED_MEMORY_SIZE   = 0x1,
        ALLOCATED_MEMORY_SIZE = 0x2,
        ALLOCATED_MEMORY_PEAK = 0x4,
        WASTED_MEMORY_SIZE    = 0x8
    };

    struct Memory_Pool_Info_Data
    {
       size_t memory_size;
       char   memory_pool_name[ MAX_POOL_NAME_LENGTH + 1 ];
    };

private:
    /// 发送内存池信息给监视程序
    static void send_memory_info( COPYDATASTRUCT & copy_data );

private:

    /// 设置发送的信息
    void set_send_info( DWORD  send_flag,
                        size_t size )
    {
        m_copy_data.dwData      = send_flag;
        m_info_data.memory_size = size;
    }

private:
    /// 监视窗口句柄
    static HWND sm_monitor_window;

    /// 存放监视程序是否存在, 缺省为TRUE
    static BOOL sm_monitor_app_existed;

private:
    /// 发送给监视程序的copy data
    COPYDATASTRUCT         m_copy_data;

    /// 要发送给监视程序的内存池信息
    Memory_Pool_Info_Data  m_info_data;

    /// 存放最近一次的内存峰值.
    size_t                 m_last_memory_peak;

};

/** 
 * @class UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy
 * 
 * @brief 一个内存池信息监视类, 通过内存映射在系统中开辟一块共享内存区, 当记录
 *        内存池的分配数据时, 往这个共享内存区写数据.
 *        另外有个UPF_Memory_Pool_Monitor.exe的程序, 定时的去共享内存区取数据并且
 *        显示在窗口中.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy 
    : public UPF_Memory_Pool_Info_Record_Policy
{
public:
    typedef UPF_Memory_Pool_Info_Record_Policy Base_Class;

public:
    /// 构造函数, 打开内存映射对象.
    UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy( void );

    /// 析构函数, 关闭内存映射对象.
    ~UPF_Memory_Pool_Info_Record_MMap_Monitor_Policy( void );

    void set_memory_pool_name( const char * memory_pool_name );
    

    void set_managed_memory_size( size_t size );

    void set_allocated_memory_size( size_t size );    

    void set_wasted_memory_size( size_t size );

    void set_allocated_memory_limit( size_t size );

private:

    struct Memory_Pool_MMap_Data
    {       
        Memory_Pool_MMap_Data()
        {
            ::memset( m_memory_pool_name, 0, sizeof( m_memory_pool_name ) );

            m_managed_memory_size    = 0;
            m_allocated_memory_size  = 0;
            m_wasted_memory_size     = 0;
            m_memory_peak_size       = 0;
            m_allocated_memory_limit = 0;
        }

        char   m_memory_pool_name[ MAX_POOL_NAME_LENGTH + 1 ];

        size_t m_managed_memory_size;
        size_t m_allocated_memory_size;
        size_t m_wasted_memory_size;
        size_t m_memory_peak_size;
        size_t m_allocated_memory_limit;
    };

    /// 用于发送给监视窗口的消息, 消息的WPARAM参数为m_mmap_id, 监视程序根据
    /// 这个id, 可以得到内存映射对象的名字, 从而可以获取共享内存数据.
    enum MMap_Window_Message
    {
        WM_UPF_MMAP_ADD    = WM_USER + 111,    /**< 当内存映射对象创建时发送的消息. */
        WM_UPF_MMAP_REMOVE = WM_USER + 112     /**< 当内存映射对象关闭时发送的消息. */
    };

private:
    /// 监视窗口句柄
    static HWND sm_monitor_window;

    /// 存放监视程序是否存在, 缺省为TRUE
    static BOOL sm_monitor_app_existed;

private:
    /// 取得内存映射对象ID, 内存映射对象的名字由内存映射对象名字前缀+内存映射对象ID组成.
    static long get_mmap_id( void )
    {
        static volatile long sm_mmap_id;
        return ::InterlockedIncrement( const_cast< long * >( &sm_mmap_id ) );
    }

    
private:
    /// 指向共享内存的指针.
    Memory_Pool_MMap_Data *  m_mmap_data;

    /// 与Memory_Pool_MMap_Data相对应的内存映射对象的句柄.
    HANDLE                   m_mmap_data_handle;

    /// 用于存放当前内存映射对象对应的ID.
    long                     m_mmap_id;

    /// 存放最近一次的内存峰值.
    size_t                   m_last_memory_peak;
};
#endif /* UPF_Memory_Pool_Info_Record_Monitor_Policy_H */

