/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Util.h                                                                     *
*       CREATE DATE     : 2007-11-22                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池工具类的定义![CN]                                                                *
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
 * @brief 内存池工具类, 包含一些辅助函数.
 * 
 */
class /*UPF_Export*/ UPF_Memory_Pool_Util
{
public:
    /// 根据size来计算出在free small block list中的相应索引值.
    static   int get_free_small_block_index( size_t   size, 
                                                  size_t & align_value );

    /// 计算出在free small block list中的最接近size的相应索引值.
    static  int get_nearest_free_small_block_index( size_t   size, 
                                                          size_t & block_size );


    /// 根据index来计算出在free small block list中的相应size值
    static  size_t get_free_small_block_size( int index );


    /// 根据size, 查找small_block_index_table, 来取得相应的分配粒度
    static  size_t get_allocate_granularity( size_t size );

    /// trace一段消息
    static void trace( const char * fmt, ... );

    /// 报告消息
    static void report( const char * fmt, ... );

public:
    /// 显示操作系统的错误消息
    static void display_os_error( const char * error_msg_fmt, ... );

    /// 数字转换成一个带逗号分割的字符串
    static const char * num_to_string( unsigned int number, char * buf );


    /// 输出memory size headers
    static void print_memory_size_headers( FILE * fp );


    /// 输出内存池的所管理的size信息.
    static void print_memory_size_infos( FILE *       fp,
                                         const char * name,
                                         size_t       managed_memory_size,
                                         size_t       allocated_memory_size,
                                         size_t       allocated_memory_peak,
                                         size_t       wasted_memory_size,
                                         size_t       allocated_memory_limit );

    /// 输出一条线
    static void print_line( FILE * fp,
                            int    width );

    /// 输出一个带边框的caption.
    static void print_framed_caption( FILE *        fp,
                                      const char *  caption, 
                                      int           width,
                                      int           gap_width = 4,
                                      const char    border_char = '*' );

    /// 输出一个带一行线的caption.
    static void print_lined_caption( FILE *        fp,
                                     const char *  caption, 
                                     int           width,
                                     int           gap_width = 4,
                                     const char    line_char = '_' );


    /// 重复输出指定个数的字符
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
    /// Small Block的分配粒度映射表的元素类型.
    struct Small_Block_Index_Table
    {
        int    high_block_index;   /* 每一组分配粒度所能管理内存块的最大索引值. */
        size_t high_block_size;    /* 每一组分配粒度所能管理内存块大小的最大值. */
        size_t align_value;        /* 用于内存对齐的一个中间值, 应为相应的分配粒度值 - 1 */
        size_t shift_bits;         /* 相应的分配粒度值的LOG2值. */   
    };

private:
    enum {
        /* Small Block的分配粒度映射表的大小 */
        SMALL_BLOCK_INDEX_TABLE_SIZE = 7
    };

private:
    /// Small Block的分配粒度映射表
    static const Small_Block_Index_Table
         sm_small_block_index_table[ SMALL_BLOCK_INDEX_TABLE_SIZE ];    
};



#endif /* UPF_MEMORY_POOL_UTIL_H */
