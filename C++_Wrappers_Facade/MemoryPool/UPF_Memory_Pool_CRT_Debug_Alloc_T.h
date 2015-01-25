/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_CRT_Debug_Alloc_T.h                                                        *
*       CREATE DATE     : 2008-1-9                                                                                   *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]����һ��ʹ��CRT Debug���������ڴ��UPF_IMemory_Pool��ʵ��![CN]                         *
********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_CRT_DEBUG_ALLOC_T_H
#define UPF_MEMORY_POOL_CRT_DEBUG_ALLOC_T_H

#include "UPF_Memory_Pool_Base_T.h" 

/** 
 * @class UPF_Memory_Pool_CRT_Debug_Alloc_T
 * 
 * @brief ʵ��UPF_IMemory_Pool�ӿڣ�ʹ��CRT Debug���������ڴ�.
 * 
 * @details ����CRT Debug���ṩ���ڴ�Խ��У�飬���ڴ�й¶���ȹ��ܣ�������ʹ��
 * CRT Debug���������ڴ棬���ڵ���һЩ�ڴ淽��Ĵ���       
 * �����ڴ�й¶ʱ��й©����Ϣ���������������Output����.
 * 
 * �����ڴ�ʹ����Ϣ�ļ���������������CRT Debug������������ֻ��
 * allocate size ����.
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Memory_Pool_CRT_Debug_Alloc_T : public UPF_Memory_Pool_Base< Allocator,
                                                                       Lock,
                                                                       RecordPolicy >
{
public:
    /// ��ʼ���ڲ�״̬.
    UPF_Memory_Pool_CRT_Debug_Alloc_T( const char *          memory_pool_name,
                                       size_t                allocated_memory_limit = MAX_MEMORY_REQ_SIZE,
                                       toplevel_allocator_t  allocator_type = UPF_TOPLEVEL_PERMANENT_ALLOCATOR );

public:
    virtual void * allocate( size_t size )
    {
        return allocate_debug( size, 0, 0 );
    }

    virtual void * reallocate( void * ptr,
                               size_t new_size )
    {
        return reallocate_debug( ptr, new_size, 0, 0 );
    }


    virtual void * allocate_debug( size_t       size, 
                                   const char * file_name,
                                   int          line_no );


    virtual void * reallocate_debug( void *       ptr,
                                     size_t       new_size,
                                     const char * file_name,
                                     int          line_no );

    virtual void deallocate( void * ptr, size_t size );

private:
    /// CRT�ڴ��Ľڵ�����, ���һ���ֶ�Ϊalloc size�ֶ�, ��Ϊ���ͷ��ڴ�ʱ��
    /// ������ڴ�ָ�����ƫ��4���ֽ�, ��ȷ�����ڴ�Ĵ�С.
    struct CRT_Debug_Block_Node
    {
        /// �ڴ��Ĵ�С���������ڴ��ID
        size_t id_alloc_size;
    };

    /// CRT_Debug_Block_Node�Ĵ�С����
    enum { CRT_DEBUG_BLOCK_NODE_SIZE = sizeof( CRT_Debug_Block_Node ) };

private:
    typedef UPF_Memory_Pool_CRT_Debug_Alloc_T< Allocator,
                                               Lock,
                                               RecordPolicy > SELF_CLASS;

private:
    /// ��ʼ��crt debug�ѵ�һЩ��־����.
    static void init_crt_debug( void );
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_CRT_Debug_Alloc_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_MEMORY_POOL_CRT_DEBUG_ALLOC_T_H */

