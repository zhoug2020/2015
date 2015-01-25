/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Print_Policy.cpp                                                                       *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]��ӡ������WINNTƽ̨��ʵ��![CN]                                                         *
*********************************************************************************************************************/

#include "UPF_Print_Policy.h"
#include "UPF_Memory_Pool_Util.h"      /* for display_os_error */

#include <windows.h>              
#include <shellapi.h>

#define vsnprintf       _vsnprintf     /* on win32 platform, be _vsnprintf */
#define snprintf        _snprintf     /* on win32 platform, be _snprintf */

#pragma comment(lib, "shell32.lib")

#include <assert.h>                    /* for assert */
#include <conio.h>                     /* for _cprintf, getch */
#include <limits.h>                    /* for SHRT_MAX */
#include <stdlib.h>                    /* for getenv */
#include <string.h>                    /* for strncpy, strlen, strrchr, strcpy, strcat */
#include <direct.h>                    /* for _getcwd, _mkdir */
#include <io.h>                        /* for _access, _unlink */
#include <stdio.h>                     /* for fopen, fprintf, fclose */
#include <time.h>                      /* for strftime, time, localtime */

// static data member definitions

// UPF_MessageBox_Print_Policy
char UPF_MessageBox_Print_Policy::sm_message_buffer[ MAX_MESSAGE_BUFFER_SIZE + 1 ];

int UPF_MessageBox_Print_Policy::sm_message_buffer_pos;


// UPF_Dump_Memory_Console_Print_Policy
UPF_Thread_Mutex UPF_Dump_Memory_Console_Print_Policy::sm_console_print_sync_lock;

bool             UPF_Dump_Memory_Console_Print_Policy::sm_is_adjust_console_buffer = false;

bool             UPF_Dump_Memory_Console_Print_Policy::sm_is_prompt = false;

bool           UPF_Dump_Memory_Console_Print_Policy::sm_is_console_closed = false;

HANDLE         UPF_Dump_Memory_Console_Print_Policy::sm_console_output_handle = NULL;

// UPF_Dump_Memory_File_Print_Policy
UPF_Thread_Mutex UPF_Dump_Memory_File_Print_Policy::sm_file_print_sync_lock;

FILE *           UPF_Dump_Memory_File_Print_Policy::sm_dump_file = NULL;

char             UPF_Dump_Memory_File_Print_Policy::sm_dump_file_name[ MAX_PATH + 1 ];

const char *     UPF_Dump_Memory_File_Print_Policy::sm_dump_file_ext_name = 0;
 

/******************* UPF_MessageBox_Print_Policy implementation ***************/

/** 
 * �ڿ�ʼ��ӡ����ǰ������, ��ʼ����ǰ��Ϣ��������λ��Ϊ0.
 * 
 */
void 
UPF_MessageBox_Print_Policy::on_before_print( const char * /* pool_name */,
                                              const char * /* file_ext_name */ )
{
    sm_message_buffer_pos = 0;
}

/** 
 * �ڴ�ӡ���������󱻵���, �������MessageBox��ʾһ���Ի���.
 * 
 */
void
UPF_MessageBox_Print_Policy::on_after_print( bool /* is_pause */ )
{
    sm_message_buffer[ sm_message_buffer_pos ] = '\0';

    MessageBox( NULL,  
                sm_message_buffer, 
                "UPF Memory Pool Info",
                MB_OK | MB_ICONINFORMATION );

}

/** 
 * ��ӡ����, ����Ҫ��ӡ�����ݾ�����ʽ����ŵ��ڲ�����Ϣ��������, �����Ҫ��ӡ�����ݳ�����
 * ��Ϣ�����������ֵ���������Ĳ��ֲ���ŵ���Ϣ�������У������Ϣ�������������ڵ�����
 * ��ӡ������Ϻ�, ����on_after_printʱ���.
 * 
 * @param fmt    [in] ��ӡ��ʽ��
 */
void
UPF_MessageBox_Print_Policy::print( const char * fmt, ... )
{
    va_list arg_list;

    va_start( arg_list, fmt );

    int written_count = vsnprintf( sm_message_buffer + sm_message_buffer_pos, 
                                   MAX_MESSAGE_BUFFER_SIZE - sm_message_buffer_pos,
                                   fmt,
                                   arg_list );

    if ( written_count > 0 )
    {
        sm_message_buffer_pos += written_count;
    }
    else if ( -1 == written_count )
    {
        sm_message_buffer_pos = MAX_MESSAGE_BUFFER_SIZE;
    }

    va_end( arg_list );
}

