/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Malloc.h                                                                               *
*       CREATE DATE     : 2007-12-11                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]封装了类似C标准库的内存分配函数, 包括malloc, realloc, free. ![CN]                      *
*********************************************************************************************************************/

#ifndef UPF_MALLOC_H
#define UPF_MALLOC_H

#include <stddef.h>                     /* for size_t */

#include "../../OS_Adapter/UPF_OS_Export.h" //wangy

#include "UPF_Memory_Pool_Define.h"     /* for MESSAGE_ASSERT */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*************************** type definitions *******************************/

/* UPF Allocator Handle. */
typedef void * UPF_Allocator_Handle;

/* UPF Transaction Allocator Handle. */
typedef void * UPF_Trans_Allocator_Handle;

/* UPF Allocator信息, 现提供自身管理的内存大小和Client端已分配的大小,
   这个信息通过调用UPF_Query_Allocator来获取. */
typedef struct 
{
    /* UPF allocator自身所管理的内存大小, 它会根据需要动态增长. */
    size_t   managed_size;

    /* Client端从allocator中已分配的大小. */
    size_t   allocated_size;

} UPF_Allocator_Info;

/************************* function prototype declarations ******************/

/* 创建一个allocator, 需要使用字符串形式的allocator名字. */
extern UPF_Export UPF_Allocator_Handle UPF_New_Allocator_By_Name( 
    const char *         allocator_name,
    size_t               allocated_memory_limit,
    toplevel_allocator_t allocator_type );


/**
 * 创建一个事务型的allocator, 需要使用字符串形式的allocator名字.
 * 
 * @param allocator_name             [in]    allocator的名字.
 * @param max_needed_memory_size     [in]    所需要的内存大小的最大值, 以M为单位.
 * 
 * @return 一个allocator句柄      创建成功. 
 *         0                      创建失败. 
 */
extern UPF_Export UPF_Trans_Allocator_Handle UPF_New_Trans_Allocator_By_Name( 
    const char *         allocator_name,
    size_t               max_needed_memory_size );


/* 定义一个UPF_New_Allocator宏, 用于使用allocator_id来创建一个allocator. 
   allocator_id通常应为一个模块ID的宏. */
#ifdef UPF_New_Allocator
# undef UPF_New_Allocator
#endif

