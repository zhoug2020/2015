/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Traits.h                                                                   *
*       CREATE DATE     : 2007-12-13                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池Traits模板 ![CN]                                                  *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_TRAITS_H
#define UPF_MEMORY_POOL_TRAITS_H

#include <stddef.h>                    /* for size_t */
#include "UPF_Memory_Pool_Define.h"    /* for MAX_MEMORY_REQ_SIZE */

/** 
 * @class UPF_Memory_Pool_Traits
 * 
 * @brief 一个内存池的Traits模板类, 用于提取模块名
 * 
 * - module_d   模块ID
 */
template < size_t module_id >
struct UPF_Memory_Pool_Traits
{
    /// 缺省是返回 "general"
    static const char * get_pool_name( void )
    {
        return "general";
    }

    /// 返回Client端申请内存的上限
    static const size_t get_allocated_memory_limit( void )
    {
        return MAX_MEMORY_REQ_SIZE;
    }
};

// Macro definitions.

// 如果各模块要使用UPF_Pool_Object_Base, 则需要定义UPF_Memory_Pool_Traits的特化,

// DEFINE_MEMORY_POOL_TRAITS用于定义相应模块的UPF_Memory_Pool_Traits特化, 
// module_id通常为一个常量宏, 返回的pool name为module_id的字符串形式.
// allocated_memory_limit为Client端申请内存的上限
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

// DEFINE_MEMORY_POOL_TRAITS_EX用于定义相应模块的UPF_Memory_Pool_Traits特化, 
// module_id通常为一个常量宏, 返回的pool name为mmodule_name.
// allocated_memory_limit为Client端申请内存的上限
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

