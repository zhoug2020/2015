/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Singleton.h                                                                  *
*       CREATE DATE     : 2007-12-13                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]һ������ģ����![CN]                                                                    *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_SINGLETON_H
#define UPF_MEMORY_POOL_SINGLETON_H

#include <stdlib.h>                         /* for atexit */

#include "UPF_Memory_Pool_Define.h"
#include "UPF_Memory_Pool_Traits.h"
#include "UPF_Thread_Sync.h"
#include "UPF_Malloc.h"


/** 
 * @class UPF_Memory_Pool_Singleton
 * 
 * @brief ����UPF_Pool_Object_Base��UPF_STL_Allocator_Adapter�����ڴ�.
 * 
 * @details ��UPF_Pool_Object_Base��UPF_STL_Allocator_Adapter�����ڴ�ʱ, ��Ҫʹ��
 *          һ���ڴ�ص����������ڴ�.
 * 
 * - module_id  ģ��ID
 * - Lock       �߳�ͬ����
 *
 * @note UPF_Memory_Pool_Singleton �� toplevel allocator type ʹ��
 *       UPF_TOPLEVEL_PERMANENT_ALLOCATOR.
 * 
 */
template < size_t module_id, class Lock >
class UPF_Memory_Pool_Singleton
{
public:
    static UPF_Allocator_Handle instance( void )
    {
        if ( 0 == sm_memory_pool )
        {
            UPF_Guard< Lock > guard( sm_instance_lock );

            if ( 0 == sm_memory_pool )
            {
                sm_memory_pool = UPF_New_Allocator_By_Name(
                    UPF_Memory_Pool_Traits< module_id >::get_pool_name(),
                    UPF_Memory_Pool_Traits< module_id >::get_allocated_memory_limit(),
                    UPF_TOPLEVEL_PERMANENT_ALLOCATOR );
                              
   //             atexit( destroy_memory_pool );
            }
        }

        return sm_memory_pool;
    }

    static void destroy_memory_pool( void )
    {
        UPF_Memory_Pool_Factory::delete_memory_pool( 
            UPF_Memory_Pool_Traits< module_id >::get_pool_name() );
    }

private:
    UPF_Memory_Pool_Singleton( void ); 

private:
    /// ������MemoryPool����ʱ, �ڶ��߳������, ��Ҫͬ��.
    static Lock sm_instance_lock;

    /// �ڴ�ؾ��.
    static UPF_Allocator_Handle sm_memory_pool;
};

// Static member definitions.
template < size_t module_id, class Lock >
Lock UPF_Memory_Pool_Singleton< module_id, Lock >::sm_instance_lock;

template < size_t module_id, class Lock >
UPF_Allocator_Handle UPF_Memory_Pool_Singleton< module_id, Lock >::sm_memory_pool = 0;

// Macro definitions.

// ��ΪUPF_Memory_Pool_Singleton��Ҫ�ڶ��̰߳汾��ʹ���̱߳���, ����Lockģ���β�
// ��Ҫ��ѭUPF_Memory_Pool_Lock_Type�Ķ���, ���Զ���������귽��ʹ��.

// Type definitions.
#if ! defined(UPF_MEMORY_POOL_MULTI_THREAD_VERSION)
    typedef UPF_Null_Mutex 
            UPF_Memory_Pool_Lock_Type;
#else
    typedef UPF_Thread_Mutex 
            UPF_Memory_Pool_Lock_Type;
#endif

#define UPF_Memory_Pool_Singleton(module_id)\
UPF_Memory_Pool_Singleton<module_id, UPF_Memory_Pool_Lock_Type>


#endif /* UPF_MEMORY_POOL_SINGLETON_H */

