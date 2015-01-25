/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Manager_Impl_T.h                                                      *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]UPF_IMemory_Pool_Info_Manager接口的实现模版类![CN]                                     *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_INFO_MANAGER_IMPL_T_H
#define UPF_MEMORY_POOL_INFO_MANAGER_IMPL_T_H

#include "UPF_IMemory_Pool_Info_Manager.h"

/**
 * @class UPF_Memory_Pool_Info_Manager_Impl
 * 
 * @brief 实现UPF_IMemory_Pool_Info_Manager接口
 * 
 *   -# Lock          线程同步锁
 *   -# PrintPolicy  是一个打印策略类, 需要实现如下接口函数
 *                   static void on_before_print( void );
 *                   static void print( const char * fmt, ... );
 *                   static void on_after_print( void );
 * 
 */
template < class Lock, class PrintPolicy >
class UPF_Memory_Pool_Info_Manager_Impl : public UPF_IMemory_Pool_Info_Manager
{
public:
    /// 一个单例方法，返回一个UPF_IMemory_Pool_Info_Manager的实例.
    static UPF_IMemory_Pool_Info_Manager * instance( void );

    /// 清除该UPF_Memory_Pool_Info_Manager_Impl对象所占用的内存.
    static void destroy( void );

public:
     /// 添加UPF_IMemory_Pool_Info实例到内存池分配信息管理器中.
    virtual void add_memory_pool_info(UPF_IMemory_Pool_Info* memory_pool_info);

     /// 根据内存池名字来取出UPF_IMemory_Pool_Info实例.
    virtual UPF_IMemory_Pool_Info* get_memory_pool_info(const char* memory_pool_name);

    /// 打印所管理的所有内存池分配信息.
    virtual void print_all_memory_pool_infos( FILE * fp );

    /// 打印所管理的所有内存池的概要信息.
    virtual void print_all_memory_pool_summary_infos( const char * dir );

    /// 根据内存块地址来查找UPF_IMemory_Pool实例.
    virtual UPF_IMemory_Pool * get_memory_pool_with_memory_addr( 
        void * ptr, UPF_IMemory_Pool * excluded_memory_pool = 0 );

    /// 从内存池分配信息管理器中移除UPF_IMemory_Pool_Info实例.
    virtual void remove_memory_pool_info(UPF_IMemory_Pool_Info* memory_pool_info);

    /// 用于判断是否与 allocator_type 对应的allocator里还有内存池。
    virtual bool is_exist_memory_pool( toplevel_allocator_t allocator_type,
                                       bool                 is_log = true,
                                       const char *         custom_msg = 0 );

private:
    /// 构造函数, 初始化内部的状态.
    UPF_Memory_Pool_Info_Manager_Impl( void );
        
    /// 析构函数，释放内部所申请的资源.
    ~UPF_Memory_Pool_Info_Manager_Impl( void );

private:
    /** 
     * @struct UPF_Memory_Pool_Info_Item
     * 
     * @brief UPF_Memory_Pool_Info_Item 链表中每个元素的类型
     * 
     */
    struct UPF_Memory_Pool_Info_Item
    {

        UPF_Memory_Pool_Info_Item( UPF_IMemory_Pool_Info *    memory_pool_info,
                                   UPF_Memory_Pool_Info_Item* next_item )
        {
            m_memory_pool_info = memory_pool_info;
            m_next_item        = next_item;
        }

        // 实现UPF_IMemory_Pool_Info接口的实例
        UPF_IMemory_Pool_Info*      m_memory_pool_info;

        // 指向下一个UPF_Memory_Pool_Info_Item
        UPF_Memory_Pool_Info_Item*  m_next_item;
    };

private:
   /// 用于创建单实例的线程同步.
    static Lock sm_instance_lock;

private:

    /// UPF_Memory_Pool_Info_Item链表的头结点.
    UPF_Memory_Pool_Info_Item * m_memory_pool_info_head;

    /// 用于线程同步
    Lock m_lock;


};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_Info_Manager_Impl_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_MEMORY_POOL_INFO_MANAGER_IMPL_T_H */
