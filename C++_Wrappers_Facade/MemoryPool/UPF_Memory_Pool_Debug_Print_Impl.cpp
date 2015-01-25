/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Debug_Print_Impl.cpp                                                       *
*       CREATE DATE     : 2008-1-24                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]ʵ��UPF_IMemory_Pool_Debug_Print�ӿ�![CN]                                              *
*********************************************************************************************************************/
#include <stdio.h>                /* for _vsnprintf */
#include <stdarg.h>               /* for va_start, va_end, va_arg */
#include <windows.h>

#include "UPF_Memory_Pool_Debug_Print_Impl.h"
#include "UPF_Print_Policy.h"
#include "UPF_Memory_Pool_Define.h"   /* for BUFSIZ */

#if defined(WIN32) || defined(_WIN32)
#define vsnprintf       _vsnprintf     /* on win32 platform, be _vsnprintf */
#endif


/********* UPF_Memory_Pool_Debug_Console_Print_Impl implementation ********/

/** 
 * �����ǰ��һЩ׼������.
 *  
 * @param pool_name         [in]    �ڴ������.
 * @param file_ext_name     [in]    ��������ļ�ʱ, �����ļ�������չ��,
 *                                  �����������̨ʱ, ����ʹ���������.
 * 
 */
void
UPF_Memory_Pool_Debug_Console_Print_Impl::on_before_print(
    const char * pool_name, const char * /* file_ext_name */ )
{
    UPF_Dump_Memory_Console_Print_Policy::on_before_print( pool_name );
}

/** 
 * ���������һЩ����.
 * 
 * @param is_pause          [in]    ����ָʾ������Ƿ���ͣ.
 */
void
UPF_Memory_Pool_Debug_Console_Print_Impl::on_after_print( bool is_pause )
{
    UPF_Dump_Memory_Console_Print_Policy::on_after_print( is_pause );
}

/** 
 * �����Ϣ.
 * 
 * @param fmt               [in]    �����ʽ.
 */
void
UPF_Memory_Pool_Debug_Console_Print_Impl::print( const char * fmt, ... )
{
    va_list arg_list;

    va_start( arg_list, fmt );

    char buffer[ BUFSIZ + 1 ];

    vsnprintf( buffer, BUFSIZ, fmt, arg_list );

    buffer[ BUFSIZ ] = '\0';  

    va_end( arg_list );

    UPF_Dump_Memory_Console_Print_Policy::print( "%s", buffer );

}

/********* UPF_Memory_Pool_Debug_File_Print_Impl implementation ********/

/** 
 * �����ǰ��һЩ׼������.
 *  
 * @param pool_name         [in]    �ڴ������.
 * @param file_ext_name     [in]    ��������ļ�ʱ, �����ļ�������չ��,
 *                                  �����������̨ʱ, ����ʹ���������.
 *                                 
 * 
 */
void
UPF_Memory_Pool_Debug_File_Print_Impl::on_before_print(
    const char * pool_name, const char * file_ext_name )
{
    UPF_Dump_Memory_File_Print_Policy::on_before_print( pool_name, 
                                                        file_ext_name );
}

/** 
 * ���������һЩ����.
 * 
 * @param is_pause          [in]    ����ָʾ������Ƿ���ͣ.
 */
void
UPF_Memory_Pool_Debug_File_Print_Impl::on_after_print( bool /* is_pause */ )
{
    UPF_Dump_Memory_File_Print_Policy::on_after_print();
}

/** 
 * �����Ϣ.
 * 
 * @param fmt               [in]    �����ʽ.
 */
void
UPF_Memory_Pool_Debug_File_Print_Impl::print( const char * fmt, ... )
{
    va_list arg_list;

    va_start( arg_list, fmt );

    char buffer[ BUFSIZ + 1 ];

    vsnprintf( buffer, BUFSIZ, fmt, arg_list );

    buffer[ BUFSIZ ] = '\0';

    va_end( arg_list );

    UPF_Dump_Memory_File_Print_Policy::print( "%s", buffer );
}

/****** UPF_Memory_Pool_Debug_Console_And_File_Print_Impl implementation *****/
/** 
 * �����ǰ��һЩ׼������.
 *  
 * @param pool_name         [in]    �ڴ������.
 * @param file_ext_name     [in]    ��������ļ�ʱ, �����ļ�������չ��,
 *                                  �����������̨ʱ, ����ʹ���������.
 *                                 
 * 
 */
void
UPF_Memory_Pool_Debug_Console_And_File_Print_Impl::on_before_print(
    const char * pool_name, const char * file_ext_name )
{
    UPF_Dump_Memory_Console_Print_Policy::on_before_print( pool_name );
    UPF_Dump_Memory_File_Print_Policy::on_before_print( pool_name, 
                                                        file_ext_name );
}

/** 
 * ���������һЩ����.
 * 
 * @param is_pause          [in]    ����ָʾ������Ƿ���ͣ.
 */
void
UPF_Memory_Pool_Debug_Console_And_File_Print_Impl::on_after_print(
     bool  is_pause )
{
    UPF_Dump_Memory_File_Print_Policy::on_after_print();
    UPF_Dump_Memory_Console_Print_Policy::on_after_print( is_pause );
}

/** 
 * �����Ϣ.
 * 
 * @param fmt               [in]    �����ʽ.
 */
void
UPF_Memory_Pool_Debug_Console_And_File_Print_Impl::print( const char * fmt,
                                                          ... )
{
    va_list arg_list;

    va_start( arg_list, fmt );

    char buffer[ BUFSIZ + 1 ];

    vsnprintf( buffer, BUFSIZ, fmt, arg_list );

    buffer[ BUFSIZ ] = '\0';

    va_end( arg_list );

    UPF_Dump_Memory_File_Print_Policy::print( "%s", buffer );
    UPF_Dump_Memory_Console_Print_Policy::print( "%s", buffer );
}


/****** UPF_Memory_Pool_OutputDebugString_Print_Impl implementation *****/

void 
UPF_Memory_Pool_OutputDebugString_Print_Impl::on_before_print(
    const char * pool_name, 
    const char * file_ext_name )
{
    UPF_Memory_Pool_OutputDebugString_Print_Impl::print(
        "********************************************************************\n" );
}


void 
UPF_Memory_Pool_OutputDebugString_Print_Impl::on_after_print( bool is_pause )
{
    UPF_Memory_Pool_OutputDebugString_Print_Impl::print(
        "\n********************************************************************\n" );
}


void 
UPF_Memory_Pool_OutputDebugString_Print_Impl::print( const char * fmt, ... )
{
    va_list arg_list;

    va_start( arg_list, fmt );

    char buffer[ BUFSIZ + 1 ];

    vsnprintf( buffer, BUFSIZ, fmt, arg_list );

    buffer[ BUFSIZ ] = '\0';

    va_end( arg_list );

   //   OutputDebugString( buffer );

	UPF_OS::trace(buffer);
}
