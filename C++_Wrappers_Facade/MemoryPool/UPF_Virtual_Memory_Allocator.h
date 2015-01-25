/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Virtual_Memory_Allocator.h                                                             *
*       CREATE DATE     : 2008-4-4                                                                                   *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]ʹ��VirtualAlloc�����������ڴ��allocator![CN]                                         *
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
 * @brief ʹ��VirtualAlloc�����������ڴ��Allocator, �������ڴ�ص�allocator.
 * 
 * @detail ��Ҫ������WINCEƽ̨��, WINCEƽ̨����VirtualAlloc Reserve 2M���ϵ��ڴ�
 *         ʱ, WINCEϵͳ�Ͳ���32M�ĳ���ռ��л����˶���0x004000000���ϵ��ڴ滮��
 *         ʹ�����Allocate�Ϳɽ��һЩWINCEƽ̨�ڴ治�������.
 *         Ŀǰʹ�õķ���������ύһ������ڴ��20M, Ȼ��ʹ�� 
 *         ���� UPF_Common_Memory_Pool �Ĳ��Խ��й���, ֻ�������������Ϊ
 *         ��ͬ, UPF_Common_Memory_Poolʹ�õ��ڴ��, ÿ�η�����СΪ8k, ����Ϊ
 *         4k.
 *         �����ڲ������size��8kΪ��ʼֵ, Ȼ�����±�ʼ����, ������
 *         3924k�ڴ�.
 *        
 * ��ŵĹ��������, �ڲ�����һ��512��Ԫ�ص�����m_free_block_list,ÿ��Ԫ��
 * ����̶���С���ڴ��, ����������μ��±�:<br><pre>
 * -------------------------------------------
 * |  ��������  |   ��������    |  ����Χ   |
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
     * �����ڴ�, ��һ�δ������ڴ����ύ.
     * 
     * @param size          [in]   Ҫ������ڴ��С, һ��Ӧ��Ϊ8k�ı���.
     * @param type          [in]   allocator type, ����ΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR��
     *                             UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR.
     * 
     * @return
     *  - <em>һ����Ч���ڴ�ָ��</em>     ����ɹ�.
     *  - <em>0</em>                      ����ʧ��.
     */
    static void * allocate( size_t size, toplevel_allocator_t type );

    /** 
     * �ͷ��ڴ�.
     * 
     * @param ptr          [in]  Ҫ�ͷŵ��ڴ�ָ��.
     * @param size         [in]  ptr��ָ����ڴ�Ĵ�С.
     * @param type         [in]  allocator type, ����ΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR��
     *                           UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR.
     * 
     */
    static void deallocate( void * ptr, size_t size, toplevel_allocator_t type );

    /** 
     * ���������ڴ�.
     * 
     * @param ptr          [in]  ԭʼָ��.
     * @param new_size     [in]  �µ�size.
     * @param type         [in]  allocator type, ����ΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR��
     *                           UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR.
     * @return
     *  - <em>һ����Ч���ڴ�ָ��</em>     ����ɹ�.
     *  - <em>0</em>                      ����ʧ��.
     */
    static void * reallocate( void * ptr, size_t new_size, toplevel_allocator_t type );

    /**
     * ����ڲ����ڴ������Ϣ.
     *
     *  @param      fp    [in]    Ҫ������ļ�ָ��.
     * 
     */
    static void print_memory_info( FILE * fp );

    /**
     * ȡ����������ڴ��ܴ�С.
     *
     */
    static size_t get_total_allocated_size( void );


    /**
     *
     * ����ĳһ���͵�allocator��������ڴ��С.
     *
     * @param  type                 [in]  allocator type, ����ΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR��
     *                                    UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR. 
     * @param  managed_memory_size  [in]  ��������ڴ��С.
     *
     * @return true  ���óɹ�.
     *         false ����ʧ��.
     */
    static bool set_managed_memory_size( toplevel_allocator_t type,
                                         size_t               managed_memory_size );

    /**
     *
     * ����ĳһ���͵�allocator��������ڴ�, �黹������ϵͳ.
     *
     * @param  type                 [in]  allocator type, ����ΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR��
     *                                    UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR. 
     *
     * @return true  ���ճɹ�.
     *         false ����ʧ��.
     */
    static bool reclaim( toplevel_allocator_t type );

    /**
     *
     * �ָ�ĳһ���͵�allocator�ķ����ڴ����.
     *
     * @param  type                 [in]  allocator type, ����ΪUPF_TOPLEVEL_PERMANENT_ALLOCATOR��
     *                                    UPF_TOPLEVEL_DISCARDABLE_ALLOCATOR. 
     *
     * @return true  �ָ��ɹ�.
     *         false �ָ�ʧ��.
     */
    static bool restore( toplevel_allocator_t type );

    /**
     * ��ȡһ����̬��toplevel allocator.
     *  
     * @param type      [out]        ��ȡ�ɹ����allocator type.
     * 
     * @return true   ��ȡ�ɹ�. 
     *         false  ��ȡʧ��.
     *  
     */
    static bool get_dynamic_toplevel_allocator( toplevel_allocator_t & type );

    /**
     * �ͷ�һ����̬��toplevel allocator.
     *  
     * @param type      [in]        ����get_dynamic_toplevel_allocator��ȡ�ɹ��� 
     *                              ��allocator type.
     * 
     * @return true   �ͷųɹ�. 
     *         false  �ͷ�ʧ��.
     *  
     */
    static bool release_dynamic_toplevel_allocator( toplevel_allocator_t type );

private:
    /** ���ڱ�ʶdynamic allocator��ʹ��״̬*/
    enum Dynamic_Allocator_State 
    {
        DAS_NONE = 0,   /**< dynamic allocatorû�д���.*/
        DAS_USED,   /**< dynamic allocator��ʹ��.   */
        DAS_UNUSED  /**< dynamic allocatorδʹ��.   */
    };

private:
    /// ������type���Ӧ��allocator
    static inline void
    create_toplevel_allocator( toplevel_allocator_t type );

    /// ���ڴ��Virtual Allocator��Ӧ����ʵ�ֵ�����, ��ʼ��Ϊ0.
    static UPF_Virtual_Memory_Allocator_Impl * 
        sm_toplevel_allocators[ UPF_TOPLEVEL_ALLOCATOR_TYPES ];

    /// ���ڴ�Ŷ�̬Virtual Allocator��״̬������, ��ʼ��ΪDAS_NONE.
    static Dynamic_Allocator_State
        sm_toplevel_dynamic_allocators_states[ UPF_DYNAMIC_TOPLEVEL_ALLOCATOR_TYPES ];

    /// ���ڱ�ʶ�Ƿ��һ��ʹ��dynamic allocator, �����, �����һЩ��ʼ��.
    static bool sm_first_use_dynamic_allocator;

private:
    /// ���ڴ���virtual allocator impl��lock
    static UPF_Thread_Mutex  sm_creator_lock;

    /// �����ڲ������lock
    static UPF_Thread_Mutex  sm_manage_lock;

private:
    /// ����һ��Virtual allocator ��������.

    class Destroyer
    {
    public:
        ~Destroyer();
    };

    friend class Destroyer;
    
    static Destroyer sm_virtual_memory_allocator_destroyer;
    
};


#endif /* UPF_VIRTUAL_MEMORY_ALLOCATOR_H */

