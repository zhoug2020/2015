/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Print_Policy.cpp                                                                       *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]打印策略类WINNT平台的实现![CN]                                                         *
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
 * 在开始打印操作前被调用, 初始化当前消息缓存区的位置为0.
 * 
 */
void 
UPF_MessageBox_Print_Policy::on_before_print( const char * /* pool_name */,
                                              const char * /* file_ext_name */ )
{
    sm_message_buffer_pos = 0;
}

/** 
 * 在打印操作结束后被调用, 这里调用MessageBox显示一个对话框.
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
 * 打印操作, 把需要打印的内容经过格式化后放到内部的消息缓存区中, 如果需要打印的内容超出了
 * 消息缓存区的最大值，则多出来的部分不会放到消息缓存区中，这个消息缓冲区的内容在调用者
 * 打印操作完毕后, 调用on_after_print时输出.
 * 
 * @param fmt    [in] 打印格式串
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
 * 在打印操作前被调用, 先进行加锁. 通常的控制台大小为80列, 300行，
 * 不能满足Dump memory的显示要求, 所以这里把控制台调整为100列, 行数为系统允许的最大值.
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
 * 在打印操作后被调用, 进行解锁.
 * 
 * @param   is_pause       [in]  输出到控制台后是否暂停
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
 * 打印操作, 调用_cprintf输出内容到控制台, 如果当前输出的行超过了系统的允许
 * 输出的最大行, 会弹出窗口给Client端以提示.
 * 
 * @param fmt    [in] 格式串.
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
 * 打印一行分界线, 当多个memory pool一起有print要求时, 用于进行分界.
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
 * 处理Console的事件
 *
 * @param dwCtrlType  [in]   控制台窗口产生的信号类型.
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
 * 得到控制台窗口的句柄
 *
 * @retval 控制台窗口的句柄    取得控制台窗口句柄成功
 * @retval NULL                取得控制台窗口句柄失败
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
 * 在打印操作前被调用, 先进行加锁, 然后取得Dump文件名并打开该文件
 * 以为后续的打印操作准备.
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
 * 取得Dump目录, Dump目录的形式为: Dump父目录\当前年月日, 如 d:\\test\dump\2007-12-04.
 * 
 * <ol>
 *   <li> 首先获取Dump的父目录,\n
 *        如果环境变量UPF_MEMORY_POOL_DUMP_DIR被设置, 则取它的值作为dump父目录.\n
 *        否则取当前目录下的dump子目录做为dump父目录.
 * 
 *   <li> 如果父目录不存在, 则创建它.
 *
 *   <li> 获取当前年月日, 然后根据Dump父目录合成一个Dump文件应存放的目录.
 * 
 *   <li> 如果这个合成的目录不存在, 则创建它.
 * </ol>
 *    
 * 
 * 
 * @param dump_dir_buffer      [out]  一个缓存区, 存放dump目录的路径名, 当函数执行
 *                                    成功时, 该缓冲区会以0终止符结尾.
 * @param buffer_size          [in]   缓存区dump_dir_buffer的大小
 * 
 * @retval true   取得Dump路径名成功
 * @retval false  确得Dump路径名失败
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
 * 取得Dump的文件名.
 * 
 * - 当pool_name为空时, 取一个临时文件名作为Dump的文件名.
 * - 当pool_name不为空时, 以pool_name作为Dump文件名的前半部分, 后接当前的时分秒
 *   和.txt构成一个Dump文件名.
 * 
 * @param dump_dir                 [in]   Dump路径名.
 * @param pool_name                [in]   内存池名.
 * @param dump_file_name_buffer    [out]  一个缓存区, 存放dump文件名,  当函数执行
 *                                        成功时, 该缓冲区会保证以0终止符结尾.
 * @param buffer_size              [in]   缓存区dump_file_name_buffer的大小.
 * 
 * @retval true  取得Dump文件名成功
 * @retval false 取得Dump文件名失败
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
 * 取得当前的日期或时间.
 *  
 * @param buffer           [out]  用于存放日期或时间的缓存区, 日期形式为YYYY-MM-DD,
 *                                时间形式为HHMMSS, 如果函数执行成功，缓存区会保证
 *                                以0终止符结尾.
 * @param buffer_length    [in]   缓存区的大小. 
 * @param is_date          [in]   如果为true表示取的是日期, buffer_length必须要大于11,
 *                                如果为false表示取的是时间, buffer_length必须要大于7.
 * 
 * @retval    true  取得当前日期或时间成功
 * @retval    false 取得当前日期或时间失败
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
 * 在打印操作后被调用, 进行解锁, 如果文件写入成功, 则会调用相应的编辑器显示该文件.
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
 * 打印操作, 把要打印的内容输出到文件.
 * 
 * @param fmt    [in] 格式串.
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
