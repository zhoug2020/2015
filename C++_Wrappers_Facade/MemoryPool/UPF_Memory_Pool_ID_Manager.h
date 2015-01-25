/*----------------Copyright(C) 2008 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Memory_Pool_ID_Manager.h                                                               *
*       CREATE DATE     : 2008-7-21                                                                                  *
*       MODULE          : Memory Pool                                                                                *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]һ��ID������, ��������ΨһIDֵ![CN]                                                    *
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
 * @brief ��������Ψһ��ID, ��ǰֻ֧�����256��(0 - 255)
 */
class /*UPF_Export*/ UPF_Memory_Pool_ID_Manager
{
public:
    enum { 
        MAX_ID_VALUE = 255, /**< ���IDֵ. */
        INVALID_ID   = -1   /**< ��ЧID. */
    };
public:
    /** 
     * ���ڻ��һ��Ψһ��ID.
     * 
     * 
     * @return һ����Ч��ID(0-255)    ��ȡ�ɹ�.
     *         INVALID_ID             ��ȡʧ��.
     */
    static size_t get_id( void );


    /** 
     * �ͷ�һ��ID.
     * 
     * @param id    [in]  ͨ��get_id����ȡ�� id.
     */
    static void release_id( size_t id );

private:
    /// ��������ID��bit set.
    static std::bitset< MAX_ID_VALUE >   sm_id_generator;

    /// ��������ID��lock.
    static UPF_Thread_Mutex              sm_lock;

};

#endif /* UPF_MEMORY_POOL_ID_MANAGER_H */

