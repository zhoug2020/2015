/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_IMemory_Pool_Info.h                                                                   *
*       CREATE DATE     : 2007-11-19                                                                                 *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�ڴ����Ϣ�ӿ�![CN]                                                                    *
*********************************************************************************************************************/

#ifndef UPF_IMEMORY_POOL_INFO_H
#define UPF_IMEMORY_POOL_INFO_H

#include <stddef.h>        /* for size_t */
#include "UPF_Memory_Pool_Define.h"

class UPF_IMemory_Pool;

/** 
 * @class UPF_IMemory_Pool_Info
 * 
 * @brief �ڴ����Ϣ�ӿ�, ͨ��Ӧ����ʵ��IMemoryPool������ʵ��, ��Ϊֻ��ʵ�����
 * ֪����ǰ�ڴ�ķ������.
 * 
 */
class UPF_IMemory_Pool_Info
{
public:
    /** 
     * ��ȡ�ڴ�ص�����.
     *  
     * @return �ڴ�ص�����.
     */
    virtual const char * get_memory_pool_name( void ) const = 0;

    /**
     * ��ȡ�ڴ�ص����ͣ���ǰ֧�����֣��̶���С�ģ��ɱ��С��.
     * 
     * @return �ڴ�ص�����.
     */
    virtual const char * get_memory_pool_type( void ) const = 0;

    /** 
     * ����ڴ����������ڴ��С, ���ڴ�ش�ϵͳ������������ڴ�Ĵ�С.
     *  
     * @return �ڴ����������ڴ��С
     */
    virtual size_t get_managed_memory_size( void ) const = 0;

    /** 
     * ����ڴ���е�ǰ�ѷ����Client�˵��ڴ��С, ���ֵ�ܱ�ʾ��ĳһʱ��Client��
     * ��ʹ�õ��ڴ��С.
     * 
     * @return  �����Client�˵��ڴ��С. 
     */
    virtual size_t get_allocated_memory_size( void ) const = 0;

    /** 
     *  ��ȡ�����Client�˵��ڴ��С�ķ�ֵ, ���ֵ�ܱ�ʾ��ĳһʱ��Client��ʹ�õ�
     *  �ڴ��С�����ֵ.
     * 
     * @return �����Client�˵��ڴ��С�ķ�ֵ
     */
    virtual size_t get_allocated_memory_peak( void ) const = 0;

    /** 
     * ��ȡ�ڴ�����˷ѵ��ڴ��С, ��Ϊ�ڴ���е�ÿ���ڵ��С���ǰ���һ����������
     * �����, ����ÿ�η���������˷ѵ��ڴ�, ʹ������ӿں������Ի�ȡĳһʱ���ڴ��
     * �����˷ѵ��ڴ��С.
     * 
     * @return �ڴ�����˷ѵ��ڴ��С
     */
    virtual size_t get_wasted_memory_size( void ) const = 0;


    /**
     * ��ȡ�ڴ���������ֵ.
     */
    virtual size_t get_allocated_memory_limit( void ) const = 0;

    /** 
     * ����һЩ����ԭ����Ҫͨ��UPF_IMemory_Pool_Info�ӿ�
     * ����UPF_IMemory_Pool�ӿ�ָ��.
     * 
     * @return ʵ��UPF_IMemory_Pool�ӿڵ�ָ��, ��ǰʵ��ȷ������ֵ�϶���Ϊ0.
     */
    virtual UPF_IMemory_Pool * get_memory_pool( void ) = 0;

    /** 
     * ����allocator type.
     *
     * @return allocator type.
     */
    virtual toplevel_allocator_t get_allocator_type( void ) const = 0;

public:
    virtual ~UPF_IMemory_Pool_Info( void ) { }
};

#endif /* UPF_IMEMORY_POOL_INFO_H */