/************ UPF_Dump_Memory_Console_Print_Policy implementation ***************/

/** 
 * �ڴ�ӡ����ǰ������, �Ƚ��м���. ͨ���Ŀ���̨��СΪ80��, 300�У�
 * ��������Dump memory����ʾҪ��, ��������ѿ���̨����Ϊ100��, ����Ϊϵͳ��������ֵ.
 * 
 */
void 
UPF_Dump_Memory_Console_Print_Policy::on_before_print( const char * /* pool_name */,
                                                       const char * /* file_ext_name */ )
{
    sm_console_print_sync_lock.acquire();

    HWND console_window_handle = NULL;
    sm_is_console_closed       = false;
    
    // create console if console is not created.

    if ( AllocConsole() )
    {
        sm_is_prompt                = false;
        sm_is_adjust_console_buffer = false;

        console_window_handle = UPF_Dump_Memory_Console_Print_Policy::get_console_window();

        if ( NULL != console_window_handle )
        {
            EnableMenuItem ( GetSystemMenu ( console_window_handle, FALSE),
                             SC_CLOSE,
                             MF_BYCOMMAND | MF_GRAYED );

            RemoveMenu( GetSystemMenu ( console_window_handle, FALSE),
                        SC_CLOSE,
                        MF_BYCOMMAND );
        }
        else
        {
            UPF_Memory_Pool_Util::display_os_error( "get console window error." );
        }

        if ( SetConsoleCtrlHandler(
            UPF_Dump_Memory_Console_Print_Policy::console_event_handler, TRUE ) == FALSE )
        {
            UPF_Memory_Pool_Util::display_os_error( "SetConsoleCtrlHandler error." );
        }
    }

    // adjust console'size
    if ( false == sm_is_adjust_console_buffer )
    {
        const int DEFAULT_BUFFER_X_VALUE = 100;

        HANDLE hStdOutput = GetStdHandle( STD_OUTPUT_HANDLE );

        if ( hStdOutput != INVALID_HANDLE_VALUE )
        {
            CONSOLE_SCREEN_BUFFER_INFO console_buffer_info;
            COORD                      old_console_screen_buffer_size;

            if ( GetConsoleScreenBufferInfo( hStdOutput, &console_buffer_info ) )
            {
                old_console_screen_buffer_size = console_buffer_info.dwSize;

                COORD new_console_screen_buffer_size = { DEFAULT_BUFFER_X_VALUE, SHRT_MAX - 1 };

                // try adjust console screen buffer's Y value.
                for ( ; new_console_screen_buffer_size.Y > old_console_screen_buffer_size.Y ;
                      -- new_console_screen_buffer_size.Y )
                {
                    if ( SetConsoleScreenBufferSize( hStdOutput, 
                                                     new_console_screen_buffer_size ) )
                    {
                        break; 
                    }
                }
            }
        }

        sm_is_adjust_console_buffer = true;

        UPF_Dump_Memory_Console_Print_Policy::print_delimit_line();
    }

    // make console window be foreground window
    if ( NULL == console_window_handle )
    {
        console_window_handle = UPF_Dump_Memory_Console_Print_Policy::get_console_window();
    }

    if ( NULL != console_window_handle )
    {
        WINDOWPLACEMENT wndpl;
        
        wndpl.length = sizeof( wndpl );
        
        if ( GetWindowPlacement( console_window_handle, &wndpl ) )
        {
            if ( SW_MINIMIZE == wndpl.showCmd ||
                 SW_SHOWMINIMIZED == wndpl.showCmd )
            {
                ShowWindow( console_window_handle, SW_SHOWNORMAL );
            }
        }
        
        if ( GetForegroundWindow() != console_window_handle )
        {
            SetForegroundWindow( console_window_handle );
            BringWindowToTop( console_window_handle );
        }
    }

    // get standard output handle for print operation.
    sm_console_output_handle = GetStdHandle( STD_OUTPUT_HANDLE );
    if ( INVALID_HANDLE_VALUE == sm_console_output_handle )
    {
        sm_console_output_handle = NULL;
    }
}

/** 
 * �ڴ�ӡ�����󱻵���, ���н���.
 * 
 * @param   is_pause       [in]  ���������̨���Ƿ���ͣ
 * 
 */
