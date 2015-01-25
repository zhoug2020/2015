/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Virtual_Memory_Allocator.h                                                             *
*       CREATE DATE     : 2008-4-4                                                                                   *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]使用VirtualAlloc来分配虚拟内存的allocator![CN]                                         *
*********************************************************************************************************************/
#ifndef UPF_VIRTUAL_MEMORY_ALLOCATOR_H
#define UPF_VIRTUAL_MEMORY_ALLOCATOR_H

#include "../../OS_Adapter/UPF_OS.h"
#include "UPF_Memory_Pool_Define.h"

#include "../UPF_Thread_Mutex.h"
#include <stdio.h>

class UPF_Virtual_Memory_Allocator_Impl;

/** 
 * @class UPF_Virtual_Memory_Allocator
 * 
 * @brief 使用VirtualAlloc来分配虚拟内存的Allocator, 可用于内存池的allocator.
 * 
 * @detail 主要可用在WINCE平台上, WINCE平台当用VirtualAlloc Reserve 2M以上的内存
 *         时, WINCE系统就不从32M的程序空间中划分了而从0x004000000以上的内存划分
 *         使用这个Allocate就可解决一些WINCE平台内存不足的问题.
 *         目前使用的分配策略是提交一个大的内存块20M, 然后使用 
 *         类似 UPF_Common_Memory_Pool 的策略进行管理, 只不过分配的粒度为
 *         不同, UPF_Common_Memory_Pool使用的内存池, 每次分配最小为8k, 粒度为
 *         4k.
 *         所以内部管理的size以8k为起始值, 然后按照下表开始管理, 最多管理
 *         3924k内存.
 *        
 * 大概的管理策略是, 内部保存一个512个元素的数组m_free_block_list,每个元素
 * 管理固定大小的内存块, 具体数据请参见下表:<br><pre>
 * -------------------------------------------
 * |  数组索引  |   分配粒度    |  管理范围   |
 * -------------------------------------------
 * |  1-64      |      4k       |   8k-260k   | 
 * |  65-512    |      8k       |  261k-3844k |
 * -------------------------------------------
 * </pre>
 *  
 */
class /*UPF_Export*/ UPF_Virtual_Memory_Allocator
{
public:
    /** 
     * 分配内存, 第一次从虚拟内存中提交.
     * 
     * @param size          [in]   要分配的内存大小, 一般应该为8k的倍数.
     * @param type          [in]   allocator type, 可以为UPF_TOPLEVEL_PERMANENT_ALLOCATOR或
     *                             UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR.
     * 
     * @return
     *  - <em>一个有效的内存指针</em>     分配成功.
     *  - <em>0</em>                      分配失败.
     */
    static void * allocate( size_t size, toplevel_allocator_t type );

    /** 
     * 释放内存.
     * 
     * @param ptr          [in]  要释放的内存指针.
     * @param size         [in]  ptr所指向的内存的大小.
     * @param type         [in]  allocator type, 可以为UPF_TOPLEVEL_PERMANENT_ALLOCATOR或
     *                           UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR.
     * 
     */
    static void deallocate( void * ptr, size_t size, toplevel_allocator_t type );

    /** 
     * 重新申请内存.
     * 
     * @param ptr          [in]  原始指针.
     * @param new_size     [in]  新的size.
     * @param type         [in]  allocator type, 可以为UPF_TOPLEVEL_PERMANENT_ALLOCATOR或
     *                           UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR.
     * @return
     *  - <em>一个有效的内存指针</em>     分配成功.
     *  - <em>0</em>                      分配失败.
     */
    static void * reallocate( void * ptr, size_t new_size, toplevel_allocator_t type );

    /**
     * 输出内部的内存管理信息.
     *
     *  @param      fp    [in]    要输出的文件指针.
     * 
     */
    static void print_memory_info( FILE * fp );

    /**
     * 取得所分配的内存总大小.
     *
     */
    static size_t get_total_allocated_size( void );


    /**
     *
     * 设置某一类型的allocator所管理的内存大小.
     *
     * @param  type                 [in]  allocator type, 可以为UPF_TOPLEVEL_PERMANENT_ALLOCATOR或
     *                                    UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR. 
     * @param  managed_memory_size  [in]  所管理的内存大小.
     *
     * @return true  设置成功.
     *         false 设置失败.
     */
    static bool set_managed_memory_size( toplevel_allocator_t type,
                                         size_t               managed_memory_size );

    /**
     *
     * 回收某一类型的allocator所管理的内存, 归还给操作系统.
     *
     * @param  type                 [in]  allocator type, 可以为UPF_TOPLEVEL_PERMANENT_ALLOCATOR或
     *                                    UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR. 
     *
     * @return true  回收成功.
     *         false 回收失败.
     */
    static bool reclaim( toplevel_allocator_t type );

    /**
     *
     * 恢复某一类型的allocator的分配内存操作.
     *
     * @param  type                 [in]  allocator type, 可以为UPF_TOPLEVEL_PERMANENT_ALLOCATOR或
     *                                    UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR. 
     *
     * @return true  恢复成功.
     *         false 恢复失败.
     */
    static bool restore( toplevel_allocator_t type );

    /**
     * 获取一个动态的toplevel allocator.
     *  
     * @param type      [out]        获取成功后的allocator type.
     * 
     * @return true   获取成功. 
     *         false  获取失败.
     *  
     */
    static bool get_dynamic_toplevel_allocator( toplevel_allocator_t & type );

    /**
     * 释放一个动态的toplevel allocator.
     *  
     * @param type      [in]        调用get_dynamic_toplevel_allocator获取成功后 
     *                              的allocator type.
     * 
     * @return true   释放成功. 
     *         false  释放失败.
     *  
     */
    static bool release_dynamic_toplevel_allocator( toplevel_allocator_t type );

private:
    /** 用于标识dynamic allocator的使用状态*/
    enum Dynamic_Allocator_State 
    {
        DAS_NONE = 0,   /**< dynamic allocator没有创建.*/
        DAS_USED,   /**< dynamic allocator已使用.   */
        DAS_UNUSED  /**< dynamic allocator未使用.   */
    };

private:
    /// 创建与type相对应的allocator
    static inline void
    create_toplevel_allocator( toplevel_allocator_t type );

    /// 用于存放Virtual Allocator相应类型实现的数组, 初始都为0.
    static UPF_Virtual_Memory_Allocator_Impl * 
        sm_toplevel_allocators[ UPF_TOPLEVEL_ALLOCATOR_TYPES ];

    /// 用于存放动态Virtual Allocator的状态的数组, 初始都为DAS_NONE.
    static Dynamic_Allocator_State
        sm_toplevel_dynamic_allocators_states[ UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES ];

    /// 用于标识是否第一次使用dynamic allocator, 如果是, 则进行一些初始化.
    static bool sm_first_use_dynamic_allocator;

private:
    /// 用于创建virtual allocator impl的lock
    static UPF_Thread_Mutex  sm_creator_lock;

    /// 用于内部管理的lock
    static UPF_Thread_Mutex  sm_manage_lock;

private:
    /// 声明一个Virtual allocator 的析构器.

    class Destroyer
    {
    public:
        ~Destroyer();
    };

    friend class Destroyer;
    
    static Destroyer sm_virtual_memory_allocator_destroyer;
    
};


#endif /* UPF_VIRTUAL_MEMORY_ALLOCATOR_H */