#define UPF_New_Allocator( allocator_id, allocated_memory_limit, allocator_type ) \
UPF_New_Allocator_By_Name( #allocator_id, allocated_memory_limit, allocator_type )

/** 
 *  定义一个UPF_New_Trans_Allocator宏, 用于使用allocator_id来创建一个allocator.
 *  
 *  allocator_id            [in] 通常应为一个模块ID的宏.
 *  max_needed_memory_size  [in] 所需要的内存大小的最大值, 以M为单位.
 *  
 */
#ifdef UPF_New_Trans_Allocator
# undef UPF_New_Trans_Allocator
#endif

#define UPF_New_Trans_Allocator( allocator_id, max_needed_memory_size ) \
UPF_New_Trans_Allocator_By_Name( #allocator_id, max_needed_memory_size )

/* 销毁一个allocator. */
extern UPF_Export void UPF_Delete_Allocator( UPF_Allocator_Handle allocator );

/* 销毁一个事务型的allocator. */
extern UPF_Export void UPF_Delete_Trans_Allocator( UPF_Trans_Allocator_Handle allocator );

/* 分配size大小的内存. */
extern UPF_Export void * UPF_Malloc( UPF_Allocator_Handle allocator, size_t size );

/* UPF_Malloc的Debug版本.*/
extern UPF_Export void * UPF_Malloc_Debug( UPF_Allocator_Handle allocator, 
                                           size_t               size,
                                           const char *         file_name,
                                           int                  line_no );


/* 重新分配内存, 并保留原有的内容. */
extern UPF_Export void * UPF_Realloc( UPF_Allocator_Handle allocator, 
                                      void *               ptr, 
                                      size_t               new_size );

/* UPF_Realloc的Debug版本. */
extern UPF_Export void * UPF_Realloc_Debug( UPF_Allocator_Handle allocator, 
                                            void *               ptr, 
                                            size_t               new_size,
                                            const char *         file_name,
                                            int                  line_no );


/* 释放由UPF_Malloc或UPF_Realloc所申请的内存. */
extern UPF_Export void UPF_Free( UPF_Allocator_Handle allocator, void * ptr ); 

/* UPF_Free的调试版本. */
extern UPF_Export void UPF_Free_Debug( UPF_Allocator_Handle allocator, 
                                       void *               ptr,
                                       const char *         file_name,
                                       int                  line_no );

/* 获取allocator的信息. */
extern UPF_Export int UPF_Query_Allocator( UPF_Allocator_Handle allocator,
                                           UPF_Allocator_Info * info_ptr );

/* 打印内存信息报告, 只打印一次. */
extern UPF_Export void UPF_Print_Memory_Statistic_Report( void );

/* 打印内存信息报告, 可以打印多次. */
extern UPF_Export void UPF_Print_Memory_Statistic_ReportEx( void );

/* 根据一个模块名字来获得一个allocator */
extern UPF_Export UPF_Allocator_Handle UPF_Get_Allocator_By_Name( const char * allocator_name );

/* 根据一个模块名字销毁一个allocator. */
extern UPF_Export void UPF_Delete_Allocator_By_Name( const char * allocator_name );


/* 从一个事务型的allocator中分配size大小的内存. */
extern UPF_Export void * UPF_Trans_Malloc( UPF_Trans_Allocator_Handle allocator, size_t size );

/* 释放由UPF_Trans_Malloc所申请的内存. */
extern UPF_Export void UPF_Trans_Free( UPF_Trans_Allocator_Handle allocator, void * ptr ); 

/**
 * 设置各部分内存块的容量, 调用时机为第一次的内存请求前，通常应为WinMain中的起始部分。
 *
 * @param toplevel_allocator_type   [in]   allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
 *                                         UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 * @param quota_value               [in]   要设置的内存块的容量值, 以M Bytes为单位.
 *
 * @return      0      设置成功.
 *              -1     设置失败, 通常是因为与toplevel_allocator_type相对应的内存
 *                     已经开始申请了, 在设置会失败.
 */
extern UPF_Export int 
UPF_Set_Toplevel_Allocator_Quota( toplevel_allocator_t  toplevel_allocator_type,
                                  int                   quota_value );

/**
 * 归还toplevel_allocator_index对应的内存块的物理内存给操作系统。调用时机为
 * 导航程序启用最小化方式时所有的从toplevel_allocator_type对应的内存块
 * 申请内存的模块在完成自己的终止操作后，最后才调用。
 * 在最小化方式启用后，除非恢复过来，否则继续从
 * toplevel_allocator_type对应的内存块申请内存的操作都将失败.
 *
 * @param toplevel_allocator_type   [in]   allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
 *                                         UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 *
 * @return      0      归还成功.
 *              -1     归还失败, 通常是因为与toplevel_allocator_type相对应的内存
 *                     已经归还过了, 再归还会失败.
 *
 * @note  如果与toplevel_allocator_type相对应的内存块中还有内存池没有delete,
 *        则该函数在目前版本中将产生断言, 并记录相应的内存池名.
 *        在正式发布中，这一检查将去掉.
 */
extern UPF_Export int 
UPF_Reclaim_Toplevel_Allocator( toplevel_allocator_t  toplevel_allocator_type );


/**
 * 恢复toplevel_allocator_index对应的内存块的内存请求.
 * 调用时机为导航程序从最小化方式恢复过来时首先调用的。
 *
 * @param toplevel_allocator_type   [in]   allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
 *                                         UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
 *
 * @return      0      恢复成功.
 *              -1     恢复失败, 通常是因为与toplevel_allocator_type相对应的内存
 *                     已经恢复过了, 再恢复会失败.
 *
 */
extern UPF_Export int 
UPF_Restore_Toplevel_Allocator( toplevel_allocator_t  toplevel_allocator_type );


 // #define UPF_MALLOC_DEBUG

#ifdef UPF_MALLOC_DEBUG
# define UPF_Malloc(allocator, size) \
UPF_Malloc_Debug(allocator, size, __FILE__, __LINE__)

# define UPF_Realloc(allocator, ptr, new_size) \
UPF_Realloc_Debug(allocator, ptr, new_size, __FILE__, __LINE__);

# define UPF_Free(allocator, ptr) \
UPF_Free_Debug(allocator, ptr, __FILE__, __LINE__)

#endif /* UPF_MALLOC_DEBUG */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* UPF_MALLOC_H */