void 
UPF_Dump_Memory_Console_Print_Policy::on_after_print( bool is_pause )
{
    UPF_Dump_Memory_Console_Print_Policy::print_delimit_line();

    if ( is_pause )
    {
        _cprintf( "Press any key to continue.\n" );
        getch();
    }

    sm_console_print_sync_lock.release();
}

/** 
 * ��ӡ����, ����_cprintf������ݵ�����̨, �����ǰ������г�����ϵͳ������
 * ����������, �ᵯ�����ڸ�Client������ʾ.
 * 
 * @param fmt    [in] ��ʽ��.
 */
void 
UPF_Dump_Memory_Console_Print_Policy::print( const char * fmt, ... )
{
    if ( false == sm_is_console_closed &&
         NULL != sm_console_output_handle )
    {
        va_list arg_list;
        
        va_start( arg_list, fmt );
        
        char buffer[ BUFSIZ + 1 ];
        
        int size = vsnprintf( buffer, BUFSIZ, fmt, arg_list );
        
        if ( -1 == size )
        {
            size = BUFSIZ;
        }
        else if ( size < 0 )
        {
            // if vsnprintf failed, then not output and return.
            return;
        }
        
        buffer[ size ] = '\0';
        
        // here, call _cprintf to print will have some problems.
        // when create console again, _cprint will output nothing.
        // whereas use WriteConsole is right.
        DWORD dwNumberOfCharsWritten = 0;
        
        WriteConsole( sm_console_output_handle, 
                      buffer, 
                      size, 
                      &dwNumberOfCharsWritten, 
                      NULL );
        
        // _cprintf( "%s", buffer );
        
        va_end( arg_list );
        
        CONSOLE_SCREEN_BUFFER_INFO console_buffer_info;
        
        if ( GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), 
            &console_buffer_info ) )
        {
            
            if ( console_buffer_info.dwCursorPosition.Y >= ( console_buffer_info.dwSize.Y - 1 ) &&
                false == sm_is_prompt )
            {
                sm_is_prompt = true;
                
                MessageBox( UPF_Dump_Memory_Console_Print_Policy::get_console_window(), 
                    "The current line count printed has exceeded the max buffer line count.",
                    "UPF Memory Pool Info",
                    MB_OK | MB_ICONINFORMATION );
            }
        }
    }
}

/** 
 * ��ӡһ�зֽ���, �����memory poolһ����printҪ��ʱ, ���ڽ��зֽ�.
 * 
 */
void
UPF_Dump_Memory_Console_Print_Policy::print_delimit_line( void )
{
    CONSOLE_SCREEN_BUFFER_INFO console_buffer_info;
    const char DELIMIT_CHAR = '*';

    if ( GetConsoleScreenBufferInfo( GetStdHandle( STD_OUTPUT_HANDLE ), 
                                     &console_buffer_info ) )
    {
        for ( int i = 0; i < console_buffer_info.dwSize.X; ++i )
        {
            UPF_Dump_Memory_Console_Print_Policy::print( "%c", DELIMIT_CHAR );
        }

        UPF_Dump_Memory_Console_Print_Policy::print( "\n" );
    }
}

/**
 * ����Console���¼�
 *
 * @param dwCtrlType  [in]   ����̨���ڲ������ź�����.
 */
BOOL WINAPI
UPF_Dump_Memory_Console_Print_Policy::console_event_handler( DWORD dwCtrlType )
{
    switch ( dwCtrlType )
    {
    case CTRL_BREAK_EVENT:
    case CTRL_C_EVENT:
        if ( MessageBox( UPF_Dump_Memory_Console_Print_Policy::get_console_window(), 
                         "Do you want to close console?", 
                         "UPF Memory Pool Info",
                         MB_YESNO | MB_ICONQUESTION ) == IDYES )
        {
            sm_is_console_closed     = true;
            sm_console_output_handle = NULL;

            FreeConsole();
        }

        return TRUE;

    default:
        return FALSE;
    }
}

/**
 * �õ�����̨���ڵľ��
 *
 * @retval ����̨���ڵľ��    ȡ�ÿ���̨���ھ���ɹ�
 * @retval NULL                ȡ�ÿ���̨���ھ��ʧ��
 */
HWND 
UPF_Dump_Memory_Console_Print_Policy::get_console_window( void )
{
    HWND result = NULL;

    HMODULE hKernel32 = GetModuleHandle( "Kernel32.dll" );

    if ( NULL != hKernel32 )
    {
        HWND (WINAPI *GetConsoleWindow)(void) = (HWND (WINAPI *)(void)) (
            GetProcAddress( hKernel32, "GetConsoleWindow" ) );

        if ( NULL != GetConsoleWindow )
        {
            result = GetConsoleWindow();
        }

    }

    return result;
}

