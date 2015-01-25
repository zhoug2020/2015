/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_Info_Manager_Impl_T.h                                                      *
*       CREATE DATE     : 2007-11-20                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]UPF_IMemory_Pool_Info_Manager�ӿڵ�ʵ��ģ����![CN]                                     *
*********************************************************************************************************************/

#ifndef UPF_MEMORY_POOL_INFO_MANAGER_IMPL_T_H
#define UPF_MEMORY_POOL_INFO_MANAGER_IMPL_T_H

#include "UPF_IMemory_Pool_Info_Manager.h"

/**
 * @class UPF_Memory_Pool_Info_Manager_Impl
 * 
 * @brief ʵ��UPF_IMemory_Pool_Info_Manager�ӿ�
 * 
 *   -# Lock          �߳�ͬ����
 *   -# PrintPolicy  ��һ����ӡ������, ��Ҫʵ�����½ӿں���
 *                   static void on_before_print( void );
 *                   static void print( const char * fmt, ... );
 *                   static void on_after_print( void );
 * 
 */
template < class Lock, class PrintPolicy >
class UPF_Memory_Pool_Info_Manager_Impl : public UPF_IMemory_Pool_Info_Manager
{
public:
    /// һ����������������һ��UPF_IMemory_Pool_Info_Manager��ʵ��.
    static UPF_IMemory_Pool_Info_Manager * instance( void );

    /// �����UPF_Memory_Pool_Info_Manager_Impl������ռ�õ��ڴ�.
    static void destroy( void );

public:
     /// ���UPF_IMemory_Pool_Infoʵ�����ڴ�ط�����Ϣ��������.
    virtual void add_memory_pool_info(UPF_IMemory_Pool_Info* memory_pool_info);

     /// �����ڴ��������ȡ��UPF_IMemory_Pool_Infoʵ��.
    virtual UPF_IMemory_Pool_Info* get_memory_pool_info(const char* memory_pool_name);

    /// ��ӡ������������ڴ�ط�����Ϣ.
    virtual void print_all_memory_pool_infos( FILE * fp );

    /// ��ӡ������������ڴ�صĸ�Ҫ��Ϣ.
    virtual void print_all_memory_pool_summary_infos( const char * dir );

    /// �����ڴ���ַ������UPF_IMemory_Poolʵ��.
    virtual UPF_IMemory_Pool * get_memory_pool_with_memory_addr( 
        void * ptr, UPF_IMemory_Pool * excluded_memory_pool = 0 );

    /// ���ڴ�ط�����Ϣ���������Ƴ�UPF_IMemory_Pool_Infoʵ��.
    virtual void remove_memory_pool_info(UPF_IMemory_Pool_Info* memory_pool_info);

    /// �����ж��Ƿ��� allocator_type ��Ӧ��allocator�ﻹ���ڴ�ء�
    virtual bool is_exist_memory_pool( toplevel_allocator_t allocator_type,
                                       bool                 is_log = true,
                                       const char *         custom_msg = 0 );

private:
    /// ���캯��, ��ʼ���ڲ���״̬.
    UPF_Memory_Pool_Info_Manager_Impl( void );
        
    /// �����������ͷ��ڲ����������Դ.
    ~UPF_Memory_Pool_Info_Manager_Impl( void );

private:
    /** 
     * @struct UPF_Memory_Pool_Info_Item
     * 
     * @brief UPF_Memory_Pool_Info_Item ������ÿ��Ԫ�ص�����
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

        // ʵ��UPF_IMemory_Pool_Info�ӿڵ�ʵ��
        UPF_IMemory_Pool_Info*      m_memory_pool_info;

        // ָ����һ��UPF_Memory_Pool_Info_Item
        UPF_Memory_Pool_Info_Item*  m_next_item;
    };

private:
   /// ���ڴ�����ʵ�����߳�ͬ��.
    static Lock sm_instance_lock;

private:

    /// UPF_Memory_Pool_Info_Item�����ͷ���.
    UPF_Memory_Pool_Info_Item * m_memory_pool_info_head;

    /// �����߳�ͬ��
    Lock m_lock;


};

#if defined (UPF_TEMPLATES_REQUIRE_SOURCE)
#include "UPF_Memory_Pool_Info_Manager_Impl_T.cpp"
#endif /* UPF_TEMPLATES_REQUIRE_SOURCE */

#endif /* UPF_MEMORY_POOL_INFO_MANAGER_IMPL_T_H */
