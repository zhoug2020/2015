/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Print_Policy.h                                                                         *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]打印策略类定义, 使用该策略类的模版所使用的方法顺序如下:                                *
*                             PrintPolicy::on_before_print                                                           *
*                             PrintPolicy::print                                                                     *
*                             PrintPolicy::on_after_print                                                            *
*                         ![CN]                                                                                      *
**********************************************************************************************************************/

#ifndef UPF_PRINT_POLICY_H
#define UPF_PRINT_POLICY_H

#include <stdio.h>                    /* for vprintf, _vsnprintf, FILE */
#include <stdarg.h>                   /* for va_start, va_end. */

#include <windows.h>

#include "UPF_Thread_Sync.h"

/**
 * @class UPF_Console_Print_Policy
 * 
 * @brief 打印到控制台.
 *  
 */
struct UPF_Console_Print_Policy
{

    static void on_before_print( const char * /* pool_name */ = "",
                                 const char * /* file_ext_name */ = "" )
    {
        // empty method body
    }

    static void on_after_print( bool /* is_pause */ = false )
    {
        // empty method body
    }

    static void print( const char * fmt, ... )
    {
        va_list arg_list;

        va_start( arg_list, fmt );

        vprintf( fmt,  arg_list );

        va_end( arg_list );
    }
};

/** 
 * @class UPF_MessageBox_Print_Policy
 * 
 * @brief 打印到消息框中
 * 
 */
struct /*UPF_Export*/ UPF_MessageBox_Print_Policy
{
    static void on_before_print( const char * /* pool_name */ = "",
                                 const char * /* file_ext_name */ = "" );

    static void on_after_print( bool /* is_pause */ = false );

    static void print( const char * fmt,  ... );

private:
    /// 消息缓存区的最大值
    enum { MAX_MESSAGE_BUFFER_SIZE = 512 };

    /// 消息缓存区.
    static char sm_message_buffer[ MAX_MESSAGE_BUFFER_SIZE + 1 ];

    /// 指示当前消息缓存区的位置.
    static int sm_message_buffer_pos;
};

/**
 * @class UPF_Dump_Memory_Console_Print_Policy
 * 
 * @brief 当Dump Memory时, 把Dump的信息打印到控制台, 如果在GUI程序的情况下 , 
 *        会产生一个新的控制台窗口进行输出.
 *  
 */
struct /*UPF_Export*/ UPF_Dump_Memory_Console_Print_Policy
{
    static void on_before_print( const char * pool_name = "",
                                 const char * /* file_ext_name */ = "" );

    static void on_after_print( bool is_pause = false );

    static void print( const char * fmt, ... );

private:
    static void print_delimit_line( void );

    static BOOL WINAPI console_event_handler( DWORD dwCtrlType );

    static HWND get_console_window( void );


private:
    /// UPF_New_Console_Print_Policy通常只被UPF_Memory_Pool_Debug_T用于打印出
    /// 所分配的所有的内存块, 所以对于每个内存池打印已分配的内存块的操作需要同步.
    static UPF_Thread_Mutex sm_console_print_sync_lock;

    /// 用于指示是否调整了控制台窗口的buffer大小.
    static bool           sm_is_adjust_console_buffer;

    /// 用于指示是否弹出超出最大打印行数的提示框.
    static bool           sm_is_prompt;

    /// 用于指示控制台窗口是否被用户关闭, 如果被关闭, 则不进行print操作.
    static bool           sm_is_console_closed;

    /// 用于存放控制台窗口的标准输出设备的句柄.
    static HANDLE         sm_console_output_handle;
};

/** 
 * @class UPF_Dump_Memory_File_Print_Policy
 * 
 * @brief 当Dump Memory时, 把Dump的信息打印到文件, 并且在Dump完毕, 会调用相应的编辑器
 *        显示文件内容.
 * 
 */
struct /*UPF_Export*/ UPF_Dump_Memory_File_Print_Policy
{
    static void on_before_print( const char * pool_name = "",
                                 const char * file_ext_name = ".txt" );

    static void on_after_print( bool /* is_pause */ = false );

    static void print( const char * fmt, ... );

private:
    static bool get_dump_dir( char * dump_dir_buffer, size_t buffer_size );

    static bool get_dump_file_name( const char * dump_dir,
                                    const char * pool_name,
                                    char *       dump_file_name_buffer, 
                                    size_t       buffer_size );

    static bool get_now_time( char * buffer,
                              size_t buffer_length,
                              bool   is_date );


private:
    /// UPF_Dump_Memory_File_Print_Policy通常只被UPF_Memory_Pool_Debug_T使用, 当前的
    /// 的实现是作为静态策略方法来使用, 故当输出文件时需要同步.
    static UPF_Thread_Mutex sm_file_print_sync_lock;

    /// 当前正在Dump的文件句柄, 在on_before_print中初始化, 在print中使用, 在
    /// on_after_print中关闭, 并且置为NULL.
    static FILE *         sm_dump_file;

    /// dump的文件名
    static char           sm_dump_file_name[ MAX_PATH + 1 ];

    /// dump的文件扩展名
    static const char *   sm_dump_file_ext_name;

};

#endif /* UPF_PRINT_POLICY_H */

