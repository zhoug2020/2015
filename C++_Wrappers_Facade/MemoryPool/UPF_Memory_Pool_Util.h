/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Util.h                                                                     *
*       CREATE DATE     : 2007-11-22                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ�ع�����Ķ���![CN]                                                                *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_UTIL_H
#define UPF_MEMORY_POOL_UTIL_H

#include "..\..\OS_Adapter\UPF_OS.h"

#include <assert.h>             /* for assert */
#include <stddef.h>             /* for size_t */
#include <stdio.h>              /* for FILE * */

#include "UPF_Memory_Pool_Define.h"   /* for MAX_SMALL_BLOCK_SIZE,
                                             MIN_ALLOC_GRANULARITY,
                                             UPF_MEMORY_ALIGN_LACK_ONE */
/** 
 * @class UPF_Memory_Pool_Util
 * 
 * @brief �ڴ�ع�����, ����һЩ��������.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Util
{
public:
    /// ����size���������free small block list�е���Ӧ����ֵ.
    static   int get_free_small_block_index( size_t   size, 
                                                  size_t & align_value );

    /// �������free small block list�е���ӽ�size����Ӧ����ֵ.
    static  int get_nearest_free_small_block_index( size_t   size, 
                                                          size_t & block_size );


    /// ����index���������free small block list�е���Ӧsizeֵ
    static  size_t get_free_small_block_size( int index );


    /// ����size, ����small_block_index_table, ��ȡ����Ӧ�ķ�������
    static  size_t get_allocate_granularity( size_t size );

    /// traceһ����Ϣ
    static void trace( const char * fmt, ... );

    /// ������Ϣ
    static void report( const char * fmt, ... );

public:
    /// ��ʾ����ϵͳ�Ĵ�����Ϣ
    static void display_os_error( const char * error_msg_fmt, ... );

    /// ����ת����һ�������ŷָ���ַ���
    static const char * num_to_string( unsigned int number, char * buf );


    /// ���memory size headers
    static void print_memory_size_headers( FILE * fp );


    /// ����ڴ�ص��������size��Ϣ.
    static void print_memory_size_infos( FILE *       fp,
                                         const char * name,
                                         size_t       managed_memory_size,
                                         size_t       allocated_memory_size,
                                         size_t       allocated_memory_peak,
                                         size_t       wasted_memory_size,
                                         size_t       allocated_memory_limit );

    /// ���һ����
    static void print_line( FILE * fp,
                            int    width );

    /// ���һ�����߿��caption.
    static void print_framed_caption( FILE *        fp,
                                      const char *  caption, 
                                      int           width,
                                      int           gap_width = 4,
                                      const char    border_char = '*' );

    /// ���һ����һ���ߵ�caption.
    static void print_lined_caption( FILE *        fp,
                                     const char *  caption, 
                                     int           width,
                                     int           gap_width = 4,
                                     const char    line_char = '_' );


    /// �ظ����ָ���������ַ�
    static inline  void print_chars( FILE * fp, 
                                     int    count, 
                                     char   ch, 
                                     bool   is_print_line_char = true )
    {
        for ( int i = 0; i < count; ++i )
        {
            fputc( ch, fp );
        }
        
        if ( is_print_line_char )
        {
            fputc( '\n', fp );
        }
    }

private:
    /// Small Block�ķ�������ӳ����Ԫ������.
    struct Small_Block_Index_Table
    {
        int    high_block_index;   /* ÿһ������������ܹ����ڴ����������ֵ. */
        size_t high_block_size;    /* ÿһ������������ܹ����ڴ���С�����ֵ. */
        size_t align_value;        /* �����ڴ�����һ���м�ֵ, ӦΪ��Ӧ�ķ�������ֵ - 1 */
        size_t shift_bits;         /* ��Ӧ�ķ�������ֵ��LOG2ֵ. */   
    };

private:
    enum {
        /* Small Block�ķ�������ӳ���Ĵ�С */
        SMALL_BLOCK_INDEX_TABLE_SIZE = 7
    };

private:
    /// Small Block�ķ�������ӳ���
    static const Small_Block_Index_Table
         sm_small_block_index_table[ SMALL_BLOCK_INDEX_TABLE_SIZE ];    
};



#endif /* UPF_MEMORY_POOL_UTIL_H */
