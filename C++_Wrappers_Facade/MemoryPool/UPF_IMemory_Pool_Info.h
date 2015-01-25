/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IMemory_Pool_Info.h                                                                   *
*       CREATE DATE     : 2007-11-19                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池信息接口![CN]                                                                    *
*********************************************************************************************************************/

#ifndef UPF_IMEMORY_POOL_INFO_H
#define UPF_IMEMORY_POOL_INFO_H

#include <stddef.h>        /* for size_t */
#include "UPF_Memory_Pool_Define.h"

class UPF_IMemory_Pool;

/** 
 * @class UPF_IMemory_Pool_Info
 * 
 * @brief 内存池信息接口, 通常应该由实现IMemoryPool的类来实现, 因为只有实现类才
 * 知道当前内存的分配情况.
 * 
 */
class UPF_IMemory_Pool_Info
{
public:
    /** 
     * 获取内存池的名字.
     *  
     * @return 内存池的名字.
     */
    virtual const char * get_memory_pool_name( void ) const = 0;

    /**
     * 获取内存池的类型，当前支持两种，固定大小的，可变大小的.
     * 
     * @return 内存池的类型.
     */
    virtual const char * get_memory_pool_type( void ) const = 0;

    /** 
     * 获得内存池所管理的内存大小, 即内存池从系统中申请的所有内存的大小.
     *  
     * @return 内存池所管理的内存大小
     */
    virtual size_t get_managed_memory_size( void ) const = 0;

    /** 
     * 获得内存池中当前已分配给Client端的内存大小, 这个值能表示在某一时刻Client端
     * 所使用的内存大小.
     * 
     * @return  分配给Client端的内存大小. 
     */
    virtual size_t get_allocated_memory_size( void ) const = 0;

    /** 
     *  获取分配给Client端的内存大小的峰值, 这个值能表示在某一时刻Client端使用的
     *  内存大小的最大值.
     * 
     * @return 分配给Client端的内存大小的峰值
     */
    virtual size_t get_allocated_memory_peak( void ) const = 0;

    /** 
     * 获取内存池中浪费的内存大小, 因为内存池中的每个节点大小都是按照一定分配粒度
     * 对齐的, 所以每次分配可能有浪费的内存, 使用这个接口函数可以获取某一时刻内存池
     * 中所浪费的内存大小.
     * 
     * @return 内存池中浪费的内存大小
     */
    virtual size_t get_wasted_memory_size( void ) const = 0;


    /**
     * 获取内存的最大限制值.
     */
    virtual size_t get_allocated_memory_limit( void ) const = 0;

    /** 
     * 由于一些管理原因，需要通过UPF_IMemory_Pool_Info接口
     * 返回UPF_IMemory_Pool接口指针.
     * 
     * @return 实现UPF_IMemory_Pool接口的指针, 当前实现确保返回值肯定不为0.
     */
    virtual UPF_IMemory_Pool * get_memory_pool( void ) = 0;

    /** 
     * 返回allocator type.
     *
     * @return allocator type.
     */
    virtual toplevel_allocator_t get_allocator_type( void ) const = 0;

public:
    virtual ~UPF_IMemory_Pool_Info( void ) { }
};

#endif /* UPF_IMEMORY_POOL_INFO_H */

