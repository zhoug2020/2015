/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Define.h                                                                *
*       CREATE DATE     : 2007-11-22                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ�ص�һЩ��������![CN]                                                              *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_DEFINE_H
#define UPF_MEMORY_POOL_DEFINE_H

#include <assert.h>                     /* for assert */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*************************  MACRO DEFINITIONS **********************************/

/* ����Memory Pool Ϊ���̰߳汾 */
#define UPF_MEMORY_POOL_MULTI_THREAD_VERSION /* wangy */


/* �̶������С�ڴ�ص�ȱʡ�Ķ����С */
#define DEFAULT_OBJECT_SIZE             ( 16 )

/* �̶������С�ڴ����ÿ���ڴ��ȱʡ�Ķ������ */
#define DEFAULT_NUM_OBJECTS_PER_BLOCK   ( 256 )

/* �ɱ�����С�ڴ�������������Small Block��С, Ϊ16K. */
#define  MAX_SMALL_BLOCK_SIZE           ( 8 * 1024 )

/* �ɱ�����С�ڴ���ڲ���һ����������Small Block, ������Ԫ�ظ�����Ϊ128��. */
#define  MAX_SMALL_BLOCKS_INDEX         ( 112 )

/* �ɱ�����С�ڴ����һ�������ڴ����С��size */
#define  MIN_ALLOC_MEM_SIZE             ( 4 * 1024 )

/* �ɱ�����С�ڴ����С�ڴ�������� */
#define  MIN_ALLOC_GRANULARITY          ( 8 )

/* һ����ڴ��ַ���뵽8�ֽ� */
#define UPF_MEMORY_ALIGN_BYTES          ( 8 )

/* �ڴ�ҳ���С, ͨ��Ϊ4K */
#define UPF_MEMORY_PAGE_SIZE            ( 4 * 1024 )

/* �ڴ�����ֵ���󳤶�. */
#define MAX_POOL_NAME_LENGTH            ( 20 )

/* Client�������ڴ������С */
#define MAX_MEMORY_REQ_SIZE             ( 0xFFFFFFE0 )

/* ���ڶ����ڴ�߽�, boundaryͨ��ӦΪ2����. */
#define UPF_MEMORY_ALIGN(size, boundary) \
(((size) + (boundary) - 1) & ~((boundary) - 1))

/* ���ڶ����ڴ�߽�, boundary_lack_oneΪ2���ݼ�1, ��ҪΪ�˼��������1����. */
#define UPF_MEMORY_ALIGN_LACK_ONE(size, boundary_lack_one) \
(((size) + (boundary_lack_one)) & ~(boundary_lack_one))

/* ȱʡ���ڴ�߽�, ���뵽8�ı���. */
#define UPF_MEMORY_ALIGN_DEFAULT(size)    UPF_MEMORY_ALIGN(size, UPF_MEMORY_ALIGN_BYTES)

/* ���ڶ����ڴ�߽�, exponentΪ2��ָ��. */
#define UPF_MEMORY_ALIGN_MULTIPLE_OF_POW2(size, exponent) \
UPF_MEMORY_ALIGN(size, 1 << exponent)

/* ������ʧ��ʱ����ʾһ�����������Ե���Ϣ. */
#define MESSAGE_ASSERT(m, e)            assert( (m && e) )

/* �ڴ�ص�size ����ֵ */
#define UPF_MEMORY_POOL_SIZE_MASK           (0x00FFFFFF)

/* �ڴ�ص�ID ����ֵ */
#define UPF_MEMORY_POOL_ID_MASK             (0xFF000000)

/* ��÷����ȥ���ڴ���size */
#define UPF_MEMORY_POOL_ALLOC_SIZE(idsize)  ((idsize) & UPF_MEMORY_POOL_SIZE_MASK)  

/* ��÷����ȥ���ڴ��������memory pool id */
#define UPF_MEMORY_POOL_ID(idsize)          ( (((idsize) & UPF_MEMORY_POOL_ID_MASK) >> 24) & 0xFF )

/* ���memory pool��id�ͷ�����ڴ��size��һ���� */
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

/* ĿǰUPF�ڴ����ģ����֧�ֵĵ�һ����̬Virtual Allocator�����͸���. */
#define UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES                ( 2 )

/* ĿǰUPF�ڴ����ģ����֧�ֵĵ�һ����̬Virtual Allocator�����͸���. */
#define UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES               ( 32 )

/* ĿǰUPF�ڴ����ģ����֧�ֵ�Virtual Allocator�����͸���. */
#define UPF_TOPLEVEL_ALLOCATOR_TYPES                       ( UPF_STATIC_TOPLEVEL_ALLOCATOR_TYPES + UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES )

/* �־ô��ڵ��ڴ������. */
#define UPF_TOPLEVEL_PERMANENT_ALLOCATOR            ( 0 )

/* ��ĳһʱ�̿��Զ������ڴ�����ͣ��������͵��ڴ����ռ�õ������ڴ���Թ黹������ϵͳ. */
#define UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR          ( 1 )

/*************************  TYPE DEFINITIONS **********************************/

/* ���� toplevel allocator type */
typedef unsigned int toplevel_allocator_t;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UPF_MEMORY_POOL_DEFINE_H */