/************ UPF_Dump_Memory_File_Print_Policy implementation ***************/

/** 
 * �ڴ�ӡ����ǰ������, �Ƚ��м���, Ȼ��ȡ��Dump�ļ������򿪸��ļ�
 * ��Ϊ�����Ĵ�ӡ����׼��.
 */
void 
UPF_Dump_Memory_File_Print_Policy::on_before_print( const char * pool_name,
                                                    const char * file_ext_name )
{
    sm_file_print_sync_lock.acquire();

    sm_dump_file_ext_name = file_ext_name;

    if ( 0 == sm_dump_file_ext_name )
    {
        sm_dump_file_ext_name = ".txt";
    }

    char dump_dir[ MAX_PATH + 1 ];

    if ( UPF_Dump_Memory_File_Print_Policy::get_dump_dir( 
             dump_dir, sizeof( dump_dir ) ) == false )
    {
        return;
    }

    if ( UPF_Dump_Memory_File_Print_Policy::get_dump_file_name( 
             dump_dir, 
             pool_name, 
             sm_dump_file_name, 
             sizeof( sm_dump_file_name ) ) == false )

    {
        return;
    }

    // open dump file for write.
    sm_dump_file = fopen( sm_dump_file_name, "w" );

    if ( NULL == sm_dump_file )
    {
        UPF_Memory_Pool_Util::display_os_error( "dump file (%s) open error.", 
                                                sm_dump_file_name );
    }

}

/** 
 * ȡ��DumpĿ¼, DumpĿ¼����ʽΪ: Dump��Ŀ¼\��ǰ������, �� d:\\test\dump\2007-12-04.
 * 
 * <ol>
 *   <li> ���Ȼ�ȡDump�ĸ�Ŀ¼,\n
 *        �����������UPF_MEMORY_POOL_DUMP_DIR������, ��ȡ����ֵ��Ϊdump��Ŀ¼.\n
 *        ����ȡ��ǰĿ¼�µ�dump��Ŀ¼��Ϊdump��Ŀ¼.
 * 
 *   <li> �����Ŀ¼������, �򴴽���.
 *
 *   <li> ��ȡ��ǰ������, Ȼ�����Dump��Ŀ¼�ϳ�һ��Dump�ļ�Ӧ��ŵ�Ŀ¼.
 * 
 *   <li> �������ϳɵ�Ŀ¼������, �򴴽���.
 * </ol>
 *    
 * 
 * 
 * @param dump_dir_buffer      [out]  һ��������, ���dumpĿ¼��·����, ������ִ��
 *                                    �ɹ�ʱ, �û���������0��ֹ����β.
 * @param buffer_size          [in]   ������dump_dir_buffer�Ĵ�С
 * 
 * @retval true   ȡ��Dump·�����ɹ�
 * @retval false  ȷ��Dump·����ʧ��
 */
