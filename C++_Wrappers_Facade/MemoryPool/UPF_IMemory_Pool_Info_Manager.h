/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IMemory_Pool_Info_Manager.h                                                            *
*       CREATE DATE     : 2007-11-19                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池分配信息管理接口定义![CN]                                                        *
*********************************************************************************************************************/

#ifndef UPF_IMEMORY_POOL_INFO_MANAGER_H
#define UPF_IMEMORY_POOL_INFO_MANAGER_H

#include <stdio.h>
#include "UPF_Memory_Pool_Define.h"

// Forward declaration.
class UPF_IMemory_Pool_Info;
class UPF_IMemory_Pool;

/** 
 * @class UPF_IMemory_Pool_Info_Manager
 * 
 * @brief 内存池分配信息管理接口
 * 
 */
class UPF_IMemory_Pool_Info_Manager
{
public:
    /** 
     * 添加UPF_IMemory_Pool_Info实例到内存池分配信息管理器中.
     * 
     * @param memory_pool_info   [in]  实现UPF_IMemory_Pool_Info接口的实例
     */
    virtual void add_memory_pool_info( UPF_IMemory_Pool_Info * memory_pool_info ) = 0;

    /** 
     * 从内存池分配信息管理器中移除UPF_IMemory_Pool_Info实例. 
     * 
     * @param memory_pool_info   [in] 实现UPF_IMemory_Pool_Info接口的实例
     */
    virtual void remove_memory_pool_info( UPF_IMemory_Pool_Info * memory_pool_info ) = 0;

    /** 
     * 根据内存池名字来取出UPF_IMemory_Pool_Info实例.
     * 
     * @param memory_pool_name      [in]   内存池名字.
     * 
     * @retval UPF_IMemory_Pool_Info实例   查找成功
     * @retval 0                           查找失败
     */
    virtual UPF_IMemory_Pool_Info * get_memory_pool_info( const char * memory_pool_name ) = 0;


    /** 
     * 根据内存块地址来查找UPF_IMemory_Pool实例.
     * 
     * @param ptr                   [in] 内存块地址.
     * @param excluded_memory_pool  [in] 在查找UPF_IMemory_Pool实例时，会忽略
     *                                   该内存池实例.
     * 
     * @retval 一个分配ptr指向的内存的UPF_IMemory_Pool实例 查找成功.
     * @retval 0                                           查找失败.
     */
    virtual UPF_IMemory_Pool * get_memory_pool_with_memory_addr( 
        void * ptr, UPF_IMemory_Pool * excluded_memory_pool = 0 ) = 0;

    /** 
     * 打印所管理的所有内存池分配信息.
     *  
     * @param   fp    [in]   要打印的文件的指针.
     * @param   dir   [in]   要打印的文件所在的目录.
     * 
     */
    virtual void print_all_memory_pool_infos( FILE * fp ) = 0;


    /** 
     * 打印所管理的所有内存池的概要信息.
     *  
     * @param   dir   [in]   要打印的文件所在的目录.
     * 
     */
    virtual void print_all_memory_pool_summary_infos( const char * dir ) = 0;

    /** 
     * 用于判断是否与 allocator_type 对应的allocator里还有内存池, 如果存在, 根据is_log的值,
     * 来判断是否log存在的内存池名.
     *  
     * @param allocator_type           [in] allocator的类型, 目前支持UPF_TOPLEVEL_PERMANENT_ALLOCATOR和
     *                                      UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, 缺省为UPF_TOPLEVEL_PERMANENT_ALLOCATOR
     * @param is_log                   [in] 如果allocator里还有内存池, 用于指定是否log存在的内存池名, 缺省值为false.
     *
     * @param custom_msg               [in] 如果allocator里还有内存池并且is_log为true时, custom_msg用于指定额外的log消息.
     * 
     */
    virtual bool is_exist_memory_pool( toplevel_allocator_t allocator_type,
                                       bool                 is_log = true,
                                       const char *         custom_msg = 0 ) = 0;

public:
    virtual ~UPF_IMemory_Pool_Info_Manager() { }

};

#endif /* UPF_IMEMORY_POOL_INFO_MANAGER_H */
