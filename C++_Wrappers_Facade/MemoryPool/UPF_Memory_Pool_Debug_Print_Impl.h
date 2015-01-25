/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Debug_Print_Impl.h                                                         *
*       CREATE DATE     : 2008-1-24                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]ʵ��UPF_IMemory_Pool_Debug_Print�ӿ�![CN]                                              *
*********************************************************************************************************************/
#ifndef UPF_MEMORY_POOL_DEBUG_PRINT_IMPL_H
#define UPF_MEMORY_POOL_DEBUG_PRINT_IMPL_H

#include "UPF_IMemory_Pool_Debug_Print.h"

/** 
 * @class UPF_Memory_Pool_Debug_Console_Print_Impl
 * 
 * @brief �����Ϣ������̨.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Debug_Console_Print_Impl :
     public UPF_IMemory_Pool_Debug_Print
{
public:
    /// �����ǰ��һЩ׼������.
    virtual void on_before_print( const char * pool_name     = "", 
                                  const char * file_ext_name = ".txt" );

    /// ���������һЩ����.
    virtual void on_after_print( bool is_pause = false );   

    /// �����Ϣ.
    virtual void print( const char * fmt, ... );

    /// �����ʱ�����ڻ�ȡ��֮��ķָ���.
    virtual char get_delimit_char( void )
    {
        return ' ';
    }
};

/** 
 * @class UPF_Memory_Pool_Debug_File_Print_Impl
 * 
 * @brief �����Ϣ���ļ�.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Debug_File_Print_Impl :
     public UPF_IMemory_Pool_Debug_Print
{
public:
    /// �����ǰ��һЩ׼������.
    virtual void on_before_print( const char * pool_name     = "", 
                                  const char * file_ext_name = ".txt" );

    /// ���������һЩ����.
    virtual void on_after_print( bool is_pause = false );   

    /// �����Ϣ.
    virtual void print( const char * fmt, ... );

    /// �����ʱ�����ڻ�ȡ��֮��ķָ���.
    virtual char get_delimit_char( void )
    {
        return ',';
    }
};

/** 
 * @class UPF_Memory_Pool_Debug_Console_And_File_Print_Impl
 * 
 * @brief �����Ϣ���ļ��Ϳ���̨.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Debug_Console_And_File_Print_Impl :
     public UPF_IMemory_Pool_Debug_Print
{
public:
    /// �����ǰ��һЩ׼������.
    virtual void on_before_print( const char * pool_name     = "", 
                                  const char * file_ext_name = ".txt" );

    /// ���������һЩ����.
    virtual void on_after_print( bool is_pause = false );   

    /// �����Ϣ.
    virtual void print( const char * fmt, ... );

    /// �����ʱ�����ڻ�ȡ��֮��ķָ���.
    virtual char get_delimit_char( void )
    {
        return ' ';
    }
};


/** 
 * @class UPF_Memory_Pool_Debug_Console_And_File_Print_Impl
 * 
 * @brief ����OutputDebugString���������Ϣ.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_OutputDebugString_Print_Impl :
     public UPF_IMemory_Pool_Debug_Print
{
public:
    /// �����ǰ��һЩ׼������.
    virtual void on_before_print( const char * pool_name     = "", 
                                  const char * file_ext_name = ".txt" );

    /// ���������һЩ����.
    virtual void on_after_print( bool is_pause = false );   

    /// �����Ϣ.
    virtual void print( const char * fmt, ... );

    /// �����ʱ�����ڻ�ȡ��֮��ķָ���.
    virtual char get_delimit_char( void )
    {
        return ' ';
    }
};


#endif /* UPF_MEMORY_POOL_DEBUG_PRINT_IMPL_H */

