/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Debug_Print_Impl.h                                                         *
*       CREATE DATE     : 2008-1-24                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]实现UPF_IMemory_Pool_Debug_Print接口![CN]                                              *
*********************************************************************************************************************/
#ifndef UPF_MEMORY_POOL_DEBUG_PRINT_IMPL_H
#define UPF_MEMORY_POOL_DEBUG_PRINT_IMPL_H

#include "UPF_IMemory_Pool_Debug_Print.h"

/** 
 * @class UPF_Memory_Pool_Debug_Console_Print_Impl
 * 
 * @brief 输出信息到控制台.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Debug_Console_Print_Impl :
     public UPF_IMemory_Pool_Debug_Print
{
public:
    /// 在输出前做一些准备工作.
    virtual void on_before_print( const char * pool_name     = "", 
                                  const char * file_ext_name = ".txt" );

    /// 在输出后做一些工作.
    virtual void on_after_print( bool is_pause = false );   

    /// 输出信息.
    virtual void print( const char * fmt, ... );

    /// 当输出时，用于获取列之间的分隔符.
    virtual char get_delimit_char( void )
    {
        return ' ';
    }
};

/** 
 * @class UPF_Memory_Pool_Debug_File_Print_Impl
 * 
 * @brief 输出信息到文件.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Debug_File_Print_Impl :
     public UPF_IMemory_Pool_Debug_Print
{
public:
    /// 在输出前做一些准备工作.
    virtual void on_before_print( const char * pool_name     = "", 
                                  const char * file_ext_name = ".txt" );

    /// 在输出后做一些工作.
    virtual void on_after_print( bool is_pause = false );   

    /// 输出信息.
    virtual void print( const char * fmt, ... );

    /// 当输出时，用于获取列之间的分隔符.
    virtual char get_delimit_char( void )
    {
        return ',';
    }
};

/** 
 * @class UPF_Memory_Pool_Debug_Console_And_File_Print_Impl
 * 
 * @brief 输出信息到文件和控制台.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Debug_Console_And_File_Print_Impl :
     public UPF_IMemory_Pool_Debug_Print
{
public:
    /// 在输出前做一些准备工作.
    virtual void on_before_print( const char * pool_name     = "", 
                                  const char * file_ext_name = ".txt" );

    /// 在输出后做一些工作.
    virtual void on_after_print( bool is_pause = false );   

    /// 输出信息.
    virtual void print( const char * fmt, ... );

    /// 当输出时，用于获取列之间的分隔符.
    virtual char get_delimit_char( void )
    {
        return ' ';
    }
};


/** 
 * @class UPF_Memory_Pool_Debug_Console_And_File_Print_Impl
 * 
 * @brief 调用OutputDebugString进行输出信息.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_OutputDebugString_Print_Impl :
     public UPF_IMemory_Pool_Debug_Print
{
public:
    /// 在输出前做一些准备工作.
    virtual void on_before_print( const char * pool_name     = "", 
                                  const char * file_ext_name = ".txt" );

    /// 在输出后做一些工作.
    virtual void on_after_print( bool is_pause = false );   

    /// 输出信息.
    virtual void print( const char * fmt, ... );

    /// 当输出时，用于获取列之间的分隔符.
    virtual char get_delimit_char( void )
    {
        return ' ';
    }
};


#endif /* UPF_MEMORY_POOL_DEBUG_PRINT_IMPL_H */

