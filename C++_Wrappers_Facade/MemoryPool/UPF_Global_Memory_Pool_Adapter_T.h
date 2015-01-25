/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Global_Memory_Pool_Adapter_T.h                                                         *
*       CREATE DATE     : 2008-7-18                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]����ʵ��UPF_IMemory_Pool��ȫ���ڴ�ص�һ��adapter![CN]                                 *
*********************************************************************************************************************/
#ifndef UPF_GLOBAL_MEMORY_POOL_ADAPTER_T_H
#define UPF_GLOBAL_MEMORY_POOL_ADAPTER_T_H

#include "UPF_Memory_Pool_Base_T.h"    
#include "UPF_Memory_Pool_Define.h"    
#include "UPF_IGlobal_Memory_Pool.h"

/** 
 * @class UPF_Global_Memory_Pool_Adapter_T
 * 
 * @brief ʵ��UPF_IMemory_Pool��ȫ���ڴ�ص�һ��adapter
 * 
 */
template < class Allocator, class Lock, class RecordPolicy > 
class UPF_Global_Memory_Pool_Adapter_T : public UPF_Memory_Pool_Base< 
                                                              Allocator,
                                                              Lock,
                                                              RecordPolicy >
{
public:
    /// ��ʼ���ڲ�״̬.
    UPF_Global_Memory_Pool_Adapter_T( const char *          memory_pool_name,
                                      size_t                allocated_memory_limit = MAX_MEMORY_REQ_SIZE,
                                      toplevel_allocator_t  allocator_type = UPF_TOPLEVEL_PERMANENT_ALLOCATOR );

    /// ����ڴ���ڲ�����������ڴ�
    ~UPF_Global_Memory_Pool_Adapter_T( void );

public:
    /// ���ڴ���з����ڴ�.
    virtual void* allocate( size_t size );
    
    /// �ͷ��ڴ�.
    virtual void deallocate( void* ptr, size_t size );

    /// �ӿɱ��С���ڴ�������·����ڴ�.
    virtual void * reallocate( void * ptr, size_t new_size );

private:
    /// ���ڱ���ȫ���ڴ�ص�һ������.
    UPF_IGlobal_Memory_Pool *   m_memory_pool;
   
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Global_Memory_Pool_Adapter_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_GLOBAL_MEMORY_POOL_ADAPTER_T_H */

