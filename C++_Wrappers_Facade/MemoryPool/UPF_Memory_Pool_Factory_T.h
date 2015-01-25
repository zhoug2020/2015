/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Factory_T.h                                                                *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]内存池工厂，用于创建内存池![CN]                                                        *
*********************************************************************************************************************/
#ifndef UPF_MEMORY_POOL_FACTORY_T_H
#define UPF_MEMORY_POOL_FACTORY_T_H

#include <stddef.h>                 /* for size_t */
#include <vector>

#include "UPF_Memory_Pool_Define.h" /* for MAX_MEMORY_REQ_SIZE */

// Forward declaration.
class UPF_IMemory_Pool;

/** 
 * @struct UPF_Memory_Pool_Map_Item
 * 
 * @brief 内存池实例和内存池名字的映射类型
 * 
 */
struct UPF_Memory_Pool_Map_Item
{
    const char *       memory_pool_name;

    UPF_IMemory_Pool * memory_pool;
};

/** 
 * @class UPF_Memory_Pool_Factory
 * 
 * @brief 内存池工厂类, 根据给定的信息创建相应的内存池.
 * 
 * @note 当使用create_memory_pool创建内存池时, 必须用delete_memory_pool来释放
 *       内存池, 而不能通过delete UPF_IMemory_Pool接口指针来释放内存池, 因为
 *       UPF_Memory_Pool_Factory_T内部维护一个内存池名和内存池实例的映射表,
 *       如果通过delete UPF_IMemory_Pool接口指针来释放内存池, 
 *       UPF_Memory_Pool_Factory_T内部的映射表将得不到更新, 以后的
 *       UPF_Memory_Pool_Factory_T调用会产生问题.
 *       另外可以通过使UPF_IMemory_Pool的析构函数为protected限制Client端
 *       delete UPF_IMemory_Pool接口指针, 但需要允许
 *       UPF_Memory_Pool_Factory_T有delete UPF_IMemory_Pool的权力, 但VC6不
 *      支持模板的友元声明.
 *  
 */
template < class Lock >
class UPF_Memory_Pool_Factory_T
{
public:
    /// 根据给定的信息创建相应的内存池.
    static UPF_IMemory_Pool * create_memory_pool( const char *          memory_pool_name,
                                                  bool                  is_fixed,
                                                  toplevel_allocator_t  allocator_type,
                                                  size_t                allocated_memory_limit = MAX_MEMORY_REQ_SIZE,
                                                  bool                  is_debug    = false,
                                                  size_t                object_size = 16 - 4,
                                                  size_t                num_objects_per_block = 256 );

    /// 根据给定的信息创建事务型的内存池.
    static UPF_IMemory_Pool * create_trans_memory_pool( 
                const char *         memory_pool_name,
                toplevel_allocator_t allocator_type,
                size_t               allocated_memory_limit = MAX_MEMORY_REQ_SIZE );


    /// 根据给定的内存池名来获取内存池接口实例.
    static UPF_IMemory_Pool * get_memory_pool( const char * memory_pool_name );


    /// 根据给定的内存池名来释放内存池.
    static void delete_memory_pool( const char * memory_pool_name );


public:
    typedef std::vector< UPF_Memory_Pool_Map_Item > Memory_Pool_Mgr;

private:
    typedef UPF_Memory_Pool_Factory_T< Lock > SELF_CLASS;

private:
    /// 取得配置信息
    static int get_profile_string( const char * entry_name, int default_value );

    /// 从系统环境变量中取出是否打开内存池调试功能的标志.
    static bool get_debug_flag_from_env( void );

    /// 根据系统环境变量来创建内存池.
    static UPF_IMemory_Pool * create_memory_pool_from_env( 
        const char *                     memory_pool_name,
        size_t                           allocated_memory_limit,
        toplevel_allocator_t             allocator_type );

private:
    /// private default constructor, prevent client from creating 
    /// UPF_MEmory_Pool_Factory instance.
    UPF_Memory_Pool_Factory_T();

private:
    /// 线程同步锁
    static Lock    sm_lock;
    static Lock    sm_env_lock;        

    /// 内存池实例和内存池名字的映射管理器
    static Memory_Pool_Mgr    sm_memory_pool_mgr;
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_Factory_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */


#endif /* UPF_MEMORY_POOL_FACTORY_T_H */

