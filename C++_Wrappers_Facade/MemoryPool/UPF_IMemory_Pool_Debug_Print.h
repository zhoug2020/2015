/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IMemory_Pool_Debug_Print.h                                                             *
*       CREATE DATE     : 2008-1-24                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]����UPF Memory Pool Debug��Ϣ�����![CN]                                               *
*********************************************************************************************************************/
#ifndef UPF_IMEMORY_POOL_DEBUG_PRINT_H
#define UPF_IMEMORY_POOL_DEBUG_PRINT_H

#include "..\..\OS_Adapter\UPF_OS.h"

/** 
 * @class UPF_IMemory_Pool_Debug_Print
 * 
 * @brief ����UPF Memory Pool Debug��Ϣ������ӿ�, ��Ҫ���ṩ��UPF_Memory_Pool_Debug_T
 * ��ʹ�õ�.
 * 
 */
class /*UPF_Export*/ UPF_IMemory_Pool_Debug_Print
{
public:
    /** 
     * �����ǰ��һЩ׼������.
     *  
     * @param pool_name         [in]    �ڴ������.
     * @param file_ext_name     [in]    ��������ļ�ʱ, �����ļ�������չ��,
     *                                  �����������̨ʱ, ����ʹ���������.
     * 
     */
    virtual void on_before_print( const char * pool_name     = "",
                                  const char * file_ext_name = ".txt" ) = 0;

    /** 
     * ���������һЩ����.
     * 
     * @param is_pause          [in]    ����ָʾ������Ƿ���ͣ.
     */
    virtual void on_after_print( bool is_pause = false ) = 0;

    /** 
     * �����Ϣ.
     * 
     * @param fmt               [in]    �����ʽ.
     */
    virtual void print( const char * fmt, ... ) = 0;

    /** 
     * �����ʱ�����ڻ�ȡ��֮��ķָ���.
     * 
     * 
     * @return �ָ����ַ�.
     */
    virtual char get_delimit_char( void ) = 0;

public:
    virtual ~UPF_IMemory_Pool_Debug_Print()
    {
    }
};


#endif /* UPF_IMEMORY_POOL_DEBUG_PRINT_H */

