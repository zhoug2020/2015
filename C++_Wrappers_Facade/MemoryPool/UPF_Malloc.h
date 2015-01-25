/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Malloc.h                                                                               *
*       CREATE DATE     : 2007-12-11                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]��װ������C��׼����ڴ���亯��, ����malloc, realloc, free. ![CN]                      *
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

/* UPF Allocator��Ϣ, ���ṩ���������ڴ��С��Client���ѷ���Ĵ�С,
   �����Ϣͨ������UPF_Query_Allocator����ȡ. */
typedef struct 
{
    /* UPF allocator������������ڴ��С, ���������Ҫ��̬����. */
    size_t   managed_size;

    /* Client�˴�allocator���ѷ���Ĵ�С. */
    size_t   allocated_size;

} UPF_Allocator_Info;

/************************* function prototype declarations ******************/

/* ����һ��allocator, ��Ҫʹ���ַ�����ʽ��allocator����. */
extern UPF_Export UPF_Allocator_Handle UPF_New_Allocator_By_Name( 
    const char *         allocator_name,
    size_t               allocated_memory_limit,
    toplevel_allocator_t allocator_type );


/**
 * ����һ�������͵�allocator, ��Ҫʹ���ַ�����ʽ��allocator����.
 * 
 * @param allocator_name             [in]    allocator������.
 * @param max_needed_memory_size     [in]    ����Ҫ���ڴ��С�����ֵ, ��MΪ��λ.
 * 
 * @return һ��allocator���      �����ɹ�. 
 *         0                      ����ʧ��. 
 */
extern UPF_Export UPF_Trans_Allocator_Handle UPF_New_Trans_Allocator_By_Name( 
    const char *         allocator_name,
    size_t               max_needed_memory_size );


/* ����һ��UPF_New_Allocator��, ����ʹ��allocator_id������һ��allocator. 
   allocator_idͨ��ӦΪһ��ģ��ID�ĺ�. */
#ifdef UPF_New_Allocator
# undef UPF_New_Allocator
#endif

