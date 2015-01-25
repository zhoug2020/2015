/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Print_Policy.h                                                                         *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]��ӡ�����ඨ��, ʹ�øò������ģ����ʹ�õķ���˳������:                                *
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
 * @brief ��ӡ������̨.
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
 * @brief ��ӡ����Ϣ����
 * 
 */
struct /*UPF_Export*/ UPF_MessageBox_Print_Policy
{
    static void on_before_print( const char * /* pool_name */ = "",
                                 const char * /* file_ext_name */ = "" );

    static void on_after_print( bool /* is_pause */ = false );

    static void print( const char * fmt,  ... );

private:
    /// ��Ϣ�����������ֵ
    enum { MAX_MESSAGE_BUFFER_SIZE = 512 };

    /// ��Ϣ������.
    static char sm_message_buffer[ MAX_MESSAGE_BUFFER_SIZE + 1 ];

    /// ָʾ��ǰ��Ϣ��������λ��.
    static int sm_message_buffer_pos;
};

/**
 * @class UPF_Dump_Memory_Console_Print_Policy
 * 
 * @brief ��Dump Memoryʱ, ��Dump����Ϣ��ӡ������̨, �����GUI���������� , 
 *        �����һ���µĿ���̨���ڽ������.
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
    /// UPF_New_Console_Print_Policyͨ��ֻ��UPF_Memory_Pool_Debug_T���ڴ�ӡ��
    /// ����������е��ڴ��, ���Զ���ÿ���ڴ�ش�ӡ�ѷ�����ڴ��Ĳ�����Ҫͬ��.
    static UPF_Thread_Mutex sm_console_print_sync_lock;

    /// ����ָʾ�Ƿ�����˿���̨���ڵ�buffer��С.
    static bool           sm_is_adjust_console_buffer;

    /// ����ָʾ�Ƿ񵯳���������ӡ��������ʾ��.
    static bool           sm_is_prompt;

    /// ����ָʾ����̨�����Ƿ��û��ر�, ������ر�, �򲻽���print����.
    static bool           sm_is_console_closed;

    /// ���ڴ�ſ���̨���ڵı�׼����豸�ľ��.
    static HANDLE         sm_console_output_handle;
};

/** 
 * @class UPF_Dump_Memory_File_Print_Policy
 * 
 * @brief ��Dump Memoryʱ, ��Dump����Ϣ��ӡ���ļ�, ������Dump���, �������Ӧ�ı༭��
 *        ��ʾ�ļ�����.
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
    /// UPF_Dump_Memory_File_Print_Policyͨ��ֻ��UPF_Memory_Pool_Debug_Tʹ��, ��ǰ��
    /// ��ʵ������Ϊ��̬���Է�����ʹ��, �ʵ�����ļ�ʱ��Ҫͬ��.
    static UPF_Thread_Mutex sm_file_print_sync_lock;

    /// ��ǰ����Dump���ļ����, ��on_before_print�г�ʼ��, ��print��ʹ��, ��
    /// on_after_print�йر�, ������ΪNULL.
    static FILE *         sm_dump_file;

    /// dump���ļ���
    static char           sm_dump_file_name[ MAX_PATH + 1 ];

    /// dump���ļ���չ��
    static const char *   sm_dump_file_ext_name;

};

#endif /* UPF_PRINT_POLICY_H */

