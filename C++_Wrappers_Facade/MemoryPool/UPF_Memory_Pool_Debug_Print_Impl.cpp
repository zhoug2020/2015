/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Debug_Print_Impl.cpp                                                       *
*       CREATE DATE     : 2008-1-24                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]实现UPF_IMemory_Pool_Debug_Print接口![CN]                                              *
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
 * 在输出前做一些准备工作.
 *  
 * @param pool_name         [in]    内存池名字.
 * @param file_ext_name     [in]    当输出到文件时, 用于文件名的扩展名,
 *                                  当输出到控制台时, 并不使用这个参数.
 * 
 */
void
UPF_Memory_Pool_Debug_Console_Print_Impl::on_before_print(
    const char * pool_name, const char * /* file_ext_name */ )
{
    UPF_Dump_Memory_Console_Print_Policy::on_before_print( pool_name );
}

/** 
 * 在输出后做一些工作.
 * 
 * @param is_pause          [in]    用于指示输出后是否暂停.
 */
void
UPF_Memory_Pool_Debug_Console_Print_Impl::on_after_print( bool is_pause )
{
    UPF_Dump_Memory_Console_Print_Policy::on_after_print( is_pause );
}

/** 
 * 输出信息.
 * 
 * @param fmt               [in]    输出格式.
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
 * 在输出前做一些准备工作.
 *  
 * @param pool_name         [in]    内存池名字.
 * @param file_ext_name     [in]    当输出到文件时, 用于文件名的扩展名,
 *                                  当输出到控制台时, 并不使用这个参数.
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
 * 在输出后做一些工作.
 * 
 * @param is_pause          [in]    用于指示输出后是否暂停.
 */
void
UPF_Memory_Pool_Debug_File_Print_Impl::on_after_print( bool /* is_pause */ )
{
    UPF_Dump_Memory_File_Print_Policy::on_after_print();
}

/** 
 * 输出信息.
 * 
 * @param fmt               [in]    输出格式.
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
 * 在输出前做一些准备工作.
 *  
 * @param pool_name         [in]    内存池名字.
 * @param file_ext_name     [in]    当输出到文件时, 用于文件名的扩展名,
 *                                  当输出到控制台时, 并不使用这个参数.
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
 * 在输出后做一些工作.
 * 
 * @param is_pause          [in]    用于指示输出后是否暂停.
 */
void
UPF_Memory_Pool_Debug_Console_And_File_Print_Impl::on_after_print(
     bool  is_pause )
{
    UPF_Dump_Memory_File_Print_Policy::on_after_print();
    UPF_Dump_Memory_Console_Print_Policy::on_after_print( is_pause );
}

/** 
 * 输出信息.
 * 
 * @param fmt               [in]    输出格式.
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