bool
UPF_Dump_Memory_File_Print_Policy::get_dump_dir( char * dump_dir_buffer, 
                                                 size_t buffer_size )
{
    assert( dump_dir_buffer );

    const char * DUMP_DIR_ENV_VAR = "UPF_MEMORY_POOL_DUMP_DIR";

    size_t dir_length    = 0;

    char * env_var_value = getenv( DUMP_DIR_ENV_VAR );

    if ( NULL == env_var_value )
    {
        const char DUMP_SUBDIR[] = "\\dump";

        // get current dir as dump file's dir
        if ( _getcwd( dump_dir_buffer, buffer_size ) == NULL  )
        {
            UPF_Memory_Pool_Util::display_os_error( "get current dir error." );
            return false;
        }

        dir_length = strlen( dump_dir_buffer );

        if ( buffer_size >= ( dir_length + sizeof( DUMP_SUBDIR ) ) )
        {
            if ( dump_dir_buffer[ dir_length - 1 ] != '\\' &&
                 dump_dir_buffer[ dir_length - 1 ] != '/' )
            {
                strcpy( dump_dir_buffer + dir_length , DUMP_SUBDIR );

                dir_length += sizeof( DUMP_SUBDIR ) - 1;
            }
            else
            {
                strcpy( dump_dir_buffer + dir_length, DUMP_SUBDIR + 1 );
                dir_length += sizeof( DUMP_SUBDIR ) - 2;
            }

        }
        else
        {
            UPF_Memory_Pool_Util::display_os_error( "dump dir name too long." );
            return false;
        }

    }
    else
    {
        strncpy( dump_dir_buffer, env_var_value, buffer_size );

        dump_dir_buffer[ buffer_size - 1 ] = '\0';

        dir_length = strlen( dump_dir_buffer );
    }

    // if parent dump dir not exist, then create it.
    if ( _access( dump_dir_buffer, 0 ) == -1 )
    {
        if ( _mkdir( dump_dir_buffer ) == -1 )
        {
            UPF_Memory_Pool_Util::display_os_error( "dump dir (%s) create error.",
                                                     dump_dir_buffer );
            return false;
        }
    }

    // current date be dump dir's suffix.
    // form of suffix is \2007-10-12,     
    const size_t DATE_BUFFER_LENGTH     = 10;

    // 1 is '/' length, 10 is '2007-10-12' length, 1 is '\0' length
    const size_t DUMP_DIR_SUFFIX_LENGTH = 1 + DATE_BUFFER_LENGTH + 1; 

    if ( buffer_size >= ( dir_length + DUMP_DIR_SUFFIX_LENGTH ) )
    {           
        char now_date_buffer[ DATE_BUFFER_LENGTH + 1 ];            
        
        if ( UPF_Dump_Memory_File_Print_Policy::get_now_time( 
            now_date_buffer, sizeof( now_date_buffer ), true ) )
        {
            if ( dump_dir_buffer[ dir_length - 1 ] != '\\' &&
                dump_dir_buffer[ dir_length - 1 ] != '/' )
            {
                dump_dir_buffer[ dir_length ] = '\\';
                strcpy( dump_dir_buffer + dir_length + 1, now_date_buffer );
            }
            else
            {
                strcpy( dump_dir_buffer + dir_length, now_date_buffer );
            }
            
            // if dump dir not exist, then create it.
            if ( _access( dump_dir_buffer, 0 ) == -1 )
            {
                if ( _mkdir( dump_dir_buffer ) == -1 )
                {
                    UPF_Memory_Pool_Util::display_os_error( "dump dir (%s) create error.",
                        dump_dir_buffer );
                    
                    return false;
                }
            }
        }

    }
    else
    {
        UPF_Memory_Pool_Util::display_os_error( "dump dir name too long." );
        return false;
    }

    return true;
}

/** 
 * ȡ��Dump���ļ���.
 * 
 * - ��pool_nameΪ��ʱ, ȡһ����ʱ�ļ�����ΪDump���ļ���.
 * - ��pool_name��Ϊ��ʱ, ��pool_name��ΪDump�ļ�����ǰ�벿��, ��ӵ�ǰ��ʱ����
 *   ��.txt����һ��Dump�ļ���.
 * 
 * @param dump_dir                 [in]   Dump·����.
 * @param pool_name                [in]   �ڴ����.
 * @param dump_file_name_buffer    [out]  һ��������, ���dump�ļ���,  ������ִ��
 *                                        �ɹ�ʱ, �û������ᱣ֤��0��ֹ����β.
 * @param buffer_size              [in]   ������dump_file_name_buffer�Ĵ�С.
 * 
 * @retval true  ȡ��Dump�ļ����ɹ�
 * @retval false ȡ��Dump�ļ���ʧ��
 */
