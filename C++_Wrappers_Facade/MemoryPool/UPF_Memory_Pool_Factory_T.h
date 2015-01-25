/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Factory_T.h                                                                *
*       CREATE DATE     : 2007-11-12                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ�ع��������ڴ����ڴ��![CN]                                                        *
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
 * @brief �ڴ��ʵ�����ڴ�����ֵ�ӳ������
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
 * @brief �ڴ�ع�����, ���ݸ�������Ϣ������Ӧ���ڴ��.
 * 
 * @note ��ʹ��create_memory_pool�����ڴ��ʱ, ������delete_memory_pool���ͷ�
 *       �ڴ��, ������ͨ��delete UPF_IMemory_Pool�ӿ�ָ�����ͷ��ڴ��, ��Ϊ
 *       UPF_Memory_Pool_Factory_T�ڲ�ά��һ���ڴ�������ڴ��ʵ����ӳ���,
 *       ���ͨ��delete UPF_IMemory_Pool�ӿ�ָ�����ͷ��ڴ��, 
 *       UPF_Memory_Pool_Factory_T�ڲ���ӳ����ò�������, �Ժ��
 *       UPF_Memory_Pool_Factory_T���û��������.
 *       �������ͨ��ʹUPF_IMemory_Pool����������Ϊprotected����Client��
 *       delete UPF_IMemory_Pool�ӿ�ָ��, ����Ҫ����
 *       UPF_Memory_Pool_Factory_T��delete UPF_IMemory_Pool��Ȩ��, ��VC6��
 *      ֧��ģ�����Ԫ����.
 *  
 */
template < class Lock >
class UPF_Memory_Pool_Factory_T
{
public:
    /// ���ݸ�������Ϣ������Ӧ���ڴ��.
    static UPF_IMemory_Pool * create_memory_pool( const char *          memory_pool_name,
                                                  bool                  is_fixed,
                                                  toplevel_allocator_t  allocator_type,
                                                  size_t                allocated_memory_limit = MAX_MEMORY_REQ_SIZE,
                                                  bool                  is_debug    = false,
                                                  size_t                object_size = 16 - 4,
                                                  size_t                num_objects_per_block = 256 );

    /// ���ݸ�������Ϣ���������͵��ڴ��.
    static UPF_IMemory_Pool * create_trans_memory_pool( 
                const char *         memory_pool_name,
                toplevel_allocator_t allocator_type,
                size_t               allocated_memory_limit = MAX_MEMORY_REQ_SIZE );


    /// ���ݸ������ڴ��������ȡ�ڴ�ؽӿ�ʵ��.
    static UPF_IMemory_Pool * get_memory_pool( const char * memory_pool_name );


    /// ���ݸ������ڴ�������ͷ��ڴ��.
    static void delete_memory_pool( const char * memory_pool_name );


public:
    typedef std::vector< UPF_Memory_Pool_Map_Item > Memory_Pool_Mgr;

private:
    typedef UPF_Memory_Pool_Factory_T< Lock > SELF_CLASS;

private:
    /// ȡ��������Ϣ
    static int get_profile_string( const char * entry_name, int default_value );

    /// ��ϵͳ����������ȡ���Ƿ���ڴ�ص��Թ��ܵı�־.
    static bool get_debug_flag_from_env( void );

    /// ����ϵͳ���������������ڴ��.
    static UPF_IMemory_Pool * create_memory_pool_from_env( 
        const char *                     memory_pool_name,
        size_t                           allocated_memory_limit,
        toplevel_allocator_t             allocator_type );

private:
    /// private default constructor, prevent client from creating 
    /// UPF_MEmory_Pool_Factory instance.
    UPF_Memory_Pool_Factory_T();

private:
    /// �߳�ͬ����
    static Lock    sm_lock;
    static Lock    sm_env_lock;        

    /// �ڴ��ʵ�����ڴ�����ֵ�ӳ�������
    static Memory_Pool_Mgr    sm_memory_pool_mgr;
};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_Factory_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */


#endif /* UPF_MEMORY_POOL_FACTORY_T_H */

