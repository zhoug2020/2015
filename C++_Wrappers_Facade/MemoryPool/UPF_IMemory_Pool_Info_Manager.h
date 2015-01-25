/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IMemory_Pool_Info_Manager.h                                                            *
*       CREATE DATE     : 2007-11-19                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ�ط�����Ϣ����ӿڶ���![CN]                                                        *
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
 * @brief �ڴ�ط�����Ϣ����ӿ�
 * 
 */
class UPF_IMemory_Pool_Info_Manager
{
public:
    /** 
     * ���UPF_IMemory_Pool_Infoʵ�����ڴ�ط�����Ϣ��������.
     * 
     * @param memory_pool_info   [in]  ʵ��UPF_IMemory_Pool_Info�ӿڵ�ʵ��
     */
    virtual void add_memory_pool_info( UPF_IMemory_Pool_Info * memory_pool_info ) = 0;

    /** 
     * ���ڴ�ط�����Ϣ���������Ƴ�UPF_IMemory_Pool_Infoʵ��. 
     * 
     * @param memory_pool_info   [in] ʵ��UPF_IMemory_Pool_Info�ӿڵ�ʵ��
     */
    virtual void remove_memory_pool_info( UPF_IMemory_Pool_Info * memory_pool_info ) = 0;

    /** 
     * �����ڴ��������ȡ��UPF_IMemory_Pool_Infoʵ��.
     * 
     * @param memory_pool_name      [in]   �ڴ������.
     * 
     * @retval UPF_IMemory_Pool_Infoʵ��   ���ҳɹ�
     * @retval 0                           ����ʧ��
     */
    virtual UPF_IMemory_Pool_Info * get_memory_pool_info( const char * memory_pool_name ) = 0;


    /** 
     * �����ڴ���ַ������UPF_IMemory_Poolʵ��.
     * 
     * @param ptr                   [in] �ڴ���ַ.
     * @param excluded_memory_pool  [in] �ڲ���UPF_IMemory_Poolʵ��ʱ�������
     *                                   ���ڴ��ʵ��.
     * 
     * @retval һ������ptrָ����ڴ��UPF_IMemory_Poolʵ�� ���ҳɹ�.
     * @retval 0                                           ����ʧ��.
     */
    virtual UPF_IMemory_Pool * get_memory_pool_with_memory_addr( 
        void * ptr, UPF_IMemory_Pool * excluded_memory_pool = 0 ) = 0;

    /** 
     * ��ӡ������������ڴ�ط�����Ϣ.
     *  
     * @param   fp    [in]   Ҫ��ӡ���ļ���ָ��.
     * @param   dir   [in]   Ҫ��ӡ���ļ����ڵ�Ŀ¼.
     * 
     */
    virtual void print_all_memory_pool_infos( FILE * fp ) = 0;


    /** 
     * ��ӡ������������ڴ�صĸ�Ҫ��Ϣ.
     *  
     * @param   dir   [in]   Ҫ��ӡ���ļ����ڵ�Ŀ¼.
     * 
     */
    virtual void print_all_memory_pool_summary_infos( const char * dir ) = 0;

    /** 
     * �����ж��Ƿ��� allocator_type ��Ӧ��allocator�ﻹ���ڴ��, �������, ����is_log��ֵ,
     * ���ж��Ƿ�log���ڵ��ڴ����.
     *  
     * @param allocator_type           [in] allocator������, Ŀǰ֧��UPF_TOPLEVEL_PERMANENT_ALLOCATOR��
     *                                      UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR, ȱʡΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR
     * @param is_log                   [in] ���allocator�ﻹ���ڴ��, ����ָ���Ƿ�log���ڵ��ڴ����, ȱʡֵΪfalse.
     *
     * @param custom_msg               [in] ���allocator�ﻹ���ڴ�ز���is_logΪtrueʱ, custom_msg����ָ�������log��Ϣ.
     * 
     */
    virtual bool is_exist_memory_pool( toplevel_allocator_t allocator_type,
                                       bool                 is_log = true,
                                       const char *         custom_msg = 0 ) = 0;

public:
    virtual ~UPF_IMemory_Pool_Info_Manager() { }

};

#endif /* UPF_IMEMORY_POOL_INFO_MANAGER_H */
