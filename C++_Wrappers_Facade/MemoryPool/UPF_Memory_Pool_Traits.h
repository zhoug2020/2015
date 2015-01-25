/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Traits.h                                                                   *
*       CREATE DATE     : 2007-12-13                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ��Traitsģ�� ![CN]                                                  *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_TRAITS_H
#define UPF_MEMORY_POOL_TRAITS_H

#include <stddef.h>                    /* for size_t */
#include "UPF_Memory_Pool_Define.h"    /* for MAX_MEMORY_REQ_SIZE */

/** 
 * @class UPF_Memory_Pool_Traits
 * 
 * @brief һ���ڴ�ص�Traitsģ����, ������ȡģ����
 * 
 * - module_d   ģ��ID
 */
template < size_t module_id >
struct UPF_Memory_Pool_Traits
{
    /// ȱʡ�Ƿ��� "general"
    static const char * get_pool_name( void )
    {
        return "general";
    }

    /// ����Client�������ڴ������
    static const size_t get_allocated_memory_limit( void )
    {
        return MAX_MEMORY_REQ_SIZE;
    }
};

// Macro definitions.

// �����ģ��Ҫʹ��UPF_Pool_Object_Base, ����Ҫ����UPF_Memory_Pool_Traits���ػ�,

// DEFINE_MEMORY_POOL_TRAITS���ڶ�����Ӧģ���UPF_Memory_Pool_Traits�ػ�, 
// module_idͨ��Ϊһ��������, ���ص�pool nameΪmodule_id���ַ�����ʽ.
// allocated_memory_limitΪClient�������ڴ������
#define DEFINE_MEMORY_POOL_TRAITS(module_id, allocated_memory_limit) \
template <>\
struct UPF_Memory_Pool_Traits<module_id>\
{\
    static const char * get_pool_name( void )\
    {\
        return #module_id;\
    }\
    static const size_t get_allocated_memory_limit( void )\
    {\
        return allocated_memory_limit;\
    }\
};

// DEFINE_MEMORY_POOL_TRAITS_EX���ڶ�����Ӧģ���UPF_Memory_Pool_Traits�ػ�, 
// module_idͨ��Ϊһ��������, ���ص�pool nameΪmmodule_name.
// allocated_memory_limitΪClient�������ڴ������
#define DEFINE_MEMORY_POOL_TRAITS_EX(module_id, module_name, allocated_memory_limit) \
template <>\
struct UPF_Memory_Pool_Traits<module_id>\
{\
    static const char * get_pool_name( void )\
    {\
        return module_name;\
    }\
    static const size_t get_allocated_memory_limit( void )\
    {\
        return allocated_memory_limit;\
    }\
};

#endif /* UPF_MEMORY_POOL_TRAITS_H */

