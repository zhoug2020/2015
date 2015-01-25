/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Global_Memory_Pool_Creator.h                                                           *
*       CREATE DATE     : 2008-7-23                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]�����ṩGlobal Memory Pool�Ĵ�����![CN]                                                *
*********************************************************************************************************************/
#ifndef UPF_GLOBAL_MEMORY_POOL_CREATOR_H
#define UPF_GLOBAL_MEMORY_POOL_CREATOR_H

#include "../../OS_Adapter/UPF_OS.h"
#include "UPF_Thread_Sync.h"
#include "UPF_IGlobal_Memory_Pool.h"

/** 
 * @class UPF_Global_Memory_Pool_Creator
 * 
 * @brief �����ṩһ��Global Memory Pool�Ĵ�����.
 * 
 */
class /*UPF_Export*/ UPF_Global_Memory_Pool_Creator
{
public:
    /** 
     * ����һ��ȫ�ֶ���, ���ü�����1.
     * 
     * @return BaseClass���͵Ķ���
     */
    static UPF_IGlobal_Memory_Pool * get_instance( void );

    /**
     * destroyһ��ȫ�ֶ���, ���ü�����1, ���Ϊ0, ������.
     */

    static void release_instance( void );

private:
    /// ���ڴ�����������ü�����lock.
    static UPF_Thread_Mutex sm_lock;

    /// �������ü���.
    static size_t                      sm_object_ref_count;

    /// ���ڴ洢һ���������� ����
    static UPF_IGlobal_Memory_Pool *   sm_object_ptr;
};

#endif /* UPF_GLOBAL_MEMORY_POOL_CREATOR_H */

