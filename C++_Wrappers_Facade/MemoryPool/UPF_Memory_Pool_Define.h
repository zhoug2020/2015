/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Define.h                                                                *
*       CREATE DATE     : 2007-11-22                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池的一些常量定义![CN]                                                              *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_DEFINE_H
#define UPF_MEMORY_POOL_DEFINE_H

#include <assert.h>                     /* for assert */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*************************  MACRO DEFINITIONS **********************************/

/* 定义Memory Pool 为多线程版本 */
#define UPF_MEMORY_POOL_MULTI_THREAD_VERSION /* wangy */


/* 固定对象大小内存池的缺省的对象大小 */
#define DEFAULT_OBJECT_SIZE             ( 16 )

/* 固定对象大小内存池中每个内存块缺省的对象个数 */
#define DEFAULT_NUM_OBJECTS_PER_BLOCK   ( 256 )

/* 可变对象大小内存池所管理的最大的Small Block大小, 为16K. */
#define  MAX_SMALL_BLOCK_SIZE           ( 8 * 1024 )

/* 可变对象大小内存池内部用一数组来管理Small Block, 数组中元素个数共为128个. */
#define  MAX_SMALL_BLOCKS_INDEX         ( 112 )

/* 可变对象大小内存池内一次申请内存的最小的size */
#define  MIN_ALLOC_MEM_SIZE             ( 4 * 1024 )

/* 可变对象大小内存池最小内存分配粒度 */
#define  MIN_ALLOC_GRANULARITY          ( 8 )

/* 一般的内存地址对齐到8字节 */
#define UPF_MEMORY_ALIGN_BYTES          ( 8 )

/* 内存页面大小, 通常为4K */
#define UPF_MEMORY_PAGE_SIZE            ( 4 * 1024 )

/* 内存池名字的最大长度. */
#define MAX_POOL_NAME_LENGTH            ( 20 )

/* Client端最大的内存请求大小 */
#define MAX_MEMORY_REQ_SIZE             ( 0xFFFFFFE0 )

/* 用于对齐内存边界, boundary通常应为2的幂. */
#define UPF_MEMORY_ALIGN(size, boundary) \
(((size) + (boundary) - 1) & ~((boundary) - 1))

/* 用于对齐内存边界, boundary_lack_one为2的幂减1, 主要为了减少这个减1运算. */
#define UPF_MEMORY_ALIGN_LACK_ONE(size, boundary_lack_one) \
(((size) + (boundary_lack_one)) & ~(boundary_lack_one))

/* 缺省的内存边界, 对齐到8的倍数. */
#define UPF_MEMORY_ALIGN_DEFAULT(size)    UPF_MEMORY_ALIGN(size, UPF_MEMORY_ALIGN_BYTES)

/* 用于对齐内存边界, exponent为2的指数. */
#define UPF_MEMORY_ALIGN_MULTIPLE_OF_POW2(size, exponent) \
UPF_MEMORY_ALIGN(size, 1 << exponent)

/* 当断言失败时，显示一个具有描述性的消息. */
#define MESSAGE_ASSERT(m, e)            assert( (m && e) )

/* 内存池的size 掩码值 */
#define UPF_MEMORY_POOL_SIZE_MASK           (0x00FFFFFF)

/* 内存池的ID 掩码值 */
#define UPF_MEMORY_POOL_ID_MASK             (0xFF000000)

/* 获得分配出去的内存块的size */
#define UPF_MEMORY_POOL_ALLOC_SIZE(idsize)  ((idsize) & UPF_MEMORY_POOL_SIZE_MASK)  

/* 获得分配出去的内存块所属的memory pool id */
#define UPF_MEMORY_POOL_ID(idsize)          ( (((idsize) & UPF_MEMORY_POOL_ID_MASK) >> 24) & 0xFF )

/* 组合memory pool的id和分配的内存块size成一个数 */
#define UPF_MMORY_POOL_COMBINE_ID_SIZE(id, size)  ( ((id) << 24) | ((size) & UPF_MEMORY_POOL_SIZE_MASK) )


// wince platform lacks BUFSIZ
#ifndef BUFSIZ
# define BUFSIZ  512
#endif /* BUFSIZ */

#if defined(UNICODE) || defined(_UNICODE)

#  if defined(UPF_OS_IS_WINCE)
/* WINCE Platform */

/* wide char version of UPF_TEXT macro only used in c++ compiler */
#    ifdef __cplusplus
}
#    include "../../OS_Adapter/WinCE_OS/UPF_Ascii_To_Wide.h"
#    define UPF_TEXT_(quote)     ( L##quote )
#    define UPF_TEXT(quote)      UPF_TEXT_(quote)
extern "C" {
#    else
// #    error widechar version of UPF_TEXT macro only used in c++ compiler.
#    endif /* __cplusplus */

#    define UPF_A2T(str)        ( UPF_Ascii_To_Wide(str).wchar_rep() ) 
    
#  elif defined(UPF_OS_IS_WINNT)
/* WINNT Platform */
#    define UPF_TEXT(quote)     ( quote )
#    define UPF_A2T(str)        ( str   )

#  endif

#else
/* non unicode */
#    define UPF_TEXT(quote)     ( quote )
#    define UPF_A2T(str)        ( str   )

#endif /* UNICODE || _UNICODE */

/* 目前UPF内存分配模块所支持的第一级静态Virtual Allocator的类型个数. */
#define UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES                ( 2 )

/* 目前UPF内存分配模块所支持的第一级动态Virtual Allocator的类型个数. */
#define UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES               ( 32 )

/* 目前UPF内存分配模块所支持的Virtual Allocator的类型个数. */
#define UPF_TOPLEVEL_ALLOCATOR_TYPES                       ( UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES + UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES )

/* 持久存在的内存块类型. */
#define UPF_TOPLEVEL_PERMANENT_ALLOCATOR            ( 0 )

/* 在某一时刻可以丢弃的内存块类型，这种类型的内存块所占用的物理内存可以归还给操作系统. */
#define UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR          ( 1 )

/*************************  TYPE DEFINITIONS **********************************/

/* 定义 toplevel allocator type */
typedef unsigned int toplevel_allocator_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UPF_MEMORY_POOL_DEFINE_H */

