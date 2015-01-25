/*----------------Copyright(C) 2007 Shenyang Familiar software Co.,Ltd. ALL RIGHTS RESERVED-------------------------*/

/*********************************************************************************************************************
*       FILE NAME       : UPF_Thread_Sync.h                                                                          *
*       CREATE DATE     : 2007-11-23                                                                                 *
*       MODULE          : 线程同步                                                                                   *
*       AUTHOR          : Gao Yanbing                                                                                *
*--------------------------------------------------------------------------------------------------------------------*
*       MEMO            : [CN]线程同步, 暂时的![CN]                                                                  *
*********************************************************************************************************************/

#ifndef UPF_THREAD_SYNC_H
#define UPF_THREAD_SYNC_H

// wangy
#include "..\UPF_Thread_Mutex.h"


/** 
 * @class UPF_Scope_Guard 
 *  
 * @brief 定义了一个Guard类，用于线程同步的，当UPF框架OK时，使用UPF框架的. 
 * 
 */

#if 0
template < class Lock > class UPF_Scope_Guard
{
public:
    UPF_Scope_Guard( Lock & lock )
        : m_lock( lock )
    {
        m_lock.lock();
        m_own = true;
    }

    ~UPF_Scope_Guard( void )
    {
        release();
    }

    void release( void )
    {
        if ( m_own )
        {
            m_own = false;
            m_lock.unlock();
        }
    }

private:
    Lock & m_lock;
    bool   m_own;
};


/** 
 * @class UPF_Null_Lock 
 *  
 * @brief 定义了一个Null Lock，用于单线程的.
 * 
 */
class UPF_Null_Lock
{
public:
    void lock( void )    { }
    void unlock( void )  { }
};
#endif 
#endif /* UPF_THREAD_SYNC_H */

