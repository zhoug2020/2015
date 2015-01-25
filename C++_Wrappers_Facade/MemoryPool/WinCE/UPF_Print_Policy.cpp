/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Print_Policy.cpp                                                                       *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]��ӡ�������WINCEƽ̨ʵ��![CN]                                                         *
*********************************************************************************************************************/

#include "UPF_Print_Policy.h"
#include "UPF_Memory_Pool_Util.h"      /* for display_os_error */
#include "../../../OS_Adapter/WinCE_OS/UPF_Ascii_To_Wide.h"

#include <windows.h>              
#include <shellapi.h>

#define vsnprintf       _vsnprintf     /* on win32 platform, be _vsnprintf */
#define snprintf        _snprintf     /* on win32 platform, be _snprintf */

#include <assert.h>                    /* for assert */
#include <limits.h>                    /* for SHRT_MAX */
#include <stdlib.h>                    /* for getenv */
#include <string.h>                    /* for strncpy, strlen, strrchr, strcpy, strcat */
#include <stdio.h>                     /* for fopen, fprintf, fclose */

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

// macro definitions.
#define TMP_FILE_NAME_PREFIX         "UPF"
 

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

    UPF_Memory_Pool_Util::report("UPF Memory Pool Info" );
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
 * @todo WINCE �ݲ�ʵ��, ��ΪĿǰconsole��صĺ���, WINCEû���ṩ.
 * 
 */
void 
UPF_Dump_Memory_Console_Print_Policy::on_before_print( const char * /* pool_name */,
                                                       const char * /* file_ext_name */ )
{
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
}

/** 
 * ��ӡһ�зֽ���, �����memory poolһ����printҪ��ʱ, ���ڽ��зֽ�.
 * 
 */
void
UPF_Dump_Memory_Console_Print_Policy::print_delimit_line( void )
{
}

/**
 * ����Console���¼�
 *
 * @param dwCtrlType  [in]   ����̨���ڲ������ź�����.
 */
BOOL WINAPI
UPF_Dump_Memory_Console_Print_Policy::console_event_handler( DWORD dwCtrlType )
{
    return TRUE;
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
    return NULL;
}

/************ UPF_Dump_Memory_File_Print_Policy implementation ***************/

/// help function.
/**
 * ȡ������ֵ
 *
 * @todo �Ժ�ʹ�������ļ���ȡ.
 *
 */
static char * get_profile_string( const char * entry_name )
{
    return NULL;
}

/**
 * �ж��ļ���Ŀ¼�Ƿ����
 *
 * @param   file_name       [in]   �ļ�����Ŀ¼��.
 *
 * @retval  true   �ļ���Ŀ¼����.
 * @retval  false  �ļ���Ŀ¼������.
 */
static bool is_file_exist( const char * file_name )
{
    WIN32_FIND_DATA find_file_data;

    HANDLE hFile = FindFirstFile( UPF_A2T(file_name), 
                                  &find_file_data );

    if ( INVALID_HANDLE_VALUE == hFile )
    {
        return false;
    }
    else
    {
        CloseHandle( hFile );
        return true;
    }
}

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

    char * env_var_value = get_profile_string( DUMP_DIR_ENV_VAR );

    if ( NULL == env_var_value )
    {
        const char DUMP_SUBDIR[] = "\\dump";

        // get current dir as dump file's dir
        strcpy( dump_dir_buffer, "./" );

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

    if ( ! is_file_exist( dump_dir_buffer ) )    
    {
        if ( CreateDirectory( UPF_A2T(dump_dir_buffer),
                              NULL ) == 0 )
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
            if ( ! is_file_exist( dump_dir_buffer ) )
            {
                if ( CreateDirectory( UPF_A2T(dump_dir_buffer),
                                      NULL ) == 0 )
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
        // get temp file name
        if ( GetTempFileName( UPF_A2T(dump_dir), 
                              UPF_TEXT(TMP_FILE_NAME_PREFIX), 
                              0,
                              UPF_A2T(dump_file_name_buffer) ) == 0 )
        {
            UPF_Memory_Pool_Util::display_os_error( "get temp file name error." );
            return false;
        }

        // delete tmp file.
        DeleteFile( UPF_A2T(dump_file_name_buffer) );

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
    SYSTEMTIME st;

    GetLocalTime( &st );
          
    if ( is_date )
    {
        assert( buffer_length >= 11 );

        snprintf( buffer, 
                  buffer_length, 
                  "%04d-%02d-%02d", 
                  st.wYear,
                  st.wMonth,
                  st.wDay );

        buffer[ buffer_length - 1 ] = '\0';
    }
    else
    {
        assert( buffer_length >= 7 );

        snprintf( buffer, 
                  buffer_length, 
                  "%02d%02d%02d", 
                  st.wHour,
                  st.wMinute,
                  st.wSecond );
        
        buffer[ buffer_length - 1 ] = '\0';
    }


    return true;
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

        if ( is_file_exist( sm_dump_file_name ) )
        {
            UPF_Ascii_To_Wide atw( sm_dump_file_name );

            SHELLEXECUTEINFO sei = { 0 };

            sei.cbSize = sizeof( SHELLEXECUTEINFO );
            sei.fMask  = SEE_MASK_NOCLOSEPROCESS;
            sei.lpVerb = UPF_TEXT("open");
            sei.lpFile = atw.wchar_rep();
            sei.nShow  = SW_SHOWNORMAL;

            ShellExecuteEx( &sei );
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