#define UPF_New_Allocator( allocator_id, allocated_memory_limit, allocator_type ) \
UPF_New_Allocator_By_Name( #allocator_id, allocated_memory_limit, allocator_type )

/** 
 *  ����һ��UPF_New_Trans_Allocator��, ����ʹ��allocator_id������һ��allocator.
 *  
 *  allocator_id            [in] ͨ��ӦΪһ��ģ��ID�ĺ�.
 *  max_needed_memory_size  [in] ����Ҫ���ڴ��С�����ֵ, ��MΪ��λ.
 *  
 */
#ifdef UPF_New_Trans_Allocator
# undef UPF_New_Trans_Allocator
#endif

#define UPF_New_Trans_Allocator( allocator_id, max_needed_memory_size ) \
UPF_New_Trans_Allocator_By_Name( #allocator_id, max_needed_memory_size )

/* ����һ��allocator. */
extern UPF_Export void UPF_Delete_Allocator( UPF_Allocator_Handle allocator );

/* ����һ�������͵�allocator. */
extern UPF_Export void UPF_Delete_Trans_Allocator( UPF_Trans_Allocator_Handle allocator );

/* ����size��С���ڴ�. */
extern UPF_Export void * UPF_Malloc( UPF_Allocator_Handle allocator, size_t size );

/* UPF_Malloc��Debug�汾.*/
extern UPF_Export void * UPF_Malloc_Debug( UPF_Allocator_Handle allocator, 
                                           size_t               size,
                                           const char *         file_name,
                                           int                  line_no );


/* ���·����ڴ�, ������ԭ�е�����. */
extern UPF_Export void * UPF_Realloc( UPF_Allocator_Handle allocator, 
                                      void *               ptr, 
                                      size_t               new_size );

/* UPF_Realloc��Debug�汾. */
extern UPF_Export void * UPF_Realloc_Debug( UPF_Allocator_Handle allocator, 
                                            void *               ptr, 
                                            size_t               new_size,
                                            const char *         file_name,
                                            int                  line_no );


/* �ͷ���UPF_Malloc��UPF_Realloc��������ڴ�. */
extern UPF_Export void UPF_Free( UPF_Allocator_Handle allocator, void * ptr ); 

/* UPF_Free�ĵ��԰汾. */
extern UPF_Export void UPF_Free_Debug( UPF_Allocator_Handle allocator, 
                                       void *               ptr,
                                       const char *         file_name,
                                       int                  line_no );

/* ��ȡallocator����Ϣ. */
extern UPF_Export int UPF_Query_Allocator( UPF_Allocator_Handle allocator,
                                           UPF_Allocator_Info * info_ptr );

/* ��ӡ�ڴ���Ϣ����, ֻ��ӡһ��. */
extern UPF_Export void UPF_Print_Memory_Statistic_Report( void );

/* ��ӡ�ڴ���Ϣ����, ���Դ�ӡ���. */
extern UPF_Export void UPF_Print_Memory_Statistic_ReportEx( void );

/* ����һ��ģ�����������һ��allocator */
extern UPF_Export UPF_Allocator_Handle UPF_Get_Allocator_By_Name( const char * allocator_name );

/* ����һ��ģ����������һ��allocator. */
extern UPF_Export void UPF_Delete_Allocator_By_Name( const char * allocator_name );


/* ��һ�������͵�allocator�з���size��С���ڴ�. */
extern UPF_Export void * UPF_Trans_Malloc( UPF_Trans_Allocator_Handle allocator, size_t size );

/* �ͷ���UPF_Trans_Malloc��������ڴ�. */
extern UPF_Export void UPF_Trans_Free( UPF_Trans_Allocator_Handle allocator, void * ptr ); 

/**
 * ���ø������ڴ�������, ����ʱ��Ϊ��һ�ε��ڴ�����ǰ��ͨ��ӦΪWinMain�е���ʼ���֡�
 *
 * @param toplevel_allocator_type   [in]   allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
 *                                         UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
 * @param quota_value               [in]   Ҫ���õ��ڴ�������ֵ, ��M BytesΪ��λ.
 *
 * @return      0      ���óɹ�.
 *              -1     ����ʧ��, ͨ������Ϊ��toplevel_allocator_type���Ӧ���ڴ�
 *                     �Ѿ���ʼ������, �����û�ʧ��.
 */
extern UPF_Export int 
UPF_Set_Toplevel_Allocator_Quota( toplevel_allocator_t  toplevel_allocator_type,
                                  int                   quota_value );

/**
 * �黹toplevel_allocator_index��Ӧ���ڴ��������ڴ������ϵͳ������ʱ��Ϊ
 * ��������������С����ʽʱ���еĴ�toplevel_allocator_type��Ӧ���ڴ��
 * �����ڴ��ģ��������Լ�����ֹ���������ŵ��á�
 * ����С����ʽ���ú󣬳��ǻָ����������������
 * toplevel_allocator_type��Ӧ���ڴ�������ڴ�Ĳ�������ʧ��.
 *
 * @param toplevel_allocator_type   [in]   allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
 *                                         UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
 *
 * @return      0      �黹�ɹ�.
 *              -1     �黹ʧ��, ͨ������Ϊ��toplevel_allocator_type���Ӧ���ڴ�
 *                     �Ѿ��黹����, �ٹ黹��ʧ��.
 *
 * @note  �����toplevel_allocator_type���Ӧ���ڴ���л����ڴ��û��delete,
 *        ��ú�����Ŀǰ�汾�н���������, ����¼��Ӧ���ڴ����.
 *        ����ʽ�����У���һ��齫ȥ��.
 */
extern UPF_Export int 
UPF_Reclaim_Toplevel_Allocator( toplevel_allocator_t  toplevel_allocator_type );


/**
 * �ָ�toplevel_allocator_index��Ӧ���ڴ����ڴ�����.
 * ����ʱ��Ϊ�����������С����ʽ�ָ�����ʱ���ȵ��õġ�
 *
 * @param toplevel_allocator_type   [in]   allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
 *                                         UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
 *
 * @return      0      �ָ��ɹ�.
 *              -1     �ָ�ʧ��, ͨ������Ϊ��toplevel_allocator_type���Ӧ���ڴ�
 *                     �Ѿ��ָ�����, �ٻָ���ʧ��.
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

