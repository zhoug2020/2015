/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_ID_Manager.h                                                               *
*       CREATE DATE     : 2008-7-21                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]一个ID管理器, 用于生成唯一ID值![CN]                                                    *
*********************************************************************************************************************/
#ifndef UPF_MEMORY_POOL_ID_MANAGER_H
#define UPF_MEMORY_POOL_ID_MANAGER_H

#include "../../OS_Adapter/UPF_OS_Export.h" //wangy
#include "UPF_Thread_Sync.h"
#include <stdexcept>
#include <bitset>

/** 
 * @class UPF_Memory_Pool_ID_Manager
 * 
 * @brief 用于生成唯一的ID, 当前只支持最多256个(0 - 255)
 */
class /*UPF_Export*/ UPF_Memory_Pool_ID_Manager
{
public:
    enum { 
        MAX_ID_VALUE = 255, /**< 最大ID值. */
        INVALID_ID   = -1   /**< 无效ID. */
    };
public:
    /** 
     * 用于获得一个唯一的ID.
     * 
     * 
     * @return 一个有效的ID(0-255)    获取成功.
     *         INVALID_ID             获取失败.
     */
    static size_t get_id( void );


    /** 
     * 释放一个ID.
     * 
     * @param id    [in]  通过get_id来获取的 id.
     */
    static void release_id( size_t id );

private:
    /// 用于生成ID的bit set.
    static std::bitset< MAX_ID_VALUE >   sm_id_generator;

    /// 用于生成ID的lock.
    static UPF_Thread_Mutex              sm_lock;

};

#endif /* UPF_MEMORY_POOL_ID_MANAGER_H */