bool 
UPF_Dump_Memory_File_Print_Policy::get_dump_file_name( const char * dump_dir,
                                                       const char * pool_name, 
                                                       char *       dump_file_name_buffer, 
                                                       size_t       buffer_size )
{
    assert( dump_dir );
    assert( dump_file_name_buffer );



    if ( 0 == pool_name || '0' == pool_name[ 0 ] )
    {
        const char * TMP_FILE_NAME_PREFIX = "UPF";

        // get temp file name
        if ( GetTempFileName( dump_dir, TMP_FILE_NAME_PREFIX, 0,
                              dump_file_name_buffer ) == 0 )
        {
            UPF_Memory_Pool_Util::display_os_error( "get temp file name error." );
            return false;
        }

        // delete tmp file.
        _unlink( dump_file_name_buffer );

        // because the forms of temp file name is "<path>\<pre><uuuu>.TMP"
        // so when get tmpe file name succeed, then repalce .TMP with .txt

        char * ext_ptr = strrchr( dump_file_name_buffer, '.' );

        if ( 0 != ext_ptr )
        {
            strcpy( ext_ptr, sm_dump_file_ext_name );
        }

    }
    else
    {
        // forms of time is HHMMSS
        const size_t TIME_BUFFER_LENGTH = 6;
       
        size_t dump_dir_length  = strlen( dump_dir );
        size_t pool_name_length = strlen( pool_name );

        // if sm_dump_file_name has no sufficient space to store,
        // then display error and return.
        if ( buffer_size <
             (dump_dir_length + 1 +  pool_name_length + TIME_BUFFER_LENGTH + 4 + 1) )
        {
            UPF_Memory_Pool_Util::display_os_error( "dump file name too long" );
            return false;
        }

        char now_time_buffer[ TIME_BUFFER_LENGTH + 1 ];

        if ( UPF_Dump_Memory_File_Print_Policy::get_now_time( 
                     now_time_buffer, sizeof( now_time_buffer ), false ) == false )
        {
            UPF_Memory_Pool_Util::display_os_error( "get current time error" );
            return false;
        }

        if ( dump_dir[ dump_dir_length - 1 ] != '\\' &&
             dump_dir[ dump_dir_length - 1 ] != '/' )
        {
            snprintf( dump_file_name_buffer, buffer_size, "%s\\%s_%s%s", 
                dump_dir, pool_name, now_time_buffer, sm_dump_file_ext_name );
        }
        else
        {
            snprintf( dump_file_name_buffer, buffer_size, "%s%s_%s%s", 
                dump_dir, pool_name, now_time_buffer, sm_dump_file_ext_name );
        }
        

        dump_file_name_buffer[ buffer_size - 1 ] = '\0';
    }

    return true;
}

/** 
 * ȡ�õ�ǰ�����ڻ�ʱ��.
 *  
 * @param buffer           [out]  ���ڴ�����ڻ�ʱ��Ļ�����, ������ʽΪYYYY-MM-DD,
 *                                ʱ����ʽΪHHMMSS, �������ִ�гɹ����������ᱣ֤
 *                                ��0��ֹ����β.
 * @param buffer_length    [in]   �������Ĵ�С. 
 * @param is_date          [in]   ���Ϊtrue��ʾȡ��������, buffer_length����Ҫ����11,
 *                                ���Ϊfalse��ʾȡ����ʱ��, buffer_length����Ҫ����7.
 * 
 * @retval    true  ȡ�õ�ǰ���ڻ�ʱ��ɹ�
 * @retval    false ȡ�õ�ǰ���ڻ�ʱ��ʧ��
 */
bool 
UPF_Dump_Memory_File_Print_Policy::get_now_time( char * buffer,
                                                 size_t buffer_length,
                                                 bool   is_date )
{
    time_t now;

    time( &now );

    tm * now_tm = localtime( &now );

    if ( NULL != now_tm )
    {            
        if ( is_date )
        {
            assert( buffer_length >= 11 );

            if ( strftime( buffer, buffer_length, "%Y-%m-%d", now_tm ) != 0 )
            {
                return true;
            }
        }
        else
        {
            assert( buffer_length >= 7 );

            if ( strftime( buffer, buffer_length, "%H%M%S", now_tm ) != 0 )
            {
                return true;
            }    
        }
    }

    return false;
}

/** 
 * �ڴ�ӡ�����󱻵���, ���н���, ����ļ�д��ɹ�, ��������Ӧ�ı༭����ʾ���ļ�.
 * 
 */
void 
UPF_Dump_Memory_File_Print_Policy::on_after_print( bool /* is_pause */ )
{
    if ( NULL != sm_dump_file )
    {
        fclose( sm_dump_file );

        sm_dump_file = NULL;

        if ( _access( sm_dump_file_name, 0 ) == 0 )
        {
            ShellExecute( NULL, "open", sm_dump_file_name, 
                          NULL, NULL, SW_SHOWNORMAL );
        }
    }

    sm_file_print_sync_lock.release();


}

/** 
 * ��ӡ����, ��Ҫ��ӡ������������ļ�.
 * 
 * @param fmt    [in] ��ʽ��.
 */
void 
UPF_Dump_Memory_File_Print_Policy::print( const char * fmt, ... )
{
    if ( NULL != sm_dump_file )
    {
        va_list arg_list;
    
        va_start( arg_list, fmt );
    
        char buffer[ BUFSIZ + 1 ];
    
        vsnprintf( buffer, BUFSIZ, fmt, arg_list );
    
        buffer[ BUFSIZ ] = '\0';
    
        fprintf( sm_dump_file, "%s", buffer );
    
        va_end( arg_list );
    }

}
