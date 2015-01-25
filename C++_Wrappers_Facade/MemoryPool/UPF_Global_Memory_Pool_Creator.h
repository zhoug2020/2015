/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Global_Memory_Pool_Creator.h                                                           *
*       CREATE DATE     : 2008-7-23                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]用于提供Global Memory Pool的创建者![CN]                                                *
*********************************************************************************************************************/
#ifndef UPF_GLOBAL_MEMORY_POOL_CREATOR_H
#define UPF_GLOBAL_MEMORY_POOL_CREATOR_H

#include "../../OS_Adapter/UPF_OS.h"
#include "UPF_Thread_Sync.h"
#include "UPF_IGlobal_Memory_Pool.h"

/** 
 * @class UPF_Global_Memory_Pool_Creator
 * 
 * @brief 用于提供一个Global Memory Pool的创建者.
 * 
 */
class /*UPF_Export*/ UPF_Global_Memory_Pool_Creator
{
public:
    /** 
     * 创建一个全局对象, 引用计数加1.
     * 
     * @return BaseClass类型的对象
     */
    static UPF_IGlobal_Memory_Pool * get_instance( void );

    /**
     * destroy一个全局对象, 引用计数减1, 如果为0, 则销毁.
     */

    static void release_instance( void );

private:
    /// 用于创建对象和引用计数的lock.
    static UPF_Thread_Mutex sm_lock;

    /// 用于引用计数.
    static size_t                      sm_object_ref_count;

    /// 用于存储一个被创建的 对象
    static UPF_IGlobal_Memory_Pool *   sm_object_ptr;
};

#endif /* UPF_GLOBAL_MEMORY_POOL_CREATOR_H */

