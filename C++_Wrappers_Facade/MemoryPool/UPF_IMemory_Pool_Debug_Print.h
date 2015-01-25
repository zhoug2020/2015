/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IMemory_Pool_Debug_Print.h                                                             *
*       CREATE DATE     : 2008-1-24                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]用于UPF Memory Pool Debug信息的输出![CN]                                               *
*********************************************************************************************************************/
#ifndef UPF_IMEMORY_POOL_DEBUG_PRINT_H
#define UPF_IMEMORY_POOL_DEBUG_PRINT_H

#include "..\..\OS_Adapter\UPF_OS.h"

/** 
 * @class UPF_IMemory_Pool_Debug_Print
 * 
 * @brief 用于UPF Memory Pool Debug信息的输出接口, 主要是提供给UPF_Memory_Pool_Debug_T
 * 来使用的.
 * 
 */
class /*UPF_Export*/ UPF_IMemory_Pool_Debug_Print
{
public:
    /** 
     * 在输出前做一些准备工作.
     *  
     * @param pool_name         [in]    内存池名字.
     * @param file_ext_name     [in]    当输出到文件时, 用于文件名的扩展名,
     *                                  当输出到控制台时, 并不使用这个参数.
     * 
     */
    virtual void on_before_print( const char * pool_name     = "",
                                  const char * file_ext_name = ".txt" ) = 0;

    /** 
     * 在输出后做一些工作.
     * 
     * @param is_pause          [in]    用于指示输出后是否暂停.
     */
    virtual void on_after_print( bool is_pause = false ) = 0;

    /** 
     * 输出信息.
     * 
     * @param fmt               [in]    输出格式.
     */
    virtual void print( const char * fmt, ... ) = 0;

    /** 
     * 当输出时，用于获取列之间的分隔符.
     * 
     * 
     * @return 分隔符字符.
     */
    virtual char get_delimit_char( void ) = 0;

public:
    virtual ~UPF_IMemory_Pool_Debug_Print()
    {
    }
};


#endif /* UPF_IMEMORY_POOL_DEBUG_PRINT_H */

